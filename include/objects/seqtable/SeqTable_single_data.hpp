/* $Id: SeqTable_single_data.hpp 457991 2015-01-29 19:26:03Z vasilche $
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

/// @file SeqTable_single_data.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'seqtable.asn'.
///
/// New methods or data members can be added to it if needed.
/// See also: SeqTable_single_data_.hpp


#ifndef OBJECTS_SEQTABLE_SEQTABLE_SINGLE_DATA_HPP
#define OBJECTS_SEQTABLE_SEQTABLE_SINGLE_DATA_HPP


// generated includes
#include <objects/seqtable/SeqTable_single_data_.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

/////////////////////////////////////////////////////////////////////////////
class NCBI_SEQ_EXPORT CSeqTable_single_data : public CSeqTable_single_data_Base
{
    typedef CSeqTable_single_data_Base Tparent;
public:
    // constructor
    CSeqTable_single_data(void);
    // destructor
    ~CSeqTable_single_data(void);

    E_Choice GetValueType(void) const {
        return Which();
    }

    // return true if the value is integer of any size.
    // Bool, Int, Int8 are all integer types.
    bool CanGetInt(void) const {
        return IsInt() || IsInt8() || IsBit();
    }
    // return size (sizeof) of integer value to store data,
    // or zero if the data is not convertible to integer value.
    // Bool, Int, Int8 are all integer types.
    // returned value is the size of data in chars.
    size_t GetIntSize(void) const {
        return IsInt()? sizeof(TInt): IsInt8()? sizeof(TInt8): IsBit()? 1: 0;
    }
    // return true if the value is convertible to double value.
    bool CanGetReal(void) const {
        return IsReal();
    }

    void GetValue(bool& v) const;
    void GetValue(Int1& v) const;
    void GetValue(Int2& v) const;
    void GetValue(int& v) const;
    void GetValue(Int8& v) const;
    void GetValue(double& v) const;
    void GetValue(string& v) const;
    void GetValue(TBytes& v) const;

    void ThrowConversionError(const char* type_name) const;
    static void ThrowOverflowError(Int8 value, const char* type_name);

private:
    // Prohibit copy constructor and assignment operator
    CSeqTable_single_data(const CSeqTable_single_data& value);
    CSeqTable_single_data& operator=(const CSeqTable_single_data& value);

};

/////////////////// CSeqTable_single_data inline methods

// constructor
inline
CSeqTable_single_data::CSeqTable_single_data(void)
{
}


/////////////////// end of CSeqTable_single_data inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE


#endif // OBJECTS_SEQTABLE_SEQTABLE_SINGLE_DATA_HPP
