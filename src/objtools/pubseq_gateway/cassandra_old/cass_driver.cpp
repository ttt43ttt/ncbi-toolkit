/*  $Id: cass_driver.cpp 562576 2018-04-24 15:48:58Z satskyse $
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
 * Authors: Dmitri Dmitrienko
 *
 * File Description:
 *
 *  Wrapper class around cassandra "C"-API
 *
 */

#include <ncbi_pch.hpp>

#include <atomic>
#include <memory>
#include <sstream>
#include <set>
#include <unistd.h>

#if __cplusplus >= 201103L
    #include <mutex>
#endif

#include "corelib/ncbitime.hpp"
#include "corelib/ncbistr.hpp"

#include <objtools/pubseq_gateway/impl/diag/AppLog.hpp>
#include <objtools/pubseq_gateway/impl/cassandra_old/cass_driver.hpp>
#include <objtools/pubseq_gateway/impl/cassandra_old/IdCassScope.hpp>

BEGIN_IDBLOB_SCOPE
USING_NCBI_SCOPE;
using namespace IdLogUtil;

#define DISCONNECT_TIMEOUT_MS           5000L
#define CORE_CONNECTIONS_PER_HOST       2
#define MAX_CONNECTIONS_PER_HOST        4
#define IO_THREADS                      32
#define CORE_MAX_REQ_PER_FLUSH          128

static atomic_int       g_in_logging;
const unsigned int      CCassQuery::DEFAULT_PAGE_SIZE = 4096;

#if __cplusplus >= 201103L
static std::mutex       s_CassLoggingLock;
#endif

static void LogCallback(const CassLogMessage *  message, void *  data)
{
    g_in_logging++;
    try {
        if (data) {
            #if __cplusplus >= 201103L
            std::lock_guard<std::mutex>     lock(s_CassLoggingLock);
            #endif

            ofstream *      of = (ofstream*)data;
            stringstream    s;

            s << "[" << message->time_ms << "] "
              << message->severity << ": " << message->message << NcbiEndl;
            (*of) << s.str();
            of->flush();
        }
        g_in_logging--;
    } catch(...) {
        g_in_logging--;
        throw;
    }
}


/** CCassConnection */

ofstream    CCassConnection::m_logstrm;
string      CCassConnection::m_logfile;


CCassConnection::~CCassConnection()
{
    LOG4(("CCassConnection::~CCassConnection >>"));
    Close();
    LOG4(("CCassConnection::~CCassConnection <<"));
}


void CCassConnection::SetLogging(int  loglevel)
{
    bool    enabled = (loglevel >= 3) && m_logstrm.is_open();
    bool    debug_enabled = enabled && (loglevel >= 4);

    cass_log_set_callback(LogCallback,
                          enabled ? &CCassConnection::m_logstrm : nullptr);
    if (debug_enabled)
//      cass_log_set_level(CASS_LOG_DEBUG);
        cass_log_set_level(CASS_LOG_TRACE);
    else if (enabled)
        cass_log_set_level(CASS_LOG_CRITICAL);
    else
        cass_log_set_level(CASS_LOG_DISABLED);
    if (!enabled) {
        while(g_in_logging)
            sleep(1);
    }
}


shared_ptr<CCassConnection> CCassConnection::Create()
{
    return shared_ptr<CCassConnection>(new CCassConnection());
}


void CCassConnection::SetLoadBalancing(loadbalancing_policy_t  policy)
{
    m_loadbalancing = policy;
}


void CCassConnection::SetTokenAware(bool  value)
{
    m_tokenaware = value;
}


void CCassConnection::SetLatencyAware(bool  value)
{
    m_latencyaware = value;
}


atomic<CassUuidGen*>    CCassConnection::m_CassUuidGen(nullptr);


string CCassConnection::NewTimeUUID()
{
    char        buf[CASS_UUID_STRING_LENGTH];

    if (!m_CassUuidGen.load()) {
        CassUuidGen *gen, *nothing = nullptr;
        gen = cass_uuid_gen_new();
        if (!m_CassUuidGen.compare_exchange_weak(nothing, gen))
            cass_uuid_gen_free(gen);
    }

    CassUuid    uuid;
    cass_uuid_gen_time(static_cast<CassUuidGen*>(m_CassUuidGen.load()), &uuid);
    cass_uuid_string(uuid, buf);
    return string(buf);
}


void CCassConnection::Connect()
{

    if (IsConnected())
        RAISE_ERROR(eSeqFailed,
                    string("cassandra driver has already been connected"));
    if (m_host.empty())
        RAISE_ERROR(eSeqFailed, string("cassandra host list is empty"));

    UpdateLogging();
    m_cluster = cass_cluster_new();

//  cass_cluster_set_max_requests_per_flush(m_cluster, CORE_MAX_REQ_PER_FLUSH);

    cass_cluster_set_connect_timeout(m_cluster, m_ctimeoutms);
    cass_cluster_set_contact_points(m_cluster, m_host.c_str());
    if (m_port > 0)
        cass_cluster_set_port(m_cluster, m_port);
    if (m_qtimeoutms > 0)
        cass_cluster_set_request_timeout(m_cluster, m_qtimeoutms);

    if (!m_user.empty())
        cass_cluster_set_credentials(m_cluster, m_user.c_str(), m_pwd.c_str());

    if (m_loadbalancing != LB_DCAWARE) {
        if (m_loadbalancing == LB_ROUNDROBIN) {
            cass_cluster_set_load_balance_round_robin(m_cluster);
        }
    }

    cass_cluster_set_token_aware_routing(
            m_cluster, m_tokenaware ? cass_true : cass_false);
    cass_cluster_set_latency_aware_routing(
            m_cluster, m_latencyaware ? cass_true : cass_false);

    if (m_numThreadsIo)
        cass_cluster_set_num_threads_io(m_cluster, m_numThreadsIo);
    else
        cass_cluster_set_num_threads_io(m_cluster, IO_THREADS);

    if (m_maxConnPerHost) {
        cass_cluster_set_max_connections_per_host(m_cluster, m_maxConnPerHost);
        cass_cluster_set_pending_requests_low_water_mark(
                m_cluster, 64 * m_maxConnPerHost);
        cass_cluster_set_pending_requests_high_water_mark(
                m_cluster, 128 * m_maxConnPerHost);
    }

    if (m_numConnPerHost) {
        cass_cluster_set_core_connections_per_host(m_cluster, m_numConnPerHost);
    }

    if (m_keepalive > 0) {
        cass_cluster_set_tcp_keepalive(m_cluster, cass_true, m_keepalive);
    }

    Reconnect();
}


void CCassConnection::CloseSession()
{
    {
        CSpinGuard      guard(m_prepared_mux);
        NON_CONST_ITERATE(typename preparedlist_t, it, m_prepared) {
            if (it->second) {
                cass_prepared_free(it->second);
                it->second = nullptr;
            }
        }
        m_prepared.clear();
    }

    if (m_session) {
        CassFuture *    close_future;
        bool            free = false;

        close_future = cass_session_close(m_session);
        if (close_future) {
            free = cass_future_wait_timed(close_future,
                                          DISCONNECT_TIMEOUT_MS * 1000L);
            cass_future_free(close_future);
        }
        if (free) // otherwise we can't free it, let better leak than crash
            cass_session_free(m_session);
        m_session = nullptr;
    }
}


void CCassConnection::Reconnect()
{
    if (!m_cluster)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "driver is not connected, can't re-connect"));

    if (m_session)
        CloseSession();

    m_session = cass_session_new();
    if (!m_session)
        RAISE_DB_ERROR(eRsrcFailed, "failed to get cassandra session handle");

    CassFuture *    __future = cass_session_connect(m_session, m_cluster);
    if (!__future)
        RAISE_DB_ERROR(eRsrcFailed,
                       "failed to obtain cassandra connection future");

    unique_ptr<CassFuture,
               function<void(CassFuture*)> >    future(
            __future,
            [](CassFuture* future)
            {
                cass_future_free(future);
            });

    CassError       rc = CASS_OK;
    cass_future_wait(future.get());
    rc = cass_future_error_code(future.get());
    if (rc != CASS_OK)
        RAISE_CASS_CONN_ERROR(future.get(), string(""));

    if (!m_keyspace.empty()) {
        string _sav = m_keyspace;
        m_keyspace.clear();
        SetKeyspace(_sav);
    }
}


void CCassConnection::Close()
{
    LOG5(("CCassConnection::Close %p", LOG_CPTR(this)));
    CloseSession();
    if (m_cluster) {
        cass_cluster_free(m_cluster);
        m_cluster = nullptr;
    }
}


void CCassConnection::SetKeyspace(const string &  keyspace)
{
    if (m_keyspace != keyspace) {
        if (IsConnected()) {
            shared_ptr<CCassQuery>      query(NewQuery());
            query->SetSQL(string("USE ") + keyspace, 0);
            query->Execute(CASS_CONSISTENCY_LOCAL_QUORUM);
        }
        m_keyspace = keyspace;
    }
}


shared_ptr<CCassQuery> CCassConnection::NewQuery()
{
    if (!m_session)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "driver is not connected"));

    shared_ptr<CCassQuery> rv(new CCassQuery(shared_from_this()));
    rv->SetTimeout(m_qtimeoutms);
    return rv;
}


void CCassConnection::getTokenRanges(vector< pair<int64_t,int64_t> > &ranges)
{
    set<int64_t>                    cluster_tokens;
    map<string, vector<int64_t> >   peer_tokens;

    shared_ptr<CCassQuery>          query(NewQuery());
    query->SetSQL("select data_center, schema_version, host_id, tokens "
                  "from system.local", 0);
    query->Query(CassConsistency::CASS_CONSISTENCY_LOCAL_ONE, false, false);

    string              host_id,  datacenter, schema;
    vector<string>      tokens;

    query->NextRow();
    query->FieldGetStrValue(0, datacenter);
    query->FieldGetStrValue(1, schema);
    query->FieldGetStrValue(2, host_id);
    query->FieldGetSetValues(3, tokens);
    auto    itr = peer_tokens.insert(make_pair(host_id,
                                               vector<int64_t>())).first;
    for(const auto &  item: tokens) {
        int64_t         value = strtol(item.c_str(), nullptr, 10);
        itr->second.push_back(value);
        cluster_tokens.insert(value);
    }

    LOG3(("GET_TOKEN_MAP: Schema version is %s", schema.c_str()));
    LOG3(("GET_TOKEN_MAP: datacenter is %s", datacenter.c_str()));
    LOG3(("GET_TOKEN_MAP: host_id is %s", host_id.c_str()));

    unsigned int    query_host_count = 0;
    unsigned int    retries = 0;
    set<string>     query_hosts;
    while (retries < 3 &&
           (query_host_count == 0 || peer_tokens.size() == query_host_count)) {
        retries++;

        if (query_host_count != 0) {
            LOG3(("GET_TOKEN_MAP: Host_id count is too small. "
                  "Retrying system.peers fetch. %u", retries));
        }

        query_host_count = 0;
        query_hosts.clear();
        query->SetSQL("select host_id, data_center, schema_version, tokens "
                      "from system.peers", 0);
        query->Query(CassConsistency::CASS_CONSISTENCY_LOCAL_ONE, false, false);
        while (query->NextRow() == ar_dataready) {
            string              peer_host_id, peer_dc, peer_schema;
            vector<string>      tokens;
            query->FieldGetStrValue(1, peer_dc);
            query->FieldGetStrValue(2, peer_schema);
            if (datacenter == peer_dc && schema == peer_schema) {
                query->FieldGetStrValue(0, peer_host_id);
                if (query_hosts.find(peer_host_id) == query_hosts.end()) {
                    query_hosts.insert(peer_host_id);
                    query_host_count++;
                }
                query->FieldGetSetValues(3, tokens);
                LOG3(("GET_TOKEN_MAP: host is %s", peer_host_id.c_str()));
                LOG3(("GET_TOKEN_MAP: tokens %lu", tokens.size()));
                auto        itr = peer_tokens.find(peer_host_id);
                if (itr == peer_tokens.end()) {
                    itr = peer_tokens.insert(make_pair(peer_host_id,
                                                       vector<int64_t>())).first;
                    for(const auto &  item: tokens) {
                        int64_t     value = strtol(item.c_str(), nullptr, 10);
                        itr->second.push_back(value);
                        cluster_tokens.insert(value);
                    }
                }
            }
        }
        LOG3(("GET_TOKEN_MAP: PEERS HOST COUNT IS %lu", peer_tokens.size()));
        LOG3(("GET_TOKEN_MAP: QUERY HOST COUNT IS %u", query_host_count));
    }

    for(const auto& token: cluster_tokens) {
        LOG5(("GET_TOKEN_MAP: \ttoken %" PRId64, token));
    }

    LOG3(("GET_TOKEN_MAP: tokens size %lu", cluster_tokens.size()));

    ranges.reserve(cluster_tokens.size() + 1);

    int64_t         lower_bound = INT64_MIN;
    for (int64_t  token: cluster_tokens) {
        LOG5(("GET_TOKEN_MAP: token %" PRId64 " : %" PRId64,
              token, token - lower_bound));
        ranges.push_back(make_pair(lower_bound, token));
        lower_bound = token;
    }
    ranges.push_back(make_pair(lower_bound, INT64_MAX));
}


const CassPrepared *  CCassConnection::Prepare(const string &  sql)
{
    const CassPrepared *    rv = nullptr;

#if CASS_PREPARED_Q
    {
        CSpinGuard      guard(m_prepared_mux);
        auto            it = m_prepared.find(sql);
        if (it != m_prepared.end()) {
            rv = it->second;
            LOG5(("Prepared hit: sql: %s, %p", sql.c_str(), LOG_CPTR(rv)));
            return rv;
        }
    }

    CSpinGuard      guard(m_prepared_mux);
    auto            it = m_prepared.find(sql);
    if (it == m_prepared.end())
    {
        const char *    query = sql.c_str();
        CassFuture *    __future = cass_session_prepare(m_session, query);

        if (!__future)
            RAISE_DB_ERROR(eRsrcFailed,
                           string("failed to obtain cassandra query future"));

        unique_ptr<CassFuture,
                   function<void(CassFuture*)> >    future(
                           __future,
                           [](CassFuture* future)
                           {
                                cass_future_free(future);
                           });

        bool        b;
        b = cass_future_wait_timed(future.get(), m_qtimeoutms * 1000L);
        if (!b)
            RAISE_DB_QRY_TIMEOUT(m_qtimeoutms, 0,
                                 string("failed to prepare query \"") + sql + "\"");

        CassError   rc = cass_future_error_code(future.get());
        if (rc != CASS_OK) {
            string      msg = (rc == CASS_ERROR_SERVER_SYNTAX_ERROR ||
                               rc == CASS_ERROR_SERVER_INVALID_QUERY) ?
                                        string(", sql: ") + sql : string("");
            RAISE_CASS_QRY_ERROR(future.get(), msg);
        }

        rv = cass_future_get_prepared(future.get());
        if (!rv)
            RAISE_DB_ERROR(eRsrcFailed,
                           string("failed to obtain prepared handle for sql: ") + sql);
        LOG5(("Prepared miss: sql: %s, %p", sql.c_str(), LOG_CPTR(rv)));

        m_prepared.emplace(sql, rv);
    } else {
        rv = it->second;
        LOG5(("Prepared hit2: sql: %s, %p", sql.c_str(), LOG_CPTR(rv)));
        return rv;
    }
#endif
    return rv;
}


void CCassConnection::Perform(
                CAppOp &  op, unsigned int  optimeoutms,
                const std::function<bool()> &  PreLoopCB,
                const std::function<void(const CCassandraException&)> &  DbExceptCB,
                const std::function<bool(bool)> &  OpCB)
{
    int     err_cnt = 0;
    bool    is_repeated = false;
    bool    is_started = CAppPerf::Started(&op);

    if (!is_started)
        CAppPerf::StartOp(&op);

    while (!PreLoopCB || PreLoopCB()) {
        try {
            if (OpCB(is_repeated))
                break;
            err_cnt = 0;
        } catch (const CCassandraException &  e) {
            // log and ignore, app-specific layer is responsible for
            // re-connetion if needed
            CAppPerf::Relax(&op);
            if (DbExceptCB)
                DbExceptCB(e);

            if (e.GetErrCode() == CCassandraException::eQueryTimeout && ++err_cnt < 10) {
                LOG3(("CAPTURED TIMEOUT: %s, RESTARTING OP", e.TimeoutMsg().c_str()));
            } else if (e.GetErrCode() == CCassandraException::eQueryFailedRestartable) {
                LOG3(("CAPTURED RESTARTABLE EXCEPTION: %s, RESTARTING OP", e.what()));
            } else {
                // timer exceeded (10 times we got timeout and havn't read
                // anyting, or got another error -> try to reconnect
                CAppLog::Err1Post(string("2. CAPTURED ") + e.what());
                if (!is_started)
                    CAppPerf::StopOp(&op);
                throw;
            }

            unsigned int    t = CAppPerf::GetTime(&op) / 1000;
            if (optimeoutms != 0 && t > optimeoutms) {
                if (!is_started)
                    CAppPerf::StopOp(&op);
                throw;
            }
        } catch(...) {
            if (!is_started)
                CAppPerf::StopOp(&op);
            throw;
        }
        is_repeated = true;
    }
}

/** CCassPrm */

void CCassPrm::Bind(CassStatement *  statement, unsigned int  idx)
{
    CassError       rc = CASS_OK;

    switch (m_type) {
        case CASS_VALUE_TYPE_UNKNOWN:
            if (!IsAssigned())
                RAISE_ERROR(eSeqFailed,
                            string("invalid sequence of operations, Param #" +
                                   NStr::NumericToString(idx) +
                                   " is not assigned"));
            rc = cass_statement_bind_null(statement, idx);
            break;
        case CASS_VALUE_TYPE_INT:
            rc = cass_statement_bind_int32(statement, idx,
                                           (cass_int32_t) m_simpleval.i32);
            break;
        case CASS_VALUE_TYPE_BIGINT:
            rc = cass_statement_bind_int64(statement, idx,
                                           (cass_int64_t) m_simpleval.i64);
            break;

        /*
         * @saprykin
         * Removed silent binding null if string is empty. It creates a lot of
         * tombstones in storage. Sometimes if user wants to write empty string
         * it means that we should write empty string.
         * There is a proper method for writing nulls => CCassQuery::BindNull
         */
        case CASS_VALUE_TYPE_VARCHAR:
            rc = cass_statement_bind_string(statement, idx, m_bytes.c_str());
            break;
        case CASS_VALUE_TYPE_BLOB:
            if (m_bytes.size() > 0)
                rc = cass_statement_bind_bytes(
                    statement, idx,
                    reinterpret_cast<const unsigned char*>(m_bytes.c_str()),
                    m_bytes.size());
            else
                rc = cass_statement_bind_null(statement, idx);
            break;
        case CASS_VALUE_TYPE_LIST:
        case CASS_VALUE_TYPE_MAP:
            if (m_collection.get())
                rc = cass_statement_bind_collection(statement, idx,
                                                    m_collection.get());
            else
                rc = cass_statement_bind_null(statement, idx);
            break;
        case CASS_VALUE_TYPE_TUPLE:
            if (m_tuple.get())
                rc = cass_statement_bind_tuple(statement, idx, m_tuple.get());
            else
                rc = cass_statement_bind_null(statement, idx);
            break;
        default:
            RAISE_DB_ERROR(eBindFailed,
                           string("Bind for (") +
                           NStr::NumericToString(static_cast<int>(m_type)) +
                           ") type is not implemented");
    }
    if (rc != CASS_OK)
        RAISE_CASS_ERROR(rc, eBindFailed,
                         string("Bind for (") +
                         NStr::NumericToString(static_cast<int>(m_type)) +
                         ") failed with rc= " +
                         NStr::NumericToString(static_cast<int>(rc)));
}


/**  CCassQuery */

CCassQuery::~CCassQuery()
{
    Close();
    m_ondata_data = nullptr;
    m_ondata2_data = nullptr;
    m_onexecute_data = nullptr;
    LOG5(("CCassQuery::~CCassQuery this=%p", LOG_CPTR(this)));
}


void CCassQuery::InternalClose(bool  closebatch)
{
    LOG5(("CCassQuery::InternalClose: this: %p, fut: %p",
          LOG_CPTR(this), m_future));
    m_params.clear();
    if (m_future) {
        cass_future_free(m_future);
        --m_connection->m_active_statements;
        m_future = nullptr;
        m_futuretime = 0;
    }
    if (m_statement) {
        cass_statement_free(m_statement);
        m_statement = nullptr;
    }
    if (m_batch && closebatch) {
        cass_batch_free(m_batch);
        m_batch = nullptr;
    }
    if (m_iterator) {
        cass_iterator_free(m_iterator);
        m_iterator = nullptr;
    }
    if (m_result) {
        cass_result_free(m_result);
        m_result = nullptr;
    }
    /*
     * @saprykin
     * Commented out.
     * InternalClose is called from SetSQL and reseting SerialConsistency
     * level adds SetSQL very nasty side effect.
     * Consider not reusing CCassQuery for different types of requests.
     */
    // m_serial_consistency = CASS_CONSISTENCY_ANY;
    m_row = nullptr;
    m_page_start = false;
    m_page_size = 0;
    m_results_expected = false;
    m_async = false;
    m_allow_prepare = CASS_PREPARED_Q;
    m_is_prepared = false;

    if (m_cb_ref) {
        LOG5(("CCassQuery::InternalClose: cb_ref detach, this: %p, cb_ref: %p",
              LOG_CPTR(this), m_cb_ref.get()));
        m_cb_ref->Detach();
        m_cb_ref = nullptr;
    }
}


void CCassQuery::SetSQL(const string &  sql, unsigned int  PrmCount)
{
    InternalClose(false);
    m_sql = sql;
    m_params.resize(PrmCount);
}


void CCassQuery::Bind()
{
    if (!m_statement)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, query is closed"));

    int     cnt = 0;
    for (auto &  it: m_params) {
        it.Bind(m_statement, cnt);
        cnt++;
    }
}


void CCassQuery::BindNull(int  iprm)
{
    if (iprm < 0 || (unsigned int)iprm >= m_params.size())
        RAISE_DB_ERROR(eBindFailed, string("Param index is out of range"));
    m_params[iprm].AssignNull();
}


void CCassQuery::BindInt32(int  iprm, int32_t  value)
{
    if (iprm < 0 || (unsigned int)iprm >= m_params.size())
        RAISE_DB_ERROR(eBindFailed, string("Param index is out of range"));
    m_params[iprm].Assign(value);
}


void CCassQuery::BindInt64(int  iprm, int64_t  value)
{
    if (iprm < 0 || (unsigned int)iprm >= m_params.size())
        RAISE_DB_ERROR(eBindFailed, string("Param index is out of range"));
    m_params[iprm].Assign(value);
}


void CCassQuery::BindStr(int  iprm, const string &  value)
{
    if (iprm < 0 || (unsigned int)iprm >= m_params.size())
        RAISE_DB_ERROR(eBindFailed, string("Param index is out of range"));
    m_params[iprm].Assign(value);
}


void CCassQuery::BindStr(int  iprm, const char *  value)
{
    if (iprm < 0 || (unsigned int)iprm >= m_params.size())
        RAISE_DB_ERROR(eBindFailed, string("Param index is out of range"));
    m_params[iprm].Assign(value);
}


void CCassQuery::BindBytes(int  iprm, const unsigned char *  buf, size_t  len)
{
    if (iprm < 0 || (unsigned int)iprm >= m_params.size())
        RAISE_DB_ERROR(eBindFailed, string("Param index is out of range"));
    m_params[iprm].Assign(buf, len);
}


int32_t CCassQuery::ParamAsInt32(int  iprm)
{
    if (iprm < 0 || (unsigned int)iprm >= m_params.size())
        RAISE_DB_ERROR(eBindFailed, string("Param index is out of range"));
    if (!m_params[iprm].IsAssigned())
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, Param #" +
                    NStr::NumericToString(iprm) + " is not assigned"));
    return m_params[iprm].AsInt32();
}


int64_t CCassQuery::ParamAsInt64(int  iprm)
{
    if (iprm < 0 || (unsigned int)iprm >= m_params.size())
        RAISE_DB_ERROR(eBindFailed, string("Param index is out of range"));
    if (!m_params[iprm].IsAssigned())
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, Param #" +
                    NStr::NumericToString(iprm) + " is not assigned"));
    return m_params[iprm].AsInt64();
}


string CCassQuery::ParamAsStr(int  iprm)
{
    string      rv;
    if (iprm < 0 || (unsigned int)iprm >= m_params.size())
        RAISE_DB_ERROR(eBindFailed, string("Param index is out of range"));
    if (!m_params[iprm].IsAssigned())
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, Param #" +
                    NStr::NumericToString(iprm) + " is not assigned"));
    m_params[iprm].AsString(rv);
    return rv;
}


void CCassQuery::ParamAsStr(int  iprm, string &  value)
{
    if (iprm < 0 || (unsigned int)iprm >= m_params.size())
        RAISE_DB_ERROR(eBindFailed, string("Param index is out of range"));
    if (!m_params[iprm].IsAssigned())
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, Param #" +
                    NStr::NumericToString(iprm) + " is not assigned"));
    m_params[iprm].AsString(value);
}


void CCassQuery::SetEOF(bool  value)
{
    if (m_EOF != value) {
        m_EOF = value;
    }
}

void CCassQuery::NewBatch()
{
    if (!m_connection)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "DB connection closed"));
    if (m_batch)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "batch has already been started"));
    if (IsActive())
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "query is active"));

    m_batch = cass_batch_new(CASS_BATCH_TYPE_LOGGED);
    if (!m_batch)
        RAISE_DB_ERROR(eRsrcFailed, string("failed to create batch"));
}


void CCassQuery::Query(CassConsistency  c, bool  run_async,
                       bool  allow_prepared, unsigned int  page_size)
{
    LOG5(("CCassQuery::Query this: %p", LOG_CPTR(this)));
    if (!m_connection)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "DB connection closed"));
    if (m_sql.empty())
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, SQL is not set"));
    if (m_batch)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "can't run select in batch mode"));
    if (IsActive())
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, Query is active"));

    const CassPrepared *    prepared = nullptr;
    if (allow_prepared) {
        prepared = m_connection->Prepare(m_sql);
        m_is_prepared = prepared != nullptr;
    }

    if (m_is_prepared)
        m_statement = cass_prepared_bind(prepared);
    else
        m_statement = cass_statement_new(m_sql.c_str(), m_params.size());

    if (!m_statement)
        RAISE_DB_ERROR(eRsrcFailed, string("failed to create cassandra query"));

    try {
        CassError   rc;
        Bind();
        rc = cass_statement_set_consistency(m_statement, c);
        if (rc != CASS_OK)
            RAISE_CASS_ERROR(rc, eQueryFailed,
                             string("Failed to set consistency level ") +
                             NStr::NumericToString(static_cast<int>(c)));
        if (m_serial_consistency != CASS_CONSISTENCY_ANY) {
            rc = cass_statement_set_serial_consistency(m_statement,
                                                       m_serial_consistency);
            if (rc != CASS_OK)
                RAISE_CASS_ERROR(rc, eQueryFailed,
                                 string("Failed to set serial consistency level ") +
                                 NStr::NumericToString(static_cast<int>(m_serial_consistency)));
        }
        if (page_size > 0) {
            rc = cass_statement_set_paging_size(m_statement, page_size);
            if (rc != CASS_OK)
                RAISE_CASS_ERROR(rc, eQueryFailed,
                                 string("Failed to set page size to ") +
                                 NStr::NumericToString(static_cast<int>(page_size)));
        }

        m_page_start = true;
        m_page_size = page_size;
        SetEOF(false);
        m_results_expected = true;
        m_async = run_async;
        if (!m_batch) {
            if (run_async) {
                GetFuture();
            } else {
                Wait(m_qtimeoutms * 1000L);
            }
        }
    } catch(...) {
        Close();
        throw;
    }
}


void CCassQuery::Execute(CassConsistency  c, bool  run_async,
                         bool  allow_prepared)
{

    if (!m_connection)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "DB connection closed"));
    if (m_sql.empty()) 
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, SQL is not set"));
    if (m_row != nullptr || m_statement != nullptr)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, Query is active"));

    const CassPrepared *    prepared = nullptr;
    if (allow_prepared) {
        prepared = m_connection->Prepare(m_sql);
        m_is_prepared = prepared != nullptr;
    }

    if (m_is_prepared)
        m_statement = cass_prepared_bind(prepared);
    else
        m_statement = cass_statement_new(m_sql.c_str(), m_params.size());

    if (!m_statement)
        RAISE_DB_ERROR(eRsrcFailed, string("failed to create cassandra query"));

    try {
        CassError       rc;
        Bind();
        rc = cass_statement_set_consistency(m_statement, c);
        if (rc != CASS_OK)
            RAISE_CASS_ERROR(rc, eQueryFailed,
                             string("Failed to set consistency level ") +
                             NStr::NumericToString(static_cast<int>(c)));
        if (m_serial_consistency != CASS_CONSISTENCY_ANY) {
            rc = cass_statement_set_serial_consistency(m_statement,
                                                       m_serial_consistency);
            if (rc != CASS_OK)
                RAISE_CASS_ERROR(rc, eQueryFailed,
                                 string("Failed to set serial consistency level ") +
                                 NStr::NumericToString(static_cast<int>(m_serial_consistency)));
        }

        if (m_batch) {
            CassError   rc = cass_batch_add_statement(m_batch, m_statement);
            if (rc != CASS_OK)
                RAISE_CASS_ERROR(rc, eQueryFailed,
                                 string("Failed to add statement to batch, sql: ") +
                                 m_sql);
            cass_statement_free(m_statement);
            m_statement = nullptr;
        }

        m_page_start = false;
        m_page_size = 0;
        SetEOF(false);
        m_results_expected = false;
        m_async = run_async;

        if (!m_batch) {
            if (run_async) {
                GetFuture();
            } else {
                Wait(m_qtimeoutms * 1000L);
            }
        }
    } catch(...) {
        if (m_statement) {
            cass_statement_free(m_statement);
            m_statement = nullptr;
        }
        throw;
    }
}


void CCassQuery::SetSerialConsistency(CassConsistency  c)
{
    m_serial_consistency = c;
}


async_rslt_t  CCassQuery::RunBatch()
{
    async_rslt_t rv = ar_wait;
    if (!m_batch)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "batch is not created"));

    if (m_async) {
        GetFuture();
    } else {
        rv = Wait(m_qtimeoutms * 1000L);
        if (m_batch) {
            cass_batch_free(m_batch);
            m_batch = nullptr;
        }
    }
    return rv;
}


async_rslt_t CCassQuery::WaitAsync(unsigned int  timeoutmks)
{
    if (!m_async)
        RAISE_ERROR(eSeqFailed,
                    string("attempt to wait on query in non-async state"));
    return Wait(timeoutmks);
}


bool CCassQuery::IsReady()
{
    GetFuture();
    return cass_future_ready(m_future) == cass_true;
}


void CCassQuery::GetFuture()
{
    if (!m_connection)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "DB connection closed"));
    if (!IsActive() && !m_batch)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "Query is not active"));
    if (m_iterator || m_result)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "results already obtained"));

    if (!m_future) {
        ++m_connection->m_active_statements;
        if (m_batch)
            m_future = cass_session_execute_batch(m_connection->m_session,
                                                  m_batch);
        else
            m_future = cass_session_execute(m_connection->m_session,
                                            m_statement);
        if (!m_future) {
            --m_connection->m_active_statements;
            RAISE_DB_ERROR(eRsrcFailed,
                           string("failed to obtain cassandra query future"));
        }
        m_futuretime = gettime();
        LOG5(("CCassQuery::GetFuture: this: %p, fut: %p",
              LOG_CPTR(this), m_future));

        if (m_ondata || m_ondata2) {
            SetupOnDataCallback();
        }
    }
}


async_rslt_t  CCassQuery::Wait(unsigned int  timeoutmks)
{
    LOG5(("CCassQuery::Wait: this: %p, fut: %p", LOG_CPTR(this), m_future));
    if (m_results_expected && m_result) {
        if (m_async)
            return ar_dataready;
        RAISE_ERROR(eSeqFailed, string("result has already been allocated"));
    }

    if (m_EOF && m_results_expected)
        return ar_done;

    GetFuture();

    bool        rv;
    if (timeoutmks != 0) {
        rv = cass_future_wait_timed(m_future, timeoutmks);
    } else if (!m_async) {
        cass_future_wait(m_future);
        rv = true;
    } else {
        rv = cass_future_ready(m_future);
    }

    if (!rv) {
        if (!m_async && timeoutmks > 0) {
            int64_t     t = (gettime() - m_futuretime) / 1000L;
            RAISE_DB_QRY_TIMEOUT(t, timeoutmks / 1000L,
                                 string("failed to perform query \"") +
                                 m_sql + "\"");
        } else
            return ar_wait;
    } else {
        ProcessFutureResult();
        if ((m_statement || m_batch) && !m_results_expected)
            // next request was run in m_ondata event handler
            return ar_wait;
        else if (m_EOF || !m_results_expected)
            return ar_done;
        else if (m_result)
            return ar_dataready;
        else
            return ar_wait;
    }
}


void CCassQuery::SetupOnDataCallback()
{
    if (!m_future)
        RAISE_ERROR(eSeqFailed, string("Future is not assigned"));
    if (!m_ondata && !m_ondata2)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "m_ondata is not set"));
    LOG5(("CCassQuery::SetupOnDataCallback: this: %p, fut: %p, ondata: %p, "
          "ondata2: %p", LOG_CPTR(this), m_future, m_ondata, m_ondata2));

    if (m_cb_ref)
        m_cb_ref->Detach();

    m_cb_ref.reset(new CCassQueryCbRef(shared_from_this()));
    m_cb_ref->Attach(m_ondata, m_ondata_data, m_ondata2, m_ondata2_data);

    CassError       rv = cass_future_set_callback(m_future,
                                                  CCassQueryCbRef::s_OnFutureCb,
                                                  m_cb_ref.get());
    if (rv != CASS_OK) {
        m_cb_ref->Detach(true);
        m_cb_ref = nullptr;
        RAISE_ERROR(eSeqFailed, string("failed to assign future callback"));
    }
}


void CCassQuery::ProcessFutureResult()
{
    if (!m_future)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "m_future is not set"));
    if (m_iterator || m_result)
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "results already obtained"));

    CassError   rc = cass_future_error_code(m_future);
    if (rc != CASS_OK) {
        string      msg = (rc == CASS_ERROR_SERVER_SYNTAX_ERROR ||
                           rc == CASS_ERROR_SERVER_INVALID_QUERY) ?
                                    string(", sql: ") + m_sql : string("");
        RAISE_CASS_QRY_ERROR(m_future, msg);
    }

    if (m_results_expected) {
        m_result = cass_future_get_result(m_future);
        if (!m_result)
            RAISE_DB_ERROR(eRsrcFailed,
                           string("failed to obtain cassandra query result"));
        if (m_iterator)
            RAISE_ERROR(eSeqFailed,
                        string("iterator has already been allocated"));

        m_iterator = cass_iterator_from_result(m_result);
        if (!m_iterator)
            RAISE_DB_ERROR(eRsrcFailed,
                           string("failed to obtain cassandra query iterator"));
        /*
            we release m_future here for statements that return dataset,
            otherwise we free it in the destructor, keeping m_future not null
            as an indication that we have already waited for query to finish
        */
        LOG5(("CCassQuery::ProcessFutureResult: release future this: %p, fut: %p",
              LOG_CPTR(this), m_future));
        cass_future_free(m_future);
        --m_connection->m_active_statements;
        m_future = nullptr;
        if (m_cb_ref) {
            LOG5(("CCassQuery::ProcessFutureResult: cb_ref detach, this: %p, cb_ref: %p",
                  LOG_CPTR(this), m_cb_ref.get()));
            m_cb_ref->Detach();
            m_cb_ref = nullptr;
        }
        m_futuretime = 0;
        m_page_start = false;
    } else {
        if (m_statement) {
            cass_statement_free(m_statement);
            m_statement = nullptr;
        }
        if (m_batch) {
            cass_batch_free(m_batch);
            m_batch = nullptr;
        }
        SetEOF(true);
        if (m_onexecute)
            m_onexecute(*this, m_onexecute_data);
    }
}


async_rslt_t  CCassQuery::NextRow()
{
    LOG5(("CCassQuery::NextRow: this: %p, fut: %p, m_row: %p, m_result: %p, "
          "m_iterator: %p", LOG_CPTR(this), m_future, m_row, m_result, m_iterator));

    if (!IsActive())
        RAISE_ERROR(eSeqFailed,
                    string("invalid sequence of operations, "
                           "Query is not active"));

    while(1) {
        m_row = nullptr;
        if (m_result == nullptr) {
            async_rslt_t wr;
            if (m_async) {
                wr = Wait(0);
            } else {
                wr = Wait(m_qtimeoutms * 1000L);
            }

            if (wr != ar_dataready) {
                LOG4(("CCassQuery::NextRow: async=%d returning wr=%d b'ze of Wait",
                      m_async, static_cast<int>(wr)));
                return wr;
            }
        }

        if (m_iterator && m_result) {
            bool    b = cass_iterator_next(m_iterator);
            if (!b) {
                LOG5(("CCassQuery::NextRow: this: %p, cass_iterator_next "
                      "returned false", LOG_CPTR(this)));
                if (m_page_size > 0) {
                    bool    has_more_pages = cass_result_has_more_pages(m_result);
                    if (has_more_pages) {
                        CassError err;
                        if ((err = cass_statement_set_paging_state(m_statement,
                                                                   m_result)) != CASS_OK)
                            RAISE_CASS_ERROR(err, eFetchFailed,
                                             string("failed to retrive next page"));
                    }

                    cass_iterator_free(m_iterator);
                    m_iterator = nullptr;
                    cass_result_free(m_result);
                    m_result = nullptr;

                    if (!has_more_pages) {
                        SetEOF(true);
                        LOG5(("CCassQuery::NextRow: this: %p, async=%d "
                              "returning wr=ar_done b'ze has_more_pages==false",
                              LOG_CPTR(this), m_async));
                        return ar_done;
                    }
                    // go to above
                    m_page_start = true; 
                } else {
                    SetEOF(true);
                    LOG5(("CCassQuery::NextRow: this: %p, async=%d returning "
                          "wr=ar_done b'ze m_page_size=%d",
                          LOG_CPTR(this), m_async, m_page_size));
                    return ar_done;
                }
            } else {
                m_row = cass_iterator_get_row(m_iterator);
                if (!m_row)
                    RAISE_DB_ERROR(eRsrcFailed,
                                   string("failed to obtain "
                                          "cassandra query result row"));
                return ar_dataready;
            }
        } else
            RAISE_ERROR(eSeqFailed,
                        string("invalid sequence of operations, "
                               "attempt to fetch next row on a closed query"));
    }
}


template<>
const CassValue *  CCassQuery::GetColumn(int  ifld)
{
    if (!m_row)
        RAISE_ERROR(eSeqFailed, string("query row is not fetched"));

    const CassValue *   clm = cass_row_get_column(m_row, ifld);
    if (!clm)
        RAISE_ERROR(eSeqFailed,
                    string("column is not fetched (index ") +
                    NStr::NumericToString(ifld) + " beyound the range?)");
    return clm;
}


template<>
const CassValue *  CCassQuery::GetColumn(const string &  name)
{
    if (!m_row)
        RAISE_ERROR(eSeqFailed, string("query row is not fetched"));

    const CassValue *   clm = cass_row_get_column_by_name_n(m_row,
                                                            name.c_str(),
                                                            name.size());
    if (!clm)
        RAISE_ERROR(eSeqFailed, string("column ") + name + " is not available");
    return clm;
}


template<>
const CassValue *  CCassQuery::GetColumn(const char *  name)
{
    if (!m_row)
        RAISE_ERROR(eSeqFailed, string("query row is not fetched"));

    const CassValue *   clm = cass_row_get_column_by_name(m_row, name);
    if (!clm)
        RAISE_ERROR(eSeqFailed, string("column ") + name + " is not available");
    return clm;
}


const CassValue *  CCassQuery::GetTupleIteratorValue(CassIterator *  itr)
{
    if (!m_row)
        RAISE_ERROR(eSeqFailed, string("query row is not fetched"));
    if (!cass_iterator_next(itr))
        RAISE_ERROR(eSeqFailed, string("cass tuple iterator next failed"));

    const CassValue *   value = cass_iterator_get_value(itr);
    if (!value)
        RAISE_ERROR(eSeqFailed, string("cass tuple iterator fetch failed"));
    return value;
}


string CCassQuery::ToString()
{
    return m_sql.empty() ? "<>" : m_sql;
}


/* Value conversion routines */

template<>
bool CassValueConvert<bool>(const CassValue *  Val)
{
    if (!Val)
        RAISE_DB_ERROR(eFetchFailed,
                       string("failed to fetch bool: cass value is NULL"));

    CassError           err;
    CassValueType       type = cass_value_type(Val);
    cass_bool_t         cb;
    bool                rv = false;
    switch (type) {
        case CASS_VALUE_TYPE_BOOLEAN:
            if ((err = cass_value_get_bool(Val, &cb)) != CASS_OK)
                RAISE_CASS_ERROR(err, eFetchFailed,
                                 string("failed to fetch boolean data"));
            rv = cb;
            break;
        default:
            if (type == CASS_VALUE_TYPE_UNKNOWN &&
                (!Val || cass_value_is_null(Val)))
                return false;
            RAISE_DB_ERROR(eFetchFailed,
                           string("failed to convert to bool: unsupported (") +
                           NStr::NumericToString(static_cast<int>(type)) +
                           ") data type");
    }
    return rv;
}


template<>
int32_t CassValueConvert<int32_t>(const CassValue *  Val)
{
    if (!Val)
        RAISE_DB_ERROR(eFetchFailed,
                       string("failed to fetch int32: cass value is NULL"));

    int32_t             rv = 0;
    CassError           err;
    CassValueType       type = cass_value_type(Val);
    switch (type) {
        case CASS_VALUE_TYPE_TIMESTAMP:
        case CASS_VALUE_TYPE_BIGINT:
        case CASS_VALUE_TYPE_COUNTER: {
            int64_t     rv64;
            if ((err = cass_value_get_int64(Val, &rv64)) != CASS_OK)
                RAISE_CASS_ERROR(err, eFetchFailed,
                                 string("failed to fetch int32 data"));
            if (rv64 < INT_MIN || rv64 > INT_MAX)
                RAISE_DB_ERROR(eFetchFailed,
                               string("failed to fetch int32 data, "
                                      "fetched value is out of range"));
            rv = rv64;
            break;
        }
        case CASS_VALUE_TYPE_INT:
            if ((err = cass_value_get_int32(Val, &rv)) != CASS_OK)
                RAISE_CASS_ERROR(err, eFetchFailed,
                                 string("failed to fetch int data"));
            break;
        default:
            if (type == CASS_VALUE_TYPE_UNKNOWN &&
                (!Val || cass_value_is_null(Val)))
                return 0;
            RAISE_DB_ERROR(eFetchFailed,
                           string("failed to convert to int32: unsupported (") +
                           NStr::NumericToString(static_cast<int>(type)) +
                           ") data type");
    }
    return rv;
}


template<>
int64_t CassValueConvert<int64_t>(const CassValue *  Val)
{
    if (!Val)
        RAISE_DB_ERROR(eFetchFailed,
                       string("failed to fetch int64: cass value is NULL"));

    CassError           err;
    int64_t             rv = 0;
    CassValueType       type = cass_value_type(Val);
    switch (type) {
        case CASS_VALUE_TYPE_TIMESTAMP:
        case CASS_VALUE_TYPE_BIGINT:
        case CASS_VALUE_TYPE_COUNTER:
            if ((err = cass_value_get_int64(Val, &rv)) != CASS_OK)
                RAISE_CASS_ERROR(err, eFetchFailed,
                                 string("failed to fetch int64 data"));
            break;
        case CASS_VALUE_TYPE_INT: {
            int32_t         rv32;
            if ((err = cass_value_get_int32(Val, &rv32)) != CASS_OK)
                RAISE_CASS_ERROR(err, eFetchFailed,
                                 string("failed to fetch int data"));
            rv = rv32;
            break;
        }
        default:
            if (type == CASS_VALUE_TYPE_UNKNOWN &&
                (!Val || cass_value_is_null(Val)))
                return 0;
            RAISE_DB_ERROR(eFetchFailed,
                           string("failed to convert to int64: unsupported (") +
                           NStr::NumericToString(static_cast<int>(type)) +
                           ") data type");
    }
    return rv;
}


template<>
double CassValueConvert<double>(const CassValue *  Val)
{
    if (!Val)
        RAISE_DB_ERROR(eFetchFailed,
                       string("failed to fetch float: cass value is NULL"));

    CassError           err;
    cass_double_t       rv = 0;
    if ((err = cass_value_get_double(Val, &rv)) != CASS_OK)
        RAISE_CASS_ERROR(err, eFetchFailed,
                         string("failed to convert to float"));
    return rv;
}


template<>
string CassValueConvert<string>(const CassValue *  Val)
{
    if (!Val)
        RAISE_DB_ERROR(eFetchFailed,
                       string("failed to fetch string: cass value is NULL"));

    CassError           err;
    string              value;
    CassValueType       type = cass_value_type(Val);
    switch (type) {
        case CASS_VALUE_TYPE_BOOLEAN:
        case CASS_VALUE_TYPE_ASCII:
        case CASS_VALUE_TYPE_BLOB:
        case CASS_VALUE_TYPE_TEXT:
        case CASS_VALUE_TYPE_VARCHAR:
        case CASS_VALUE_TYPE_CUSTOM: {
            const char *    rv = nullptr;
            size_t          len = 0;

            if ((err = cass_value_get_string(Val, &rv, &len)) != CASS_OK)
                RAISE_CASS_ERROR(err, eFetchFailed,
                                 string("failed to fetch string data"));
            value.assign(rv, len);
            break;
        }
        case CASS_VALUE_TYPE_TIMESTAMP: {
            int64_t     v = CassValueConvert<int64_t>(Val);
            CTime       c(v / 1000);

            c.SetMilliSecond(v % 1000);
            value = c;
            break;
        }
        case CASS_VALUE_TYPE_TIMEUUID:
        case CASS_VALUE_TYPE_UUID: {
            CassUuid    fetched_value;
            char        fetched_buffer[CASS_UUID_STRING_LENGTH];

            cass_value_get_uuid(Val, &fetched_value);
            cass_uuid_string(fetched_value, fetched_buffer);
            value = string(fetched_buffer);
            break;
        }
        default:
            if (type == CASS_VALUE_TYPE_UNKNOWN &&
                (!Val || cass_value_is_null(Val))) {
                return "";
            }
            RAISE_DB_ERROR(eFetchFailed,
                           string("failed to convert to string: unsupported (") +
                           NStr::NumericToString(static_cast<int>(type)) +
                           ") data type");
    }
    return value;
}


template<>
void CassDataToCollection<bool>(CassCollection *  coll, const bool &  v)
{
    CassError       err;
    cass_bool_t     b = v ? cass_true : cass_false;
    if ((err = cass_collection_append_bool(coll, b)) != CASS_OK)
        RAISE_CASS_ERROR(err, eBindFailed,
                         string("failed to bind boolean data to collection"));
}


template<>
void CassDataToCollection<int32_t>(CassCollection *  coll, const int32_t &  v)
{
    CassError       err;
    if ((err = cass_collection_append_int32(coll, v)) != CASS_OK)
        RAISE_CASS_ERROR(err, eBindFailed,
                         string("failed to bind int32 data to collection"));
}


template<>
void CassDataToCollection<int64_t>(CassCollection *  coll, const int64_t &  v)
{
    CassError       err;
    if ((err = cass_collection_append_int64(coll, v)) != CASS_OK)
        RAISE_CASS_ERROR(err, eBindFailed,
                         string("failed to bind int64 data to collection"));
}

template<>
void CassDataToCollection<double>(CassCollection *  coll, const double &  v)
{
    CassError       err;
    if ((err = cass_collection_append_double(coll, v)) != CASS_OK)
        RAISE_CASS_ERROR(err, eBindFailed,
                         string("failed to bind double data to collection"));
}

template<>
void CassDataToCollection<float>(CassCollection *  coll, const float &  v)
{
    CassError       err;
    if ((err = cass_collection_append_float(coll, v)) != CASS_OK)
        RAISE_CASS_ERROR(err, eBindFailed,
                         string("failed to bind float data to collection"));
}


template<>
void CassDataToCollection<string>(CassCollection *  coll, const string &  v)
{
    CassError err;
    if ((err = cass_collection_append_string(coll, v.c_str())) != CASS_OK)
        RAISE_CASS_ERROR(err, eBindFailed,
                         string("failed to bind string data to collection"));
}


END_IDBLOB_SCOPE
