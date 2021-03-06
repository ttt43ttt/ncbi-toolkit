/*  $Id: ns_server.cpp 542378 2017-07-31 13:02:26Z satskyse $
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
 * Authors:  Anatoliy Kuznetsov, Victor Joukov
 *
 * File Description: NetScheduler threaded server
 *
 */

#include <ncbi_pch.hpp>

#include "ns_server.hpp"
#include "ns_ini_params.hpp"
#include "queue_database.hpp"
#include "ns_types.hpp"

#include <corelib/request_ctx.hpp>

USING_NCBI_SCOPE;


CNetScheduleServer* CNetScheduleServer::sm_netschedule_server = 0;

//////////////////////////////////////////////////////////////////////////
/// NetScheduler threaded server implementation
CNetScheduleServer::CNetScheduleServer(const string &  dbpath)
    : m_BackgroundHost(this),
      m_RequestExecutor(this),
      m_Port(0),
      m_HostNetAddr(0),
      m_Shutdown(false),
      m_SigNum(0),
      m_InactivityTimeout(default_network_timeout),
      m_QueueDB(NULL),
      m_StartTime(GetFastLocalTime()),
      m_LogFlag(default_is_log),
      m_LogBatchEachJobFlag(default_log_batch_each_job),
      m_LogNotificationThreadFlag(default_log_notification_thread),
      m_LogCleaningThreadFlag(default_log_cleaning_thread),
      m_LogExecutionWatcherThreadFlag(default_log_execution_watcher_thread),
      m_LogStatisticsThreadFlag(default_log_statistics_thread),
      m_RefuseSubmits(false),
      m_UseHostname(default_use_hostname),
      m_DBDrained(false),
      m_DeleteBatchSize(default_del_batch_size),
      m_MarkdelBatchSize(default_markdel_batch_size),
      m_ScanBatchSize(default_scan_batch_size),
      m_PurgeTimeout(default_purge_timeout),
      m_StatInterval(default_stat_interval),
      m_JobCountersInterval(default_job_counters_interval),
      m_MaxClientData(default_max_client_data),
      m_NodeID("not_initialized"),
      m_SessionID("s" + x_GenerateGUID()),
      m_StartIDs(dbpath),
      m_AnybodyCanReconfigure(false),
      m_ReserveDumpSpace(default_reserve_dump_space),
      m_WSTCacheSize(default_wst_cache_size)
{
    m_CurrentSubmitsCounter.Set(kSubmitCounterInitialValue);
    sm_netschedule_server = this;
}


CNetScheduleServer::~CNetScheduleServer()
{
    delete m_QueueDB;
}


void CNetScheduleServer::AddDefaultListener(IServer_ConnectionFactory* factory)
{
    // port must be set before listening
    _ASSERT(m_Port);
    AddListener(factory, m_Port);
}


// Returns what was changed
CJsonNode CNetScheduleServer::SetNSParameters(const SNS_Parameters &  params,
                                              bool                    limited)
{
    CJsonNode   what_changed = CJsonNode::NewObjectNode();
    CJsonNode   changes = CJsonNode::NewObjectNode();
    bool        new_val;

    if (m_LogFlag != params.is_log) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendBoolean(m_LogFlag);
        values.AppendBoolean(params.is_log);
        changes.SetByKey("log", values);
    }
    m_LogFlag = params.is_log;

    new_val = (params.log_batch_each_job && m_LogFlag);
    if (m_LogBatchEachJobFlag != new_val) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendBoolean(m_LogBatchEachJobFlag);
        values.AppendBoolean(new_val);
        changes.SetByKey("log_batch_each_job", values);
    }
    m_LogBatchEachJobFlag = new_val;

    new_val = (params.log_notification_thread && m_LogFlag);
    if (m_LogNotificationThreadFlag != new_val) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendBoolean(m_LogNotificationThreadFlag);
        values.AppendBoolean(new_val);
        changes.SetByKey("log_notification_thread", values);
    }
    m_LogNotificationThreadFlag = new_val;

    new_val = (params.log_cleaning_thread && m_LogFlag);
    if (m_LogCleaningThreadFlag != new_val) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendBoolean(m_LogCleaningThreadFlag);
        values.AppendBoolean(new_val);
        changes.SetByKey("log_cleaning_thread", values);
    }
    m_LogCleaningThreadFlag = new_val;

    new_val = (params.log_execution_watcher_thread&& m_LogFlag);
    if (m_LogExecutionWatcherThreadFlag != new_val) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendBoolean(m_LogExecutionWatcherThreadFlag);
        values.AppendBoolean(new_val);
        changes.SetByKey("log_execution_watcher_thread", values);
    }
    m_LogExecutionWatcherThreadFlag = new_val;

    new_val = (params.log_statistics_thread && m_LogFlag);
    if (m_LogStatisticsThreadFlag != new_val) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendBoolean(m_LogStatisticsThreadFlag);
        values.AppendBoolean(new_val);
        changes.SetByKey("log_statistics_thread", values);
    }
    m_LogStatisticsThreadFlag = new_val;

    if (m_StatInterval != params.stat_interval) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendInteger(m_StatInterval);
        values.AppendInteger(params.stat_interval);
        changes.SetByKey("stat_interval", values);
    }
    m_StatInterval = params.stat_interval;

    if (m_JobCountersInterval != params.job_counters_interval) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendInteger(m_JobCountersInterval);
        values.AppendInteger(params.job_counters_interval);
        changes.SetByKey("job_counters_interval", values);
    }
    m_JobCountersInterval = params.job_counters_interval;

    if (m_MaxClientData != params.max_client_data) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendInteger(m_MaxClientData);
        values.AppendInteger(params.max_client_data);
        changes.SetByKey("max_client_data", values);
    }
    m_MaxClientData = params.max_client_data;

    CJsonNode   accepted_hosts = m_AdminHosts.SetHosts(params.admin_hosts);
    if (accepted_hosts.GetSize() > 0)
        changes.SetByKey("admin_host", accepted_hosts);

    CJsonNode   admin_diff = x_SetFromList(params.admin_client_names,
                                           m_AdminClientNames,
                                           m_AdminClientsLock);
    if (admin_diff.GetSize() > 0)
        changes.SetByKey("admin_client_name", admin_diff);

    CJsonNode   state_transition_perf_log_queues_diff =
                        x_SetFromList(params.state_transition_perf_log_queues,
                                      m_StateTransitionPerfLogQueues,
                                      m_STPerfLogQCLock);
    if (state_transition_perf_log_queues_diff.GetSize() > 0)
        changes.SetByKey("state_transition_perf_log_queues",
                         state_transition_perf_log_queues_diff);

    CJsonNode   state_transition_perf_log_classes_diff =
                        x_SetFromList(params.state_transition_perf_log_classes,
                                      m_StateTransitionPerfLogClasses,
                                      m_STPerfLogQCLock);
    if (state_transition_perf_log_classes_diff.GetSize() > 0)
        changes.SetByKey("state_transition_perf_log_classes",
                         state_transition_perf_log_classes_diff);

    if (m_InactivityTimeout != params.network_timeout) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendInteger(m_InactivityTimeout);
        values.AppendInteger(params.network_timeout);
        changes.SetByKey("network_timeout", values);
    }
    m_InactivityTimeout = params.network_timeout;

    if (m_ReserveDumpSpace != params.reserve_dump_space) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendInteger(m_ReserveDumpSpace);
        values.AppendInteger(params.reserve_dump_space);
        changes.SetByKey("reserve_dump_space", values);
    }
    m_ReserveDumpSpace = params.reserve_dump_space;

    if (m_WSTCacheSize != params.wst_cache_size) {
        CJsonNode       values = CJsonNode::NewArrayNode();
        values.AppendInteger(m_WSTCacheSize);
        values.AppendInteger(params.wst_cache_size);
        changes.SetByKey("wst_cache_size", values);
    }
    m_WSTCacheSize = params.wst_cache_size;


    if (limited) {
        // max_connections CServer parameter could be changed on the fly via
        // the RECO command. So check if it was changed.
        SServer_Parameters      current_params;
        CServer::GetParameters(&current_params);
        if (current_params.max_connections != params.max_connections) {
            CJsonNode       values = CJsonNode::NewArrayNode();
            values.AppendInteger(current_params.max_connections);
            values.AppendInteger(params.max_connections);
            changes.SetByKey("max_connections", values);

            current_params.max_connections = params.max_connections;
            CServer::SetParameters(current_params);
        }
    }


    if (changes.GetSize() > 0)
        what_changed.SetByKey("server_changes", changes);

    #if defined(_DEBUG) && !defined(NDEBUG)
    debug_fd_count = params.debug_fd_count;
    debug_mem_count = params.debug_mem_count;
    debug_write_delay = params.debug_write_delay;
    debug_conn_drop_before_write = params.debug_conn_drop_before_write;
    debug_conn_drop_after_write = params.debug_conn_drop_after_write;
    debug_reply_with_garbage = params.debug_reply_with_garbage;
    debug_garbage = params.debug_garbage;
    #endif

    if (limited)
        return what_changed;


    CServer::SetParameters(params);
    m_Port = params.port;
    m_HostNetAddr = CSocketAPI::gethostbyname(kEmptyStr);
    m_UseHostname = params.use_hostname;
    if (m_UseHostname)
        m_Host = CSocketAPI::gethostname();
    else
        m_Host = CSocketAPI::ntoa(m_HostNetAddr);

    // Purge related parameters
    m_DeleteBatchSize = params.del_batch_size;
    m_MarkdelBatchSize = params.markdel_batch_size;
    m_ScanBatchSize = params.scan_batch_size;
    m_PurgeTimeout = params.purge_timeout;

    m_AffRegistrySettings = params.affinity_reg;
    m_GroupRegistrySettings = params.group_reg;
    m_ScopeRegistrySettings = params.scope_reg;

    // The difference is required only at the stage of RECO.
    // RECO always calls the function with the limited flag so there is no need
    // to provide the difference here
    return CJsonNode::NewObjectNode();
}


// Return: difference
// Note: it is always called after the configuration is validated so
//       no messages should be logged
CJsonNode CNetScheduleServer::ReadServicesConfig(const CNcbiRegistry &  reg)
{
    CJsonNode               diff = CJsonNode::NewObjectNode();
    const string            section = "service_to_queue";
    map< string, string >   new_services;

    // Read the new list -- new alerts if so
    list<string>            entries;
    reg.EnumerateEntries(section, &entries);


    for (list<string>::const_iterator  k = entries.begin();
         k != entries.end(); ++k) {
        string      service_name = *k;
        string      qname = reg.Get("service_to_queue", service_name);
        if (qname.empty())
            continue;

        // Check that the queue name has been provided
        if (!m_QueueDB->QueueExists(qname))
            continue;

        // Config line is fine, memorize it
        new_services[service_name] = qname;
    }

    // Compare with the old list -- combine report string
    CJsonNode   new_items = CJsonNode::NewArrayNode();
    CJsonNode   deleted_items = CJsonNode::NewArrayNode();
    CJsonNode   changed_items = CJsonNode::NewObjectNode();

    for (map< string, string>::const_iterator  k = new_services.begin();
         k != new_services.end(); ++k) {
        map< string, string>::const_iterator    found;
        for (found = m_Services.begin(); found != m_Services.end(); ++found)
            if (NStr::CompareNocase(found->first, k->first) == 0)
                break;

        if (found == m_Services.end()) {
            new_items.AppendString(k->first);
            continue;
        }
        if (found->second != k->second) {
            CJsonNode       vals = CJsonNode::NewArrayNode();
            vals.AppendString(found->second);
            vals.AppendString(k->second);
            changed_items.SetByKey(k->first, vals);
        }
    }

    for (map< string, string>::const_iterator  k = m_Services.begin();
         k != m_Services.end(); ++k) {
        map< string, string>::const_iterator    found;
        for (found = new_services.begin(); found != new_services.end(); ++found)
            if (NStr::CompareNocase(found->first, k->first) == 0)
                break;

        if (found == new_services.end())
            deleted_items.AppendString(k->first);
    }


    if (new_items.GetSize() > 0)
        diff.SetByKey("services_added", new_items);

    if (deleted_items.GetSize() > 0)
        diff.SetByKey("services_deleted", deleted_items);

    if (changed_items.GetSize() > 0)
        diff.SetByKey("services_changed", changed_items);

    // Set the current as the new one
    CFastMutexGuard     guard(m_ServicesLock);
    m_Services = new_services;
    return diff;
}


bool CNetScheduleServer::ShutdownRequested(void)
{
    return m_Shutdown;
}


void CNetScheduleServer::SetQueueDB(CQueueDataBase* qdb)
{
    delete m_QueueDB;
    m_QueueDB = qdb;
}


void CNetScheduleServer::SetShutdownFlag(int  signum, bool  db_was_drained)
{
    if (!m_Shutdown) {
        m_Shutdown = true;
        m_SigNum = signum;
        m_DBDrained = db_was_drained;
    }
}


// Queue handling
unsigned int  CNetScheduleServer::Configure(const IRegistry &  reg,
                                            CJsonNode &        diff)
{
    return m_QueueDB->Configure(reg, diff);
}

unsigned CNetScheduleServer::CountActiveJobs() const
{
    return m_QueueDB->CountActiveJobs();
}


CRef<CQueue> CNetScheduleServer::OpenQueue(const std::string& name)
{
    return m_QueueDB->OpenQueue(name);
}


void CNetScheduleServer::CreateDynamicQueue(const CNSClientId &  client,
                                            const string &  qname,
                                            const string &  qclass,
                                            const string &  description)
{
    m_QueueDB->CreateDynamicQueue(client, qname, qclass, description);
}


void CNetScheduleServer::DeleteDynamicQueue(const CNSClientId &  client,
                                            const std::string& qname)
{
    m_QueueDB->DeleteDynamicQueue(client, qname);
}


SQueueParameters
CNetScheduleServer::QueueInfo(const string &  qname) const
{
    return m_QueueDB->QueueInfo(qname);
}


std::string CNetScheduleServer::GetQueueNames(const string &  sep) const
{
    return m_QueueDB->GetQueueNames(sep);
}


void CNetScheduleServer::PrintMutexStat(CNcbiOstream& out)
{
    m_QueueDB->PrintMutexStat(out);
}


void CNetScheduleServer::PrintLockStat(CNcbiOstream& out)
{
    m_QueueDB->PrintLockStat(out);
}


void CNetScheduleServer::PrintMemStat(CNcbiOstream& out)
{
    m_QueueDB->PrintMemStat(out);
}


string CNetScheduleServer::PrintTransitionCounters(void)
{
    return m_QueueDB->PrintTransitionCounters();
}


string CNetScheduleServer::PrintJobsStat(const CNSClientId &  client)
{
    return m_QueueDB->PrintJobsStat(client);
}


string CNetScheduleServer::GetQueueClassesInfo(void) const
{
    return m_QueueDB->GetQueueClassesInfo();
}


string CNetScheduleServer::GetQueueClassesConfig(void) const
{
    return m_QueueDB->GetQueueClassesConfig();
}


string CNetScheduleServer::GetQueueInfo(void) const
{
    return m_QueueDB->GetQueueInfo();
}


string CNetScheduleServer::GetQueueConfig(void) const
{
    return m_QueueDB->GetQueueConfig();
}


string CNetScheduleServer::GetLinkedSectionConfig(void) const
{
    return m_QueueDB->GetLinkedSectionConfig();
}


string CNetScheduleServer::GetServiceToQueueSectionConfig(void) const
{
    string                                  output;
    map< string, string >::const_iterator   k;
    CFastMutexGuard                         guard(m_ServicesLock);

    if (m_Services.empty())
        return output;

    output = "\n[service_to_queue]";
    for (k = m_Services.begin(); k != m_Services.end(); ++k) {
        if (!output.empty())
            output += "\n";
        output += k->first + "=\"" + k->second + "\"";
    }

    return output;
}


string CNetScheduleServer::ResolveService(const string &  service) const
{
    map< string, string >::const_iterator   k;
    CFastMutexGuard                         guard(m_ServicesLock);

    for (k = m_Services.begin(); k != m_Services.end(); ++k) {
        if (NStr::CompareNocase(k->first, service) == 0)
            return k->second;
    }
    return "";
}


void CNetScheduleServer::GetServices(map<string, string> &  services) const
{
    CFastMutexGuard     guard(m_ServicesLock);
    services = m_Services;
}


bool CNetScheduleServer::AdminHostValid(unsigned host) const
{
    return m_AdminHosts.IsAllowed(host);
}


bool CNetScheduleServer::IsAdminClientName(const string &  name) const
{
    CReadLockGuard      guard(m_AdminClientsLock);

    for (vector<string>::const_iterator  k(m_AdminClientNames.begin());
         k != m_AdminClientNames.end(); ++k)
        if (*k == name)
            return true;
    return false;
}

string CNetScheduleServer::GetAdminClientNames(void) const
{
    string              ret;
    CReadLockGuard      guard(m_AdminClientsLock);

    for (vector<string>::const_iterator  k(m_AdminClientNames.begin());
         k != m_AdminClientNames.end(); ++k) {
        if (!ret.empty())
            ret += ", ";
        ret += *k;
    }
    return ret;
}


string CNetScheduleServer::GetStateTransitionPerfLogQueues(void) const
{
    string              ret;
    CReadLockGuard      guard(m_STPerfLogQCLock);

    for (vector<string>::const_iterator
            k(m_StateTransitionPerfLogQueues.begin());
            k != m_StateTransitionPerfLogQueues.end(); ++k) {
        if (!ret.empty())
            ret += ", ";
        ret += *k;
    }
    return ret;
}


string CNetScheduleServer::GetStateTransitionPerfLogClasses(void) const
{
    string              ret;
    CReadLockGuard      guard(m_STPerfLogQCLock);

    for (vector<string>::const_iterator
            k(m_StateTransitionPerfLogClasses.begin());
            k != m_StateTransitionPerfLogClasses.end(); ++k) {
        if (!ret.empty())
            ret += ", ";
        ret += *k;
    }
    return ret;
}


bool
CNetScheduleServer::ShouldPerfLogTransitions(const string &  queue_name,
                                             const string &  class_name) const
{
    CReadLockGuard      guard(m_STPerfLogQCLock);
    for (vector<string>::const_iterator
            k(m_StateTransitionPerfLogQueues.begin());
            k != m_StateTransitionPerfLogQueues.end(); ++k) {
        if (*k == "*")
            return true;
        if (NStr::CompareNocase(*k, queue_name) == 0)
            return true;
    }

    if (!class_name.empty()) {
        for (vector<string>::const_iterator
                k(m_StateTransitionPerfLogClasses.begin());
                k != m_StateTransitionPerfLogClasses.end(); ++k) {
            if (*k == "*")
                return true;
            if (NStr::CompareNocase(*k, class_name) == 0)
                return true;
        }
    }

    return false;
}


string CNetScheduleServer::GetAlerts(void) const
{
    return m_Alerts.GetURLEncoded();
}


string CNetScheduleServer::SerializeAlerts(void) const
{
    return m_Alerts.Serialize();
}


enum EAlertAckResult
CNetScheduleServer::AcknowledgeAlert(const string &  id,
                                     const string &  user)
{
    return m_Alerts.Acknowledge(id, user);
}


enum EAlertAckResult
CNetScheduleServer::AcknowledgeAlert(EAlertType      alert_type,
                                     const string &  user)
{
    return m_Alerts.Acknowledge(alert_type, user);
}


void CNetScheduleServer::RegisterAlert(EAlertType  alert_type,
                                       const string &  message)
{
    m_Alerts.Register(alert_type, message);
}


// The method is called after the database is created or loaded.
// This guarantees that the directory is there.
// The file with an identifier could be read or created safely.
// Returns: true if everything is fine.
void CNetScheduleServer::InitNodeID(const string &  db_path)
{
    CFile   node_id_file(CFile::MakePath(
                            CDirEntry::AddTrailingPathSeparator(db_path),
                            kNodeIDFileName));

    if (node_id_file.Exists()) {
        // File exists, read the ID from it
        CFileIO     f;
        char        buffer[64];

        f.Open(node_id_file.GetPath(), CFileIO_Base::eOpen,
                                       CFileIO_Base::eRead);
        size_t      n = f.Read(buffer, sizeof(buffer));

        m_NodeID = string(buffer, n);
        NStr::TruncateSpacesInPlace(m_NodeID, NStr::eTrunc_End);
        f.Close();
    } else {
        // No file, need to be created
        m_NodeID = "n" + x_GenerateGUID();

        CFileIO     f;
        f.Open(node_id_file.GetPath(), CFileIO_Base::eCreate,
                                       CFileIO_Base::eReadWrite);
        f.Write(m_NodeID.data(), m_NodeID.size());
        f.Close();
    }
}


CNetScheduleServer*  CNetScheduleServer::GetInstance(void)
{
    return sm_netschedule_server;
}


void CNetScheduleServer::Exit()
{
    // This method is called by the CServer::Run() when all the threads are
    // shut down but the port is still posessed

    // If it was a signal initiated shutdown then it is a good idea to log it.
    // The signal number is set to non-zero value only in case of an OS signal
    if (m_SigNum != 0) {
        if (IsLog()) {
            // Imitate the SHUTDOWN command logging with an extra information
            // about the signal number
            CRef<CRequestContext>   ctx;
            ctx.Reset(new CRequestContext());
            ctx->SetRequestID();

            CDiagContext &      diag_context = GetDiagContext();
            diag_context.SetRequestContext(ctx);
            CDiagContext_Extra      extra = diag_context.PrintRequestStart();

            extra.Print("_type", "cmd")
                 .Print("_queue", "")
                 .Print("cmd", "SHUTDOWN")
                 .Print("signum", m_SigNum);
            extra.Flush();

            ctx->SetRequestStatus(200);
            diag_context.PrintRequestStop();
            ctx.Reset();
            diag_context.SetRequestContext(NULL);
        }
    }

    m_QueueDB->Close();
}


string  CNetScheduleServer::x_GenerateGUID(void) const
{
    // Naive implementation of the unique identifier.
    Int8        pid = CProcess::GetCurrentPid();
    Int8        current_time = time(0);

    return NStr::NumericToString((pid << 32) | current_time);
}


CJsonNode
CNetScheduleServer::x_SetFromList(const string &  from,
                                  vector<string> &  to,
                                  CRWLock &  lock)
{
    CJsonNode           diff = CJsonNode::NewArrayNode();
    CWriteLockGuard     guard(lock);
    vector<string>      old = to;

    to.clear();
    NStr::Split(from, ";, \n\r", to,
                NStr::fSplit_MergeDelimiters | NStr::fSplit_Truncate);
    sort(to.begin(), to.end());

    if (old != to) {
        CJsonNode       old_vals = CJsonNode::NewArrayNode();
        CJsonNode       new_vals = CJsonNode::NewArrayNode();

        for (vector<string>::const_iterator  k = old.begin();
                k != old.end(); ++k)
            old_vals.AppendString(*k);
        for (vector<string>::const_iterator  k = to.begin();
                k != to.end(); ++k)
            new_vals.AppendString(*k);

        diff.Append(old_vals);
        diff.Append(new_vals);
    }
    return diff;
}



void CNetScheduleServer::SetRAMConfigFileChecksum(const string &  checksum)
{
    m_RAMConfigFileChecksum = checksum;
}


void CNetScheduleServer::SetDiskConfigFileChecksum(const string &  checksum)
{
    m_DiskConfigFileChecksum = checksum;
}


map<string, int> CNetScheduleServer::GetPauseQueues(void) const
{
    return m_QueueDB->GetPauseQueues();
}


vector<string> CNetScheduleServer::GetRefuseSubmitQueues(void) const
{
    return m_QueueDB->GetRefuseSubmitQueues();
}


string CNetScheduleServer::GetDataPath(void) const
{
    return m_QueueDB->GetDataPath();
}
