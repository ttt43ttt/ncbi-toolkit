#ifndef CASSFACTORY__HPP
#define CASSFACTORY__HPP

/*  $Id: cass_factory.hpp 563957 2018-05-17 16:38:47Z saprykin $
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
 *  Casssandra connection factory class
 *
 */

#include "cass_driver.hpp"
#include "IdCassScope.hpp"

#include <corelib/ncbiargs.hpp>

BEGIN_IDBLOB_SCOPE
USING_NCBI_SCOPE;

class CCassConnectionFactory:
    public enable_shared_from_this<CCassConnectionFactory>
{
protected:
    CCassConnectionFactory();
    void ProcessParams(void);
    void GetHostPort(string &  cass_hosts, short &  cass_port);

public:
    ~CCassConnectionFactory();
    void AppParseArgs(const CArgs &  args);
    void LoadConfig(const string &  cfg_name, const string &  section);
    void LoadConfig(const CNcbiRegistry &  registry, const string &  section);
    void ReloadConfig(void);
    void ReloadConfig(const CNcbiRegistry &  registry);
    shared_ptr<CCassConnection> CreateInstance(void);

    static shared_ptr<CCassConnectionFactory> s_Create(void)
    {
        return shared_ptr<CCassConnectionFactory>(new CCassConnectionFactory());
    }

    void SetLogging(EDiagSev  severity)
    {
        m_LogSeverity = severity;
    }

private:
    void x_ValidateArgs(void);

    CCassConnectionFactory(const CCassConnectionFactory&) = delete;
    CCassConnectionFactory& operator=(const CCassConnectionFactory&) = delete;

    CFastMutex              m_RunTimeParams;
    string                  m_CfgName;
    string                  m_Section;
    string                  m_CassHosts;
    string                  m_CassUserName;
    string                  m_CassPassword;
    string                  m_CassDataNamespace;
    string                  m_PassFile;
    string                  m_PassSection;
    string                  m_LoadBalancingStr;
    unsigned int            m_CassConnTimeoutMs;
    unsigned int            m_CassQueryTimeoutMs;
    bool                    m_CassFallbackRdConsistency;
    unsigned int            m_CassFallbackWrConsistency;
    loadbalancing_policy_t  m_LoadBalancing;
    bool                    m_TokenAware;
    bool                    m_LatencyAware;
    unsigned int            m_NumThreadsIo;
    unsigned int            m_NumConnPerHost;
    unsigned int            m_MaxConnPerHost;
    unsigned int            m_Keepalive;

    EDiagSev                m_LogSeverity;
    bool                    m_LogEnabled;
};

END_IDBLOB_SCOPE

#endif
