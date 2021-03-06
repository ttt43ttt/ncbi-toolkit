#ifndef CONNECT___NAMERD__H
#define CONNECT___NAMERD__H

/* $Id: ncbi_namerd.h 533505 2017-04-17 15:01:36Z mcelhany $
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
 * Author:  David McElhany
 *
 * File Description:
 *   Low-level API to resolve NCBI service name to the server meta-address
 *   with the use of NAMERD.
 *
 */

#include "ncbi_servicep.h"


#ifdef __cplusplus
extern "C" {
#endif

extern const SSERV_VTable* SERV_NAMERD_Open(SERV_ITER           iter,
                                            const SConnNetInfo* net_info,
                                            SSERV_Info**        info);

extern int SERV_NAMERD_SetConnectorSource(const char* mock_body);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* CONNECT___NAMERD__H */
