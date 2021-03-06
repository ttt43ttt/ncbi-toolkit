/*  $Id: delete.cpp 564326 2018-05-24 18:29:46Z saprykin $
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
 * Authors: Dmitrii Saprykin
 *
 * File Description:
 *
 * Cassandra delete blob task.
 *
 */

#include <ncbi_pch.hpp>

#include "delete.hpp"

#include <objtools/pubseq_gateway/impl/cassandra/cass_blob_op.hpp>
#include <objtools/pubseq_gateway/impl/cassandra/cass_driver.hpp>
#include <objtools/pubseq_gateway/impl/cassandra/IdCassScope.hpp>

BEGIN_IDBLOB_SCOPE
USING_NCBI_SCOPE;

CCassBlobTaskDelete::CCassBlobTaskDelete(
        unsigned int op_timeout_ms,
        shared_ptr<CCassConnection> conn,
        const string & keyspace,
        bool extended,
        int32_t key,
        bool async,
        unsigned int max_retries,
        TDataErrorCallback error_cb
)
    : CCassBlobWaiter(op_timeout_ms, conn, keyspace, key,
            async, max_retries, move(error_cb))
    , m_ExtendedSchema(extended)
{}

void CCassBlobTaskDelete::Wait1()
{
    bool b_need_repeat;
    do {
        b_need_repeat = false;
        switch (m_State) {
            case eError:
            case eDone:
                return;

            case eInit: {
                CloseAll();
                m_QueryArr.clear();
                m_QueryArr.emplace_back(m_Conn->NewQuery());
                if (m_ExtendedSchema) {
                    auto qry = m_QueryArr[0];
                    string sql = "SELECT last_modified FROM " + GetKeySpace() + ".blob_prop WHERE sat_key = ?";
                    qry->SetSQL(sql, 1);
                    qry->BindInt32(0, m_Key);
                    qry->Query(CASS_CONSISTENCY_LOCAL_QUORUM, m_Async);
                    UpdateLastActivity();
                    m_State = eReadingVersions;
                }
                else {
                    m_State = eDeleteData;
                    b_need_repeat = true;
                }
                break;
            }

            case eReadingVersions: {
                string sql;
                auto qry = m_QueryArr[0];
                if (!CheckReady(qry, eInit, &b_need_repeat)) {
                    if (b_need_repeat)
                        ERR_POST(Info << "Restart stReadingEntity key=" << m_Keyspace << "." << m_Key);
                    break;
                }
                async_rslt_t wr = static_cast<async_rslt_t>(-1);
                if (!qry->IsEOF()) {
                    UpdateLastActivity();
                    while ((wr = qry->NextRow()) == ar_dataready) {
                        m_ExtendedVersions.push_back(qry->FieldGetInt64Value(0));
                        ERR_POST(Trace << "blob, key=" << m_Keyspace << "."
                                 << m_Key << ", last_modified: "
                                 << m_ExtendedVersions[m_ExtendedVersions.size() - 1]
                        );
                    }
                    if (wr == ar_wait) {
                        break;
                    }
                }

                CloseAll();
                if (m_ExtendedVersions.empty()) {
                    RAISE_DB_ERROR(eInconsistentData, string("Blob versions not found. key: ")
                        + m_Keyspace + "." + NStr::NumericToString(m_Key)
                    );
                }
                m_State = eDeleteData;
                b_need_repeat = true;
                break;
            }

            case eDeleteData: {
                UpdateLastActivity();
                auto qry = m_QueryArr[0];
                qry->NewBatch();
                if (m_ExtendedSchema) {
                    for (auto version : m_ExtendedVersions) {
                        string sql = "DELETE FROM " + GetKeySpace() + ".blob_chunk WHERE sat_key = ? and last_modified = ?";
                        qry->SetSQL(sql, 2);
                        qry->BindInt32(0, m_Key);
                        qry->BindInt64(1, version);
                        qry->Execute(CASS_CONSISTENCY_LOCAL_QUORUM, m_Async);
                    }
                    string sql = "DELETE FROM " + GetKeySpace() + ".blob_prop WHERE sat_key = ?";
                    qry->SetSQL(sql, 1);
                    qry->BindInt32(0, m_Key);
                    qry->Execute(CASS_CONSISTENCY_LOCAL_QUORUM, m_Async);
                }
                else {
                    string sql = "DELETE FROM " + GetKeySpace() + ".largeentity WHERE ent = ?";
                    qry->SetSQL(sql, 1);
                    qry->BindInt32(0, m_Key);
                    qry->Execute(CASS_CONSISTENCY_LOCAL_QUORUM, m_Async);
                    sql = "DELETE FROM " + GetKeySpace() + ".entity WHERE ent = ?";
                    qry->SetSQL(sql, 1);
                    qry->BindInt32(0, m_Key);
                    qry->Execute(CASS_CONSISTENCY_LOCAL_QUORUM, m_Async);
                }
                qry->RunBatch();
                m_State = eWaitDeleteData;
                break;
            }

            case eWaitDeleteData: {
                if (!CheckReady(m_QueryArr[0], eDeleteData, &b_need_repeat)) {
                    if (b_need_repeat)
                        ERR_POST(Info << "Restart eDeleteData key=" << m_Keyspace << "." << m_Key);
                    break;
                }
                CloseAll();
                m_State = eDone;
                break;
            }

            default: {
                char msg[1024];
                snprintf(msg, sizeof(msg),
                         "Failed to delete blob (key=%s.%d) unexpected state (%d)",
                         m_Keyspace.c_str(), m_Key, static_cast<int>(m_State));
                Error(CRequestStatus::e502_BadGateway,
                      CCassandraException::eQueryFailed,
                      eDiag_Error, msg);
            }
        }
    } while (b_need_repeat);
}

END_IDBLOB_SCOPE
