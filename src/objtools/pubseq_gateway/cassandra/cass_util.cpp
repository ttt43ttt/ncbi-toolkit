/*  $Id: cass_util.cpp 563503 2018-05-10 16:26:31Z saprykin $
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
 * Authors: Dmitri Dmitrienko
 *
 * File Description:
 *
 * Utility functions
 *
 */

#include <ncbi_pch.hpp>
#include <objtools/pubseq_gateway/impl/cassandra/cass_util.hpp>
#include <sys/time.h>

BEGIN_IDBLOB_SCOPE

int64_t gettime(void)
{
    int             cnt = 100;
    struct timeval  tv = {0};

    while (gettimeofday(&tv, NULL) != 0 && cnt-- > 0);
    return (int64_t)tv.tv_usec + ((int64_t)tv.tv_sec) * 1000000L;
}

END_IDBLOB_SCOPE
