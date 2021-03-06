/* $Id: id2_client.cpp 477911 2015-09-02 16:03:12Z vasilche $
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
 * Author:  .......
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using the following specifications:
 *   'id2.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>

// generated includes
#include <objects/id2/id2_client.hpp>
#include <objects/id2/ID2_Request_Packet.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CID2Client::~CID2Client(void)
{
}


void CID2Client::WriteRequest(CObjectOStream& out, const TRequest& request)
{
    // send a packet with single request
    CID2_Request_Packet packet;
    packet.Set().push_back(Ref(&const_cast<TRequest&>(request)));
    out << packet;
}


void CID2Client::ReadReply(CObjectIStream& in, TReply& reply)
{
    m_Replies.clear();
    // read all replies until 'end-of-reply' is set
    do {
        in >> reply;
        if ( reply.IsSetDiscard() ) {
            // if 'discard' is set we should ignore the reply
            continue;
        }
        if ( reply.IsSetEnd_of_reply() && reply.CanBeDeleted() ) {
            // last reply can be stored directly if it's allocated in heap
            m_Replies.push_back(Ref(&reply));
        }
        else {
            // otherwise we store a copy of the reply
            m_Replies.push_back(Ref(SerialClone(reply)));
        }
    } while ( !reply.IsSetEnd_of_reply() );
}


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE
