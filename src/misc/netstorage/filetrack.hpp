#ifndef CONNECT_SERVICES___FILETRACK_HPP
#define CONNECT_SERVICES___FILETRACK_HPP

/*  $Id: filetrack.hpp 521502 2016-12-09 15:34:37Z sadyrovr $
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
 * Author:  Dmitry Kazimirov
 *
 * File Description:
 *   FileTrack API declarations.
 *
 */

#include <misc/netstorage/netstorage.hpp>

#include <connect/ncbi_conn_stream.hpp>
#include <connect/ncbi_http_session.hpp>
#include <connect/ncbi_connutil.h>

#include <corelib/ncbimisc.hpp>
#include <corelib/reader_writer.hpp>

#include <util/random_gen.hpp>

#include <atomic>
#include <memory>

BEGIN_NCBI_SCOPE

struct SFileTrackConfig
{
    bool enabled = false;
    const bool check_locator_site = false;
    string token;
    const STimeout comm_timeout;

    SFileTrackConfig(EVoid = eVoid); // Means no FileTrack as a backend storage
    SFileTrackConfig(const IRegistry& registry, const string& section = kEmptyStr);

    bool ParseArg(const string&, const string&);

    struct SSite
    {
        using TValue = CNetStorageObjectLoc::EFileTrackSite;
        SSite();
        operator TValue();
        void operator=(const string&);

    private:
        atomic<TValue> value;
    };

    static SSite site;
};

struct SFileTrackRequest : public CObject
{
    SFileTrackRequest(const SFileTrackConfig& config,
            const CNetStorageObjectLoc& object_loc,
            const string& url,
            SConnNetInfo* net_info = NULL,
            const string& user_header = kEmptyStr,
            THTTP_Flags flags = 0);

    CJsonNode GetFileInfo(const char* const when);

protected:
    const SFileTrackConfig& m_Config;
    const CNetStorageObjectLoc& m_ObjectLoc;
    string m_URL;
    CConn_HttpStream m_HTTPStream;
};

struct SFileTrackDownload : public SFileTrackRequest
{
    SFileTrackDownload(const SFileTrackConfig& config,
            const CNetStorageObjectLoc& object_loc);

    ERW_Result Read(void* buf, size_t count, size_t* bytes_read);
    bool Eof() const;
    void FinishDownload();

private:
    bool m_FirstRead = true;
};

struct SFileTrackUpload : public SFileTrackRequest
{
    SFileTrackUpload(const SFileTrackConfig& config,
            const CNetStorageObjectLoc& object_loc,
            const string& user_header, SConnNetInfo* net_info);

    void Write(const void* buf, size_t count, size_t* bytes_written);
    void FinishUpload();

private:
    void RenameFile(const string& from, const string& to);
};

struct SFileTrackAPI
{
    SFileTrackAPI(const SFileTrackConfig&);

    CJsonNode GetFileInfo(const CNetStorageObjectLoc& object_loc);

    CRef<SFileTrackUpload> StartUpload(const CNetStorageObjectLoc& object_loc);
    CRef<SFileTrackDownload> StartDownload(const CNetStorageObjectLoc& object_loc);

    void Remove(const CNetStorageObjectLoc& object_loc);
    string GetPath(const CNetStorageObjectLoc& object_loc);

    DECLARE_OPERATOR_BOOL(config.enabled);

    const SFileTrackConfig config;

private:
    shared_ptr<SConnNetInfo> m_NetInfo;
};

END_NCBI_SCOPE

#endif  /* CONNECT_SERVICES___FILETRACK_HPP */
