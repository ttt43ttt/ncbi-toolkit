#ifndef NST_METADATA_OPTIONS__HPP
#define NST_METADATA_OPTIONS__HPP
/*  $Id: nst_metadata_options.hpp 452173 2014-11-17 15:42:49Z satskyse $
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
 * File Description: Metadata options
 */



BEGIN_NCBI_SCOPE



// Metadata option provided at the HELLO time
enum EMetadataOption {
    eMetadataUnknown = -1,      // Not recognized option

    eMetadataNotSpecified = 0,  // Optional parameter is not provided
    eMetadataRequired = 1,      // Service requested
    eMetadataDisabled = 2,      // No DB interaction at all
    eMetadataMonitoring = 3     // DB interaction is allowed however there will
                                // be no accounting
};

enum EMetadataOption  g_IdToMetadataOption(const string &  option_id);
string  g_MetadataOptionToId(enum EMetadataOption  option);
vector<string>  g_GetSupportedMetadataOptions(void);


END_NCBI_SCOPE

#endif /* NST_METADATA_OPTIONS__HPP */

