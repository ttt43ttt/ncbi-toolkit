/*  $Id: gvf_write_data.hpp 521299 2016-12-07 18:06:34Z foleyjp $
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
 * Author: Frank Ludwig
 *
 * File Description:
 *   GFF3 transient data structures
 *
 */

#ifndef OBJTOOLS_WRITERS___GVFDATA__HPP
#define OBJTOOLS_WRITERS___GVFDATA__HPP

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/util/feature.hpp>
#include <objtools/writers/gff3_write_data.hpp>

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE // namespace ncbi::objects::

//  ----------------------------------------------------------------------------
class CGvfWriteRecord
//  ----------------------------------------------------------------------------
    : public CGff3WriteRecordFeature
{
public:
    CGvfWriteRecord(
        CGffFeatureContext&
    );
    CGvfWriteRecord(
        const CGff3WriteRecordFeature&
    );
    virtual ~CGvfWriteRecord();

protected:
    virtual string StrAttributes() const;

    virtual bool x_AssignSource(
        const CMappedFeat& );
    virtual bool x_AssignType(
        const CMappedFeat&,
        unsigned int =0 );
    virtual bool x_AssignAttributes(
        const CMappedFeat&,
        unsigned int =0);
    virtual bool x_AssignAttributeID(
        const CMappedFeat& );
    virtual bool x_AssignAttributeParent(
        const CMappedFeat& );
    virtual bool x_AssignAttributeName(
        const CMappedFeat& );
    virtual bool x_AssignAttributeVarType(
        const CMappedFeat& );
    virtual bool x_AssignAttributeStartRange(
        const CMappedFeat& );
    virtual bool x_AssignAttributeEndRange(
        const CMappedFeat& );
    virtual bool x_AssignAttributesCustom(
        const CMappedFeat& );

    static int s_unique;
    string s_UniqueId();

    void x_AppendAttribute(
        TAttrCit,
        string& ) const;
};

END_objects_SCOPE
END_NCBI_SCOPE

#endif // OBJTOOLS_WRITERS___GVFDATA__HPP
