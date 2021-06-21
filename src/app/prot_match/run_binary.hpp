/*  $Id: run_binary.hpp 541430 2017-07-18 15:12:17Z foleyjp $
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
* Author:  Justin Foley
*
* File Description:
*   implicit protein matching driver code
*
* ===========================================================================
*/

#include <corelib/ncbiapp.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbifile.hpp>

BEGIN_NCBI_SCOPE

class CBinRunner 
{

public:
    CBinRunner(const string& bin_dir, const string& bin_name);
    virtual ~CBinRunner(void);

    bool CheckBinary(void) const;
    void Exec(const vector<string>& arguments);

private:
    unique_ptr<CFile> m_pBinary;
    bool m_IsSet;
};

END_NCBI_SCOPE



