/* $Id: test_ncbi_linkerd_proxy.cpp 570469 2018-09-10 13:46:13Z ivanov $
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
 * Authors:  David McElhany
 *
 * File Description:
 *  Test support of the 'http_proxy' environment variable when using the
 *  Linkerd and namerd service mappers.
 *
 */

#include <ncbi_pch.hpp>     // This header must go first

#include <connect/ncbi_http_session.hpp>
#include <connect/ncbi_socket.h>
#include <corelib/ncbi_url.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbidiag.hpp>
#include <corelib/ncbistr.hpp>
#include <util/xregexp/regexp.hpp>

#include <stdlib.h>

#include "test_assert.h"    // This header must go last


#ifdef _MSC_VER
#define unsetenv(n)     _putenv_s(n,"")
#define setenv(n,v,w)   _putenv_s(n,v)
#endif


USING_NCBI_SCOPE;


// Mappers
enum { eLinkerd, eNamerd };

// Proxies
enum {
    eProxy_fail_empty, eProxy_fail_no_colon,
    eProxy_fail_no_host, eProxy_fail_no_port,
    eProxy_fail_bad_host, eProxy_fail_bad_port,
    eProxy_pass_unset,
    eProxy_pass_linkerd,
    eProxy_pass_pool,
    eProxy_pass_localhost, eProxy_pass_127_0_0_1
};


// Helper class to change service mappers
class CMapper
{
public:
    void Configure(void)
    {
        for (auto const& env : m_EnvSet) {
            setenv(env.first.c_str(), env.second.c_str(), 1);
        }
        for (auto const& env : m_EnvUnset) {
            unsetenv(env.c_str());
        }
    }
    static void Init(vector<CMapper>& mappers);

    int                     m_Id;
    string                  m_Name;
    bool                    m_Enabled;
    map<string, string>     m_EnvSet;
    list<string>            m_EnvUnset;
};


// Helper class to change proxies
class CProxy
{
public:
    void Configure(void)
    {
        for (auto const& env : m_EnvSet) {
            setenv(env.first.c_str(), env.second.c_str(), 1);
        }
        for (auto const& env : m_EnvUnset) {
            unsetenv(env.c_str());
        }
    }
    static void Init(vector<CProxy>& proxies);

    int                     m_Id;
    string                  m_Name;
    bool                    m_Enabled;
    bool                    m_PassExpected;
    map<string, string>     m_EnvSet;
    list<string>            m_EnvUnset;
};


class CTestNcbiLinkerdCxxApp : public CNcbiApplication
{
public:
    virtual void Init(void);
    virtual int  Run(void);

private:
    void SelectMapper(int id);
    void SelectProxy(int id);

    int CompareResponse(const string& expected, const string& got);
    int ProcessResponse(CHttpResponse& resp, const string& expected);

    void TestCaseLine(
        int           id,
        const string& header,
        const string& footer,
        const string& prefix,
        const string& sep,
        bool          show_result,
        int           result);
    void TestCaseStart(int id);
    void TestCaseEnd(int id, int result);

    int Test(int id, bool pass_expected);

private:
    char                m_Hostname[300];

    vector<CMapper>     m_Mappers;
    string              m_MapperName;

    vector<CProxy>      m_Proxies;
    string              m_ProxyName;
};


void CMapper::Init(vector<CMapper>& mappers)
{
    {{
        CMapper mapper;
        mapper.m_Id = eLinkerd;
        mapper.m_Name = "linkerd";
        mapper.m_Enabled = true;
        mapper.m_EnvSet["CONN_DISPD_DISABLE"] = "1";
        mapper.m_EnvSet["CONN_LBSMD_DISABLE"] = "1";
        mapper.m_EnvSet["CONN_LINKERD_ENABLE"] = "1";
        mapper.m_EnvSet["CONN_LOCAL_ENABLE"] = "0";
        mapper.m_EnvSet["CONN_NAMERD_ENABLE"] = "0";
        mapper.m_EnvUnset.push_back("CONN_LOCAL_SERVICES");
        mapper.m_EnvUnset.push_back("TEST_CONN_LOCAL_SERVER_1");
        mapper.m_EnvUnset.push_back("BOUNCEHTTP_CONN_LOCAL_SERVER_1");
        mappers.push_back(mapper);
    }}

    {{
        CMapper mapper;
        mapper.m_Id = eNamerd;
        mapper.m_Name = "namerd";
        mapper.m_Enabled = true;
        mapper.m_EnvSet["CONN_DISPD_DISABLE"] = "1";
        mapper.m_EnvSet["CONN_LBSMD_DISABLE"] = "1";
        mapper.m_EnvSet["CONN_LINKERD_ENABLE"] = "0";
        mapper.m_EnvSet["CONN_LOCAL_ENABLE"] = "0";
        mapper.m_EnvSet["CONN_NAMERD_ENABLE"] = "1";
        mapper.m_EnvUnset.push_back("CONN_LOCAL_SERVICES");
        mapper.m_EnvUnset.push_back("TEST_CONN_LOCAL_SERVER_1");
        mapper.m_EnvUnset.push_back("BOUNCEHTTP_CONN_LOCAL_SERVER_1");
        mappers.push_back(mapper);
    }}
}


void CProxy::Init(vector<CProxy>& proxies)
{
    {{
        CProxy proxy;
        proxy.m_Id = eProxy_fail_empty;
        proxy.m_Name = "fail_empty";
        proxy.m_Enabled = true;
        proxy.m_PassExpected = false;
        proxy.m_EnvSet["http_proxy"] = "";
        proxies.push_back(proxy);
    }}

    {{
        CProxy proxy;
        proxy.m_Id = eProxy_fail_no_colon;
        proxy.m_Name = "fail_no_colon";
        proxy.m_Enabled = true;
        proxy.m_PassExpected = false;
        proxy.m_EnvSet["http_proxy"] = "host";
        proxies.push_back(proxy);
    }}

    {{
        CProxy proxy;
        proxy.m_Id = eProxy_fail_no_host;
        proxy.m_Name = "fail_no_host";
        proxy.m_Enabled = true;
        proxy.m_PassExpected = false;
        proxy.m_EnvSet["http_proxy"] = ":54321";
        proxies.push_back(proxy);
    }}

    {{
        CProxy proxy;
        proxy.m_Id = eProxy_fail_no_port;
        proxy.m_Name = "fail_no_port";
        proxy.m_Enabled = true;
        proxy.m_PassExpected = false;
        proxy.m_EnvSet["http_proxy"] = "host:";
        proxies.push_back(proxy);
    }}

    {{
        CProxy proxy;
        proxy.m_Id = eProxy_fail_bad_host;
        proxy.m_Name = "fail_bad_host";
        proxy.m_Enabled = true;
        proxy.m_PassExpected = false;
        proxy.m_EnvSet["http_proxy"] = "Bad host, bad!:54321";
        proxies.push_back(proxy);
    }}

    {{
        CProxy proxy;
        proxy.m_Id = eProxy_fail_bad_port;
        proxy.m_Name = "fail_bad_port";
        proxy.m_Enabled = true;
        proxy.m_PassExpected = false;
        proxy.m_EnvSet["http_proxy"] = "host:Bad port, bad!";
        proxies.push_back(proxy);
    }}

    {{
        CProxy proxy;
        proxy.m_Id = eProxy_pass_unset;
        proxy.m_Name = "pass_unset";
        proxy.m_Enabled = true;
        proxy.m_PassExpected = true;
        proxy.m_EnvUnset.push_back("http_proxy");
        proxies.push_back(proxy);
    }}

#ifndef _MSC_VER
    {{
        CProxy proxy;
        proxy.m_Id = eProxy_pass_linkerd;
        proxy.m_Name = "pass_linkerd";
        proxy.m_Enabled = true;
        proxy.m_PassExpected = true;
        proxy.m_EnvSet["http_proxy"] = "linkerd:4140";
        proxies.push_back(proxy);
    }}
#endif

    {{
        CProxy proxy;
        proxy.m_Id = eProxy_pass_pool;
        proxy.m_Name = "pass_pool";
        proxy.m_Enabled = true;
        proxy.m_PassExpected = true;
        proxy.m_EnvSet["http_proxy"] = "pool.linkerd-proxy.service.bethesda-dev.consul.ncbi.nlm.nih.gov:4140";
        proxies.push_back(proxy);
    }}
}


void CTestNcbiLinkerdCxxApp::SelectMapper(int id)
{
    for (auto& mapper : m_Mappers) {
        if (mapper.m_Id == id) {
            mapper.Configure();
            m_MapperName = mapper.m_Name;
            return;
        }
    }
    NCBI_USER_THROW(string("MAPPER ") + NStr::NumericToString<int>(id) + " NOT FOUND");
}


void CTestNcbiLinkerdCxxApp::SelectProxy(int id)
{
    for (auto& proxy : m_Proxies) {
        if (proxy.m_Id == id) {
            proxy.Configure();
            m_ProxyName = proxy.m_Name;
            return;
        }
    }
    NCBI_USER_THROW(string("PROXY ") + NStr::NumericToString<int>(id) + " NOT FOUND");
}


int CTestNcbiLinkerdCxxApp::CompareResponse(const string& expected, const string& got)
{
    CRegexp re(expected, CRegexp::fCompile_dotall);
    if (re.IsMatch(got)) {
        ERR_POST(Info << "--- Response Body (STDOUT) ---");
        ERR_POST(Info << got);
        return 0;
    } else {
        ERR_POST(Error << "--- Response Body (STDOUT) ---  did not match expected value");
        size_t pos = 0, len;
        string escaped = expected;
        len = escaped.length();
        while ((pos = escaped.find("\\", pos)) != NPOS) { escaped.replace(pos, 1, "\\\\"); pos += 2; }
        while ((pos = escaped.find("\r", pos)) != NPOS) { escaped.replace(pos, 1, "\\r");  pos += 2; }
        while ((pos = escaped.find("\n", pos)) != NPOS) { escaped.replace(pos, 1, "\\n");  pos += 2; }
        ERR_POST(Info << "Escaped exp string (length " << len << "): [" << escaped << "]");
        escaped = got;
        len = escaped.length();
        while ((pos = escaped.find("\\", pos)) != NPOS) { escaped.replace(pos, 1, "\\\\"); pos += 2; }
        while ((pos = escaped.find("\r", pos)) != NPOS) { escaped.replace(pos, 1, "\\r");  pos += 2; }
        while ((pos = escaped.find("\n", pos)) != NPOS) { escaped.replace(pos, 1, "\\n");  pos += 2; }
        ERR_POST(Info << "Escaped got string (length " << len << "): [" << escaped << "]");
    }
    return 1;
}


int CTestNcbiLinkerdCxxApp::ProcessResponse(CHttpResponse& resp, const string& expected)
{
    ERR_POST(Info << "HTTP Status: " << resp.GetStatusCode() << " " << resp.GetStatusText());

    string  got;
    if (resp.CanGetContentStream()) {
        CNcbiIstream& in = resp.ContentStream();
        if ( in.good() ) {
            CNcbiOstrstream out;
            NcbiStreamCopy(out, in);
            got = CNcbiOstrstreamToString(out);
            return CompareResponse(expected, got);
        }
        else {
            ERR_POST(Error << "Bad content stream.");
        }
    }
    else {
        CNcbiIstream& in = resp.ErrorStream();
        if (in.good()) {
            ERR_POST(Info << "--- Response Body (STDERR) ---");
            CNcbiOstrstream out;
            NcbiStreamCopy(out, in);
            got = CNcbiOstrstreamToString(out);
            ERR_POST(Info << got);
        }
        else {
            ERR_POST(Error << "Bad error stream.");
        }
    }
    return 1;
}


void CTestNcbiLinkerdCxxApp::TestCaseLine(
    int           id,
    const string& header,
    const string& footer,
    const string& prefix,
    const string& sep,
    bool          show_result,
    int           result)
{
    string msg("\n");
    msg += header + prefix + sep + NStr::IntToString(id) + sep + m_Hostname;
    msg += sep + m_MapperName + sep + m_ProxyName;
    if (show_result)
        msg += sep + (result == 0 ? "PASS" : "FAIL");
    msg += footer + "\n";
    ERR_POST(Info << msg);
}


void CTestNcbiLinkerdCxxApp::TestCaseStart(int id)
{
    TestCaseLine(
        id, string(80, '=') + "\n", "",
        "TestCaseStart", "\t",
        false, -1);
}


// Result records can easily be transformed into a CSV.  For example:
//      ./test_ncbi_linkerd_proxy | grep -P '^TestCaseEnd\t' | tr '\t' ,
void CTestNcbiLinkerdCxxApp::TestCaseEnd(int id, int result)
{
    TestCaseLine(
        id, "", string("\n") + string(80, '-') + "\n",
        "TestCaseEnd", "\t",
        true, result);
}


int CTestNcbiLinkerdCxxApp::Test(int id, bool pass_expected)
{
    static const char* url = "cxx-fast-cgi-sample";
    static const char* post_data = "message=hi%20there%0A";
    static const char* exp_data = "(^.*?C\\+\\+ GIT FastCGI Sample.*?<p>Your previous message: +'hi there\\n'.*$)";

    TestCaseStart(id);
    bool test_passed = false;
    bool post_worked = false;
    int result;
    CHttpResponse resp = g_HttpPost(CUrl(url), post_data);
    result = ProcessResponse(resp, exp_data);
    post_worked = (result == 0);
    if ((   pass_expected  &&    post_worked)  ||
        ( ! pass_expected  &&  ! post_worked))
    {
        test_passed = true;
    }
    result = (test_passed ? 0 : 1);
    TestCaseEnd(id, result);
    return result;
}


void CTestNcbiLinkerdCxxApp::Init(void)
{
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "Test 'http_proxy with linkerd and namerd.'");

    SetupArgDescriptions(arg_desc.release());

    SOCK_gethostname(m_Hostname, sizeof(m_Hostname));

    CMapper::Init(m_Mappers);
    CProxy::Init(m_Proxies);
}


int CTestNcbiLinkerdCxxApp::Run(void)
{
    int num_tests = 0, num_errors = 0;

    for (auto proxy : m_Proxies) {
        for (auto mapper : m_Mappers) {
            SelectMapper(mapper.m_Id);
            SelectProxy(proxy.m_Id);
            num_errors += Test(num_tests, proxy.m_PassExpected);
            ++num_tests;
        }
    }

    if (num_errors == 0)
        if (num_tests == 0)
            ERR_POST(Info << "No tests were run!");
        else
            ERR_POST(Info << "All " << num_tests << " tests passed.");
    else
        ERR_POST(Info << num_tests - num_errors << " tests passed; " << num_errors << " failed.");

    return num_tests > 0 ? num_errors : -1;
}


int main(int argc, char* argv[])
{
    int exit_code = 1;

    SetDiagTrace(eDT_Enable);
    SetDiagPostLevel(eDiag_Info);
    SetDiagPostAllFlags((SetDiagPostAllFlags(eDPF_Default) & ~eDPF_All)
                        | eDPF_Severity | eDPF_ErrorID | eDPF_Prefix);
    SetDiagTraceAllFlags(SetDiagPostAllFlags(eDPF_Default));

    try {
        exit_code = CTestNcbiLinkerdCxxApp().AppMain(argc, argv);
    }
    catch (...) {
        // ERR_POST may not work
        cerr << "\n\nunhandled exception" << endl;
    }

    return exit_code;
}
