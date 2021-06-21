/* $Id: Mod.hpp 161846 2009-06-01 18:54:29Z lewisg $
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
 *   using specifications from the data definition file
 *   'omssa.asn'.
 */

#ifndef OBJECTS_OMSSA_MOD_HPP
#define OBJECTS_OMSSA_MOD_HPP

#include <vector>

// generated includes
#include <objects/omssa/MSMod.hpp>
#include <objects/omssa/MSModType.hpp>
#include <objects/omssa/MSSearchSettings.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::


/////////////////////////////////////////////////////////////////////////////
//
//  CMSMod::
//
//  Given a set of variable mods, sorts them into categories for quick access
//

class NCBI_XOMSSA_EXPORT CMSMod {
public:
    CMSMod(void) {};
    CMSMod(const CMSSearchSettings::TVariable &Mods, CRef<CMSModSpecSet> Modset);

    /**
     *  initialize variable mod type array
     * 
     * @param Mods list of input mods
     * @param Modset container for mods
     * @return was methionine cleavage one of the mods?
     */
    bool Init(const CMSSearchSettings::TVariable &Mods,
               CRef<CMSModSpecSet> Modset);

    typedef vector < int > TModLists;
    const TModLists &GetAAMods(EMSModType Type) const;    
private:
    TModLists ModLists[eMSModType_modmax];
};

///////////////////  CMSMod  inline methods

inline 
const CMSMod::TModLists & CMSMod::GetAAMods(EMSModType Type) const
{ 
    return ModLists[Type]; 
}


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

#endif // OBJECTS_OMSSA_MSMOD_HPP
/* Original file checksum: lines: 63, chars: 1907, CRC32: 6c23d0ae */