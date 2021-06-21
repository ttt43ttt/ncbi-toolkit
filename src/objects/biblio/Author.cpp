/* $Id: Author.cpp 544496 2017-08-23 17:52:58Z foleyjp $
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
 * Author:  .......
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using the following specifications:
 *   'biblio.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>
#include <objects/general/Person_id.hpp>

// generated includes
#include <objects/biblio/Author.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CAuthor::~CAuthor(void)
{
}


bool CAuthor::GetLabelV1(string* label, TLabelFlags) const
{
    // XXX - honor flags?
    GetName().GetLabel(label);
    return true;
}


bool CAuthor::GetLabelV2(string* label, TLabelFlags flags) const
{
    const CPerson_id& pid = GetName();
    switch (pid.Which()) {
    case CPerson_id::e_Name:
    {
        const CName_std& name = pid.GetName();
        if (HasText(name.GetLast())) {
            return x_GetLabelV2
                (label, flags, name.GetLast(),
                 name.CanGetInitials() ? name.GetInitials() : kEmptyStr,
                 name.CanGetSuffix() ? name.GetSuffix() : kEmptyStr);
        } else if (name.IsSetFull()  &&  HasText(name.GetFull())) {
            return x_GetLabelV2(label, flags, name.GetFull());
        } else {
            return false;
        }
    }
    case CPerson_id::e_Ml:
        return x_GetLabelV2(label, flags, pid.GetMl());
    case CPerson_id::e_Str:
        return x_GetLabelV2(label, flags, pid.GetStr()); 
    case CPerson_id::e_Consortium:
        return x_GetLabelV2(label, flags, pid.GetConsortium());
    default:
        return false;
    }
}


bool CAuthor::x_GetLabelV2(string* label, TLabelFlags flags, CTempString name,
                           CTempString initials, CTempString suffix)
{
    if (name.empty()) {
        return false;
    }

    if (name.size() <= 6  &&  (SWNC(name, "et al")  ||  SWNC(name, "et,al"))) {
        name = "et al.";
        if (NStr::EndsWith(*label, " and ")) {
            label->replace(label->size() - 5, NPOS, ", ");
        }
    }

    SIZE_TYPE pos = label->size();
    *label += name;
    if (HasText(initials)) {
        *label += ',';
        *label += initials;
    }
    if (HasText(suffix)) {
        *label += ' ';
        *label += suffix;
    }

    if ((flags & fLabel_FlatEMBL) != 0) {
        NStr::ReplaceInPlace(*label, ",", " ", pos);
    }

    return true;
}

bool CAuthor::x_IsAllCaps(const string& str)
{
    for(const char c  : str) {
        if (!isalpha(c) || !isupper(c)) {
            return false;
        }
    }
    return true;
}

string CAuthor::x_GetInitials(vector<string>& tokens) 
{
    string init = "";
    while (tokens.size() > 1) {
        string val = tokens.back();
        if (!s_IsAllCaps(val)) {
            break;
        }
        init += val;
        tokens.pop_back();
    }
    return init;
}


void CAuthor::x_NormalizeSuffix(string& suffix) 
{
    static const map<string, string> smap = {
                                {"1d",  "I"  },
                                {"1st", "I"  },
                                {"2d",  "II" },
                                {"2nd", "II" },
                                {"3d",  "III"},
                                {"3rd", "III"},
                                {"4th", "IV" },
                                {"5th", "V"  },
                                {"6th", "VI" },
                                {"Jr",  "Jr."},
                                {"Sr",  "Sr."}};

    auto search = smap.find(suffix);
    if (search != smap.end()) {
        suffix = search->second;  
    } 
}


bool CAuthor::x_IsPossibleSuffix(const string& str) {    
    if (!x_IsAllCaps(str)) {
        return true;
    }
    static set<string> suffixes = {"II", "III", "IV", "VI"}; 
    auto search = suffixes.find(str);

    return (search != suffixes.end());
}


CRef<CPerson_id> CAuthor::x_ConvertMlToStandard(const string& name, const bool normalize_suffix) 
{
    CRef<CPerson_id> person_id(new CPerson_id());

    if (!NStr::IsBlank(name)) {
        vector<string> tokens;
        NStr::Split(name, " ", tokens, NStr::fSplit_Tokenize);
        // Check for suffix
        const size_t num_tokens = tokens.size();
        string suffix = "";
        if (num_tokens >= 3 && 
            !x_IsPossibleSuffix(tokens.back()) &&
            x_IsAllCaps(tokens[num_tokens-2])) {
            suffix = tokens.back();
            tokens.pop_back();
        }

        const string init = x_GetInitials(tokens);
        const string last = NStr::Join(tokens, " ");
        person_id->SetName().SetLast(last);


        if (!NStr::IsBlank(suffix)) {
            if (normalize_suffix) {
                x_NormalizeSuffix(suffix);
            }
            person_id->SetName().SetSuffix(suffix);
        }

        if (!NStr::IsBlank(init)) {
            person_id->SetName().SetFirst(init.substr(0,1));
            string initials = "";
            for (const char& c : init) {
                initials.push_back(c);
                initials.push_back('.');
            }
            person_id->SetName().SetInitials(initials);
        }
    }
    return person_id;
}


CRef<CAuthor> CAuthor::ConvertMlToStandard(const string& ml_name, const bool normalize_suffix)
{
    CRef<CAuthor> new_author(new CAuthor());
    if (!NStr::IsBlank(ml_name)) {
        CRef<CPerson_id> std_name = x_ConvertMlToStandard(ml_name, normalize_suffix);
        new_author->SetName(*std_name);
    }
    return new_author;
}


CRef<CAuthor> CAuthor::ConvertMlToStandard(const CAuthor& author, const bool normalize_suffix) 
{
    CRef<CAuthor> new_author(new CAuthor());
    new_author->Assign(author);

    if (new_author->IsSetName() &&
        new_author->GetName().IsMl()) {
        const string ml_name = new_author->GetName().GetMl();
        CRef<CPerson_id> std_name = x_ConvertMlToStandard(ml_name, normalize_suffix);
        new_author->ResetName();
        new_author->SetName(*std_name);
    }
    return new_author;
}



END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 65, chars: 1880, CRC32: f9047a4f */