#ifndef DBAPI_DRIVER_FTDS64_IMPL___RENAME_FTDS_TDS__H
#define DBAPI_DRIVER_FTDS64_IMPL___RENAME_FTDS_TDS__H

/*  $Id: rename_ftds_tds.h 487470 2015-12-17 19:23:15Z ucko $
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
 * Author:  Denis Vakatov
 *
 * File Description:
 *   Macros to rename FreeTDS TDS symbols -- to avoid their clashing
 *   with other versions'
 *
 */

#define generate_random_buffer          generate_random_buffer_ver64
#define hmac_md5                        hmac_md5_ver64
#define hmac_md5_final                  hmac_md5_final_ver64
#define hmac_md5_init_limK_to_64        hmac_md5_init_limK_to_64_ver64
#define hmac_md5_init_rfc2104           hmac_md5_init_rfc2104_ver64
#define hmac_md5_update                 hmac_md5_update_ver64
#define tds7_crypt_pass                 tds7_crypt_pass_ver64
#define tds7_get_instance_port          tds7_get_instance_port_ver64
#define tds7_send_auth                  tds7_send_auth_ver64
#define tds7_srv_charset_changed        tds7_srv_charset_changed_ver64
#define tds_add_row_column_size         tds_add_row_column_size_ver64
#define tds_alloc_bcp_column_data       tds_alloc_bcp_column_data_ver64
#define tds_alloc_client_sqlstate       tds_alloc_client_sqlstate_ver64
#define tds_alloc_compute_results       tds_alloc_compute_results_ver64
#define tds_alloc_compute_row           tds_alloc_compute_row_ver64
#define tds_alloc_connection            tds_alloc_connection_ver64
#define tds_alloc_context               tds_alloc_context_ver64
#define tds_alloc_cursor                tds_alloc_cursor_ver64
#define tds_alloc_dynamic               tds_alloc_dynamic_ver64
#define tds_alloc_locale                tds_alloc_locale_ver64
#define tds_alloc_login                 tds_alloc_login_ver64
#define tds_alloc_lookup_sqlstate       tds_alloc_lookup_sqlstate_ver64
#define tds_alloc_param_result          tds_alloc_param_result_ver64
#define tds_alloc_param_row             tds_alloc_param_row_ver64
#define tds_alloc_results               tds_alloc_results_ver64
#define tds_alloc_row                   tds_alloc_row_ver64
#define tds_alloc_socket                tds_alloc_socket_ver64
#define tds_answer_challenge            tds_answer_challenge_ver64
#define tds_canonical_charset_name      tds_canonical_charset_name_ver64
#define tds_check_column_extra          tds_check_column_extra_ver64
#define tds_check_context_extra         tds_check_context_extra_ver64
#define tds_check_cursor_extra          tds_check_cursor_extra_ver64
#define tds_check_dynamic_extra         tds_check_dynamic_extra_ver64
#define tds_check_env_extra             tds_check_env_extra_ver64
#define tds_check_resultinfo_extra      tds_check_resultinfo_extra_ver64
#define tds_check_tds_extra             tds_check_tds_extra_ver64
#define tds_client_msg                  tds_client_msg_ver64
#define tds_close_socket                tds_close_socket_ver64
#define tds_config_verstr               tds_config_verstr_ver64
#define tds_connect                     tds_connect_ver64
#define tds_convert                     tds_convert_ver64
#define tds_count_placeholders          tds_count_placeholders_ver64
#define tds_cursor_close                tds_cursor_close_ver64
#define tds_cursor_dealloc              tds_cursor_dealloc_ver64
#define tds_cursor_deallocated          tds_cursor_deallocated_ver64
#define tds_cursor_declare              tds_cursor_declare_ver64
#define tds_cursor_fetch                tds_cursor_fetch_ver64
#define tds_cursor_fetch_095            tds_cursor_fetch_095_ver64
#define tds_cursor_open                 tds_cursor_open_ver64
#define tds_cursor_setname              tds_cursor_setname_ver64
#define tds_cursor_setrows              tds_cursor_setrows_ver64
#define tds_cursor_update               tds_cursor_update_ver64
#define tds_datecrack                   tds_datecrack_ver64
#define tds_debug_flags                 tds_debug_flags_ver64
#define tds_des_ecb_encrypt             tds_des_ecb_encrypt_ver64
#define tds_des_encrypt                 tds_des_encrypt_ver64
#define tds_des_set_key                 tds_des_set_key_ver64
#define tds_des_set_odd_parity          tds_des_set_odd_parity_ver64
#define tds_dstr_copy                   tds_dstr_copy_ver64
#define tds_dstr_copyn                  tds_dstr_copyn_ver64
#define tds_dstr_free                   tds_dstr_free_ver64
#if ENABLE_EXTRA_CHECKS
#  define tds_dstr_cstr                   tds_dstr_cstr_ver64
#  define tds_dstr_init                   tds_dstr_init_ver64
#  define tds_dstr_isempty                tds_dstr_isempty_ver64
#  define tds_dstr_len                    tds_dstr_len_ver64
#endif
#define tds_dstr_set                    tds_dstr_set_ver64
#define tds_dstr_zero                   tds_dstr_zero_ver64
#define tds_fix_connection              tds_fix_connection_ver64
#define tds_flush_packet                tds_flush_packet_ver64
#define tds_free_all_results            tds_free_all_results_ver64
#define tds_free_bcp_column_data        tds_free_bcp_column_data_ver64
#define tds_free_connection             tds_free_connection_ver64
#define tds_free_context                tds_free_context_ver64
#define tds_free_cursor                 tds_free_cursor_ver64
#define tds_free_dynamic                tds_free_dynamic_ver64
#define tds_free_input_params           tds_free_input_params_ver64
#define tds_free_locale                 tds_free_locale_ver64
#define tds_free_login                  tds_free_login_ver64
#define tds_free_msg                    tds_free_msg_ver64
#define tds_free_param_result           tds_free_param_result_ver64
#define tds_free_param_results          tds_free_param_results_ver64
#define tds_free_results                tds_free_results_ver64
#define tds_free_row                    tds_free_row_ver64
#define tds_free_socket                 tds_free_socket_ver64
#define tds_g_append_mode               tds_g_append_mode_ver64
#define tds_get_byte                    tds_get_byte_ver64
#define tds_get_cardinal_type           tds_get_cardinal_type_ver64
#define tds_get_char_data               tds_get_char_data_ver64
#define tds_get_compiletime_settings    tds_get_compiletime_settings_ver64
#define tds_get_conversion_type         tds_get_conversion_type_ver64
#define tds_get_dynid                   tds_get_dynid_ver64
#define tds_get_homedir                 tds_get_homedir_ver64
#define tds_get_int                     tds_get_int_ver64
#define tds_get_locale                  tds_get_locale_ver64
#define tds_get_n                       tds_get_n_ver64
#define tds_get_null_type               tds_get_null_type_ver64
#define tds_get_parent                  tds_get_parent_ver64
#define tds_get_size_by_type            tds_get_size_by_type_ver64
#define tds_get_smallint                tds_get_smallint_ver64
#define tds_get_string                  tds_get_string_ver64
#define tds_get_token_size              tds_get_token_size_ver64
#define tds_get_varint_size             tds_get_varint_size_ver64
#define tds_gethostbyname_r             tds_gethostbyname_r_ver64
#define tds_getmac                      tds_getmac_ver64
#define tds_getservbyname_r             tds_getservbyname_r_ver64
#define tds_gss_get_auth                tds_gss_get_auth_ver64
#define tds_hex_digits                  tds_hex_digits_ver64
#define tds_iconv                       tds_iconv_ver64
#define tds_iconv_alloc                 tds_iconv_alloc_ver64
#define tds_iconv_close                 tds_iconv_close_ver64
#define tds_iconv_fread                 tds_iconv_fread_ver64
#define tds_iconv_free                  tds_iconv_free_ver64
#define tds_iconv_from_collate          tds_iconv_from_collate_ver64
#define tds_iconv_open                  tds_iconv_open_ver64
#define tds_inet_ntoa_r                 tds_inet_ntoa_r_ver64
#define tds_init_write_buf              tds_init_write_buf_ver64
#define tds_lookup_dynamic              tds_lookup_dynamic_ver64
#define tds_lookup_host                 tds_lookup_host_ver64
#define tds_money_to_string             tds_money_to_string_ver64
#define tds_multiple_done               tds_multiple_done_ver64
#define tds_multiple_execute            tds_multiple_execute_ver64
#define tds_multiple_init               tds_multiple_init_ver64
#define tds_multiple_query              tds_multiple_query_ver64
#define tds_next_placeholder            tds_next_placeholder_ver64
#define tds_numeric_bytes_per_prec      tds_numeric_bytes_per_prec_ver64
#define tds_numeric_change_prec_scale   tds_numeric_change_prec_scale_ver64
#define tds_numeric_to_string           tds_numeric_to_string_ver64
#define tds_open_socket                 tds_open_socket_ver64
#define tds_peek                        tds_peek_ver64
#define tds_process_cancel              tds_process_cancel_ver64
#define tds_process_login_tokens        tds_process_login_tokens_ver64
#define tds_process_simple_query        tds_process_simple_query_ver64
#define tds_process_tokens              tds_process_tokens_ver64
#define tds_prtype                      tds_prtype_ver64
#define tds_put_buf                     tds_put_buf_ver64
#define tds_put_byte                    tds_put_byte_ver64
#define tds_put_int                     tds_put_int_ver64
#define tds_put_int8                    tds_put_int8_ver64
#define tds_put_n                       tds_put_n_ver64
#define tds_put_smallint                tds_put_smallint_ver64
#define tds_put_string                  tds_put_string_ver64
#define tds_quote_id                    tds_quote_id_ver64
#define tds_quote_string                tds_quote_string_ver64
#define tds_read_conf_file              tds_read_conf_file_ver64
#define tds_read_conf_section           tds_read_conf_section_ver64
#define tds_read_config_info            tds_read_config_info_ver64
#define tds_read_packet                 tds_read_packet_ver64
#define tds_realloc_socket              tds_realloc_socket_ver64
#define tds_release_cursor              tds_release_cursor_ver64
#define tds_send_cancel                 tds_send_cancel_ver64
#define tds_set_app                     tds_set_app_ver64
#define tds_set_bulk                    tds_set_bulk_ver64
#define tds_set_capabilities            tds_set_capabilities_ver64
#define tds_set_client_charset          tds_set_client_charset_ver64
#define tds_set_column_type             tds_set_column_type_ver64
#define tds_set_host                    tds_set_host_ver64
#define tds_set_interfaces_file_loc     tds_set_interfaces_file_loc_ver64
#define tds_set_language                tds_set_language_ver64
#define tds_set_library                 tds_set_library_ver64
#define tds_set_packet                  tds_set_packet_ver64
#define tds_set_param_type              tds_set_param_type_ver64
#define tds_set_parent                  tds_set_parent_ver64
#define tds_set_passwd                  tds_set_passwd_ver64
#define tds_set_port                    tds_set_port_ver64
#define tds_set_server                  tds_set_server_ver64
#define tds_set_server_addr             tds_set_server_addr_ver64
#define tds_set_state                   tds_set_state_ver64
#define tds_set_user                    tds_set_user_ver64
#define tds_set_version                 tds_set_version_ver64
#define tds_skip_quoted                 tds_skip_quoted_ver64
#define tds_srv_charset_changed         tds_srv_charset_changed_ver64
#ifdef NCBI_FTDS_ALLOW_TDS_80
#define tds_ssl_deinit                  tds_ssl_deinit_ver64
#define tds_ssl_init                    tds_ssl_init_ver64
#endif
#define tds_str_empty                   tds_str_empty_ver64
#define tds_strftime                    tds_strftime_ver64
#define tds_submit_execdirect           tds_submit_execdirect_ver64
#define tds_submit_execute              tds_submit_execute_ver64
#define tds_submit_optioncmd            tds_submit_optioncmd_ver64
#define tds_submit_prepare              tds_submit_prepare_ver64
#define tds_submit_query                tds_submit_query_ver64
#define tds_submit_query_params         tds_submit_query_params_ver64
#define tds_submit_queryf               tds_submit_queryf_ver64
#define tds_submit_rpc                  tds_submit_rpc_ver64
#define tds_submit_unprepare            tds_submit_unprepare_ver64
#define tds_swap_bytes                  tds_swap_bytes_ver64
#define tds_swap_datatype               tds_swap_datatype_ver64
#define tds_swap_numeric                tds_swap_numeric_ver64
#define tds_sybase_charset_name         tds_sybase_charset_name_ver64
#define tds_timestamp_str               tds_timestamp_str_ver64
#define tds_unget_byte                  tds_unget_byte_ver64
#define tds_version                     tds_version_ver64
#define tds_vstrbuild                   tds_vstrbuild_ver64
#define tds_willconvert                 tds_willconvert_ver64
#define tds_write_packet                tds_write_packet_ver64
#define tdsdump_close                   tdsdump_close_ver64
#define tdsdump_dump_buf                tdsdump_dump_buf_ver64
#define tdsdump_log                     tdsdump_log_ver64
#define tdsdump_off                     tdsdump_off_ver64
#define tdsdump_state                   tdsdump_state_ver64
#define tdsdump_on                      tdsdump_on_ver64
#define tdsdump_open                    tdsdump_open_ver64

#endif  /* DBAPI_DRIVER_FTDS64_IMPL___RENAME_FTDS_TDS__H */
