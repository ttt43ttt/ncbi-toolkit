/*  $Id: srv_lib.cpp 544799 2017-08-28 14:32:44Z gouriano $
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
 * Author: Pavel Ivanov
 *
 * File Description: Network cache daemon
 *
 */

#include "task_server_pch.hpp"

// Additional sources to avoid linking with related libraries and to force
// everybody to use CTaskServer's infrastructure of threads, diagnostics and
// application-related stuff

class CDiagContextThreadData {
public:
    static bool IsInitialized(void) {return false;}
};
class CThread {
public:
    static unsigned int GetThreadsCount() {return 0;}
};

#include "../../corelib/ncbistr.cpp"
#undef NCBI_USE_ERRCODE_X
#undef CHECK_RANGE
#include "../../corelib/ncbiobj.cpp"
#undef NCBI_USE_ERRCODE_X
#undef STACK_THRESHOLD
#include "../../corelib/ddumpable.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/ncbienv.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/request_ctx.cpp"
#include "../../corelib/ncbi_strings.c"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/ncbitime.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/ncbireg.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/metareg.cpp"
#undef NCBI_USE_ERRCODE_X
#define s_Hex s_HexStre
#include "../../corelib/ncbistre.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/stream_utils.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/ncbifile.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/ncbimtx.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/ncbi_safe_static.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/ncbi_param.cpp"
#undef NCBI_USE_ERRCODE_X
#ifdef NCBI_OS_LINUX
# include "../../corelib/ncbi_system.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/ncbi_os_unix.cpp"
#elif defined(NCBI_OS_MSWIN)
# undef NCBI_OS_MSWIN
# include "../../corelib/ncbi_system.cpp"
# define NCBI_OS_MSWIN 1
#endif
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/env_reg.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/ncbiexpt.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/ncbi_process.cpp"
#undef NCBI_USE_ERRCODE_X
#include "../../corelib/interprocess_lock.cpp"
#undef NCBI_USE_ERRCODE_X

#if 1

#if NCBI_DEVELOPMENT_VER < 20160726
void  ncbi::CNcbiError::Set( ncbi::CNcbiError::ECode, const ncbi::CTempString& ) {}
void  ncbi::CNcbiError::SetErrno( int, const ncbi::CTempString& )                {}
void  ncbi::CNcbiError::SetFromErrno( const ncbi::CTempString& )                 {}
#else
void  ncbi::CNcbiError::Set( ncbi::CNcbiError::ECode )                          {}
void  ncbi::CNcbiError::Set( ncbi::CNcbiError::ECode, const ncbi::CTempString ) {}
void  ncbi::CNcbiError::Set( ncbi::CNcbiError::ECode, std::string&& )           {}
void  ncbi::CNcbiError::Set( ncbi::CNcbiError::ECode, const std::string& )      {}
void  ncbi::CNcbiError::SetFromErrno( const ncbi::CTempString )                 {}
void  ncbi::CNcbiError::SetFromErrno( void )                                    {}
void  ncbi::CNcbiError::SetFromErrno( const char* )                             {}
void  ncbi::CNcbiError::SetFromErrno( const std::string& )                      {}
void  ncbi::CNcbiError::SetErrno( int )                                         {}
void  ncbi::CNcbiError::SetErrno( int, std::string&& )                          {}
void  ncbi::CNcbiError::SetErrno( int, const char* )                            {}
void  ncbi::CNcbiError::SetErrno( int, const std::string& )                     {}
void  ncbi::CNcbiError::SetErrno( int, const ncbi::CTempString )                {}
#endif

#if defined(NCBI_OS_MSWIN)
void ncbi::CNcbiError::SetWindowsError(int)                                     {}
void ncbi::CNcbiError::SetFromWindowsError(void)                                {}
void  ncbi::CNcbiError::SetWindowsError( int, const char* )                     {}
void  ncbi::CNcbiError::SetWindowsError( int, const std::string& )              {}
void  ncbi::CNcbiError::SetWindowsError( int, std::string&& )                   {}
void  ncbi::CNcbiError::SetWindowsError( int, const ncbi::CTempString )         {}
void  ncbi::CNcbiError::SetFromWindowsError( const std::string& )               {}
void  ncbi::CNcbiError::SetFromWindowsError( std::string&& )                    {}
void  ncbi::CNcbiError::SetFromWindowsError( const char* )                      {}
void  ncbi::CNcbiError::SetFromWindowsError( const ncbi::CTempString )          {}
#endif

#else

#include "../../corelib/ncbierror.cpp"
#undef NCBI_USE_ERRCODE_X

#endif


#if 1
std::string ncbi::CNcbiEncrypt::Encrypt(const std::string& str, const std::string& password) {
    return str;
}
std::string ncbi::CNcbiEncrypt::Encrypt(const std::string& str) {
    return str;
}
std::string ncbi::CNcbiEncrypt::Decrypt(const std::string& str, const std::string& password) {
    return str;
}
std::string ncbi::CNcbiEncrypt::Decrypt(const std::string& str) {
    return str;
}
bool ncbi::CNcbiEncrypt::IsEncrypted(const std::string& ) {
    return false;
}
#endif

BEGIN_NCBI_SCOPE


SDiagMessage::SDiagMessage(EDiagSev severity, const char* buf, size_t len,
                           const char* file /* = 0 */, size_t line /* = 0 */,
                           TDiagPostFlags flags /* = eDPF_Default */,
                           const char* prefix /* = 0 */,
                           int err_code /* = 0 */, int err_subcode /* = 0 */,
                           const char* err_text /* = 0 */,
                           const char* module /* = 0 */,
                           const char* nclass /* = 0 */,
                           const char* func /* = 0 */)
    : severity(severity),
      buf(buf),
      len(len),
      file(file),
      line(line),
      flags(flags),
      prefix(prefix),
      err_code(err_code),
      err_subcode(err_subcode),
      err_text(err_text),
      module(module),
      nclass(nclass),
      func(func)
{}

#if 1
DEFINE_STATIC_MUTEX(s_InstanceMutex);
SSystemMutex&
CNcbiApplication::GetInstanceMutex(void) {
    return s_InstanceMutex;
}
#endif

#if NCBI_DEVELOPMENT_VER > 20160725
CSharedHitId CDiagContext::x_GetDefaultHitID(EDefaultHitIDFlags)
{
    return CSharedHitId(m_hid);
}
#endif

END_NCBI_SCOPE
