/* $Id: Location_constraint.hpp 539791 2017-06-27 16:51:54Z bollin $
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

/// @file Location_constraint.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'macro.asn'.
///
/// New methods or data members can be added to it if needed.
/// See also: Location_constraint_.hpp


#ifndef OBJECTS_MACRO_LOCATION_CONSTRAINT_HPP
#define OBJECTS_MACRO_LOCATION_CONSTRAINT_HPP


// generated includes
#include <objects/macro/Location_constraint_.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

/////////////////////////////////////////////////////////////////////////////
class CLocation_constraint : public CLocation_constraint_Base
{
    typedef CLocation_constraint_Base Tparent;
public:
    // constructor
    CLocation_constraint(void);
    // destructor
    ~CLocation_constraint(void);

    bool Match(const CSeq_feat& feat, CConstRef <CSeq_feat> feat_to, CConstRef <CBioseq> feat_bioseq) const;
    bool IsEmpty() const;

private:
    // Prohibit copy constructor and assignment operator
    CLocation_constraint(const CLocation_constraint& value);
    CLocation_constraint& operator=(const CLocation_constraint& value);

    bool x_DoesStrandMatchConstraint(const CSeq_loc& loc) const;
    bool x_DoesBioseqMatchSequenceType(CConstRef <CBioseq> bioseq,
                                   const ESeqtype_constraint& seq_type) const;
    bool x_DoesLocationMatchPartialnessConstraint(const CSeq_loc& loc) const;
    bool x_DoesLocationMatchTypeConstraint(const CSeq_loc& seq_loc) const;
    bool x_DoesLocationMatchDistanceConstraint(CConstRef <CBioseq> bioseq_hl, 
                                                 const CSeq_loc& loc)  const;
};

/////////////////// CLocation_constraint inline methods

// constructor
inline
CLocation_constraint::CLocation_constraint(void)
{
}


/////////////////// end of CLocation_constraint inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE


#endif // OBJECTS_MACRO_LOCATION_CONSTRAINT_HPP
/* Original file checksum: lines: 86, chars: 2576, CRC32: 53f5cf21 */
