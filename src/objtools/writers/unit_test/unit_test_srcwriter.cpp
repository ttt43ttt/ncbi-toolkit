/*  $Id: unit_test_srcwriter.cpp 572649 2018-10-17 16:58:53Z ivanov $
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
* Author: Justin Foley, NCBI
*
* File Description:
*   src writer unit test.
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbi_system.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbifile.hpp>
#include <connect/ncbi_core_cxx.hpp>
#include <serial/serial.hpp>
#include <serial/objistr.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <dbapi/driver/drivers.hpp>

#include <objtools/writers/writer_exception.hpp>
#include <objtools/readers/message_listener.hpp>
#include <objtools/writers/src_writer.hpp>
#include "error_logger.hpp"
#include <objmgr/bioseq_ci.hpp>


#include <cstdio>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>


USING_NCBI_SCOPE;
USING_SCOPE(objects);


//  ============================================================================
//  Customization data:
const string extInput("input");
const string extOutput("output");
const string extErrors("errors");
const string extKeep("new");
const string dirTestFiles("srcwriter_test_cases");
//  ============================================================================

struct STestInfo {
    CFile mInFile;
    CFile mOutFile;
    CFile mErrorFile;
    string mObjType;
};

typedef string TTestName;
typedef map<TTestName, STestInfo> TTestNameToInfoMap;


class CTestNameToInfoMapLoader {
public:
    CTestNameToInfoMapLoader(
        TTestNameToInfoMap * pTestNameToInfoMap,
        const string& extInput,
        const string& extOutput,
        const string& extErrors)
        : m_pTestNameToInfoMap(pTestNameToInfoMap),
          mExtInput(extInput),
          mExtOutput(extOutput),
          mExtErrors(extErrors)
    { }


    void operator()( const CDirEntry & dirEntry ) {

        if( ! dirEntry.IsFile() ) {
            return;
        }
        CFile file(dirEntry);
        string name = file.GetName();
        if (NStr::EndsWith(name, ".txt")  ||  NStr::StartsWith(name, ".")) {
            return;
        }
        if (NStr::EndsWith(name, extKeep)) {
            return;
        }

        // extract info from the file name
        const string sFileName = file.GetName();
        list<string> fileNamePieces;
        NStr::Split( sFileName, ".", fileNamePieces, NStr::fSplit_MergeDelimiters | NStr::fSplit_Truncate);
        BOOST_REQUIRE(fileNamePieces.size() == 3);

        

        string sTestName = fileNamePieces.front();
        BOOST_REQUIRE(!sTestName.empty());
        string sObjType = *(next(fileNamePieces.begin()));
        BOOST_REQUIRE(!sObjType.empty());
        string sFileType = fileNamePieces.back();
        BOOST_REQUIRE(!sFileType.empty());
       
            
        STestInfo & test_info_to_load =
            (*m_pTestNameToInfoMap)[sTestName];
   

        // Figure out which test to perform
        if (sObjType == "seqentry" || sObjType == "ids" || sObjType == "srcchk") {
            test_info_to_load.mObjType = sObjType;
        } else {
            BOOST_FAIL("Unknown object type " << sObjType << ".");
        }
        // figure out what type of file we have and set appropriately
        if (sFileType == mExtInput) {
            BOOST_REQUIRE( test_info_to_load.mInFile.GetPath().empty() );
            test_info_to_load.mInFile = file;
        } 
        else if (sFileType == mExtOutput) {
            BOOST_REQUIRE( test_info_to_load.mOutFile.GetPath().empty() );
            test_info_to_load.mOutFile = file;
        } 
        else if (sFileType == mExtErrors) {
            BOOST_REQUIRE( test_info_to_load.mErrorFile.GetPath().empty() );
            test_info_to_load.mErrorFile = file;
        } 

        else {
            BOOST_FAIL("Unknown file type " << sFileName << ".");
        }
    }

private:
    // raw pointer because we do NOT own this
    TTestNameToInfoMap * m_pTestNameToInfoMap;
    string mExtInput;
    string mExtOutput;
    string mExtErrors;
};


//  ----------------------------------------------------------------------------
void sUpdateCase(CDir& test_cases_dir, const string& test_name)
//  ----------------------------------------------------------------------------
{   
    string input = CDir::ConcatPath( test_cases_dir.GetPath(), test_name + "." + extInput);
    string output = CDir::ConcatPath( test_cases_dir.GetPath(), test_name + "." + extOutput);
    string errors = CDir::ConcatPath( test_cases_dir.GetPath(), test_name + "." + extErrors);
    if (!CFile(input).Exists()) {
         BOOST_FAIL("input file " << input << " does not exist.");
    }
    string test_base, test_type;
    NStr::SplitInTwo(test_name, ".", test_base, test_type);
    cerr << "Creating new test case from " << input << " ..." << endl;

    CErrorLogger logger(errors);

    //get a scope
    CRef<CObjectManager> pObjMngr = CObjectManager::GetInstance();
    CGBDataLoader::RegisterInObjectManager(*pObjMngr).GetLoader()->SetAlwaysLoadExternal(false);
    CRef<CScope> pScope(new CScope(*pObjMngr));
    pScope->AddDefaults();

    //get a writer object
    CNcbiIfstream ifstr(input.c_str());
    unique_ptr<CObjectIStream> pI(CObjectIStream::Open(eSerial_AsnText, ifstr));

    CNcbiOfstream ofstr(output.c_str());

    CSrcWriter writer(0);
    writer.SetDelimiter("\t");

    if (test_type == "seqentry") {
        CRef<CSeq_entry> pEntry(new CSeq_entry);
        *pI >> *pEntry;
        writer.WriteSeqEntry(*pEntry, *pScope, ofstr);
    } else if (test_type == "srcchk") {
        vector<pair<string, CBioseq_Handle> > vecIdBsh;
      
        while (!pI->EndOfData()) {
            CRef<CSeq_entry> pEntry(new CSeq_entry);
            *pI >> *pEntry;
             CSeq_entry_Handle handle = pScope->AddTopLevelSeqEntry(*pEntry);
             for (CBioseq_CI bci(handle); bci; ++bci) {
                 vecIdBsh.push_back(make_pair("",*bci));
             }
        }
        writer.WriteBioseqHandles(vecIdBsh, CSrcWriter::sAllSrcCheckFields, ofstr);
    }

    ofstr.flush();
    ifstr.close();
    ofstr.close();

    cerr << "    Produced new srcwriter file " << output << "." << endl;
    cerr << " ... Done." << endl;
}


//  ----------------------------------------------------------------------------
void sUpdateAll(CDir& test_cases_dir)
//  ----------------------------------------------------------------------------
{
    const vector<string> kEmptyStringVec;
    TTestNameToInfoMap testNameToInfoMap;
    CTestNameToInfoMapLoader testInfoLoader(
        &testNameToInfoMap, extInput, extOutput, extErrors);
    FindFilesInDir(
        test_cases_dir,
        kEmptyStringVec,
        kEmptyStringVec,
        testInfoLoader,
        fFF_Default | fFF_Recursive );

    ITERATE(TTestNameToInfoMap, name_to_info_it, testNameToInfoMap) {
        const string & sName = name_to_info_it->first + 
            "." + name_to_info_it->second.mObjType;
        sUpdateCase(test_cases_dir, sName);
    }
}


//  ----------------------------------------------------------------------------
void sRunTest(const string &sTestName, const STestInfo & testInfo, bool keep)
//  ----------------------------------------------------------------------------
{
    cerr << "Testing " << testInfo.mInFile.GetName() << " against " <<
        testInfo.mOutFile.GetName() << " and " << 
        testInfo.mErrorFile.GetName() << endl;

    string logName = CDirEntry::GetTmpName();
    CErrorLogger logger(logName);
  

    // get a scope
    CRef<CObjectManager> pObjMngr = CObjectManager::GetInstance();
    CGBDataLoader::RegisterInObjectManager(*pObjMngr).GetLoader()->SetAlwaysLoadExternal(false);
    CRef<CScope> pScope(new CScope(*pObjMngr));
    pScope->AddDefaults();
   
    // get a writer object
    CSrcWriter writer(0);
    writer.SetDelimiter("\t");
   
    
    CNcbiIfstream ifstr(testInfo.mInFile.GetPath().c_str());
    unique_ptr<CObjectIStream> pI(CObjectIStream::Open(eSerial_AsnText, ifstr));
    // create a temporary result file
    string resultName = CDirEntry::GetTmpName();
    CNcbiOfstream ofstr(resultName.c_str());

    if (testInfo.mObjType == "seqentry") {
        CRef<CSeq_entry> pEntry(new CSeq_entry);
        *pI >> *pEntry;
        writer.WriteSeqEntry(*pEntry, *pScope, ofstr);
    } else if (testInfo.mObjType == "srcchk") {
        vector<pair<string, CBioseq_Handle> > vecIdBsh;
        while (!pI->EndOfData()) {
            CRef<CSeq_entry> pEntry(new CSeq_entry);
            *pI >> *pEntry;
            CSeq_entry_Handle handle = pScope->AddTopLevelSeqEntry(*pEntry);
            for (CBioseq_CI bci(handle); bci; ++bci) {
                vecIdBsh.push_back(make_pair("",*bci));
            }
        }
        writer.WriteBioseqHandles(vecIdBsh, CSrcWriter::sAllSrcCheckFields, ofstr);
    }

    ofstr.flush();
    ifstr.close();
    ofstr.close();


    bool success = testInfo.mOutFile.CompareTextContents(resultName, CFile::eIgnoreWs);
    if (!success) {
        CDirEntry deResult = CDirEntry(resultName);
        if (keep) {
            deResult.Copy(testInfo.mOutFile.GetPath() + "." + extKeep,
                CDirEntry::fCF_Overwrite);
        }
        deResult.Remove();
        CDirEntry(logName).Remove();
        BOOST_ERROR("Error: " << sTestName << " failed due to post processing diffs.");
    }
    // remove the temporay result file
    CDirEntry(resultName).Remove();

    success = testInfo.mErrorFile.CompareTextContents(logName, CFile::eIgnoreWs);
    CDirEntry deErrors = CDirEntry(logName);
    if (!success && keep) {
        deErrors.Copy(testInfo.mErrorFile.GetPath() + "." + extKeep);
    }
    deErrors.Remove();
    if (!success) {
        BOOST_ERROR("Error: " << sTestName << " failed due to error handling diffs.");
    }
};

NCBITEST_AUTO_INIT()
{
}

NCBITEST_INIT_CMDLINE(arg_descrs)
{
    arg_descrs->AddDefaultKey("test-dir", "TEST_FILE_DIRECTORY",
        "Set the root directory under which all test files can be found.",
        CArgDescriptions::eDirectory,
        dirTestFiles );
    arg_descrs->AddDefaultKey("update-case", "UPDATE_CASE",
        "Produce .asn and .error files from given name for new or updated test case.",
        CArgDescriptions::eString,
        "" );
    arg_descrs->AddFlag("update-all",
        "Update all test cases to current reader code (dangerous).",
        true );
    arg_descrs->AddFlag("keep-diffs",
        "Keep output files that are different from the expected.",
        true );
}

NCBITEST_AUTO_FINI()
{
}


BOOST_AUTO_TEST_CASE(RunTests)
{
    const CArgs& args = CNcbiApplication::Instance()->GetArgs();

    CDir test_cases_dir( args["test-dir"].AsDirectory() );
    BOOST_REQUIRE_MESSAGE( test_cases_dir.IsDir(), 
        "Cannot find dir: " << test_cases_dir.GetPath() );

    bool update_all = args["update-all"].AsBoolean();
    if (update_all) {
        sUpdateAll(test_cases_dir);
        return;
    }

    string update_case = args["update-case"].AsString();
    if (!update_case.empty()) {
        sUpdateCase(test_cases_dir, update_case);
        return;
    }

    const vector<string> kEmptyStringVec;
    TTestNameToInfoMap testNameToInfoMap;
    CTestNameToInfoMapLoader testInfoLoader(
        &testNameToInfoMap, extInput, extOutput, extErrors);
    FindFilesInDir(
        test_cases_dir,
        kEmptyStringVec,
        kEmptyStringVec,
        testInfoLoader,
        fFF_Default | fFF_Recursive );

    ITERATE(TTestNameToInfoMap, name_to_info_it, testNameToInfoMap) {
        const string &sName = name_to_info_it->first;
        const STestInfo &testInfo = name_to_info_it->second;
        cout <<  "Verifying: " << sName << endl;
        BOOST_REQUIRE_MESSAGE( testInfo.mInFile.Exists(),
            extInput + " file does not exist: " << testInfo.mInFile.GetPath() );
        BOOST_REQUIRE_MESSAGE( testInfo.mOutFile.Exists(),
            extOutput + " file does not exist: " << testInfo.mOutFile.GetPath() );
        BOOST_REQUIRE_MESSAGE( testInfo.mErrorFile.Exists(),
            extErrors + " file does not exist: " << testInfo.mErrorFile.GetPath() );
    }
    ITERATE(TTestNameToInfoMap, name_to_info_it, testNameToInfoMap) {
        const string & sName = name_to_info_it->first;
        const STestInfo & testInfo = name_to_info_it->second;

        cout << "Running test: " << sName << endl;

        BOOST_CHECK_NO_THROW(sRunTest(sName, testInfo, args["keep-diffs"]));
    }
}


