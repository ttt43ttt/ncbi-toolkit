#ifndef NETSCHEDULE_STATISTICS_COUNTERS__HPP
#define NETSCHEDULE_STATISTICS_COUNTERS__HPP

/*  $Id: ns_statistics_counters.hpp 516557 2016-10-14 14:14:58Z satskyse $
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
 * Authors:  Denis Vakatov (design), Sergey Satskiy (implementation)
 *
 * File Description: NetSchedule statistics counters
 *
 */

#include "background_host.hpp"

BEGIN_NCBI_SCOPE

class CDiagContext_Extra;



// Each queue has this set of counters.
// The counters are never reset, they are only increased.
class CStatisticsCounters
{
public:
    // The Pending(Done) state can be reached via 3 ways:
    // - RETURN (RDRB) received
    // - Timeout has happened
    // - FPUT (FRED) received.
    // So, to separate the cases, the ETransitionPathOption is introduced
    enum ETransitionPathOption {
        eNone       = 0,   // Basically, no timeout, no fail. Normal case for
                           // the vast majority of the transitions.
        eTimeout    = 1,
        eFail       = 2,
        eClear      = 3,
        eNewSession = 4
    };

    enum ECountersScope {
        eQueueCounters      = 0,
        eServerWideCounters = 1
    };

    CStatisticsCounters(ECountersScope  scope);

    void PrintTransitions(CDiagContext_Extra &  extra) const;
    void PrintDelta(CDiagContext_Extra &  extra,
                    const CStatisticsCounters &  prev) const;
    string PrintTransitions(void) const;
    void CountTransition(CNetScheduleAPI::EJobStatus  from,
                         CNetScheduleAPI::EJobStatus  to,
                         ETransitionPathOption        path_option = eNone);
    void CountTransitionToDeleted(CNetScheduleAPI::EJobStatus  from,
                                  size_t                       count);
    void CountDBDeletion(size_t  count);
    void CountSubmit(size_t  count);
    void CountNSSubmitRollback(size_t  count);
    void CountNSGetRollback(size_t  count);
    void CountNSReadRollback(size_t  count);
    void CountOutdatedPick(ECommandGroup  cmd_group);
    void CountToPendingWithoutBlacklist(size_t  count);
    void CountToPendingRescheduled(size_t  count);
    void CountJobInfoCacheHit(size_t  count);
    void CountJobInfoCacheMiss(size_t  count);
    void CountJobInfoCacheGCRemoved(size_t  count);
    void CountRedo(CNetScheduleAPI::EJobStatus  from);
    void CountReread(CNetScheduleAPI::EJobStatus  from,
                     CNetScheduleAPI::EJobStatus  to);

    static void  PrintServerWide(size_t  affinities);

private:
    static string x_GetTransitionCounterName(size_t  index_from,
                                             size_t  index_to);

private:
    ECountersScope                  m_Scope;

    // +1 is for eDeleted
    CAtomicCounter_WithAutoInit     m_Transitions[g_ValidJobStatusesSize]
                                                 [g_ValidJobStatusesSize + 1];
    CAtomicCounter_WithAutoInit     m_SubmitCounter;
    CAtomicCounter_WithAutoInit     m_NSSubmitRollbackCounter;
    CAtomicCounter_WithAutoInit     m_NSGetRollbackCounter;
    CAtomicCounter_WithAutoInit     m_NSReadRollbackCounter;
    CAtomicCounter_WithAutoInit     m_DBDeleteCounter;

    // There are some special counters:
    CAtomicCounter_WithAutoInit     m_ToPendingDueToTimeoutCounter;
    CAtomicCounter_WithAutoInit     m_ToPendingDueToFailCounter;
    CAtomicCounter_WithAutoInit     m_ToPendingDueToClearCounter;
    CAtomicCounter_WithAutoInit     m_ToPendingDueToNewSessionCounter;
    CAtomicCounter_WithAutoInit     m_ToPendingWithoutBlacklist;
    CAtomicCounter_WithAutoInit     m_ToPendingRescheduled;
    CAtomicCounter_WithAutoInit     m_ToDoneDueToTimeoutCounter;
    CAtomicCounter_WithAutoInit     m_ToDoneDueToFailCounter;
    CAtomicCounter_WithAutoInit     m_ToDoneDueToClearCounter;
    CAtomicCounter_WithAutoInit     m_ToDoneDueToNewSessionCounter;
    CAtomicCounter_WithAutoInit     m_ToFailedDueToTimeoutCounter;
    CAtomicCounter_WithAutoInit     m_ToFailedDueToClearCounter;
    CAtomicCounter_WithAutoInit     m_ToFailedDueToNewSessionCounter;
    CAtomicCounter_WithAutoInit     m_ToReadFailedDueToTimeoutCounter;
    CAtomicCounter_WithAutoInit     m_ToReadFailedDueToClearCounter;
    CAtomicCounter_WithAutoInit     m_ToReadFailedDueToNewSessionCounter;
    CAtomicCounter_WithAutoInit     m_ToCanceledDueToReadTimeoutCounter;
    CAtomicCounter_WithAutoInit     m_ToFailedDueToReadTimeoutCounter;
    CAtomicCounter_WithAutoInit     m_ToCanceledDueToReadNewSessionCounter;
    CAtomicCounter_WithAutoInit     m_ToCanceledDueToReadClearCounter;
    CAtomicCounter_WithAutoInit     m_ToFailedDueToReadNewSessionCounter;
    CAtomicCounter_WithAutoInit     m_ToFailedDueToReadClearCounter;

    CAtomicCounter_WithAutoInit     m_PickedAsPendingOutdated;
    CAtomicCounter_WithAutoInit     m_PickedAsReadOutdated;

    CAtomicCounter_WithAutoInit     m_JobInfoCacheHit;
    CAtomicCounter_WithAutoInit     m_JobInfoCacheMiss;
    CAtomicCounter_WithAutoInit     m_JobInfoGCRemoved;

    // REDO related counters
    CAtomicCounter_WithAutoInit     m_FromDoneJobRedo;
    CAtomicCounter_WithAutoInit     m_FromFailedJobRedo;
    CAtomicCounter_WithAutoInit     m_FromConfirmedJobRedo;
    CAtomicCounter_WithAutoInit     m_FromReadFailedJobRedo;
    CAtomicCounter_WithAutoInit     m_FromCanceledJobRedo;

    // REREAD related counters
    CAtomicCounter_WithAutoInit     m_CancelToCancelJobReread;
    CAtomicCounter_WithAutoInit     m_CancelToDoneJobReread;
    CAtomicCounter_WithAutoInit     m_CancelToFailedJobReread;
    CAtomicCounter_WithAutoInit     m_ReadFailedToCancelJobReread;
    CAtomicCounter_WithAutoInit     m_ReadFailedToDoneJobReread;
    CAtomicCounter_WithAutoInit     m_ReadFailedToFailedJobReread;
    CAtomicCounter_WithAutoInit     m_ConfirmedToCancelJobReread;
    CAtomicCounter_WithAutoInit     m_ConfirmedToDoneJobReread;
    CAtomicCounter_WithAutoInit     m_ConfirmedToFailedJobReread;
};


END_NCBI_SCOPE

#endif /* NETSCHEDULE_STATISTICS_COUNTERS__HPP */

