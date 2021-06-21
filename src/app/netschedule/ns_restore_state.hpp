#ifndef NETSCHEDULE_RESTORE_STATE__HPP
#define NETSCHEDULE_RESTORE_STATE__HPP

/*  $Id: ns_restore_state.hpp 542378 2017-07-31 13:02:26Z satskyse $
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
 * File Description:
 *   NetSchedule server utilities to restore the state after restart and also
 *   save the state when it is changed.
 *   At the moment the state includes the queue pause state and the refuse
 *   submit state.
 *
 */


BEGIN_NCBI_SCOPE

// Forward declaration
class CNetScheduleServer;

// Saves into a file a pause state for all the queues
void SerializePauseState(const string &  data_path,
                         const map<string, int> &  paused_queues);
void SerializePauseState(CNetScheduleServer *  server);
void SerializeRefuseSubmitState(CNetScheduleServer *  server);
void SerializeRefuseSubmitState(const string &  data_path,
                                bool server_refuse_state,
                                const vector<string> refuse_submit_queues);

// Reads from a file a pause state for the queues saved by the previous NS
// session
map<string, int> DeserializePauseState(const string &  data_path);

// Reads from a file a list of queues which were in a refuse submit state when
// the previous NS session ended.
vector<string> DeserializeRefuseSubmitState(const string &  data_path);


END_NCBI_SCOPE

#endif /* NETSCHEDULE_RESTORE_STATE__HPP */
