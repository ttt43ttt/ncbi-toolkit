#ifndef NETSCHEDULE_DB_DUMP__HPP
#define NETSCHEDULE_DB_DUMP__HPP

/*  $Id: ns_db_dump.hpp 551762 2017-11-22 14:16:14Z satskyse $
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
 * File Description:
 *   NetSchedule database dumping support.
 *
 */

#include <corelib/ncbitype.h>

#include <stdio.h>
#include "ns_db.hpp"


// NB
// The dump format is extendable and the extensions will work in both
// directions: forward and backward. The only requirement to keep it working is
// to extend the structures at the end without touching the existing part.
// In this case:
// - if a newer format is provided the older format dump (new NS starts after
// old NS saved the data) then: the extended fields will have zeroes.
// - if an older format is provided the newer data (old NS starts after a new
// NS, e.g. due to a rollback) then: the extended values are stripped.



BEGIN_NCBI_SCOPE


// Common header for the dump files
#pragma pack(push, 1)
struct SCommonDumpHeader
{
    Int4        magic;
    Uint4       storage_ver_major;
    Uint4       storage_ver_minor;
    Uint4       storage_ver_patch;

    Uint4       server_ver_major;
    Uint4       server_ver_minor;
    Uint4       server_ver_patch;

    SCommonDumpHeader();
};
#pragma pack(pop)


// The header appears ones in the job info file
#pragma pack(push, 1)
struct SJobDumpHeader
{
    SCommonDumpHeader   common_header;
    Uint4               job_props_fixed_size;
    Uint4               job_io_fixed_size;
    Uint4               job_event_fixed_size;

    SJobDumpHeader();

    void Write(FILE *  f);
    int Read(FILE *  f);
};
#pragma pack(pop)


// A few dump files store a sequence of one structure. They will use the header
// below
#pragma pack(push, 1)
struct SOneStructDumpHeader
{
    SCommonDumpHeader   common_header;
    Uint4               fixed_size;

    SOneStructDumpHeader();

    void Write(FILE *  f);
    int Read(FILE *  f);
};
#pragma pack(pop)


// The structures mostly match the Berkeley DB tables structure
// but use only fixed size POD fields

// It is supposed that the progress message follows each instance of the
// structure below. The progress message is the largest (and rare used) field
// in the structure and it affects the dumping performance and disk usage.
#pragma pack(push, 1)
struct SJobDump
{
    Uint4               total_size;
    Uint4               id;

    Uint4               passport;
    Int4                status;
    double              timeout;
    double              run_timeout;
    double              read_timeout;

    Uint4               subm_notif_port;
    double              subm_notif_timeout;

    Uint4               listener_notif_addr;
    Uint4               listener_notif_port;
    double              listener_notif_abstime;
    bool                need_subm_progress_msg_notif;
    bool                need_lsnr_progress_msg_notif;
    bool                need_stolen_notif;

    Uint4               run_counter;
    Uint4               read_counter;

    Uint4               aff_id;
    Uint4               mask;

    Uint4               group_id;

    double              last_touch;
    Uint4               progress_msg_size;
    Uint4               number_of_events;

    Uint4               client_ip_size;
    Uint4               client_sid_size;
    Uint4               ncbi_phid_size;
    char                client_ip[kMaxClientIpSize];
    char                client_sid[kMaxSessionIdSize];
    char                ncbi_phid[kMaxHitIdSize];

    SJobDump();

    void Write(FILE *  f, const char *  progress_msg);
    int Read(FILE *  f, size_t  fixed_size_from_header,
             char *  progress_msg);
};
#pragma pack(pop)


// It is supposed that the input and/or output follow each instance of the
// structure below. The limit of the input/output is too large to have
// it as a member of this structure
#pragma pack(push, 1)
struct SJobIODump
{
    Uint4           total_size;
    Uint4           input_size;
    Uint4           output_size;

    SJobIODump();

    void Write(FILE *  f, const char *  input, const char *  output);
    int Read(FILE *  f, size_t  fixed_size_from_header,
             char *  input, char *  output);
};
#pragma pack(pop)


// It is supposed that the client_node, client_session and err_msg follow
// each instance of the structure below. There are a lot of the job events
// and otherwise it leads to both dumping speed and disk usage problems.
#pragma pack(push, 1)
struct SJobEventsDump
{
    Uint4               total_size;
    Uint4               event;
    Int4                status;
    double              timestamp;
    Uint4               node_addr;
    Int4                ret_code;
    Uint4               client_node_size;
    Uint4               client_session_size;
    Uint4               err_msg_size;

    SJobEventsDump();

    void Write(FILE *  f, const char *  client_node,
                          const char *  client_session,
                          const char *  err_msg);
    int Read(FILE *  f, size_t  fixed_size_from_header,
             char *  client_node,
             char *  client_session,
             char *  err_msg);
};
#pragma pack(pop)


#pragma pack(push, 1)
struct SAffinityDictDump
{
    Uint4               total_size;
    Uint4               aff_id;
    Uint4               token_size;
    char                token[kNetScheduleMaxDBDataSize];

    SAffinityDictDump();

    void Write(FILE *  f);
    int Read(FILE *  f, size_t  fixed_size_from_header);
};
#pragma pack(pop)


#pragma pack(push, 1)
struct SGroupDictDump
{
    Uint4               total_size;
    Uint4               group_id;
    Uint4               token_size;
    char                token[kNetScheduleMaxDBDataSize];

    SGroupDictDump();

    void Write(FILE *  f);
    int Read(FILE *  f, size_t  fixed_size_from_header);
};
#pragma pack(pop)


#pragma pack(push, 1)
struct SQueueDescriptionDump
{
    Uint4               total_size;
    bool                is_queue;
    Uint4               qname_size;
    char                qname[kMaxQueueNameSize];
    Uint4               qclass_size;
    char                qclass[kMaxQueueNameSize];
    double              timeout;
    double              notif_hifreq_interval;
    double              notif_hifreq_period;
    Uint4               notif_lofreq_mult;
    double              notif_handicap;
    Uint4               dump_buffer_size;
    Uint4               dump_client_buffer_size;
    Uint4               dump_aff_buffer_size;
    Uint4               dump_group_buffer_size;
    double              run_timeout;
    double              read_timeout;
    Uint4               program_name_size;
    char                program_name[kMaxQueueLimitsSize];
    Uint4               failed_retries;
    Uint4               read_failed_retries;
    double              blacklist_time;
    double              read_blacklist_time;
    Uint4               max_input_size;
    Uint4               max_output_size;
    Uint4               subm_hosts_size;
    char                subm_hosts[kMaxQueueLimitsSize];
    Uint4               wnode_hosts_size;
    char                wnode_hosts[kMaxQueueLimitsSize];
    Uint4               reader_hosts_size;
    char                reader_hosts[kMaxQueueLimitsSize];
    double              wnode_timeout;
    double              reader_timeout;
    double              pending_timeout;
    double              max_pending_wait_timeout;
    double              max_pending_read_wait_timeout;
    Uint4               description_size;
    char                description[kMaxDescriptionSize];
    bool                scramble_job_keys;
    double              client_registry_timeout_worker_node;
    Uint4               client_registry_min_worker_nodes;
    double              client_registry_timeout_admin;
    Uint4               client_registry_min_admins;
    double              client_registry_timeout_submitter;
    Uint4               client_registry_min_submitters;
    double              client_registry_timeout_reader;
    Uint4               client_registry_min_readers;
    double              client_registry_timeout_unknown;
    Uint4               client_registry_min_unknowns;

    Uint4               linked_section_prefixes_size;
    char                linked_section_prefixes[kLinkedSectionsList];
    Uint4               linked_section_names_size;
    char                linked_section_names[kLinkedSectionsList];

    SQueueDescriptionDump();

    void Write(FILE *  f);
    int Read(FILE *  f, size_t  fixed_size_from_header);
};
#pragma pack(pop)


#pragma pack(push, 1)
struct SLinkedSectionDump
{
    Uint4       total_size;
    Uint4       section_size;
    Uint4       value_name_size;
    Uint4       value_size;
    char        section[kLinkedSectionValueNameSize];
    char        value_name[kLinkedSectionValueNameSize];
    char        value[kLinkedSectionValueSize];

    SLinkedSectionDump();

    void Write(FILE *  f);
    int Read(FILE *  f, size_t  fixed_size_from_header);
};
#pragma pack(pop)


END_NCBI_SCOPE

#endif /* NETSCHEDULE_DB_DUMP__HPP */
