/*
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
* Author:  Alexey Dobronadezhdin
*
* File Description:
*
* ===========================================================================
*/

#ifndef WGS_TAX_H
#define WGS_TAX_H

#include <list>

#include <corelib/ncbiobj.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(objects);

// forward declarations
BEGIN_NCBI_NAMESPACE;
BEGIN_NAMESPACE(objects);

class CSeq_entry;
class COrg_ref;

END_NAMESPACE(objects);
END_NCBI_NAMESPACE;

namespace wgsparse
{

struct COrgRefInfo;

void CollectOrgRefs(const CSeq_entry& entry, list<COrgRefInfo>& org_refs);
void LookupCommonOrgRefs(list<COrgRefInfo>& org_refs);
bool PerformTaxLookup(CBioSource& biosource, const list<COrgRefInfo>& org_refs, bool is_tax_lookup);

}

#endif // WGS_TAX_H
