#ifndef CONNECT___NCBI_SERVICE_CXX__HPP
#define CONNECT___NCBI_SERVICE_CXX__HPP

/* $Id: ncbi_service_cxx.hpp 548166 2017-10-11 10:11:44Z mcelhany $
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
 * Author:  David McElhany
 *
 * File description:
 * @file ncbi_service_cxx.hpp
 *   C++-only API for named network services.
 *
 */

#include <string>
#include <vector>

#include <corelib/ncbistl.hpp>

#include <connect/ncbi_server_info.h>
#include <connect/ncbi_service.h>


/** @addtogroup UtilityFunc
 *
 * @{
 */


BEGIN_NCBI_SCOPE


/// Service mapper types.
///
enum ESERV_Mapper {
    fSERV_Local     = 0x01,
    fSERV_Lbsmd     = 0x02,
    fSERV_Dispd     = 0x04,
    fSERV_Namerd    = 0x08,
    fSERV_Linkerd   = 0x10
};
typedef unsigned int   TSERV_Mapper;    ///< Bitwise OR of ESERV_Mapper


/// Attributes of a given service.
///
class NCBI_XCONNECT_EXPORT CSERV_Info {
public:
    CSERV_Info(const string& host, unsigned int port, double rate, ESERV_Type type) :
        m_Host(host), m_Port(port), m_Rate(rate), m_Type(type)
    {
    }

    string          GetHost(void) const { return m_Host; }
    unsigned int    GetPort(void) const { return m_Port; }
    double          GetRate(void) const { return m_Rate; }
    ESERV_Type      GetType(void) const { return m_Type; }

private:
    string          m_Host;
    unsigned int    m_Port;
    double          m_Rate;
    ESERV_Type      m_Type;
};


/// Get the servers for a given service
///
/// @param[in] service
///  Service name
/// @param[in] types
///  Which service types to check
/// @param[in] mapper_types
///  Which mapper(s) to use
/// @return
///  List of servers (ordered according to their rates)
extern NCBI_XCONNECT_EXPORT
vector<CSERV_Info> SERV_GetServers(const string& service,
                                   TSERV_Type    types    = fSERV_Http,
                                   TSERV_Mapper  mappers  = fSERV_Namerd);


END_NCBI_SCOPE


/* @} */

#endif  // CONNECT___NCBI_SERVICE_CXX__HPP
