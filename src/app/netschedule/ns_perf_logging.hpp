#ifndef NETSCHEDULE_PERF_LOGGING__HPP
#define NETSCHEDULE_PERF_LOGGING__HPP

/*  $Id: ns_perf_logging.hpp 515257 2016-09-29 15:49:32Z satskyse $
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
 * File Description: NetSchedule performance logging
 *
 */

#include <connect/services/netschedule_api.hpp>

#include "ns_types.hpp"

BEGIN_NCBI_SCOPE


class CJob;
class CQueue;


// Produces a record for the performance logging
void g_DoPerfLogging(const CQueue &  queue, const CJob &  job, int  status);
void g_DoPerfLogging(const CQueue &  queue,
                     const vector<TJobStatus> &  statuses,
                     const vector<unsigned int> &  counters);
void g_DoErasePerfLogging(const CQueue &  queue, const CJob &  job);



END_NCBI_SCOPE

#endif /* NETSCHEDULE_PERF_LOGGING__HPP */

