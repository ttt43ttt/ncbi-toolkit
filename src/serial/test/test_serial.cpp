/*  $Id: test_serial.cpp 555021 2018-01-11 16:45:57Z gouriano $
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
 * Author:  Eugene Vasilchenko
 *
 * File Description:
 *   .......
 *
 */

#include <ncbi_pch.hpp>
#include "test_serial.hpp"
#ifndef HAVE_NCBI_C

/////////////////////////////////////////////////////////////////////////////
// Test ASN serialization

BOOST_AUTO_TEST_CASE(s_TestAsnSerialization)
{
    string text_in("webenv.ent"), text_out("webenv.ento");
    string  bin_in("webenv.bin"),  bin_out("webenv.bino");
    {
        CRef<CWeb_Env> env(new CWeb_Env);
        {
            // read ASN text
            // specify input as a file name
            unique_ptr<CObjectIStream> in(
                CObjectIStream::Open(text_in,eSerial_AsnText));
            *in >> *env;
        }
        {
            // write ASN binary
            // specify output as a file name
            unique_ptr<CObjectOStream> out(
                CObjectOStream::Open(bin_out,eSerial_AsnBinary));
            *out << *env;
        }
        BOOST_CHECK( CFile( bin_in).Compare( bin_out) );
        {
            // write ASN binary
            // specify output as a stream, use MSerial manipulator
            CNcbiOfstream ofs(bin_out.c_str(),
                IOS_BASE::out | IOS_BASE::trunc | IOS_BASE::binary);
            ofs << MSerial_AsnBinary << *env;
        }
        BOOST_CHECK( CFile( bin_in).Compare( bin_out) );
        {
            // write ASN binary
            // specify output as a file name, use Write method
            unique_ptr<CObjectOStream> out(
                CObjectOStream::Open(bin_out,eSerial_AsnBinary));
            out->Write( env, env->GetThisTypeInfo());
        }
        BOOST_CHECK( CFile( bin_in).Compare( bin_out) );
    }
    {
        CRef<CWeb_Env> env(new CWeb_Env);
        {
            // read ASN binary
            // specify input as a file name
            unique_ptr<CObjectIStream> in(
                CObjectIStream::Open(bin_in,eSerial_AsnBinary));
            *in >> *env;
        }
        {
            // write ASN text
            // specify output as a file name
            unique_ptr<CObjectOStream> out(
                CObjectOStream::Open(text_out,eSerial_AsnText));
            *out << *env;
        }
        BOOST_CHECK( CFile(text_in).CompareTextContents(text_out, CFile::eIgnoreEol) );
        {
            // write ASN binary
            // specify output as a stream, use MSerial manipulator
            CNcbiOfstream ofs(bin_out.c_str(),
                IOS_BASE::out | IOS_BASE::trunc | IOS_BASE::binary);
            ofs << MSerial_AsnBinary << *env;
        }
        BOOST_CHECK( CFile( bin_in).Compare( bin_out) );
    }
}
#endif

/////////////////////////////////////////////////////////////////////////////
// TestObjectHooks

BOOST_AUTO_TEST_CASE(s_TestObjectHooks)
{
    CTestSerialObject obj;
    CTestSerialObject2 write1;
#ifdef HAVE_NCBI_C
    WebEnv* env = 0;
#else
    CRef<CWeb_Env> env(new CWeb_Env);
#endif    
    InitializeTestObject( env, obj, write1);

    string data;
    const char* buf = 0;
    {
        // write hook
        CNcbiOstrstream ostrs;
        {
            unique_ptr<CObjectOStream> os(
                CObjectOStream::Open(eSerial_AsnText, ostrs));
            // set local write hook
            // use CObjectHookGuard
            CObjectHookGuard<CTestSerialObject> w_hook(
                *(new CWriteSerialObjectHook(&obj)), &(*os));
            *os << obj;
        }
		data = CNcbiOstrstreamToString(ostrs);
		buf = data.c_str();
    }
    {
        // read hook
        // use CObjectHookGuard
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CTestSerialObject obj_copy;
        CObjectHookGuard<CTestSerialObject> r_hook(
            *(new CReadSerialObjectHook(&obj_copy)), &(*is));
        *is >> obj_copy;
        // Can not use SerialEquals<> with C-objects
#ifndef HAVE_NCBI_C
        BOOST_CHECK(SerialEquals<CTestSerialObject>(obj, obj_copy));
#endif
    }

    {
        // write hook
        CNcbiOstrstream ostrs;
        {
            unique_ptr<CObjectOStream> os(
                CObjectOStream::Open(eSerial_AsnText, ostrs));
            // set local write hook
            CObjectTypeInfo type = CType<CTestSerialObject>();
            type.SetLocalWriteHook(*os, new CWriteSerialObjectHook(&obj));
            *os << obj;
        }
		data = CNcbiOstrstreamToString(ostrs);
		buf = data.c_str();
    }
    {
        // read hook
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CTestSerialObject obj_copy;
        // set local read hook
        CObjectTypeInfo type = CType<CTestSerialObject>();
        type.SetLocalReadHook(*is, new CReadSerialObjectHook(&obj_copy));
        *is >> obj_copy;
#ifndef HAVE_NCBI_C
        BOOST_CHECK(SerialEquals<CTestSerialObject>(obj, obj_copy));
#endif
    }
#ifdef HAVE_NCBI_C
    WebEnvFree(env);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// TestObjectStackPathHooks

BOOST_AUTO_TEST_CASE(s_TestObjectStackPathHooks)
{
    CTestSerialObject obj;
    CTestSerialObject2 write1;
#ifdef HAVE_NCBI_C
    WebEnv* env = 0;
#else
    CRef<CWeb_Env> env(new CWeb_Env);
#endif    
    InitializeTestObject( env, obj, write1);

    string data;
    const char* buf = 0;

    TTypeInfo type_info = CTestSerialObject::GetTypeInfo();
    {
        // write, stack path hook
        CNcbiOstrstream ostrs;
        {
            unique_ptr<CObjectOStream> os(
                CObjectOStream::Open(eSerial_AsnText, ostrs));
            CObjectTypeInfo(type_info).SetPathWriteHook(
                os.get(), "CTestSerialObject.*", new CWriteSerialObjectHook(&obj));
            *os << obj;
//            CObjectTypeInfo(type_info).SetPathWriteHook(
//                os.get(), "CTestSerialObject.*", NULL);
        }
		data = CNcbiOstrstreamToString(ostrs);
		buf = data.c_str();
    }
    {
        // read, stack path hook
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CTestSerialObject obj_copy;
        CObjectTypeInfo(type_info).SetPathReadHook(
            is.get(), "CTestSerialObject.*", new CReadSerialObjectHook(&obj_copy));
        *is >> obj_copy;
//        CObjectTypeInfo(type_info).SetPathReadHook(
//            is.get(), "CTestSerialObject.*", NULL);
#ifndef HAVE_NCBI_C
        BOOST_CHECK(SerialEquals<CTestSerialObject>(obj, obj_copy));
#endif
    }
#ifdef HAVE_NCBI_C
    WebEnvFree(env);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// TestSerialFilter

#ifndef HAVE_NCBI_C
BOOST_AUTO_TEST_CASE(s_TestSerialFilter)
{
    CTestSerialObject obj;
    CTestSerialObject2 write1;
#ifdef HAVE_NCBI_C
    WebEnv* env = 0;
#else
    CRef<CWeb_Env> env(new CWeb_Env);
#endif    
    InitializeTestObject( env, obj, write1);

    string data;
    const char* buf = 0;
    {
        // prepare data to read later
        CNcbiOstrstream ostrs;
        {
            unique_ptr<CObjectOStream> os(
                CObjectOStream::Open(eSerial_AsnText, ostrs));
            CObjectHookGuard<CTestSerialObject> w_hook(
                *(new CWriteSerialObjectHook(&obj)), &(*os));
            *os << obj;
        }
		data = CNcbiOstrstreamToString(ostrs);
		buf = data.c_str();
    }
    {
        // find serial objects of a specific type
        // process them in CTestSerialObjectHook::Process()
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        Serial_FilterObjects<CTestSerialObject>(*is, new CTestSerialObjectHook);
    }
    {
        // find non-serial objects, here - strings
        // process them in CStringObjectHook::Process()
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        Serial_FilterStdObjects<CTestSerialObject>(*is, new CStringObjectHook);
    }
#if defined(NCBI_THREADS)
    {
        // find serial objects of a specific type
        // process them right here
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CObjectIStreamIterator<CTestSerialObject> i(*is);
        for ( ; i.IsValid(); ++i) {
            const CTestSerialObject& obj = *i;
            LOG_POST("CTestSerialObject @ " << NStr::PtrToString(&obj));
            cout << MSerial_AsnText << obj << endl;
        }
    }
    {
        // find serial objects of a specific type
        // process them right here
        CNcbiIstrstream istrs(buf);
        for (const CTestSerialObject& obj : CObjectIStreamIterator<CTestSerialObject>(
                *CObjectIStream::Open(eSerial_AsnText, istrs), eTakeOwnership)) {
            LOG_POST("CTestSerialObject @ " << NStr::PtrToString(&obj));
            cout << MSerial_AsnText << obj << endl;
        }
    }
    {
        // find serial objects of a specific type
        // process them right here
        CNcbiIstrstream istrs(buf);
        CObjectIStreamIterator<CTestSerialObject>::CParams params;
        params.FilterByMember(1,
            [](const CObjectIStream& istr, CTestSerialObject& obj,
            TMemberIndex mem_index, CObjectInfo* mem, void* extra)->bool {
                return mem != nullptr;
            });
        for (const CTestSerialObject& obj : CObjectIStreamIterator<CTestSerialObject>(
                *CObjectIStream::Open(eSerial_AsnText, istrs), eTakeOwnership, params)) {
            LOG_POST("CTestSerialObject @ " << NStr::PtrToString(&obj));
            cout << MSerial_AsnText << obj << endl;
        }
    }
    // with exception transfer
    {
        CNcbiIstrstream istrs(buf);
        CObjectIStreamIterator<CTestSerialObject>::CParams params;
        params.FilterByMember(1,
            [](const CObjectIStream& istr, CTestSerialObject& obj,
            TMemberIndex mem_index, CObjectInfo* mem, void* extra)->bool {
                return mem != nullptr;
            });
        // generates NCBI exception (wrong format)
        bool gotit = false;
        try {
            for (const CTestSerialObject& obj : CObjectIStreamIterator<CTestSerialObject>(
                    *CObjectIStream::Open(eSerial_AsnBinary, istrs), eTakeOwnership, params)) {
                cout << MSerial_AsnText << obj << endl;
            }
        } catch (CSerialException& e) {
            gotit = e.GetErrCode() == CSerialException::eFormatError;
        } catch (...) {
        }
        BOOST_CHECK(gotit);
    }
    // with exception transfer
    {
        CNcbiIstrstream istrs(buf);
        CObjectIStreamIterator<CTestSerialObject>::CParams params;
        params.FilterByMember(1,
            [](const CObjectIStream& istr, CTestSerialObject& obj,
            TMemberIndex mem_index, CObjectInfo* mem, void* extra)->bool {
                throw runtime_error("filter");
                return mem != nullptr;
            });
        // generates runtime_error in filter function
        bool gotit = false;
        try {
            for (const CTestSerialObject& obj : CObjectIStreamIterator<CTestSerialObject>(
                    *CObjectIStream::Open(eSerial_AsnText, istrs), eTakeOwnership, params)) {
                cout << MSerial_AsnText << obj << endl;
            }
        } catch (CException&) {
        } catch (exception&) {
            gotit = true;
        }
        BOOST_CHECK(gotit);
    }
    {
        // find serial objects of a specific type
        // process them right here
        CNcbiIstrstream istrs(buf);
        CObjectIStreamIterator<CTestSerialObject>::CParams params;
        params.FilterByMember(2,
            [](const CObjectIStream& istr, CTestSerialObject& obj,
            TMemberIndex mem_index, CObjectInfo* mem, void* extra)->bool {
                return mem != nullptr && mem->GetPrimitiveValueBool();
            });
        for (const CTestSerialObject& obj : CObjectIStreamIterator<CTestSerialObject, CTestSerialObject>(
                *CObjectIStream::Open(eSerial_AsnText, istrs), eTakeOwnership, params)) {
            LOG_POST("CTestSerialObject @ " << NStr::PtrToString(&obj));
            cout << MSerial_AsnText << obj << endl;
        }
    }
    {
        // find serial objects of a specific type
        // process them right here
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CObjectIStreamAsyncIterator<CTestSerialObject> i(*is);
        for ( ; i.IsValid(); ++i) {
            const CTestSerialObject& obj = *i;
            LOG_POST("CTestSerialObject @ " << NStr::PtrToString(&obj));
            cout << MSerial_AsnText << obj << endl;
        }
    }
    {
        // find serial objects of a specific type
        // process them right here
        CNcbiIstrstream istrs(buf);
        for (CTestSerialObject& obj : CObjectIStreamAsyncIterator<CTestSerialObject>(
            *CObjectIStream::Open(eSerial_AsnText, istrs), eTakeOwnership)) {
            LOG_POST("CTestSerialObject @ " << NStr::PtrToString(&obj));
            cout << MSerial_AsnText << obj << endl;
        }
    }
    // with exception transfer
    {
        CNcbiIstrstream istrs(buf);
        // generates NCBI exception (wrong format)
        bool gotit = false;
        try {
            for (CTestSerialObject& obj : CObjectIStreamAsyncIterator<CTestSerialObject>(
                *CObjectIStream::Open(eSerial_AsnBinary, istrs), eTakeOwnership)) {
                cout << MSerial_AsnText << obj << endl;
            }
        } catch (CSerialException& e) {
            gotit = e.GetErrCode() == CSerialException::eFormatError;
        } catch (...) {
        }
        BOOST_CHECK(gotit);
    }
    {
        // find serial objects of a specific type
        // process them right here
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CObjectIStreamAsyncIterator<CTestSerialObject> i(*is);
        CObjectIStreamAsyncIterator<CTestSerialObject> eos;
        for_each( i, eos, [](const CTestSerialObject& obj) {
            LOG_POST("CTestSerialObject @ " << NStr::PtrToString(&obj));
            cout << MSerial_AsnText << obj << endl;
        });
    }
    {
        // find serial objects of a specific type
        // process them right here
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CObjectIStreamIterator<CTestSerialObject,CWeb_Env> i(*is);
        for ( ; i.IsValid(); ++i) {
            const CWeb_Env& obj = *i;
            LOG_POST("CWeb_Env @ " << NStr::PtrToString(&obj)
                << " at " << i.GetObjectIStream().GetStackPath());
            cout << MSerial_AsnText << obj << endl;
        }
    }
    {
        // find serial objects of a specific type
        // process them right here
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CObjectIStreamAsyncIterator<CTestSerialObject,CWeb_Env> i(*is);
        for ( ; i.IsValid(); ++i) {
            const CWeb_Env& obj = *i;
            LOG_POST("CWeb_Env @ " << NStr::PtrToString(&obj));
            cout << MSerial_AsnText << obj << endl;
        }
    }
    {
        // find serial objects of a specific type
        // process them right here
        CNcbiIstrstream istrs(buf);
        for ( const CWeb_Env& obj : CObjectIStreamIterator<CTestSerialObject,CWeb_Env>(
                *CObjectIStream::Open(eSerial_AsnText, istrs), eTakeOwnership)) {
            LOG_POST("CWeb_Env @ " << NStr::PtrToString(&obj));
            cout << MSerial_AsnText << obj << endl;
        }
    }
    {
        // find serial objects of a specific type with filter
        // process them right here
        CNcbiIstrstream istrs(buf);
        CObjectIStreamIterator<CWeb_Env>::CParams params;
        params.FilterByMember(1,
            [](const CObjectIStream& istr, CWeb_Env& obj,
            TMemberIndex mem_index, CObjectInfo* mem, void* extra)->bool {
                return true;
            });
        for ( const CWeb_Env& obj : CObjectIStreamIterator<CTestSerialObject,CWeb_Env>(
                *CObjectIStream::Open(eSerial_AsnText, istrs), eTakeOwnership, params)) {
            LOG_POST("CWeb_Env @ " << NStr::PtrToString(&obj));
            cout << MSerial_AsnText << obj << endl;
        }
    }
    // with exception transfer
    {
        CNcbiIstrstream istrs(buf);
        CObjectIStreamIterator<CWeb_Env>::CParams params;
        params.FilterByMember(1,
            [](const CObjectIStream& istr, CWeb_Env& obj,
            TMemberIndex mem_index, CObjectInfo* mem, void* extra)->bool {
                return true;
            });
        // generates NCBI exception (wrong format)
        bool gotit = false;
        try {
            for ( const CWeb_Env& obj : CObjectIStreamIterator<CTestSerialObject,CWeb_Env>(
                    *CObjectIStream::Open(eSerial_AsnBinary, istrs), eTakeOwnership, params)) {
                cout << MSerial_AsnText << obj << endl;
            }
        } catch (CSerialException& e) {
            gotit = e.GetErrCode() == CSerialException::eFormatError;
        } catch (...) {
        }
        BOOST_CHECK(gotit);
    }
    // with exception transfer
    {
        CNcbiIstrstream istrs(buf);
        CObjectIStreamIterator<CWeb_Env>::CParams params;
        params.FilterByMember(1,
            [](const CObjectIStream& istr, CWeb_Env& obj,
            TMemberIndex mem_index, CObjectInfo* mem, void* extra)->bool {
                throw runtime_error("filter");
                return true;
            });
        // generates runtime_error in filter function
        bool gotit = false;
        try {
            for ( const CWeb_Env& obj : CObjectIStreamIterator<CTestSerialObject,CWeb_Env>(
                    *CObjectIStream::Open(eSerial_AsnText, istrs), eTakeOwnership, params)) {
                cout << MSerial_AsnText << obj << endl;
            }
        } catch (CException&) {
        } catch (exception&) {
            gotit = true;
        }
        BOOST_CHECK(gotit);
    }
    {
        // find non-serial objects, here - strings
        // process them right here
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CObjectIStreamIterator<CTestSerialObject,string> i(*is);
        for ( ; i.IsValid(); ++i) {
            const string& obj = *i;
            LOG_POST("CIStreamStdIterator: obj = " << obj
                << " at " << i.GetObjectIStream().GetStackPath());
        }
    }
    {
        // find non-serial objects, here - strings
        // process them right here
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CObjectIStreamIterator<CTestSerialObject,string> i(*is);
        CObjectIStreamIterator<CTestSerialObject,string> eos;
        for_each( i, eos, [](const string& obj) {
            LOG_POST("CIStreamStdIterator: obj = " << obj);
        });
    }
// try to read empty file
    string filename( CDirEntry::GetTmpName() );
    {
        ofstream ofs(filename.c_str());
    }
    {
        ifstream ifs(filename.c_str());
        if (ifs.is_open()) {
            unique_ptr<CObjectIStream> is(
                CObjectIStream::Open(eSerial_AsnText, ifs));
            CObjectIStreamIterator<CTestSerialObject> i(*is);
            for ( ; i.IsValid(); ++i) {
                BOOST_CHECK(false);
            }
        }
    }
    {
        ifstream ifs(filename.c_str());
        if (ifs.is_open()) {
            unique_ptr<CObjectIStream> is(
                CObjectIStream::Open(eSerial_AsnText, ifs));
            CObjectIStreamIterator<CTestSerialObject, CWeb_Env> i(*is);
            for ( ; i.IsValid(); ++i) {
                BOOST_CHECK(false);
            }
        }
    }
    {
        ifstream ifs(filename.c_str());
        if (ifs.is_open()) {
            unique_ptr<CObjectIStream> is(
                CObjectIStream::Open(eSerial_AsnText, ifs));
            CObjectIStreamAsyncIterator<CTestSerialObject> i(*is);
            for ( ; i.IsValid(); ++i) {
                BOOST_CHECK(false);
            }
        }
    }
    {
        ifstream ifs(filename.c_str());
        if (ifs.is_open()) {
            unique_ptr<CObjectIStream> is(
                CObjectIStream::Open(eSerial_AsnText, ifs));
            CObjectIStreamAsyncIterator<CTestSerialObject> i(*is,
                eNoOwnership,
                CObjectIStreamAsyncIterator<CTestSerialObject>::CParams().ReadAndSkipInTheSameThread(true));
            for ( ; i.IsValid(); ++i) {
                BOOST_CHECK(false);
            }
        }
    }
    {
        ifstream ifs(filename.c_str());
        if (ifs.is_open()) {
            unique_ptr<CObjectIStream> is(
                CObjectIStream::Open(eSerial_AsnText, ifs));
            CObjectIStreamAsyncIterator<CTestSerialObject, CWeb_Env> i(*is);
            for ( ; i.IsValid(); ++i) {
                BOOST_CHECK(false);
            }
        }
    }
    CFile(filename).Remove();
#endif  //defined(NCBI_THREADS)
}
#endif    

/////////////////////////////////////////////////////////////////////////////
// TestMemberHooks

BOOST_AUTO_TEST_CASE(s_TestMemberHooks)
{
    CTestSerialObject obj;
    CTestSerialObject2 write1;
#ifdef HAVE_NCBI_C
    WebEnv* env = 0;
#else
    CRef<CWeb_Env> env(new CWeb_Env);
#endif    
    InitializeTestObject( env, obj, write1);

    string data;
    const char* buf = 0;
    {
        // write hook
        // use CObjectHookGuard
        CNcbiOstrstream ostrs;
        unique_ptr<CObjectOStream> os(
            CObjectOStream::Open(eSerial_AsnText, ostrs));
        CObjectHookGuard<CTestSerialObject> w_hook(
            "m_Name", *(new CWriteSerialObject_NameHook), &(*os));
        *os << obj;
		os->FlushBuffer();
		data = CNcbiOstrstreamToString(ostrs);
		buf = data.c_str();
    }
    {
        // read hook
        // use CObjectHookGuard
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CTestSerialObject obj_copy;
        CObjectHookGuard<CTestSerialObject> r_hook(
            "m_Name", *(new CReadSerialObject_NameHook), &(*is));
        *is >> obj_copy;
        // Can not use SerialEquals<> with C-objects
#ifndef HAVE_NCBI_C
        BOOST_CHECK(SerialEquals<CTestSerialObject>(obj, obj_copy));
#endif
    }

    {
        // write hook
        CNcbiOstrstream ostrs;
        unique_ptr<CObjectOStream> os(
            CObjectOStream::Open(eSerial_AsnText, ostrs));
        // set local write hook
        CObjectTypeInfo type = CType<CTestSerialObject>();
        type.FindMember("m_Name").SetLocalWriteHook(
            *os, new CWriteSerialObject_NameHook);
        *os << obj;
		os->FlushBuffer();
		data = CNcbiOstrstreamToString(ostrs);
		buf = data.c_str();
    }
    {
        // read hook
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CTestSerialObject obj_copy;
        // set local read hook
        CObjectTypeInfo type = CType<CTestSerialObject>();
        type.FindMember("m_Name").SetLocalReadHook(
            *is, new CReadSerialObject_NameHook);
        *is >> obj_copy;
        // Can not use SerialEquals<> with C-objects
#ifndef HAVE_NCBI_C
        BOOST_CHECK(SerialEquals<CTestSerialObject>(obj, obj_copy));
#endif
    }
    {
        // read, stack path hook
        CNcbiIstrstream istrs(buf);
        unique_ptr<CObjectIStream> is(
            CObjectIStream::Open(eSerial_AsnText, istrs));
        CTestSerialObject obj_copy;
        is->SetPathReadMemberHook( "CTestSerialObject.m_Name",new CReadSerialObject_NameHook);
        *is >> obj_copy;
        // Can not use SerialEquals<> with C-objects
#ifndef HAVE_NCBI_C
        BOOST_CHECK(SerialEquals<CTestSerialObject>(obj, obj_copy));
#endif
    }
#ifdef HAVE_NCBI_C
    WebEnvFree(env);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// Test iterators

BOOST_AUTO_TEST_CASE(s_TestIterators)
{
    CTestSerialObject write;
    CTestSerialObject2 write1;
#ifdef HAVE_NCBI_C
    WebEnv* env = 0;
#else
    CRef<CWeb_Env> env(new CWeb_Env);
#endif    
    InitializeTestObject( env, write, write1);
    const CTestSerialObject& cwrite = write;

    // traverse a structured object,
    // stopping at data members of a specified type
    CTypeIterator<CTestSerialObject> j1; j1 = Begin(write);
    BOOST_CHECK( j1.IsValid() );
    CTypeIterator<CTestSerialObject> j2(Begin(write));
    BOOST_CHECK( j2.IsValid() );
    CTypeIterator<CTestSerialObject> j3 = Begin(write);
    BOOST_CHECK( j3.IsValid() );

    CTypeConstIterator<CTestSerialObject> k1; k1 = Begin(write);
    BOOST_CHECK( k1.IsValid() );
    CTypeConstIterator<CTestSerialObject> k2(Begin(write));
    BOOST_CHECK( k2.IsValid() );
    CTypeConstIterator<CTestSerialObject> k3 = ConstBegin(write);
    BOOST_CHECK( k3.IsValid() );

    CTypeConstIterator<CTestSerialObject> l1; l1 = ConstBegin(write);
    BOOST_CHECK( l1.IsValid() );
    CTypeConstIterator<CTestSerialObject> l2(ConstBegin(write));
    BOOST_CHECK( l2.IsValid() );
    CTypeConstIterator<CTestSerialObject> l3 = ConstBegin(write);
    BOOST_CHECK( l3.IsValid() );

    CTypeConstIterator<CTestSerialObject> m1; m1 = ConstBegin(cwrite);
    BOOST_CHECK( m1.IsValid() );
    CTypeConstIterator<CTestSerialObject> m2(ConstBegin(cwrite));
    BOOST_CHECK( m2.IsValid() );
    CTypeConstIterator<CTestSerialObject> m3 = ConstBegin(cwrite);
    BOOST_CHECK( m3.IsValid() );

    // iterate over a set of types contained inside an object
    CTypesIterator n1; n1 = Begin(write);
    BOOST_CHECK( !n1.IsValid() );
    CTypesConstIterator n2; n2 = ConstBegin(write);
    BOOST_CHECK( !n2.IsValid() );
    CTypesConstIterator n3; n3 = ConstBegin(cwrite);
    BOOST_CHECK( !n3.IsValid() );

    {
        // visit all CTestSerialObjects
        for ( CTypeIterator<CTestSerialObject> oi = Begin(write); oi; ++oi ) {
            const CTestSerialObject& obj = *oi;
            NcbiCerr<<"CTestSerialObject @ "<<NStr::PtrToString(&obj)<<
                NcbiEndl;
            CTypeIterator<CTestSerialObject> oi1(Begin(write));
            BOOST_CHECK( oi1.IsValid() );
            CTypeIterator<CTestSerialObject> oi2;
            oi2 = Begin(write);
            BOOST_CHECK( oi2.IsValid() );
        }
    }
    {
        // visit all CTestSerialObjects
        for ( CTypeConstIterator<CTestSerialObject> oi = ConstBegin(write); oi;
              ++oi ) {
            const CTestSerialObject& obj = *oi;
            NcbiCerr<<"CTestSerialObject @ "<<NStr::PtrToString(&obj)<<
                NcbiEndl;
            CTypeConstIterator<CTestSerialObject> oi1(Begin(write));
            BOOST_CHECK( oi1.IsValid() );
            CTypeConstIterator<CTestSerialObject> oi2;
            oi2 = Begin(write);
            BOOST_CHECK( oi2.IsValid() );
        }
    }
    {
        // visit all CTestSerialObjects
        for ( CTypeConstIterator<CTestSerialObject> oi = ConstBegin(cwrite);
              oi; ++oi ) {
            const CTestSerialObject& obj = *oi;
            NcbiCerr<<"CTestSerialObject @ "<<NStr::PtrToString(&obj)<<
                NcbiEndl;
            CTypeConstIterator<CTestSerialObject> oi1(ConstBegin(cwrite));
            BOOST_CHECK( oi1.IsValid() );
            CTypeConstIterator<CTestSerialObject> oi2;
            oi2 = ConstBegin(cwrite);
            BOOST_CHECK( oi2.IsValid() );
        }
    }

    {
        // visit all strings
        for ( CStdTypeConstIterator<string> si = ConstBegin(cwrite); si;
              ++si ) {
            BOOST_CHECK( si.IsValid() );
            NcbiCerr <<"String: \""<<*si<<"\""<<NcbiEndl;
        }
    }

    {
        // visit all CObjects
        for ( CObjectConstIterator oi = ConstBegin(cwrite); oi; ++oi ) {
            const CObject& obj = *oi;
            BOOST_CHECK( oi.IsValid() );
            NcbiCerr <<"CObject: @ "<<NStr::PtrToString(&obj)<<NcbiEndl;
        }
    }

    {
        CTypesIterator i;
        CType<CTestSerialObject>::AddTo(i);
        CType<CTestSerialObject2>::AddTo(i);
        for ( i = Begin(write); i; ++i ) {
            BOOST_CHECK( i.IsValid() );
            CTestSerialObject* obj = CType<CTestSerialObject>::Get(i);
            BOOST_CHECK( obj );
            if ( obj ) {
                NcbiCerr <<"CObject: @ "<<NStr::PtrToString(obj)<<NcbiEndl;
            }
            else {
                NcbiCerr <<"CObject2: @ "<<
                    NStr::PtrToString(CType<CTestSerialObject2>::Get(i))<<
                    NcbiEndl;
            }
        }
    }

    {
        CTypesConstIterator i;
        CType<CTestSerialObject>::AddTo(i);
        CType<CTestSerialObject2>::AddTo(i);
        for ( i = ConstBegin(write); i; ++i ) {
            BOOST_CHECK( i.IsValid() );
            const CTestSerialObject* obj = CType<CTestSerialObject>::Get(i);
            BOOST_CHECK( obj );
            if ( obj ) {
                NcbiCerr <<"CObject: @ "<<NStr::PtrToString(obj)<<NcbiEndl;
            }
            else {
                NcbiCerr <<"CObject2: @ "<<
                    NStr::PtrToString(CType<CTestSerialObject2>::Get(i))<<
                    NcbiEndl;
            }
        }
    }
#ifdef HAVE_NCBI_C
    WebEnvFree(env);
#endif
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(s_TestSerialization2)
{
    CTestSerialObject write;
    CTestSerialObject2 write1;
#ifdef HAVE_NCBI_C
    WebEnv* env = 0;
#else
    CRef<CWeb_Env> env(new CWeb_Env);
#endif    
    InitializeTestObject( env, write, write1);

#ifdef HAVE_NCBI_C
    string text_in("ctest_serial.asn");
    string bin_in("ctest_serial.asb");
#else
    string text_in("cpptest_serial.asn");
    string bin_in("cpptest_serial.asb");
#endif
    string text_out("test_serial.asno"), text_out2("test_serial.asno2");
    string bin_out("test_serial.asbo");
    {
        {
            // write ASN text
            unique_ptr<CObjectOStream> out(
                CObjectOStream::Open(text_out,eSerial_AsnText));
            *out << write;
        }
        BOOST_CHECK( CFile(text_in).CompareTextContents(text_out, CFile::eIgnoreEol) );
        {
            // write ASN text
            CNcbiOfstream out(text_out2.c_str());
            PrintAsn(out, ConstObjectInfo(write));
        }
        CTestSerialObject read;
        {
            // read ASN text
            unique_ptr<CObjectIStream> in(
                CObjectIStream::Open(text_in,eSerial_AsnText));
            *in >> read;
        }
#ifdef HAVE_NCBI_C
        // Some functions does not work with C-style objects.
        // Reset WebEnv for SerialEquals() to work.
        TWebEnv* saved_env_read = read.m_WebEnv;
        TWebEnv* saved_env_write = write.m_WebEnv;
        read.m_WebEnv = 0;
        write.m_WebEnv = 0;
#endif
        BOOST_CHECK(SerialEquals<CTestSerialObject>(read, write));
#ifdef HAVE_NCBI_C
        // Restore C-style objects
        read.m_WebEnv = saved_env_read;
        write.m_WebEnv = saved_env_write;
#endif
        read.Dump(NcbiCerr);
        read.m_Next->Dump(NcbiCerr);
#ifndef HAVE_NCBI_C
        {
            // skip data
            unique_ptr<CObjectIStream> in(
                CObjectIStream::Open(text_in,eSerial_AsnText));
            in->Skip(ObjectType(read));
        }
#endif
    }

    {
        {
            // write ASN binary
            unique_ptr<CObjectOStream> out(
                CObjectOStream::Open(bin_out,eSerial_AsnBinary));
            *out << write;
        }
        BOOST_CHECK( CFile(bin_in).Compare(bin_out) );
        CTestSerialObject read;
        {
            // read ASN binary
            unique_ptr<CObjectIStream> in(
                CObjectIStream::Open(bin_in,eSerial_AsnBinary));
            *in >> read;
        }
#ifdef HAVE_NCBI_C
        // Some functions does not work with C-style objects.
        // Reset WebEnv for SerialEquals() to work.
        TWebEnv* saved_env_read = read.m_WebEnv;
        TWebEnv* saved_env_write = write.m_WebEnv;
        read.m_WebEnv = 0;
        write.m_WebEnv = 0;
#endif
        BOOST_CHECK(SerialEquals<CTestSerialObject>(read, write));
#ifdef HAVE_NCBI_C
        // Restore C-style objects
        read.m_WebEnv = saved_env_read;
        write.m_WebEnv = saved_env_write;
#endif
        read.Dump(NcbiCerr);
        read.m_Next->Dump(NcbiCerr);
#ifndef HAVE_NCBI_C
        {
            // skip data
            unique_ptr<CObjectIStream> in(
                CObjectIStream::Open(bin_in,eSerial_AsnBinary));
            in->Skip(ObjectType(read));
        }
#endif
    }
#ifdef HAVE_NCBI_C
    WebEnvFree(env);
#endif
}

#ifndef HAVE_NCBI_C
/////////////////////////////////////////////////////////////////////////////
// TestPrintAsn

BOOST_AUTO_TEST_CASE(s_TestPrintAsn)
{
    string text_in("webenv.ent"), text_out("webenv.ento");
    {
        CRef<CWeb_Env> env(new CWeb_Env);
        {
            // read ASN text
            unique_ptr<CObjectIStream> in(
                CObjectIStream::Open(text_in,eSerial_AsnText));
            *in >> *env;
        }
        {
            // write ASN text
            CNcbiOfstream out(text_out.c_str());
            PrintAsn(out, ConstObjectInfo(*env));
        }
        BOOST_CHECK( CFile(text_in).CompareTextContents(text_out, CFile::eIgnoreEol) );
    }
}

#endif
