#ifndef NETSTORAGE_APPLICATION__HPP
#define NETSTORAGE_APPLICATION__HPP
/*  $Id: nst_application.hpp 451219 2014-11-04 20:00:40Z satskyse $
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
 * Authors:  Denis Vakatov
 *
 * File Description: Network Storage middle man server
 *
 */

#include <corelib/ncbiapp.hpp>
#include <connect/ncbi_types.h>


USING_NCBI_SCOPE;


// NetStorage daemon application
class CNetStorageDApp : public CNcbiApplication
{
public:
    CNetStorageDApp();

    void Init(void);
    int Run(void);

protected:
    EPreparseArgs PreparseArgs(int                argc,
                               const char* const* argv);

private:
    STimeout                m_ServerAcceptTimeout;
    string                  m_CommandLine;

    void x_WritePid(CNetStorageServer *  server) const;
};

#endif

