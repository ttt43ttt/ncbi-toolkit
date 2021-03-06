#ifndef CONNECT_SERVICES___NETSERVICE_API__HPP
#define CONNECT_SERVICES___NETSERVICE_API__HPP

/*  $Id: netservice_api.hpp 555600 2018-01-19 16:46:14Z sadyrovr $
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
 * Authors:  Dmitry Kazimirov, Maxim Didenko
 *
 * File Description:
 *   Helper classes for client APIs of network services
 *   like NetCache and NetSchedule.
 *
 */

#include "srv_connections.hpp"
#include "json_over_uttp.hpp"

#include <functional>

BEGIN_NCBI_SCOPE

struct SNetServiceIteratorImpl;

class NCBI_XCONNECT_EXPORT CNetServiceIterator
{
    NCBI_NET_COMPONENT(NetServiceIterator);

    CNetServer GetServer();
    CNetServer operator *() {return GetServer();}

    bool Next();
    CNetServiceIterator& operator ++() {Next(); return *this;}
    bool Prev();
    CNetServiceIterator& operator --() {Prev(); return *this;}

    double GetRate() const;
};

class INetServerFinder
{
public:
    virtual bool Consider(CNetServer server) = 0;

    virtual ~INetServerFinder() {}
};

struct SNetServerPoolImpl;

class NCBI_XCONNECT_EXPORT CNetServerPool
{
    NCBI_NET_COMPONENT(NetServerPool);

    void StickToServer(const string& host, unsigned short port);

    void SetCommunicationTimeout(const STimeout& to);
    const STimeout& GetCommunicationTimeout() const;
};

struct SNetServiceImpl;

class NCBI_XCONNECT_EXPORT CNetService
{
    NCBI_NET_COMPONENT(NetService);

    const string& GetServiceName() const;

    CNetServerPool GetServerPool();

    enum EIterationMode {
        eSortByLoad,
        eRandomize,
        eIncludePenalized
    };

    CNetServer GetServer(unsigned host, unsigned short port);
    CNetServer GetServer(const string& host, unsigned short port);

    CNetServiceIterator Iterate(EIterationMode mode = eSortByLoad);
    CNetServiceIterator Iterate(CNetServer::TInstance priority_server);
    CNetServiceIterator IterateByWeight(const string& key);

    /// Start iteration excluding 'server' (return the next server
    /// after 'server' or NULL).
    CNetServiceIterator ExcludeServer(CNetServer::TInstance server);

    CNetServiceIterator FindServer(INetServerFinder* finder,
        EIterationMode mode = eSortByLoad);

    bool IsLoadBalanced() const;

    CNetServer::SExecResult FindServerAndExec(const string& cmd,
            bool multiline_output = false);

    void ExecOnAllServers(const string& cmd);

    enum ECmdOutputStyle {
        eSingleLineOutput,
        eUrlEncodedOutput,
        eMultilineOutput,
        eMultilineOutput_NoHeaders,
        eMultilineOutput_NetCacheStyle
    };

    void PrintCmdOutput(const string& cmd,
        CNcbiOstream& output_stream,
        ECmdOutputStyle output_style,
        CNetService::EIterationMode = CNetService::eSortByLoad);

    CNetService Clone(const string& name);

    using TEventHandler = function<bool(const string&, CNetServer)>;

    void SetErrorHandler(TEventHandler error_handler);
    void SetWarningHandler(TEventHandler warning_handler);

#ifdef NCBI_GRID_XSITE_CONN_SUPPORT
    static void AllowXSiteConnections();
    static bool IsUsingXSiteProxy();
#endif
};

/// This class is for use by the grid_cli utility only.
/// @internal
class NCBI_XCONNECT_EXPORT IExecToJson
{
public:
    virtual CJsonNode ExecOn(CNetServer server) = 0;

    virtual ~IExecToJson() {}
};

/// This function is for use by the grid_cli utility only.
/// @internal
extern NCBI_XCONNECT_EXPORT
CJsonNode g_ExecToJson(IExecToJson& exec_to_json,
        CNetService service,
        CNetService::EIterationMode iteration_mode = CNetService::eSortByLoad);

/// This function is for use by the grid_cli utility only.
/// @internal
extern NCBI_XCONNECT_EXPORT
CNetService g_DiscoverService(const string& service_name,
        const string& client_name);

/// @internal
extern NCBI_XCONNECT_EXPORT
void g_AppendClientIPSessionIDHitID(string& cmd, bool = false);

END_NCBI_SCOPE

#endif  /* CONNECT_SERVICES___NETSERVICE_API__HPP */
