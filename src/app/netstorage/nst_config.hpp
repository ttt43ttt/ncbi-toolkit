#ifndef NETSTORAGE_CONFIG__HPP
#define NETSTORAGE_CONFIG__HPP

/*  $Id: nst_config.hpp 496547 2016-03-29 16:10:07Z satskyse $
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
 * Authors:  Sergey Satskiy
 *
 * File Description: NetStorage config file utilities
 *
 */

#include <corelib/ncbireg.hpp>
#include <corelib/ncbitime.hpp>

#include "nst_database.hpp"


BEGIN_NCBI_SCOPE

// Forward declaration
class CNetStorageServer;


// Validates the config file - it does LOG_POST(...) of the problems it found
// Returns true if the config file is perfectly well formed
void NSTValidateConfigFile(const IRegistry &  reg,
                           vector<string> &  warnings,
                           bool  throw_port_exception);
CJsonNode NSTGetBackendConfiguration(const IRegistry &  reg,
                                     CNetStorageServer *  server,
                                     vector<string> &  warnings);
TNSTDBValue<CTimeSpan>  ReadTimeSpan(const string &  reg_value,
                                     bool  allow_infinity);

END_NCBI_SCOPE

#endif /* NETSTORAGE_CONFIG__HPP */

