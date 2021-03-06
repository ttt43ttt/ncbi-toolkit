#ifndef NETSTORAGE___ERROR_CODES__HPP
#define NETSTORAGE___ERROR_CODES__HPP

/*  $Id: error_codes.hpp 491141 2016-02-02 16:55:22Z satskyse $
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
 * Authors:  Sergey Satskiy
 *
 */


#include <corelib/ncbidiag.hpp>


BEGIN_NCBI_SCOPE


NCBI_DEFINE_ERRCODE_X(NetStorageServer_ErrorCode,    3000, 0);
NCBI_DEFINE_ERRCODE_X(NetServiceException_ErrorCode, 3010, 0);
NCBI_DEFINE_ERRCODE_X(NetStorageException_ErrorCode, 3020, 0);


END_NCBI_SCOPE


#endif  /* NETSTORAGE___ERROR_CODES__HPP */

