#ifndef NETSCHEDULE_NS_UTIL__HPP
#define NETSCHEDULE_NS_UTIL__HPP

/*  $Id: ns_util.hpp 476355 2015-08-18 14:43:35Z satskyse $
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
 * Authors:  Victor Joukov
 *
 * File Description: Utility functions for NetSchedule
 *
 */

#include "ns_types.hpp"
#include <corelib/ncbistl.hpp>
#include <corelib/ncbireg.hpp>


BEGIN_NCBI_SCOPE


class IRegistry;


const string    g_ValidPrefix = "Validating config file: ";
const string    g_WarnPrefix = g_ValidPrefix + "unexpected value of ";



// A few helpers
string NS_RegValName(const string &  section, const string &  entry);
bool NS_ValidateDouble(const IRegistry &  reg,
                       const string &  section, const string &  entry,
                       vector<string> &  warnings);
bool NS_ValidateBool(const IRegistry &  reg,
                     const string &  section, const string &  entry,
                     vector<string> &  warnings);
bool NS_ValidateInt(const IRegistry &  reg,
                    const string &  section, const string &  entry,
                    vector<string> &  warnings);
bool NS_ValidateString(const IRegistry &  reg,
                       const string &  section, const string &  entry,
                       vector<string> &  warnings);
bool NS_ValidateDataSize(const IRegistry &  reg,
                         const string &  section, const string &  entry,
                         vector<string> &  warnings);
unsigned int NS_GetDataSize(const IRegistry &  reg,
                            const string &  section, const string &  entry,
                            unsigned int  default_val);



// Validates the config file and populates a warnings list if the file has
// problems.
void NS_ValidateConfigFile(const IRegistry &  reg, vector<string> &  warnings,
                           bool  throw_port_exception,
                           bool &  decrypting_error);

string NS_GetConfigFileChecksum(const string &  file_name,
                                vector<string> &  warnings);


END_NCBI_SCOPE

#endif /* NETSCHEDULE_NS_UTIL__HPP */

