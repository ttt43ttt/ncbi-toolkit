#ifndef NETCACHE__SERVER_CORE__HPP
#define NETCACHE__SERVER_CORE__HPP
/*  $Id: server_core.hpp 456784 2015-01-15 15:10:29Z gouriano $
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
 * Authors:  Pavel Ivanov
 *
 * File Description: 
 */


BEGIN_NCBI_SCOPE


string GetConfName(void);
void ExtractFileName(const char* file, const char*& file_name, size_t& name_size);
void TrackShuttingDown(void);


END_NCBI_SCOPE

#endif /* NETCACHE__SERVER_CORE__HPP */
