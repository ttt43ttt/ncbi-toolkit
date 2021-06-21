/*  $Id: apply_mods.hpp 571048 2018-09-19 15:01:32Z ivanov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Justin Foley
 */
#ifndef _APPLY_MODS_HPP_
#define _APPLY_MODS_HPP_

#include <corelib/ncbistd.hpp>
#include <objects/seq/Bioseq.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(edit)

class CModApply_Impl;

class NCBI_XOBJEDIT_EXPORT CModApply  
{
public:
    CModApply();
    virtual ~CModApply();

    void AddMod(const string& name, const string& value);
    void Apply(CBioseq& bioseq);

protected:
    unique_ptr<CModApply_Impl> m_pImpl;
};

END_SCOPE(edit)
END_SCOPE(objects)
END_NCBI_SCOPE

#endif
// _APPLY_MODS_HPP_

