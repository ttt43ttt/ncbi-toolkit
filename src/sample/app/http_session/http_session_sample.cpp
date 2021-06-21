/*  $Id: http_session_sample.cpp 559548 2018-03-12 18:34:15Z grichenk $
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
 * Author:  Aleksey Grichenko
 *
 * File Description:
 *   Plain example of CHTTPSession.
 *
 *   USAGE:  http_session_sample
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbistr.hpp>
#include <connect/ncbi_http_session.hpp>

// This header is not necessary for real application,
// but required for automatic testsuite.
#include <common/test_assert.h>  /* This header must go last */


using namespace ncbi;


/////////////////////////////////////////////////////////////////////////////
//  CHttpSessionApplication::
//

class CHttpSessionApp : public CNcbiApplication
{
public:
    CHttpSessionApp(void);
    virtual void Init(void);
    virtual int Run(void);

    bool PrintResponse(const CHttpSession* session,
                       const CHttpResponse& response);

private:
    void SetupRequest(CHttpRequest& request);

    bool m_PrintHeaders;
    bool m_PrintCookies;
    bool m_PrintBody;
    int m_Errors;
};


CHttpSessionApp::CHttpSessionApp(void)
    : m_PrintHeaders(true),
      m_PrintCookies(true),
      m_PrintBody(true),
      m_Errors(0)
{
}


void CHttpSessionApp::SetupRequest(CHttpRequest& request)
{
    const CArgs& args = GetArgs();
    if ( args["timeout"] ) {
        request.SetTimeout(CTimeout(args["timeout"].AsDouble()));
    }
    if ( args["retries"] ) {
        request.SetRetries((unsigned short)args["retries"].AsInteger());
    }
    if ( args["deadline"] ) {
        request.SetDeadline(CTimeout(args["deadline"].AsDouble()));
    }
    if ( args["retry-processing"] ) {
        request.SetRetryProcessing(args["retry-processing"].AsBoolean() ? eOn : eOff);
    }
}


void CHttpSessionApp::Init()
{
    CNcbiApplication::Init();

    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "HTTP session sample application");

    arg_desc->AddFlag("http-11", "Use HTTP/1.1 protocol", true);
    arg_desc->AddFlag("print-headers", "Print HTTP response headers", true);
    arg_desc->AddFlag("print-cookies", "Print HTTP cookies", true);
    arg_desc->AddFlag("print-body", "Print HTTP response body", true);

    // Simple requests
    arg_desc->AddOptionalKey("head", "url", "URL to load using HEAD request",
        CArgDescriptions::eString,
        CArgDescriptions::fAllowMultiple);
    arg_desc->AddOptionalKey("get", "url", "URL to load using GET request",
        CArgDescriptions::eString,
        CArgDescriptions::fAllowMultiple);
    arg_desc->AddOptionalKey("post", "url", "URL to POST data to (the data is read from STDIN)",
        CArgDescriptions::eString);

    // Requests to specific url/service
    arg_desc->AddOptionalKey("method", "method", "HTTP request method: HEAD/GET/POST",
        CArgDescriptions::eString);
    CArgAllow_Strings allow_methods;
    allow_methods.Allow("HEAD");
    allow_methods.Allow("GET");
    allow_methods.Allow("POST");
    arg_desc->SetConstraint("method", allow_methods);
    arg_desc->SetDependency("method", CArgDescriptions::eExcludes, "head");
    arg_desc->SetDependency("method", CArgDescriptions::eExcludes, "get");
    arg_desc->SetDependency("method", CArgDescriptions::eExcludes, "post");

    arg_desc->AddOptionalKey("url", "url", "URL to access using the specified method.",
        CArgDescriptions::eString);
    arg_desc->SetDependency("url", CArgDescriptions::eRequires, "method");

    arg_desc->AddOptionalKey("service-name", "name", "Named service to access using the specified method.",
        CArgDescriptions::eString);
    arg_desc->SetDependency("service-name", CArgDescriptions::eRequires, "method");
    arg_desc->SetDependency("service-name", CArgDescriptions::eExcludes, "url");

    arg_desc->AddOptionalKey("service-path", "path", "Path to be appended to the service name.",
        CArgDescriptions::eString);
    arg_desc->SetDependency("service-path", CArgDescriptions::eRequires, "service-name");

    arg_desc->AddOptionalKey("service-args", "name", "Named service arguments.",
        CArgDescriptions::eString);
    arg_desc->SetDependency("service-args", CArgDescriptions::eRequires, "service-name");

    // Connection parameters
    arg_desc->AddOptionalKey("timeout", "double", "Timeout in seconds",
        CArgDescriptions::eDouble);
    arg_desc->AddOptionalKey("retries", "int", "Number of retries",
        CArgDescriptions::eInteger);
    arg_desc->AddOptionalKey("deadline", "double", "Deadline for request total processing, in seconds",
        CArgDescriptions::eDouble);
    arg_desc->AddOptionalKey("retry-processing", "bool", "Whether to wait for actual response",
        CArgDescriptions::eBoolean);

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}


class CTestDataProvider : public CFormDataProvider_Base
{
public:
    CTestDataProvider(void) {}
    virtual string GetContentType(void) const { return "text/plain"; }
    virtual string GetFileName(void) const { return "test.txt"; }
    virtual void WriteData(CNcbiOstream& out) const
    {
        out << "Provider test";
    }
};


int CHttpSessionApp::Run(void)
{
    const CArgs& args = GetArgs();
    m_PrintHeaders = args["print-headers"];
    m_PrintCookies = args["print-cookies"];
    m_PrintBody = args["print-body"];
    bool http11 = args["http-11"];

    CHttpSession session;
    if ( http11 ) {
        session.SetProtocol(CHttpSession::eHTTP_11);
    }

    bool skip_defaults = false;
    if (args["method"]) {
        CHttpSession::ERequestMethod method = CHttpSession::eHead;
        if (args["method"].AsString() == "GET") method = CHttpSession::eGet;
        else if (args["method"].AsString() == "POST") method = CHttpSession::ePost;
        CUrl url;
        if (args["url"]) {
            url.SetUrl(args["url"].AsString());
        }
        else if (args["service-name"]) {
            url.SetService(args["service-name"].AsString());
            if (args["service-path"]) {
                url.SetPath(args["service-path"].AsString());
            }
            if (args["service-args"]) {
                url.GetArgs().SetQueryString(args["service-args"].AsString());
            }
        }
        else {
            cout << "Missing 'url' or 'service-name' argument for " << args["method"] << " request." << endl;
            return 0;
        }
        cout << args["method"].AsString() << " " << url.ComposeUrl(CUrlArgs::eAmp_Char) << endl;
        CHttpRequest req = session.NewRequest(url, method);
        SetupRequest(req);
        if (method == CHttpSession::ePost) {
            NcbiStreamCopy(req.ContentStream(), cin);
        }
        PrintResponse(&session, req.Execute());
        return 0;
    }
    if ( args["head"] ) {
        skip_defaults = true;
        const CArgValue::TStringArray& urls = args["head"].GetStringList();
        ITERATE(CArgValue::TStringArray, it, urls) {
            cout << "HEAD " << *it << endl;
            CHttpRequest req = session.NewRequest(*it, CHttpSession::eHead);
            SetupRequest(req);
            if (!PrintResponse(&session, req.Execute())) m_Errors++;
            cout << "-------------------------------------" << endl << endl;
        }
    }
    if ( args["get"] ) {
        skip_defaults = true;
        const CArgValue::TStringArray& urls = args["get"].GetStringList();
        ITERATE(CArgValue::TStringArray, it, urls) {
            cout << "GET " << *it << endl;
            CHttpRequest req = session.NewRequest(*it);
            SetupRequest(req);
            if (!PrintResponse(&session, req.Execute())) m_Errors++;
            cout << "-------------------------------------" << endl << endl;
        }
    }
    if ( args["post"] ) {
        skip_defaults = true;
        string url = args["post"].AsString();
        cout << "POST " << url << endl;
        CHttpRequest req = session.NewRequest(url, CHttpSession::ePost);
        SetupRequest(req);
        NcbiStreamCopy(req.ContentStream(), cin);
        if (!PrintResponse(&session, req.Execute())) m_Errors++;
        cout << "-------------------------------------" << endl << endl;
    }

    if ( skip_defaults ) {
        return 0;
    }

    const string sample_url = "https://dev.ncbi.nlm.nih.gov/Service/sample/cgi_sample.cgi";
    const string bad_url = "https://dev.ncbi.nlm.nih.gov/Service/sample/404";
    CUrl url(sample_url);

    {{
        // HEAD request
        cout << "HEAD " << sample_url << endl;
        // Requests can be initialized with either a string or a CUrl
        CHttpRequest req = session.NewRequest(url, CHttpSession::eHead);
        SetupRequest(req);
        CHttpResponse resp = req.Execute();
        if (!PrintResponse(&session, resp)) m_Errors++;
        cout << "-------------------------------------" << endl << endl;
    }}

    {{
        // Simple GET request
        cout << "GET (no args) " << sample_url << endl;
        CHttpRequest req = session.NewRequest(url);
        SetupRequest(req);
        if (!PrintResponse(&session, req.Execute())) m_Errors++;
        cout << "-------------------------------------" << endl << endl;
    }}

    {{
        CUrl url_with_args(sample_url);
        // GET request with arguments
        cout << "GET (with args) " << sample_url << endl;
        url_with_args.GetArgs().SetValue("message", "GET data");
        CHttpRequest req = session.NewRequest(url_with_args);
        SetupRequest(req);
        if (!PrintResponse(&session, req.Execute())) m_Errors++;
        cout << "-------------------------------------" << endl << endl;
    }}

    {{
        // POST request with form data
        cout << "POST (form data) " << sample_url << endl;
        CHttpRequest req = session.NewRequest(url, CHttpSession::ePost);
        SetupRequest(req);
        CHttpFormData& data = req.FormData();
        data.AddEntry("message", "POST data");
        if (!PrintResponse(&session, req.Execute())) m_Errors++;
        cout << "-------------------------------------" << endl << endl;
    }}

    {{
        // POST using a provider
        cout << "POST (provider) " << sample_url << endl;
        CHttpRequest req = session.NewRequest(url, CHttpSession::ePost);
        SetupRequest(req);
        CHttpFormData& data = req.FormData();
        data.AddProvider("message", new CTestDataProvider);
        if (!PrintResponse(&session, req.Execute())) m_Errors++;
        cout << "-------------------------------------" << endl << endl;
    }}

    {{
        // POST some data manually
        cout << "POST (manual) " << sample_url << endl;
        CHttpRequest req = session.NewRequest(url, CHttpSession::ePost);
        SetupRequest(req);
        req.Headers().SetValue(CHttpHeaders::eContentType, "application/x-www-form-urlencoded");
        CNcbiOstream& out = req.ContentStream();
        out << "message=POST manual data";
        if (!PrintResponse(&session, req.Execute())) m_Errors++;
        cout << "-------------------------------------" << endl << endl;
    }}

    {{
        // POST using shortcut
        cout << "POST (shortcut) " << sample_url << endl;
        CHttpResponse response = g_HttpPost(url, "message=POST%20shortcut%20data");
        if (!PrintResponse(0, response)) m_Errors++;
        cout << "-------------------------------------" << endl << endl;
    }}

    {{
        // PUT using a provider
        cout << "PUT (provider) " << sample_url << endl;
        CHttpRequest req = session.NewRequest(url, CHttpSession::ePut);
        SetupRequest(req);
        CHttpFormData& data = req.FormData();
        data.AddProvider("message", new CTestDataProvider);
        if (!PrintResponse(&session, req.Execute())) m_Errors++;
        cout << "-------------------------------------" << endl << endl;
    }}

    {{
        // Bad GET request
        cout << "GET (404) " << bad_url << endl;
        CHttpRequest req = session.NewRequest(bad_url);
        SetupRequest(req);
        PrintResponse(&session, req.Execute());
        cout << "-------------------------------------" << endl << endl;
    }}

    {{
        // Service request
        cout << "GET service: test" << endl;
        CHttpRequest req = session.NewRequest(CUrl("test"));
        if (!PrintResponse(&session, req.Execute())) m_Errors++;
        cout << "-------------------------------------" << endl << endl;
    }}

    {{
        // Service request
        cout << "GET service (shortcut): test" << endl;
        CHttpResponse response = g_HttpGet(CUrl("test"));
        if (!PrintResponse(0, response)) m_Errors++;
        cout << "-------------------------------------" << endl << endl;
    }}

    if (m_Errors > 0) {
        cout << m_Errors << " requests failed." << endl;
        return 1;
    }
    return 0;
}



bool CHttpSessionApp::PrintResponse(const CHttpSession* session,
                                    const CHttpResponse& response)
{
    cout << "Status: " << response.GetStatusCode() << " "
        << response.GetStatusText() << endl;
    if ( m_PrintHeaders ) {
        list<string> headers;
        NStr::Split(response.Headers().GetHttpHeader(), "\n\r", headers,
            NStr::fSplit_MergeDelimiters | NStr::fSplit_Truncate);
        cout << "--- Headers ---" << endl;
        ITERATE(list<string>, it, headers) {
            cout << *it << endl;
        }
    }

    if (m_PrintCookies  &&  session) {
        cout << "--- Cookies ---" << endl;
        ITERATE(CHttpCookies, it, session->Cookies()) {
            cout << it->AsString(CHttpCookie::eHTTPResponse) << endl;
        }
    }

    if ( m_PrintBody ) {
        cout << "--- Body ---" << endl;
        if ( response.CanGetContentStream() ) {
            CNcbiIstream& in = response.ContentStream();
            if ( in.good() ) {
                NcbiStreamCopy(cout, in);
            }
        }
        else {
            CNcbiIstream& in = response.ErrorStream();
            if ( in.good() ) {
                NcbiStreamCopy(cout, in);
            }
        }
    }
    return response.GetStatusCode() == 200;
}


/////////////////////////////////////////////////////////////////////////////
//  MAIN
//

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    return CHttpSessionApp().AppMain(argc, argv);
}
