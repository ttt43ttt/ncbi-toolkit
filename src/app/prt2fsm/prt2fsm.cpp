/*  $Id: prt2fsm.cpp 569299 2018-08-18 16:47:41Z kachalos $
 * =========================================================================
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
 * =========================================================================
 *
 * Author: Sema
 *
 * File Description:
 *   Main() of Multipattern Search Code Generator
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <objects/macro/Search_func.hpp>
#include <objects/macro/Suspect_rule.hpp>
#include <objects/macro/Suspect_rule_set.hpp>
#include <serial/objistr.hpp>
#include <util/multipattern_search.hpp>

USING_NCBI_SCOPE;


class CPrt2FsmApp : public CNcbiApplication
{
public:
    CPrt2FsmApp(void);
    virtual void Init(void);
    virtual int  Run (void);
};


CPrt2FsmApp::CPrt2FsmApp(void) {}


void CPrt2FsmApp::Init(void)
{
    // Prepare command line descriptions
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(), "Suspect Product Rules to FSM");
    arg_desc->AddOptionalKey("i", "InFile", "Input File", CArgDescriptions::eInputFile);
    SetupArgDescriptions(arg_desc.release());  // call CreateArgs
};


string QuoteString(const string& s)
{
    string str = "\"";
    for (size_t i = 0; i < s.length(); i++) {
        switch (s[i]) {
            case '\"':
            case '\'':
            case '\\':
                str += '\\';
                // no break!
            default:
                str += s[i];
        }
    }
    str += "\"";
    return str;
}


int CPrt2FsmApp::Run(void)
{
	const CArgs& args = GetArgs();
    string fname;
    string params;
    if (!args["i"]) {
        ERR_POST("No input file");
        return 1;
    }
    fname = args["i"].AsString();
    LOG_POST("Reading from " + fname);

    objects::CSuspect_rule_set ProductRules;
    auto_ptr<CObjectIStream> in;
    in.reset(CObjectIStream::Open(fname, eSerial_AsnText));
    string header = in->ReadFileHeader();
    in->Read(ObjectInfo(ProductRules), CObjectIStream::eNoFileHeader);

    ifstream file(fname);
    if (!file) {
        ERR_POST("Cannot open file");
        return 1;
    }
	
    CMultipatternSearch FSM;

    vector<string> patterns;
    for (auto rule : ProductRules.Get()) {
        patterns.push_back(rule->GetFind().GetRegex());
    }
    FSM.AddPatterns(patterns);

    cout << "//\n// This code was generated by the prt2fsm application.\n//\n// Command line:\n//   prt2fsm";
    if (!fname.empty()) {
        cout << " -i " << fname;
    }
    cout << "\n//\n";
    cout << "_FSM_RULES = { ";
    std::string line;
    bool first_line = true;
    while (std::getline(file, line)) {
        cout << (first_line ? "\n" : ",\n") << QuoteString(line);
        first_line = false;
    }
    cout << "};\n";
    FSM.GenerateArrayMapData(cout);
    return 0;
}


int main(int argc, const char* argv[])
{
    return CPrt2FsmApp().AppMain(argc, argv);
}
