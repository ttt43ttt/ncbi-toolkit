/* $Id: cleanup_utils.cpp 574795 2018-11-19 16:47:08Z ivanov $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Author:  Mati Shomrat
 *
 * File Description:
 *   General utilities for data cleanup.
 *
 * ===========================================================================
 */
#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include "cleanup_utils.hpp"

#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>

#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seq/seq_loc_from_string.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/misc/sequence_macros.hpp>

#include <objmgr/seqdesc_ci.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

#define IS_LOWER(c)     ('a'<=(c) && (c)<='z')
#define IS_UPPER(c)     ('A'<=(c) && (c)<='Z')

using namespace sequence;

//  ----------------------------------------------------------------------------
//  File scope helper functions:
//  ----------------------------------------------------------------------------
bool s_IsAllUpperCase( const string& str )
{
    for(string::size_type i=0; i<str.length(); i++) {
        if( !IS_UPPER(str[i])) return false;
    }
    return true;
}

string s_NormalizeInitials( const string& raw_initials )
{
    //
    //  Note:
    //  Periods _only_ after CAPs to avoid decorating hyphens (which _are_ 
    //  legal in the "initials" part.
    //
    string normal_initials;
    for ( const char* p=raw_initials.c_str(); *p != 0; ++p ) {
        normal_initials += *p;
        if ( IS_UPPER(*p) ) {
            normal_initials += '.';
        }
    }
    return normal_initials;
}

string s_NormalizeSuffix( const string& raw_suffix )
{
    //
    //  Note: (2008-02-13) Suffixes I..VI no longer have trailing periods.
    //
    if ( raw_suffix == "1d" ) {
        return "1st";
    }
    if ( raw_suffix == "2d" ) {
        return "2nd";
    }
    if ( raw_suffix == "3d" ) {
        return "3rd";
    }
    if ( raw_suffix == "Sr" ) {
        return "Sr.";
    }
    if ( raw_suffix == "Jr" ) {
        return "Jr.";
    }
    
    return raw_suffix;
}

void s_SplitMLAuthorName(string name, string& last, string& initials, string& suffix)
{
    NStr::TruncateSpacesInPlace( name );
    if ( name.empty() ) {
        return;
    }

    vector<string> parts;
    NStr::Split( name, " ", parts, NStr::fSplit_Tokenize );
    if ( parts.empty() ) {
        return;
    }
    if ( parts.size() == 1 ) {
        //
        //  Designate the only part we have as the last name.
        //
        last = parts[0];
        return;
    }

    
    const string& last_part = parts[ parts.size()-1 ];
    const string& second_to_last_part = parts[ parts.size()-2 ];

    if ( parts.size() == 2 ) {
        //
        //  Designate the first part as the last name and the second part as the
        //  initials.
        //
        last = parts[0];
        initials = s_NormalizeInitials( last_part );
        return;
    }

    //
    //  At least three parts.
    //
    //  If the second to last part is all CAPs then those are the initials. The 
    //  last part is the suffix, and everything up to the initials is the last 
    //  name.
    //
    if ( s_IsAllUpperCase( second_to_last_part ) ) {
        last = NStr::Join( vector<string>( parts.begin(), parts.end()-2 ), " " );
        initials = s_NormalizeInitials( second_to_last_part );
        suffix = s_NormalizeSuffix( last_part );
        return;
    }

    //
    //  Fall through:
    //  Guess that the last part is the initials and everything leading up to it 
    //  is a (rather unusual) last name.
    //
    last = NStr::Join( vector<string>( parts.begin(), parts.end()-1 ), " " );
    initials = s_NormalizeInitials( last_part );
    return;

    //  ------------------------------------------------------------------------
    //  CASE NOT HANDLED:
    //
    //  (1) Initials with a blank in them. UNFIXABLE!
    //  (2) Initials with non CAPs in them. Probably fixable through a 
    //      white list of allowable exceptions. Tedious, better let the indexers
    //      fix it.
    //  ------------------------------------------------------------------------
}

bool CleanVisString( string &str )
{
    bool changed = false;

    if( str.empty() ) {
        return false;
    }

    // chop off initial junk
    {
        string::size_type first_good_char_pos = str.find_first_not_of(" ;,");
        if( first_good_char_pos == string::npos ) {
            // string is completely junk
            str.clear();
            return true;
        } else if( first_good_char_pos > 0 ) {
            copy( str.begin() + first_good_char_pos, str.end(), str.begin() );
            str.resize( str.length() - first_good_char_pos );
            changed = true;
        }
    }

    // chop off end junk

    string::size_type last_good_char_pos = str.find_last_not_of(" ;,");
    _ASSERT( last_good_char_pos != string::npos ); // we checked this case so it shouldn't happen
    if( last_good_char_pos == (str.length() - 1) ) {
        // nothing to chop of the end
        return changed;
    } else if( str[last_good_char_pos+1] == ';' ) {
        // special extra logic for semicolons because it might be part of
        // an HTML character like "&nbsp;"

        // see if there's a '&' before the semicolon
        // ( ' ' and ',' would break the '&' and make it irrelevant, though )
        string::size_type last_ampersand_pos = str.find_last_of("& ,", last_good_char_pos );
        if( last_ampersand_pos == string::npos ) {
            // no ampersand, so just chop off as normal
            str.resize( last_good_char_pos + 1 );
            return true;
        }
        switch( str[last_ampersand_pos] ) {
            case '&':
                // can't chop semicolon, so chop just after it
                if( (last_good_char_pos + 2) == str.length() ) {
                    // semicolon is at end, so no chopping occurs
                    return changed;
                } else {
                    // chop after semicolon
                    str.resize( last_good_char_pos + 2 );
                    return true;
                }
            case ' ':
            case ',':
                // ampersand (if any) is irrelevant due to intervening
                // space or comma
                str.resize( last_good_char_pos + 1 );
                return true;
            default:
                _ASSERT(false);
                return changed;  // should be impossible to reach here
        }

    } else {
        str.resize( last_good_char_pos + 1 );
        return true;
    }
}

bool CleanVisStringJunk( string &str, bool allow_ellipses )
{
    // This is based on the C function TrimSpacesAndJunkFromEnds.
    // Although it's updated to use iterators and such and to
    // return whether it changed the string, it should
    // have the same output.

    // TODO: This function is copy-pasted from TrimSpacesAndJunkFromEnds,
    // so we should do something about that since duplicate code is evil.

    if ( str.empty() ) {
        return false;
    }

    // make start_of_junk_pos hold the beginning of the "junk" at the end
    // (where junk is defined as one of several characters)
    // while we're at it, also check if the junk contains a tilde and/or period
    bool isPeriod = false;
    bool isTilde = false;
    int start_of_junk_pos = str.length() - 1;
    for( ; start_of_junk_pos >= 0 ; --start_of_junk_pos ) {
        const char ch = str[start_of_junk_pos];
        if (ch <= ' ' || ch == '.' || ch == ',' || ch == '~' || ch == ';') {
            // found junk character

            // also, keep track of whether the junk includes a period and/or tilde
            isPeriod = (isPeriod || ch == '.');
            isTilde = (isTilde || ch == '~');
        } else {
            // found non-junk character.  Last junk character is just after this
            ++start_of_junk_pos;
            break;
        }
    }
    // special case of the whole string being junk
    if( start_of_junk_pos < 0 ) {
        start_of_junk_pos = 0;
    }

    bool changed = false;

    // if there's junk, chop it off (but leave period/tildes/ellipsis as appropriate)
    if ( start_of_junk_pos < (int)str.length() ) {

        // holds the suffix to add after we remove the junk
        const char * suffix = ""; // by default, just remove junk

        const int chars_in_junk = ( str.length() - start_of_junk_pos );
        _ASSERT( chars_in_junk >= 1 );
        // allow one period at end
        if (isPeriod) {
            suffix = ".";
            if ( allow_ellipses && (chars_in_junk >= 3) && 
                str[start_of_junk_pos+1] == '.' && str[start_of_junk_pos+2] == '.' ) {
                suffix = "...";
            }
        } else if (isTilde ) {
            // allow double tilde(s) at the end
            if ( str[start_of_junk_pos] == '~' ) {
                const bool doubleTilde = ( (chars_in_junk >= 2) && str[start_of_junk_pos+1] == '~' );
                suffix = ( doubleTilde  ? "~~" : "" );
            }
        }
        if( suffix[0] != '\0' ) {
            if( 0 != str.compare( start_of_junk_pos, INT_MAX, suffix) ) {
                str.erase( start_of_junk_pos );
                str += suffix;
                changed = true;
            }
        } else if ( start_of_junk_pos < (int)str.length() ) {
            str.erase( start_of_junk_pos );
            changed = true;
        }
    }

    // copy the part after the initial whitespace to the destination
    string::iterator input_iter = str.begin();
    while ( input_iter != str.end() && *input_iter <= ' ') {
        ++input_iter;
    }
    if( input_iter != str.begin() ) {
        str.erase( str.begin(), input_iter );
        changed = true;
    }

    return changed;
}


bool  RemoveSpacesBetweenTildes(string& str)
{
    static string whites(" \t\n\r");
    bool changed = false;
    SIZE_TYPE tilde1 = str.find('~');
    if (tilde1 == NPOS) {
        return changed; // no tildes in str.
    }
    SIZE_TYPE tilde2 = str.find_first_not_of(whites, tilde1 + 1);
    while (tilde2 != NPOS) {
        if (str[tilde2] == '~') {
            if ( tilde2 > tilde1 + 1) {
                // found two tildes with only spaces between them.
                str.erase(tilde1+1, tilde2 - tilde1 - 1);
                ++tilde1;
                changed = true;
            } else {
                // found two tildes side by side.
                tilde1 = tilde2;
            }
        } else {
            // found a tilde with non-space non-tilde after it.
            tilde1 = str.find('~', tilde2 + 1);
            if (tilde1 == NPOS) {
                return changed; // no more tildes in str.
            }
        }
        tilde2 = str.find_first_not_of(whites, tilde1 + 1);
    }
    return changed;

}


bool CleanDoubleQuote(string& str)
{
    bool changed = false;
    NON_CONST_ITERATE(string, it, str) {
        if (*it == '\"') {
            *it = '\'';
            changed = true;
        }
    }
    return changed;
}


void TrimInternalSemicolons (string& str)
{
    size_t pos, next_pos;
  
    pos = NStr::Find (str, ";");
    while (pos != string::npos) {
        next_pos = pos + 1;
        bool has_space = false;
        while (next_pos < str.length() && (str[next_pos] == ';' || str[next_pos] == ' ' || str[next_pos] == '\t')) {
            if (str[next_pos] == ' ') {
                has_space = true;
            }
            next_pos++;
        }
        if (next_pos == pos + 1 || (has_space && next_pos == pos + 2)) {
            // nothing to fix, advance semicolon search
            pos = NStr::Find (str, ";", next_pos);
        } else if (next_pos == str.length()) {
            // nothing but semicolons, spaces, and tabs from here to the end of the string
            // just truncate it
            str = str.substr(0, pos);
            pos = string::npos;
        } else {
            if (has_space) {
                str = str.substr(0, pos + 1) + " " + str.substr(next_pos);
            } else {
                str = str.substr(0, pos + 1) + str.substr(next_pos);
            }
            pos = NStr::Find (str, ";", pos + 1);
        }
    }
}

#define twocommas ((',') << 8 | (','))
#define twospaces ((' ') << 8 | (' '))
#define twosemicolons ((';') << 8 | (';'))
#define space_comma ((' ') << 8 | (','))
#define space_bracket ((' ') << 8 | (')'))
#define bracket_space (('(') << 8 | (' '))
#define space_semicolon ((' ') << 8 | (';'))
#define comma_space ((',') << 8 | (' '))
#define semicolon_space ((';') << 8 | (' '))

bool Asn2gnbkCompressSpaces(string& val)
{
    if (val.length() == 0) return false;

    char * str = new char[sizeof(char) * (val.length() + 1)];
    strcpy(str, val.c_str());

    char     ch;
    char *   dst;
    char *   ptr;

    char     curr;
    char     next;
    char *   in;
    char *   out;
    unsigned short   two_chars;


    in = str;
    out = str;

    curr = *in;
    in++;

    next = 0;
    two_chars = curr;

    while (curr != '\0') {
        next = *in;
        in++;

        two_chars = (two_chars << 8) | next;

        if (two_chars == twocommas) {
            *out++ = curr;
            next = ' ';
            two_chars = next;
        }
        else if (two_chars == twospaces) {
        }
        else if (two_chars == twosemicolons) {
        }
        else if (two_chars == bracket_space) {
            next = curr;
            two_chars = curr;
        }
        else if (two_chars == space_bracket) {
        }
        else if (two_chars == space_comma) {
            *out++ = next;
            next = curr;
            *out++ = ' ';
            while (next == ' ' || next == ',') {
                next = *in;
                in++;
            }
            two_chars = next;
        }
        else if (two_chars == space_semicolon) {
            *out++ = next;
            next = curr;
            *out++ = ' ';
            while (next == ' ' || next == ';') {
                next = *in;
                in++;
            }
            two_chars = next;
        }
        else if (two_chars == comma_space) {
            *out++ = curr;
            *out++ = ' ';
            while (next == ' ' || next == ',') {
                next = *in;
                in++;
            }
            two_chars = next;
        }
        else if (two_chars == semicolon_space) {
            *out++ = curr;
            *out++ = ' ';
            while (next == ' ' || next == ';') {
                next = *in;
                in++;
            }
            two_chars = next;
        }
        else {
            *out++ = curr;
        }

        curr = next;
    }

    *out = '\0';

    /* TrimSpacesAroundString but allow leading/trailing tabs/newlines */

    if (str[0] != '\0') {
        dst = str;
        ptr = str;
        ch = *ptr;
        while (ch == ' ') {
            ptr++;
            ch = *ptr;
        }
        while (ch != '\0') {
            *dst = ch;
            dst++;
            ptr++;
            ch = *ptr;
        }
        *dst = '\0';
        dst = NULL;
        ptr = str;
        ch = *ptr;
        while (ch != '\0') {
            if (ch != ' ') {
                dst = NULL;
            }
            else if (dst == NULL) {
                dst = ptr;
            }
            ptr++;
            ch = *ptr;
        }
        if (dst != NULL) {
            *dst = '\0';
        }
    }
    string new_val;
    new_val = str;
    delete[] str;

    if (!NStr::Equal(val, new_val)) {
#ifdef _DEBUG
#if 0
        printf("Use new string\n");
#endif
#endif
        val = new_val;
        return true;
    }
    else {
        return false;
    }
}

bool TrimSpacesSemicolonsAndCommas(string& val)
{
    if (val.length() == 0) return false;

    char * str = new char[sizeof(char) * (val.length() + 1)];
    strcpy(str, val.c_str());

    char *  amp;
    unsigned char    ch;    /* to use 8bit characters in multibyte languages */
    char *  dst;
    char *  ptr;

    dst = str;
    ptr = str;
    ch = *ptr;
    if (ch != '\0' && (ch <= ' ' || ch == ';' || ch == ',')) {
        while (ch != '\0' && (ch <= ' ' || ch == ';' || ch == ',')) {
            ptr++;
            ch = *ptr;
        }
        while (ch != '\0') {
            *dst = ch;
            dst++;
            ptr++;
            ch = *ptr;
        }
        *dst = '\0';
    }
    amp = NULL;
    dst = NULL;
    ptr = str;
    ch = *ptr;
    while (ch != '\0') {
        if (ch == '&') {
            amp = ptr;
            dst = NULL;
        }
        else if (ch <= ' ') {
            if (dst == NULL) {
                dst = ptr;
            }
            amp = NULL;
        }
        else if (ch == ';') {
            if (dst == NULL && amp == NULL) {
                dst = ptr;
            }
        }
        else if (ch == ',') {
            if (dst == NULL) {
                dst = ptr;
            }
            amp = NULL;
        }
        else {
            dst = NULL;
        }
        ptr++;
        ch = *ptr;
    }
    if (dst != NULL) {
        *dst = '\0';
    }

    string new_val;
    new_val = str;
    delete[] str;

    if (!NStr::Equal(val, new_val)) {
#ifdef _DEBUG
#if 0
        printf("Use new string\n");
#endif
#endif
        val = new_val;
        return true;
    }
    else {
        return false;
    }
}


bool RemoveSpaces(string& str)
{
    if (str.empty()) {
        return false;
    }

    size_t next = 0;

    NON_CONST_ITERATE(string, it, str) {
        if (!isspace((unsigned char)(*it))) {
            str[next++] = *it;
        }
    }
    if (next < str.length()) {
        str.resize(next);
        return true;
    }
    return false;
}

class CGetSeqLocFromStringHelper_ReadLocFromText : public CGetSeqLocFromStringHelper {
public:
    CGetSeqLocFromStringHelper_ReadLocFromText( CScope *scope )
        : m_scope(scope) { }

    virtual CRef<CSeq_loc> Seq_loc_Add(
        const CSeq_loc&    loc1,
        const CSeq_loc&    loc2,
        CSeq_loc::TOpFlags flags )
    {
        return sequence::Seq_loc_Add( loc1, loc2, flags, m_scope );
    }

private:
    CScope *m_scope;
};

CRef<CSeq_loc> ReadLocFromText(const string& text, const CSeq_id *id, CScope *scope)
{
    CGetSeqLocFromStringHelper_ReadLocFromText helper(scope);
    return GetSeqLocFromString(text, id, &helper);
}

typedef struct proteinabbrev {
     string abbreviation;
    char letter;
} ProteinAbbrevData;

static ProteinAbbrevData abbreviation_list[] = 
{ 
    {"Ala", 'A'},
    {"Asx", 'B'},
    {"Cys", 'C'},
    {"Asp", 'D'},
    {"Glu", 'E'},
    {"Phe", 'F'},
    {"Gly", 'G'},
    {"His", 'H'},
    {"Ile", 'I'},
    {"Xle", 'J'},  /* was - notice no 'J', breaks naive meaning of index -Karl */
    {"Lys", 'K'},
    {"Leu", 'L'},
    {"Met", 'M'},
    {"Asn", 'N'},
    {"Pyl", 'O'},  /* was - no 'O' */
    {"Pro", 'P'},
    {"Gln", 'Q'},
    {"Arg", 'R'},
    {"Ser", 'S'},
    {"Thr", 'T'},
    {"Val", 'V'},
    {"Trp", 'W'}, 
    {"Sec", 'U'}, /* was - not in iupacaa */
    {"Xxx", 'X'},
    {"Tyr", 'Y'},
    {"Glx", 'Z'},
    {"TERM", '*'}, /* not in iupacaa */ /*changed by Tatiana 06.07.95?`*/
    {"OTHER", 'X'}
};

// Find the single-letter abbreviation for either the single letter abbreviation
// or three-letter abbreviation.  
// Use X if the abbreviation is not found.

char ValidAminoAcid (const string& abbrev)
{
    char ch = 'X';
    
    for (unsigned int k = 0; k < sizeof(abbreviation_list) / sizeof (ProteinAbbrevData); k++) {
        if (NStr::EqualNocase (abbrev, abbreviation_list[k].abbreviation)) {
            ch = abbreviation_list[k].letter;
            break;
        }
    }
    
    if (abbrev.length() == 1) {
        for (unsigned int k = 0; k < sizeof(abbreviation_list) / sizeof (ProteinAbbrevData); k++) {
            if (abbrev.c_str()[0] == abbreviation_list[k].letter) {
                ch = abbreviation_list[k].letter;
                break;
            }
        }
    }
    
    return ch;
}


bool s_DbtagCompare (const CRef<CDbtag>& dbt1, const CRef<CDbtag>& dbt2)
{
    // is dbt1 < dbt2
    return dbt1->Compare(*dbt2) < 0;
}


bool s_DbtagEqual (const CRef<CDbtag>& dbt1, const CRef<CDbtag>& dbt2)
{
    // is dbt1 == dbt2
    return dbt1->Compare(*dbt2) == 0;
}

bool s_OrgrefSynCompare( const string & syn1, const string & syn2 )
{
    return NStr::CompareNocase(syn1, syn2) < 0;
}

bool s_OrgrefSynEqual( const string & syn1, const string & syn2 )
{
    return NStr::EqualNocase(syn1, syn2);
}


CRef<CAuthor> ConvertMltoSTD( const string& token)
{
    string last, initials, suffix;
    s_SplitMLAuthorName(token, last, initials, suffix);

    if ( ! last.empty() ) {
        CRef < CAuthor > au(new CAuthor); 
        au->SetName().SetName().SetLast(last);
        if(initials.size()) {
            au->SetName().SetName().SetInitials(initials);
        }
        if(suffix.size()) {
            au->SetName().SetName().SetSuffix(suffix);
        }
        return au;
    }
    return CRef<CAuthor>( 0 );
}


bool ConvertAuthorContainerMlToStd( CAuth_list& authors )
{
    CAuth_list::C_Names* names = new CAuth_list::C_Names;
    CAuth_list::C_Names::TStd& names_std = names->SetStd();
    for (auto& author : authors.GetNames().GetMl()) {
        names_std.push_back( ConvertMltoSTD( author ) );
    }
    authors.SetNames( *names );
    return true;
}


END_SCOPE(objects)
END_NCBI_SCOPE
