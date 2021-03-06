/* $Id: grid_app_version_info.hpp 549816 2017-10-27 14:56:47Z sadyrovr $
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
 * Authors:  Dmitry Kazimirov
 *
 * File Description:
 *   Declaration of the application version info string.
 *
 */


#ifndef CONNECT_SERVICES__APP_VERSION_INFO_HPP
#define CONNECT_SERVICES__APP_VERSION_INFO_HPP

#include <common/ncbi_source_ver.h>
#include <common/ncbi_package_ver.h>

#include <connect/services/json_over_uttp.hpp>

#include <sstream>

#if defined(NCBI_PACKAGE) && NCBI_PACKAGE
#define GRID_APP_VERSION "Grid " NCBI_PACKAGE_VERSION
#elif defined(NCBI_DEVELOPMENT_VER)
#define GRID_APP_VERSION "codebase version " \
        NCBI_AS_STRING(NCBI_DEVELOPMENT_VER)
#else
#define GRID_APP_VERSION "development version"
#endif

#define GRID_APP_VERSION_INFO GRID_APP_NAME ": " GRID_APP_VERSION \
        " built on " __DATE__

#define GRID_APP_CHECK_VERSION_ARGS() \
    for (int i = argc; --i > 0; ) { \
        if (NStr::CompareCase(argv[i], "-version") == 0 || \
                NStr::CompareCase(argv[i], "-version-full") == 0 || \
                NStr::CompareCase(argv[i], "--version") == 0) { \
            puts(GRID_APP_VERSION_INFO); \
            return 0; \
        } \
    }

namespace ncbi {
namespace grid {

template <class TBase>
class CVersionReporting : public TBase
{
public:
    template<typename... Args>
    CVersionReporting(Args&&... args) : TBase(std::forward<Args>(args)...)
    {
        auto& version = this->GetFullVersion();
        auto package_version =  version.GetPackageVersion();
        this->SetVersion(package_version);
    }
};

}
}

#endif // CONNECT_SERVICES__APP_VERSION_INFO_HPP
