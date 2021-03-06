#ifndef OBJECTS_TRACKMGR_GRIDCLI__BLAST_CLIENT_HPP
#define OBJECTS_TRACKMGR_GRIDCLI__BLAST_CLIENT_HPP

/* $Id: blast_client.hpp 488465 2015-12-30 23:55:06Z meric $
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
 * Authors: Peter Meric
 *
 * File Description:  NetSchedule grid client for TrackManager BLAST request/reply
 *
 */

/// @file tmgr_blast_client.hpp
/// NetSchedule grid client for TrackManager BLAST request/reply

#include <objects/trackmgr/gridrpcclient.hpp>


BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CTMgr_BlastRIDRequest;
class CTMgr_BlastRIDReply;
END_SCOPE(objects)


class CTMS_BLAST_Client : private CGridRPCBaseClient<CAsnBinCompressed>
{
private:
    typedef CGridRPCBaseClient<ncbi::CAsnBinCompressed> TBaseClient;

public:
    typedef objects::CTMgr_BlastRIDRequest TRequest;
    typedef objects::CTMgr_BlastRIDReply TReply;
    typedef CConstRef<TRequest> TRequestCRef;
    typedef CRef<TReply> TReplyRef;


public:
    CTMS_BLAST_Client(const string& NS_service,
                      const string& NS_queue,
                      const string& client_name,
                      const string& NC_registry_section
                     );

    CTMS_BLAST_Client(const string& NS_registry_section = "netschedule_api",
                      const string& NC_registry_section = kEmptyStr
                     );

    virtual ~CTMS_BLAST_Client();

    TReplyRef FetchRID(const string& rid) const;
    TReplyRef Fetch(const TRequest& request) const;
};


END_NCBI_SCOPE

#endif // OBJECTS_TRACKMGR_GRIDCLI__BLAST_CLIENT_HPP

