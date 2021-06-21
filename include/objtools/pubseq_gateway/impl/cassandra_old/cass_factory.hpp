#ifndef CASSFACTORY__HPP
#define CASSFACTORY__HPP

/*  $Id: cass_factory.hpp 557999 2018-02-23 14:52:14Z satskyse $
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

#include <corelib/ncbiargs.hpp>

#include <objtools/pubseq_gateway/impl/diag/IdLogUtl.hpp>
#include <objtools/pubseq_gateway/impl/diag/AppLog.hpp>
#include "cass_driver.hpp"
#include "IdCassScope.hpp"


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
    void AppInit(CArgDescriptions *  argdesc);
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

private:
    void x_ValidateArgs(void);

    DISALLOW_COPY_AND_ASSIGN(CCassConnectionFactory);

    CFastMutex              m_RunTimeParams;
    string                  m_CfgName;
    string                  m_Section;
    string                  m_CassHosts;
    string                  m_CassUserName;
    string                  m_CassPassword;
    string                  m_CassDataNamespace;
    string                  m_CassDriverLogFile;
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
};

END_IDBLOB_SCOPE

#endif
