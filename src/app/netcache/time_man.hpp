#ifndef NETCACHE__TIME_MAN__HPP
#define NETCACHE__TIME_MAN__HPP
/*  $Id: time_man.hpp 530755 2017-03-17 13:19:30Z gouriano $
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


void InitTime(void);
void InitTimeMan(void);
void ConfigureTimeMan(const CNcbiRegistry* reg, CTempString section);
bool ReConfig_TimeMan(const CTempString& section, const CNcbiRegistry& new_reg, string& err_message);
void WriteSetup_TimeMan(CSrvSocketTask& task);
void IncCurJiffies(void);


END_NCBI_SCOPE

#endif /* NETCACHE__TIME_MAN__HPP */
