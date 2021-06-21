/*  $Id: ns_automation.hpp 555600 2018-01-19 16:46:14Z sadyrovr $
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
 *   Government have not placed any restriction on its use or reproduction.
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
 * File Description: Automation processor - NetSchedule declarations.
 *
 */

#ifndef NS_AUTOMATION__HPP
#define NS_AUTOMATION__HPP

#include "automation.hpp"

BEGIN_NCBI_SCOPE

namespace NAutomation
{

struct SNetScheduleService : public SNetService
{
    using TSelf = SNetScheduleService;

    virtual const string& GetType() const { return kName; }

    CNetService GetService() { return m_NetScheduleAPI.GetService(); }

    void ExecSetClientType(const TArguments& args, SInputOutput& io);
    void ExecSetNodeSession(const TArguments& args, SInputOutput& io);
    void ExecQueueInfo(const TArguments& args, SInputOutput& io);
    void ExecQueueClassInfo(const TArguments& args, SInputOutput& io);
    void ExecReconf(const TArguments& args, SInputOutput& io);
    void ExecSuspend(const TArguments& args, SInputOutput& io);
    void ExecResume(const TArguments& args, SInputOutput& io);
    void ExecShutdown(const TArguments& args, SInputOutput& io);
    void ExecParseKey(const TArguments& args, SInputOutput& io);
    void ExecJobInfo(const TArguments& args, SInputOutput& io);
    void ExecJobCounters(const TArguments& args, SInputOutput& io);
    void ExecGetServers(const TArguments& args, SInputOutput& io);

    static CCommand CallCommand();
    static TCommands CallCommands();
    static CCommand NewCommand();
    static CAutomationObject* Create(const TArguments& args, CAutomationProc* automation_proc);

    static const string kName;

protected:
    CNetScheduleAPIExt m_NetScheduleAPI;

    SNetScheduleService(CAutomationProc* automation_proc,
            CNetScheduleAPI ns_api);
};

struct SNetScheduleServer : public SNetScheduleService
{
    using TSelf = SNetScheduleServer;

    SNetScheduleServer(CAutomationProc* automation_proc,
            CNetScheduleAPIExt ns_api, CNetServer::TInstance server);

    virtual const string& GetType() const { return kName; }

    void ExecServerStatus(const TArguments& args, SInputOutput& io);
    void ExecJobGroupInfo(const TArguments& args, SInputOutput& io);
    void ExecClientInfo(const TArguments& args, SInputOutput& io);
    void ExecNotificationInfo(const TArguments& args, SInputOutput& io);
    void ExecAffinityInfo(const TArguments& args, SInputOutput& io);
    void ExecChangePreferredAffinities(const TArguments& args, SInputOutput& io);

    static CCommand CallCommand();
    static TCommands CallCommands();
    static CCommand NewCommand();
    static CAutomationObject* Create(const TArguments& args, CAutomationProc* automation_proc);

    static const string kName;
};

}

END_NCBI_SCOPE

#endif // NS_AUTOMATION__HPP
