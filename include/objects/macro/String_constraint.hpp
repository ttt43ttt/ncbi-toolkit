/* $Id: String_constraint.hpp 572090 2018-10-09 13:36:47Z ivanov $
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
 */

/// @file String_constraint.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'macro.asn'.
///
/// New methods or data members can be added to it if needed.
/// See also: String_constraint_.hpp


#ifndef OBJECTS_MACRO_STRING_CONSTRAINT_HPP
#define OBJECTS_MACRO_STRING_CONSTRAINT_HPP

#include <objects/macro/String_constraint_.hpp>
#include <serial/iterator.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE // namespace ncbi::objects::

class CAutoLowerCase
{
public:
    CAutoLowerCase() {};

    CAutoLowerCase(const string& v) :
        m_original(v)
    {
    }
    CAutoLowerCase(CAutoLowerCase&& v): 
        m_original(move(v.m_original)),
        m_lowercase(move(v.m_lowercase)),
        m_uppercase(move(v.m_uppercase))
    {
    }
    CAutoLowerCase& operator=(CAutoLowerCase&& v)
    {
        m_original = move(v.m_original);
        m_lowercase = move(v.m_lowercase);
        m_uppercase = move(v.m_uppercase);
        return *this;
    }
    CAutoLowerCase& operator=(string&& v)
    {
        m_original = move(v);
        m_lowercase.clear();
        m_uppercase.clear();
        return *this;
    }
    CAutoLowerCase& operator=(const string& v)
    {
        m_original = v;
        m_lowercase.clear();
        m_uppercase.clear();
        return *this;
    }

    const string& lowercase() const
    {
        if (m_lowercase.empty() && !m_original.empty())
        {
            m_lowercase = m_original;
            NStr::ToLower(m_lowercase);
        }
        return m_lowercase;
    }
    const string& uppercase() const
    {
        if (m_uppercase.empty() && !m_original.empty())
        {
            m_uppercase = m_original;
            NStr::ToUpper(m_uppercase);
        }
        return m_uppercase;
    }
    operator const string&() const
    {
        return m_original;
    }
    const string& original() const
    {
        return m_original;
    }

private:
    CAutoLowerCase(const CAutoLowerCase&); 
    CAutoLowerCase& operator=(const CAutoLowerCase&);

    string m_original;
    mutable string m_lowercase;
    mutable string m_uppercase;
};

class CMatchString
{
public:
    CMatchString() {};

    CMatchString(const string& v) : m_original(v), m_noweasel_start(CTempString::npos), m_weaselmask(0)
    {
    }
    CMatchString(const char* v) : m_original(v), m_noweasel_start(CTempString::npos), m_weaselmask(0)
    {
    }
    const CAutoLowerCase& original() const
    {
        return m_original;
    }
    CMatchString& operator=(const string&s)
    {
        m_original = s;
        m_noweasel_start = CTempString::npos;
        m_weaselmask = 0;
        return *this;
    };

    operator const string&() const
    {
        return m_original;
    }

    void PopWeasel() const { if (m_noweasel_start == CTempString::npos) x_PopWeasel(); }

    CTempString GetNoweasel() const
    {
        PopWeasel();
        return CTempString(m_original.original(), m_noweasel_start, m_original.original().length() - m_noweasel_start);
    }
    CTempString GetNoweaselLC() const
    {
        PopWeasel();
        return CTempString(m_original.lowercase(), m_noweasel_start, m_original.lowercase().length() - m_noweasel_start);
    }
    CTempString GetNoweaselUC() const
    {
        PopWeasel();
        return CTempString(m_original.uppercase(), m_noweasel_start, m_original.uppercase().length() - m_noweasel_start);
    }

    unsigned GetWeaselMask() const { PopWeasel(); return m_weaselmask; }

private:
    void x_PopWeasel() const;
    CAutoLowerCase m_original;
    mutable CTempString::size_type m_noweasel_start;
    mutable unsigned m_weaselmask;
};

/////////////////////////////////////////////////////////////////////////////
class CString_constraint : public CString_constraint_Base
{
    typedef CString_constraint_Base Tparent;
public:
    CString_constraint();
    virtual ~CString_constraint();

    // get all string type data from object
    template <class T>
    void GetStringsFromObject(const T& obj, vector <string>& strs) const
    {
       CTypesConstIterator it(CStdTypeInfo<string>::GetTypeInfo(),
                          CStdTypeInfo<utf8_string_type>::GetTypeInfo());
       for (it = ConstBegin(obj);  it;  ++it) {
          strs.push_back(*static_cast<const string*>(it.GetFoundPtr()));
       }
    }

    bool Match(const CMatchString& str) const;
    bool Empty() const;
    bool ReplaceStringConstraintPortionInString(string& result, const CMatchString& str, const string& replace) const;
    void SetMatch_text(const TMatch_text& value)
    {
        m_match = kEmptyStr;
        CString_constraint_Base::SetMatch_text(value);
    }
    TMatch_text& SetMatch_text(void)
    {
        m_match = kEmptyStr;
        return CString_constraint_Base::SetMatch_text();
    }

    static const vector<string> s_WeaselWords;

private:
    // Prohibit copy constructor and assignment operator
    CString_constraint(const CString_constraint& value) = delete;
    CString_constraint& operator=(const CString_constraint& value) = delete;

    typedef enum 
    {
        e_original,
        e_lowercase,
        e_uppercase,
        e_automatic
    } ECase;

    CTempString x_GetConstraintString(ECase e_case = e_automatic) const;
    CTempString x_GetCompareString(const CMatchString& s, ECase e_case = e_automatic) const;

    bool x_DoesSingleStringMatchConstraint(const CMatchString& str) const;
    bool x_IsAllCaps(const CMatchString& str) const;
    bool x_IsAllLowerCase(const CMatchString& str) const;
    bool x_IsAllPunctuation(const CMatchString& str) const;
    bool x_IsSkippable(const char ch) const;
    bool x_IsAllSkippable(const CTempString& str) const;
    // Checks whether the first letter of the first word is capitalized
    bool x_IsFirstCap(const CMatchString& str) const;
    // Checks whether the first letter of each word is capitalized
    bool x_IsFirstEachCap(const CMatchString& str) const;
        
    bool x_PartialCompare(const string& str, const string& pattern, char prev_char, size_t & match_len) const;
    bool x_AdvancedStringCompare(const string& str, 
                                const string& str_match, 
                                const char prev_char, 
                                size_t * ini_target_match_len = 0) const;
    bool x_AdvancedStringMatch(const string& str,const string& tmp_match) const;
    bool x_IsWholeWordMatch(const CTempString& start,
                              size_t found,
                              size_t match_len,
                              bool disallow_slash = false) const;
    bool x_MatchFound(CTempString& search, CTempString& pattern) const;

    bool x_ReplaceContains(string& val, const string& replace) const;

    mutable CMatchString m_match;
};


END_objects_SCOPE // namespace ncbi::objects::
END_NCBI_SCOPE

#endif // OBJECTS_MACRO_STRING_CONSTRAINT_HPP