/*  $Id: remote_cgi_wn.cpp 549809 2017-10-27 14:16:47Z sadyrovr $
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
 * Authors:  Maxim Didenko, Dmitry Kazimirov
 *
 * File Description:  NetSchedule worker node sample
 *
 */

#include <ncbi_pch.hpp>

#include "exec_helpers.hpp"

#include <connect/services/grid_globals.hpp>

#include <cgi/ncbicgi.hpp>

#include <corelib/ncbiapp.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbiexec.hpp>

#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>

USING_NCBI_SCOPE;

struct IsStandard : unary_function <string,bool>
{
    bool operator() (const string& val) const;
    static const char* const sm_StandardCgiEnv[];
};

const char* const IsStandard::sm_StandardCgiEnv[] = {
    "DOCUMENT_ROOT",
    "GATEWAY_INTERFACE",
    "PROXIED_IP",
    "QUERY_STRING",
    "REMOTE_ADDR",
    "REMOTE_PORT",
    "REMOTE_IDENT",
    "REQUEST_METHOD",
    "REQUEST_URI",
    "SCRIPT_FILENAME",
    "SCRIPT_NAME",
    "SCRIPT_URI",
    "SCRIPT_URL",
    "SERVER_ADDR",
    "SERVER_ADMIN",
    "SERVER_NAME",
    "SERVER_PORT",
    "SERVER_PROTOCOL",
    "SERVER_SIGNATURE",
    "SERVER_SOFTWARE",
    "SERVER_METHOD",
    "PATH_INFO",
    "PATH_TRANSLATED",
    "AUTH_TYPE",
    "CONTENT_TYPE",
    "CONTENT_LENGTH",
    "UNIQUE_ID",
    NULL
};

inline bool IsStandard::operator() (const string& val) const
{
    if (NStr::StartsWith(val, "HTTP_")) return true;
    for( int i = 0; sm_StandardCgiEnv[i] != NULL; ++i) {
        if (NStr::Compare(val, sm_StandardCgiEnv[i]) == 0) return true;
    }
    return false;
}

template <typename Cont>
struct HasValue : unary_function <string,bool>
{
    HasValue(const Cont& cont) : m_Cont(cont) {}
    inline bool operator() (const string& val) const
    {
        return find(m_Cont.begin(), m_Cont.end(), val) != m_Cont.end();
    }
    const Cont& m_Cont;
};

class CCgiEnvHolder
{
public:
    CCgiEnvHolder(const CRemoteAppLauncher& remote_app_launcher,
        const CNcbiEnvironment& client_env,
        const CNetScheduleJob& job,
        const string& service_name,
        const string& queue_name);

    const char* const* GetEnv() const { return &m_Env[0]; }

private:
    list<string> m_EnvValues;
    vector<const char*> m_Env;
};

CCgiEnvHolder::CCgiEnvHolder(const CRemoteAppLauncher& remote_app_launcher,
    const CNcbiEnvironment& client_env,
    const CNetScheduleJob& job,
    const string& service_name,
    const string& queue_name)
{
    list<string> cln_names;
    client_env.Enumerate(cln_names);
    list<string> names(cln_names.begin(), cln_names.end());

    names.erase(remove_if(names.begin(), names.end(),
            not1(IsStandard())), names.end());

    names.erase(remove_if(names.begin(), names.end(),
            HasValue<list<string> >(remote_app_launcher.GetExcludedEnv())),
            names.end());

    list<string> inc_names(cln_names.begin(), cln_names.end());

    inc_names.erase(remove_if(inc_names.begin(),inc_names.end(),
        not1(HasValue<list<string> >(remote_app_launcher.GetIncludedEnv()))),
            inc_names.end());

    names.insert(names.begin(),inc_names.begin(), inc_names.end());
    const CRemoteAppLauncher::TEnvMap& added_env =
            remote_app_launcher.GetAddedEnv();
    ITERATE(CRemoteAppLauncher::TEnvMap, it, added_env) {
        m_EnvValues.push_back(it->first + "=" +it->second);
    }

    ITERATE(list<string>, it, names) {
        if (added_env.find(*it) == added_env.end())
            m_EnvValues.push_back(*it + "=" + client_env.Get(*it));
    }

    list<string> local_names;
    const CNcbiEnvironment& local_env = remote_app_launcher.GetLocalEnv();
    local_env.Enumerate(local_names);
    ITERATE(list<string>, it, local_names) {
        const string& s = *it;
        if (added_env.find(s) == added_env.end()
            && find(names.begin(), names.end(), s) == names.end())
            m_EnvValues.push_back(s + "=" + local_env.Get(s));
    }

    m_EnvValues.push_back("NCBI_NS_SERVICE=" + service_name);
    m_EnvValues.push_back("NCBI_NS_QUEUE=" + queue_name);
    m_EnvValues.push_back("NCBI_NS_JID=" + job.job_id);
    m_EnvValues.push_back("NCBI_JOB_AFFINITY=" + job.affinity);

    if (!job.client_ip.empty())
        m_EnvValues.push_back("NCBI_LOG_CLIENT_IP=" + job.client_ip);

    if (!job.session_id.empty())
        m_EnvValues.push_back("NCBI_LOG_SESSION_ID=" + job.session_id);

    if (!job.page_hit_id.empty())
        m_EnvValues.push_back("NCBI_LOG_HIT_ID=" + job.page_hit_id);

    ITERATE(list<string>, it, m_EnvValues) {
        m_Env.push_back(it->c_str());
    }
    m_Env.push_back(NULL);
}


///////////////////////////////////////////////////////////////////////

class CRemoteCgiJob : public IWorkerNodeJob
{
public:
    CRemoteCgiJob(const IWorkerNodeInitContext& context,
            const CRemoteAppLauncher& remote_app_launcher);

    virtual ~CRemoteCgiJob() {}

    int Do(CWorkerNodeJobContext& context);

private:
    const CRemoteAppLauncher& m_RemoteAppLauncher;
};

int CRemoteCgiJob::Do(CWorkerNodeJobContext& context)
{
    if (context.IsLogRequested()) {
        LOG_POST(Note << "Job " << context.GetJobKey() + " input: " +
            context.GetJobInput());
    }

    auto_ptr<CCgiRequest> request;

    try {
        request.reset(new CCgiRequest(context.GetIStream(),
            CCgiRequest::fIgnoreQueryString |
            CCgiRequest::fDoNotParseContent));
    }
    catch (exception&) {
        ERR_POST("Cannot deserialize remote_cgi job");
        context.CommitJobWithFailure(
            "Error while parsing CGI request stream");
        return -1;
    }

    CCgiEnvHolder env(m_RemoteAppLauncher,
            request->GetEnvironment(),
            context.GetJob(),
            context.GetWorkerNode().GetServiceName(),
            context.GetQueueName());
    vector<string> args;

    CNcbiOstrstream err;
    CNcbiStrstream str_in;
    CNcbiIstream* in = request->GetInputStream();
    if (!in)
        in = &str_in;

    int ret = -1;
    bool finished_ok = m_RemoteAppLauncher.ExecRemoteApp(args,
                                        *in,
                                        context.GetOStream(),
                                        err,
                                        ret,
                                        context,
                                        0,
                                        env.GetEnv());

    m_RemoteAppLauncher.FinishJob(finished_ok, ret, context);

    if (context.IsLogRequested()) {
        if ( !IsOssEmpty(err) )
            LOG_POST(Note << "STDERR: " << (string)CNcbiOstrstreamToString(err));

        LOG_POST(Note << "Job " << context.GetJobKey() <<
            " is " << context.GetCommitStatusDescription(
                    context.GetCommitStatus()) <<
            ". Exit code: " << ret <<
            "; output: " << context.GetJobOutput());
    }

    return ret;
}

CRemoteCgiJob::CRemoteCgiJob(const IWorkerNodeInitContext&,
        const CRemoteAppLauncher& remote_app_launcher) :
    m_RemoteAppLauncher(remote_app_launcher)
{
    CGridGlobals::GetInstance().SetReuseJobObject(true);
}

class CRemoteCgiIdleTask : public IWorkerNodeIdleTask
{
public:
    CRemoteCgiIdleTask(const string& idle_app_cmd) : m_AppCmd(idle_app_cmd)
    {
    }

    virtual ~CRemoteCgiIdleTask() {}

    virtual void Run(CWorkerNodeIdleTaskContext&)
    {
        if (!m_AppCmd.empty())
            CExec::System(m_AppCmd.c_str());
    }

private:
    string m_AppCmd;
};

typedef CRemoteAppBaseListener CRemoteCgiListener;

#define GRID_APP_NAME "remote_cgi"

class CRemoteCgiJobFactory : public IWorkerNodeJobFactory
{
public:
    virtual void Init(const IWorkerNodeInitContext& context)
    {
        m_RemoteAppLauncher.reset(
                new CRemoteAppLauncher("remote_cgi", context.GetConfig()));

        CFile file(m_RemoteAppLauncher->GetAppPath());

        if (!file.Exists()) {
            NCBI_THROW_FMT(CException, eInvalid, "File \"" <<
                    m_RemoteAppLauncher->GetAppPath() <<
                    "\" does not exists.");
        }

        if (!CRemoteAppLauncher::CanExec(file)) {
            NCBI_THROW_FMT(CException, eInvalid, "File \"" <<
                    m_RemoteAppLauncher->GetAppPath() <<
                    "\" is not executable.");
        }

        m_WorkerNodeInitContext = &context;
        string idle_app_cmd = context.GetConfig().GetString("remote_cgi",
                "idle_app_cmd", kEmptyStr);
        if (!idle_app_cmd.empty())
            m_IdleTask.reset(new CRemoteCgiIdleTask(idle_app_cmd));
    }
    virtual IWorkerNodeJob* CreateInstance(void)
    {
        return new CRemoteCgiJob(*m_WorkerNodeInitContext, *m_RemoteAppLauncher);
    }
    virtual IWorkerNodeIdleTask* GetIdleTask() { return m_IdleTask.get(); }

    virtual string GetJobVersion() const
    {
        return m_WorkerNodeInitContext->GetNetScheduleAPI().GetProgramVersion();
    }
    virtual string GetAppName() const
    {
        return GRID_APP_NAME;
    }
    virtual string GetAppVersion() const
    {
        _ASSERT(m_RemoteAppLauncher.get());
        return m_RemoteAppLauncher->GetAppVersion(GRID_APP_VERSION);
    }

    CRemoteCgiListener* CreateListener() const
    {
        return new CRemoteCgiListener(m_RemoteAppLauncher);
    }

private:
    auto_ptr<CRemoteAppLauncher> m_RemoteAppLauncher;
    const IWorkerNodeInitContext* m_WorkerNodeInitContext;
    auto_ptr<CRemoteCgiIdleTask> m_IdleTask;
};

int main(int argc, const char* argv[])
{
    GRID_APP_CHECK_VERSION_ARGS();
    return Main<CRemoteCgiJobFactory, CRemoteCgiListener>(argc, argv);
}
