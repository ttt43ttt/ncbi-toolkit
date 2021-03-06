#ifndef GRID_CLI__HPP
#define GRID_CLI__HPP

/*  $Id: grid_cli.hpp 558507 2018-02-27 18:46:10Z kazimird $
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
 * Authors:  Dmitry Kazimirov
 *
 */

/// @file grid_cli.hpp
/// Declarations of command line interface arguments and handlers.
///

#include <corelib/ncbiapp.hpp>

#include <connect/services/grid_client.hpp>
#include <connect/services/ns_output_parser.hpp>
#include <connect/services/compound_id.hpp>

#include <misc/netstorage/netstorage.hpp>

#include "util.hpp"

#define GRID_APP_NAME "grid_cli"

#define LOGIN_TOKEN_ENV "GRID_CLI_LOGIN_TOKEN"
#define DEFAULT_APP_UID GRID_APP_NAME

#define LOGIN_TOKEN_APP_UID_FIELD "app"
#define LOGIN_TOKEN_AUTH_FIELD "cn"
#define LOGIN_TOKEN_USER_FIELD "u"
#define LOGIN_TOKEN_HOST_FIELD "h"
#define LOGIN_TOKEN_NETCACHE_FIELD "nc"
#define LOGIN_TOKEN_ICACHE_NAME_FIELD "ic"
#define LOGIN_TOKEN_ENABLE_MIRRORING "mm"
#define LOGIN_TOKEN_NETSCHEDULE_FIELD "ns"
#define LOGIN_TOKEN_QUEUE_FIELD "q"
#define LOGIN_TOKEN_SESSION_PID_FIELD "pid"
#define LOGIN_TOKEN_SESSION_TIMESTAMP_FIELD "ts"
#define LOGIN_TOKEN_SESSION_UID_FIELD "uid"
#define LOGIN_TOKEN_ALLOW_XSITE_CONN "xs"
#define LOGIN_TOKEN_NO_CONN_RETRIES "r0"
#define LOGIN_TOKEN_FILETRACK_SITE "fts"
#define LOGIN_TOKEN_FILETRACK_TOKEN "ftt"

#define LOGIN_TOKEN_OPTION "login-token"
#define NETCACHE_OPTION "netcache"
#define CACHE_OPTION "cache"
#define TRY_ALL_SERVERS_OPTION "try-all-servers"
#define NETSTORAGE_OPTION "netstorage"
#define OBJECT_KEY_OPTION "object-key"
#define USER_KEY_OPTION "user-key"
#define NAMESPACE_OPTION "namespace"
#define PERSISTENT_OPTION "persistent"
#define FAST_STORAGE_OPTION "fast-storage"
#define MOVABLE_OPTION "movable"
#define CACHEABLE_OPTION "cacheable"
#define NETSCHEDULE_OPTION "netschedule"
#define WORKER_NODE_OPTION "worker-node"
#define INPUT_OPTION "input"
#define INPUT_FILE_OPTION "input-file"
#define REMOTE_APP_ARGS_OPTION "remote-app-args"
#define OUTPUT_FILE_OPTION "output-file"
#define QUEUE_OPTION "queue"
#define BATCH_OPTION "batch"
#define AFFINITY_OPTION "affinity"
#define CLAIM_NEW_AFFINITIES_OPTION "claim-new-affinities"
#define ANY_AFFINITY_OPTION "any-affinity"
#define JOB_OUTPUT_OPTION "job-output"
#define JOB_OUTPUT_BLOB_OPTION "job-output-blob"
#define LIMIT_OPTION "limit"
#define TIMEOUT_OPTION "timeout"
#define RELIABLE_READ_OPTION "reliable-read"
#define CONFIRM_READ_OPTION "confirm-read"
#define ROLLBACK_READ_OPTION "rollback-read"
#define FAIL_READ_OPTION "fail-read"
#define JOB_ID_OPTION "job-id"
#define BRIEF_OPTION "brief"
#define WAIT_FOR_JOB_STATUS_OPTION "wait-for-job-status"
#define WAIT_FOR_JOB_EVENT_AFTER_OPTION "wait-for-job-event-after"
#define JOB_GROUP_OPTION "job-group"
#define WAIT_TIMEOUT_OPTION "wait-timeout"
#define FAIL_JOB_OPTION "fail-job"
#define ALL_QUEUES_OPTION "all-queues"
#define QUEUE_CLASSES_OPTION "queue-classes"
#define QUEUE_CLASS_OPTION "queue-class"
#define QUEUE_ARG "QUEUE"
#define SWITCH_ARG "SWITCH"
#define PULLBACK_OPTION "pullback"
#define WAIT_FOR_JOB_COMPLETION_OPTION "wait-for-job-completion"
#define NOW_OPTION "now"
#define DIE_OPTION "die"
#define DRAIN_OPTION "drain"
#define JOB_INPUT_DIR_OPTION "job-input-dir"
#define JOB_OUTPUT_DIR_OPTION "job-output-dir"
#define PROTOCOL_DUMP_OPTION "protocol-dump"
#define PASSWORD_OPTION "password"
#define OFFSET_OPTION "offset"
#define SIZE_OPTION "length"
#define FT_TOKEN_OPTION "ft-token"
#define DIRECT_MODE_OPTION "direct"
#define ATTR_VALUE_ARG "ATTR_VALUE"

#define LOGIN_COMMAND "login"
#define JOBINFO_COMMAND "jobinfo"
#define READJOB_COMMAND "readjob"
#define WATCHJOB_COMMAND "watchjob"
#define QUEUEINFO_COMMAND "queueinfo"
#define SUSPEND_COMMAND "suspend"

#define HUMAN_READABLE_OUTPUT_FORMAT "human-readable"
#define RAW_OUTPUT_FORMAT "raw"
#define JSON_OUTPUT_FORMAT "json"

#define NETSCHEDULE_CHECK_QUEUE "netschedule_check_queue"

#ifndef NCBI_OS_MSWIN
#define IO_BUFFER_SIZE (512 * 1024)
#else
#define IO_BUFFER_SIZE (16 * 1024)
#endif

BEGIN_NCBI_SCOPE

enum EOption {
    eUntypedArg,
    eOptionalID,
    eID,
    eAppUID,
    eAttrName,
    eAttrValue,
#ifdef NCBI_GRID_XSITE_CONN_SUPPORT
    eAllowXSiteConn,
#endif
    eNoConnRetries,
    eLoginToken,
    eAuth,
    eInput,
    eInputFile,
    eRemoteAppArgs,
    eRemoteAppStdIn,
    eRemoteAppStdOut,
    eRemoteAppStdErr,
    eOutputFile,
    eOutputFormat,
    eNetCache,
    eCache,
    eCacheArg,
    ePassword,
    eOffset,
    eSize,
    eTTL,
    eEnableMirroring,
    eTryAllServers,
    eUseCompoundID,
    eNetStorage,
    eObjectKey,
    eUserKey,
    eNamespace,
    ePersistent,
    eFastStorage,
    eMovable,
    eCacheable,
    eNoMetaData,
    eNetSchedule,
    eQueue,
    eWorkerNode,
    eBatch,
    eAffinity,
    eAffinityList,
    eUsePreferredAffinities,
    eClaimNewAffinities,
    eAnyAffinity,
    eExclusiveJob,
    eJobOutput,
    eJobOutputBlob,
    eReturnCode,
    eLimit,
    eAuthToken,
    eReliableRead,
    eConfirmRead,
    eRollbackRead,
    eFailRead,
    eErrorMessage,
    eJobId,
    eJobGroupInfo,
    eClientInfo,
    eNotificationInfo,
    eAffinityInfo,
    eActiveJobCount,
    eJobsByStatus,
    eStartAfterJob,
    eJobCount,
    eJobStatus,
    eVerbose,
    eBrief,
    eStatusOnly,
    eProgressMessageOnly,
    eDeferExpiration,
    eWaitForJobStatus,
    eWaitForJobEventAfter,
    eExtendLifetime,
    eProgressMessage,
    eJobGroup,
    eAllJobs,
    eWaitTimeout,
    eFailJob,
    eQueueArg,
    eAllQueues,
    eQueueClasses,
    eTargetQueueArg,
    eQueueClassArg,
    eQueueClass,
    eQueueDescription,
    eSwitchArg,
    ePullback,
    eWaitForJobCompletion,
    eNow,
    eDie,
    eDrain,
    eCompatMode,
    eJobInputDir,
    eJobOutputDir,
    eDumpCGIEnv,
    eDumpCGIStdIn,
    eAggregationInterval,
    ePreviousInterval,
    eFileTrackSite,
    eFileTrackToken,
    eMirror,
    eServiceName,
    eNoDNSLookup,
    eNCID,
    eOptionalNCID,
    eDirectMode,
    eNoServerCheck,
    eReportProgress,

    eExtendedOptionDelimiter,

    eClientNode,
    eClientSession,
    eCommand,
    eMultiline,
    eProtocolDump,
    eDebugConsole,
    eDumpNSNotifications,
    eNumberOfOptions
};

enum EOutputFormat {
    eHumanReadable,
    eRaw,
    eJSON,
    eNumberOfOutputFormats
};

#define OPTION_ACCEPTED 1
#define OPTION_SET 2
#define OPTION_EXPLICITLY_SET 4
#define OPTION_N(number) (1 << number)

class IJobInfoProcessor;

class CGridCommandLineInterfaceApp : public CNcbiApplication
{
public:
    CGridCommandLineInterfaceApp(int argc, const char* argv[]);

    virtual int Run();

    static void PrintLine(const string& line);

private:
    int m_ArgC;
    const char** m_ArgV;

    bool m_AdminMode;

    struct SOptions {
        string id;
        string auth;
        string app_uid;
        EOutputFormat output_format;
        string nc_service;
        string cache_name;
        string app_domain;
        string password;
        size_t offset;
        size_t size;
        unsigned ttl;
        string nst_service;
        string ns_service;
        string queue;
        string affinity;
        string job_output;
        string job_output_blob;
        int return_code;
        unsigned batch_size;
        unsigned limit;
        unsigned timeout;
        string auth_token;
        string start_after_job;
        size_t job_count;
        string job_statuses;
        int job_status_mask;
        int last_event_index;
        time_t extend_lifetime_by;
        string client_node;
        string client_session;
        string progress_message;
        string job_group;
        string queue_class;
        string queue_description;
        ESwitch on_off_switch;
        string error_message;
        string input;
        string remote_app_args;
        string job_input_dir;
        string job_output_dir;
        string aggregation_interval;
        string command;
        istream* input_stream;
        FILE* output_stream;
        FILE* protocol_dump;
        TNetStorageFlags netstorage_flags;
        string attr_name;
        string attr_value;
        string service_name;
        string ft_site;
        string ft_token;

        struct SNCID {
            string key;
            int version;
            string subkey;

            SNCID() : version(0), parts(0) {}
            bool AddPart(const string& part);
            void Parse(bool icache_mode, bool require_version);
            bool HasVersion() const { return !ver.empty(); }

        private:
            string ver;
            int parts;
        } ncid;

        char option_flags[eNumberOfOptions];

        SOptions() : offset(0), size(0), ttl(0), return_code(0),
            batch_size(0), limit(0), timeout(0), job_count(0),
            job_status_mask(0),
            last_event_index(kMax_Int), extend_lifetime_by(0),
            on_off_switch(eDefault),
            input_stream(NULL), output_stream(NULL), protocol_dump(NULL),
            netstorage_flags(0)
        {
            memset(option_flags, 0, sizeof(option_flags));
        }
    } m_Opts;

private:
    CNetScheduleAPI::EJobStatus StringToJobStatus(const char* status_str);
    bool ParseLoginToken(const string& token);

    void MarkOptionAsAccepted(int option)
    {
        m_Opts.option_flags[option] |= OPTION_ACCEPTED;
    }

    bool IsOptionAccepted(EOption option) const
    {
        return (m_Opts.option_flags[option] & OPTION_ACCEPTED) != 0;
    }

    int IsOptionAccepted(EOption option, int mask) const
    {
        return (m_Opts.option_flags[option] & OPTION_ACCEPTED) ? mask : 0;
    }

    bool IsOptionAcceptedButNotSet(EOption option) const
    {
        return m_Opts.option_flags[option] == OPTION_ACCEPTED;
    }

    bool IsOptionAcceptedAndSetImplicitly(EOption option) const
    {
        return m_Opts.option_flags[option] == (OPTION_ACCEPTED | OPTION_SET);
    }

    void MarkOptionAsSet(int option)
    {
        m_Opts.option_flags[option] |= OPTION_SET;
    }

    bool IsOptionSet(int option) const
    {
        return (m_Opts.option_flags[option] & OPTION_SET) != 0;
    }

    int IsOptionSet(int option, int mask) const
    {
        return (m_Opts.option_flags[option] & OPTION_SET) ? mask : 0;
    }

    void MarkOptionAsExplicitlySet(int option)
    {
        m_Opts.option_flags[option] |= OPTION_SET | OPTION_EXPLICITLY_SET;
    }

    bool IsOptionExplicitlySet(int option) const
    {
        return (m_Opts.option_flags[option] & OPTION_EXPLICITLY_SET) != 0;
    }

    int IsOptionExplicitlySet(int option, int mask) const
    {
        return (m_Opts.option_flags[option] & OPTION_EXPLICITLY_SET) ? mask : 0;
    }

    CNetCacheAPI m_NetCacheAPI;
    CNetCacheAdmin m_NetCacheAdmin;
    CNetICacheClient m_NetICacheClient;
    CNetScheduleAPIExt m_NetScheduleAPI;
    CNetScheduleAdmin m_NetScheduleAdmin;
    CNetScheduleSubmitter m_NetScheduleSubmitter;
    CNetScheduleExecutor m_NetScheduleExecutor;
    auto_ptr<CGridClient> m_GridClient;
    CNetStorage m_NetStorage;
    CNetStorageByKey m_NetStorageByKey;
    CNetStorageAdmin m_NetStorageAdmin;
    CCompoundIDPool m_CompoundIDPool;

// NetCache commands.
public:
    int Cmd_GetBlob();
    int Cmd_PutBlob();
    int Cmd_BlobInfo();
    int Cmd_RemoveBlob();
    int Cmd_Purge();

// NetStorage commands.
public:
    int Cmd_Upload();
    int Cmd_Download();
    int Cmd_Relocate();
    int Cmd_NetStorageObjectInfo();
    int Cmd_RemoveNetStorageObject();
    int Cmd_GetAttrList();
    int Cmd_GetAttr();
    int Cmd_SetAttr();

// NetSchedule commands.
public:
    int Cmd_JobInfo();
    int Cmd_SubmitJob();
    int Cmd_WatchJob();
    int Cmd_GetJobInput();
    int Cmd_GetJobOutput();
    int Cmd_ReadJob();
    int Cmd_CancelJob();
    int Cmd_RequestJob();
    int Cmd_CommitJob();
    int Cmd_ReturnJob();
    int Cmd_ClearNode();
    int Cmd_UpdateJob();
    int Cmd_QueueInfo();
    int Cmd_DumpQueue();
    int Cmd_CreateQueue();
    int Cmd_GetQueueList();
    int Cmd_DeleteQueue();

    int Cmd_Replay();

// Miscellaneous commands.
public:
    int Cmd_WhatIs();
    int Cmd_Login();
    int Cmd_ServerInfo();
    int Cmd_Stats();
    int Cmd_Health();
    int Cmd_SanityCheck();
    int Cmd_GetConf();
    int Cmd_Reconf();
    int Cmd_Drain();
    int Cmd_Suspend();
    int Cmd_Resume();
    int Cmd_Shutdown();
    int Cmd_Discover();
    int Cmd_Exec();
    int Cmd_Automate();

// Implementation details.
private:
    static bool OnWarning(bool worker_node_admin, const string& warn_msg, CNetServer server);

    enum EAPIClass {
        eNetCacheAPI,
        eNetICacheClient,
        eNetCacheAdmin,
        eNetScheduleAPI,
        eNetScheduleAdmin,
        eNetScheduleSubmitter,
        eNetScheduleExecutor,
        eWorkerNodeAdmin,
        eNetStorageAPI,
        eNetStorageAdmin,
    } m_APIClass;

    enum EAdminCmdSeverity {
        eReadOnlyAdminCmd,
        eAdminCmdWithSideEffects
    };

    void SetUp_AdminCmd(EAdminCmdSeverity cmd_severity);
    int NetCacheSanityCheck();
    int NetScheduleSanityCheck();
    void SetUp_NetCache();
    void SetUp_NetCacheCmd(bool icache_mode, bool require_version = true, bool require_service = true);
    void SetUp_NetCacheCmd() { SetUp_NetCacheCmd(IsOptionSet(eCache)); }
    void SetUp_NetCacheAdminCmd(EAdminCmdSeverity cmd_severity);

    static void PrintBlobMeta(const CNetCacheKey& key);
    void PrintServerAddress(CNetServer server);
    void SetUp_NetScheduleCmd(EAPIClass api_class,
            EAdminCmdSeverity cmd_severity = eReadOnlyAdminCmd, bool require_queue = true);
    void JobInfo_PrintStatus(CNetScheduleAPI::EJobStatus status);
    void PrintJobStatusNotification(
            CNetScheduleNotificationHandler& submit_job_handler,
            const string& job_key,
            const string& server_host);
    void CheckJobInputStream(CNcbiOstream& job_input_ostream);
    void PrepareRemoteAppJobInput(
            size_t max_embedded_input_size,
            const string& args,
            CNcbiIstream& remote_app_stdin,
            CNcbiOstream& job_input_ostream);
    void x_LoadJobInput(
            size_t max_embedded_input_size,
            CNcbiOstream &job_input_ostream);
    void SubmitJob_Batch();
    int DumpJobInputOutput(const string& data_or_blob_id);
    int PrintJobAttrsAndDumpInput(const CNetScheduleJob& job);

    void NetSchedule_SuspendResume(bool suspend);
    int PrintNetScheduleStats();
    void PrintNetScheduleStats_Generic(ENetScheduleStatTopic topic);

    void SetUp_NetStorageCmd(EAPIClass api_class,
            EAdminCmdSeverity cmd_severity = eReadOnlyAdminCmd);
    void NetStorage_PrintServerReply(CJsonNode& server_reply);
    int PrintNetStorageServerInfo();
    int PrintNetStorageServerConfig();
    int ShutdownNetStorageServer();
    int ReconfigureNetStorageServer();
    CNetStorageObject GetNetStorageObject();
    void CheckNetStorageOptions() const;

    int Automation_PipeServer();
    int Automation_DebugConsole();

    void ReadFromCin();
};

END_NCBI_SCOPE

#endif // GRID_CLI__HPP
