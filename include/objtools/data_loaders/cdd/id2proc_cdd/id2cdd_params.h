#ifndef OBJTOOLS_DATA_LOADERS_CDD_ID2PROC_CDD__ID2CDD_PARAMS__H
#define OBJTOOLS_DATA_LOADERS_CDD_ID2PROC_CDD__ID2CDD_PARAMS__H

/*  $Id: id2cdd_params.h 534723 2017-05-01 18:22:26Z grichenk $
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
*  ===========================================================================
*
*  Author: Eugene Vasilchenko, Aleksey Grichenko
*
*  File Description:
*    CID2Processor CDD plugin configuration parameters
*
* ===========================================================================
*/

/* Name of CDD plugin driver */
#define NCBI_ID2PROC_CDD_DRIVER_NAME "cdd"

/*
 * configuration of CID2Processor CDD plugin is read from app config section
 * [id2proc/cdd]
 *
 */

/* Service name for CDD annotations retrieval */
#define NCBI_ID2PROC_CDD_PARAM_SERVICE_NAME "service_name"

/* Data compression for CDD annotaions */
#define NCBI_ID2PROC_CDD_PARAM_COMPRESS_DATA "compress_data"

#endif//OBJTOOLS_DATA_LOADERS_CDD_ID2PROC_CDD__ID2CDD_PARAMS__H
