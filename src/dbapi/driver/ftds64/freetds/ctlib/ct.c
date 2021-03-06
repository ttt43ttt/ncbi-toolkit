/* FreeTDS - Library of routines accessing Sybase and Microsoft databases
 * Copyright (C) 1998, 1999, 2000, 2001  Brian Bruns
 * Copyright (C) 2002, 2003, 2004, 2005  James K. Lowden
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <assert.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include "ctpublic.h"
#include "ctlib.h"
#include "tdsstring.h"
#include "replacements.h"

TDS_RCSID(var, "$Id: ct.c 487448 2015-12-17 18:43:08Z ucko $");


static char * ct_describe_cmd_state(CS_INT state);
/**
 * Read a row of data
 * @return 0 on success
 */
static int _ct_fetch_cursor(CS_COMMAND * cmd, CS_INT type, CS_INT offset, CS_INT option, CS_INT * rows_read);
int _ct_get_client_type(CS_CONTEXT *ctx, int datatype, int usertype, int size);
static int _ct_fetchable_results(CS_COMMAND * cmd);
static int _ct_process_return_status(TDSSOCKET * tds);

static int _ct_fill_param(CS_INT cmd_type, CS_PARAM * param, CS_DATAFMT * datafmt, CS_VOID * data,
              CS_INT * datalen, CS_SMALLINT * indicator, CS_BYTE byvalue);
void _ctclient_msg(CS_CONNECTION * con, const char *funcname, int layer, int origin, int severity, int number,
              const char *fmt, ...);
int _ct_bind_data(CS_CONTEXT *ctx, TDSRESULTINFO * resinfo, TDSRESULTINFO *bindinfo, CS_INT offset);
static void _ct_initialise_cmd(CS_COMMAND *cmd);
static CS_RETCODE _ct_cancel_cleanup(CS_COMMAND * cmd);
static CS_RETCODE _ct_cmd_drop(CS_COMMAND * cmd, CS_INT free_conn_ref);

/* Added for CT_DIAG */
/* Code changes starts here - CT_DIAG - 01 */

static CS_INT ct_diag_storeclientmsg(CS_CONTEXT * context, CS_CONNECTION * conn, CS_CLIENTMSG * message);
static CS_INT ct_diag_storeservermsg(CS_CONTEXT * context, CS_CONNECTION * conn, CS_SERVERMSG * message);
static CS_INT ct_diag_countmsg(CS_CONTEXT * context, CS_INT type, CS_INT * count);
static CS_INT ct_diag_getclientmsg(CS_CONTEXT * context, CS_INT idx, CS_CLIENTMSG * message);
static CS_INT ct_diag_getservermsg(CS_CONTEXT * context, CS_INT idx, CS_SERVERMSG * message);

/* Code changes ends here - CT_DIAG - 01 */

/* Added code for RPC functionality -SUHA */
/* RPC Code changes starts here */

static void rpc_clear(CSREMOTE_PROC * rpc);
static void param_clear(CSREMOTE_PROC_PARAM * pparam);

static TDSPARAMINFO *paraminfoalloc(TDSSOCKET * tds, CS_PARAM * first_param);

static CS_DYNAMIC * _ct_allocate_dynamic(CS_CONNECTION * con, char *id, int idlen);
static CS_INT  _ct_deallocate_dynamic(CS_CONNECTION * con, CS_DYNAMIC *dyn);
static CS_DYNAMIC * _ct_locate_dynamic(CS_CONNECTION * con, char *id, int idlen);

/* RPC Code changes ends here */

void _csclient_msg(CS_CONTEXT * ctx, const char *funcname, int layer, int origin, int severity, int number, const char *fmt, ...);


static const char *
_ct_get_layer(int layer)
{
    switch (layer) {
    case 1:
        return "user api layer";
        break;
    case 2:
        return "blk layer";
        break;
    default:
        break;
    }
    return "unrecognized layer";
}

static const char *
_ct_get_origin(int origin)
{
    switch (origin) {
    case 1:
        return "external error";
        break;
    case 2:
        return "internal CT-Library error";
        break;
    case 4:
        return "common library error";
        break;
    case 5:
        return "intl library error";
        break;
    case 6:
        return "user error";
        break;
    case 7:
        return "internal BLK-Library error";
        break;
    default:
        break;
    }
    return "unrecognized origin";
}

static const char *
_ct_get_user_api_layer_error(int error)
{
    switch (error) {
    case 137:
        return "A bind count of %1! is not consistent with the count supplied for existing binds. The current bind count is %2!.";
        break;
    case 138:
        return "Use direction CS_BLK_IN or CS_BLK_OUT for a bulk copy operation.";
        break;
    case 139:
        return "The parameter tblname cannot be NULL.";
        break;
    case 140:
        return "Failed when processing results from server.";
        break;
    case 141:
        return "Parameter %1! has an illegal value of %2!";
        break;
    case 142:
        return "No value or default value available and NULL not allowed. col = %1! row = %2! .";
        break;
    case 143:
        return "parameter name(s) must be supplied for LANGUAGE command.";
        break;
    case 16843163:
        return "This routine cannot be called when the command structure is idle.";
        break;
    case 16843164:
        return "Cannot send anything else but Text/Image for column %1! when server datatype is Text/Image";
        break;
    default:
        break;
    }
    return "unrecognized error";
}

static char *
_ct_get_msgstr(const char *funcname, int layer, int origin, int severity, int number)
{
    char *m;

    if (asprintf(&m,
             "%s: %s: %s: %s", funcname, _ct_get_layer(layer), _ct_get_origin(origin), _ct_get_user_api_layer_error(number)
        ) < 0) {
        return NULL;
    }
    return m;
}

void
_ctclient_msg(CS_CONNECTION * con, const char *funcname, int layer, int origin, int severity, int number, const char *fmt, ...)
{
    CS_CONTEXT *ctx = con->ctx;
    va_list ap;
    CS_CLIENTMSG cm;
    char *msgstr;

    va_start(ap, fmt);

    if (ctx->_clientmsg_cb) {
        cm.severity = severity;
        cm.msgnumber = (((layer << 24) & 0xFF000000)
                | ((origin << 16) & 0x00FF0000)
                | ((severity << 8) & 0x0000FF00)
                | ((number) & 0x000000FF));
        msgstr = _ct_get_msgstr(funcname, layer, origin, severity, number);
        tds_vstrbuild(cm.msgstring, CS_MAX_MSG, &(cm.msgstringlen), msgstr, CS_NULLTERM, fmt, CS_NULLTERM, ap);
        cm.msgstring[cm.msgstringlen] = '\0';
        free(msgstr);
        cm.osnumber = 0;
        cm.osstring[0] = '\0';
        cm.osstringlen = 0;
        cm.status = 0;
        /* cm.sqlstate */
        cm.sqlstatelen = 0;
        ctx->_clientmsg_cb(ctx, con, &cm);
    }

    va_end(ap);
}

static CS_RETCODE
ct_set_command_state(CS_COMMAND *cmd, CS_INT state) {

    tdsdump_log(TDS_DBG_FUNC, "setting command state from %s to %s\n",
            ct_describe_cmd_state(cmd->command_state),
            ct_describe_cmd_state(state));

    cmd->command_state = state;

    return CS_SUCCEED;
}

static char *
ct_describe_cmd_state(CS_INT state) {
    switch (state) {
    case _CS_COMMAND_IDLE:
        return "IDLE";
    case _CS_COMMAND_BUILDING:
        return "BUILDING";
    case _CS_COMMAND_READY:
        return "READY";
    case _CS_COMMAND_SENT:
        return "SENT";
    }
    return "???";
}

CS_RETCODE
ct_exit(CS_CONTEXT * ctx, CS_INT unused)
{
    tdsdump_log(TDS_DBG_FUNC, "ct_exit()\n");
    return CS_SUCCEED;
}

CS_RETCODE
ct_init(CS_CONTEXT * ctx, CS_INT version)
{
    /* uncomment the next line to get pre-login trace */
    /* tdsdump_open("/tmp/tds2.log"); */
    tdsdump_log(TDS_DBG_FUNC, "ct_init()\n");
    ctx->tds_ctx->msg_handler = _ct_handle_server_message;
    ctx->tds_ctx->err_handler = _ct_handle_client_message;

    return CS_SUCCEED;
}

CS_RETCODE
ct_con_alloc(CS_CONTEXT * ctx, CS_CONNECTION ** con)
{
    TDSLOGIN *login;

    tdsdump_log(TDS_DBG_FUNC, "ct_con_alloc()\n");
    login = tds_alloc_login();
    if (!login)
        return CS_FAIL;
    *con = (CS_CONNECTION *) malloc(sizeof(CS_CONNECTION));
    if (!*con) {
        tds_free_login(login);
        return CS_FAIL;
    }
    memset(*con, '\0', sizeof(CS_CONNECTION));
    (*con)->tds_login = login;

    /* so we know who we belong to */
    (*con)->ctx = ctx;

    /* set default values */
    tds_set_library((*con)->tds_login, "CT-Library");
    /* tds_set_client_charset((*con)->tds_login, "iso_1"); */
    /* tds_set_packet((*con)->tds_login, TDS_DEF_BLKSZ); */
    return CS_SUCCEED;
}

CS_RETCODE
ct_callback(CS_CONTEXT * ctx, CS_CONNECTION * con, CS_INT action, CS_INT type, CS_VOID * func)
{
    int (*funcptr) (void *, void *, void *) = (int (*)(void *, void *, void *)) func;

    tdsdump_log(TDS_DBG_FUNC, "ct_callback() action = %s\n", CS_GET ? "CS_GET" : "CS_SET");
    /* one of these has to be defined */
    if (!ctx && !con)
        return CS_FAIL;

    if (action == CS_GET) {
        switch (type) {
        case CS_CLIENTMSG_CB:
            *(void **) func = (CS_VOID *) (con ? con->_clientmsg_cb : ctx->_clientmsg_cb);
            return CS_SUCCEED;
        case CS_SERVERMSG_CB:
            *(void **) func = (CS_VOID *) (con ? con->_servermsg_cb : ctx->_servermsg_cb);
            return CS_SUCCEED;
        default:
            _csclient_msg(ctx, "ct_callback", 2, 1, 16, 27, "%d", type);
            *(void **) func = NULL;
            return CS_SUCCEED;
        }
    }
    /* CS_SET */
    switch (type) {
    case CS_CLIENTMSG_CB:
        if (con)
            con->_clientmsg_cb = (CS_CLIENTMSG_FUNC) funcptr;
        else
            ctx->_clientmsg_cb = (CS_CLIENTMSG_FUNC) funcptr;
        break;
    case CS_SERVERMSG_CB:
        if (con)
            con->_servermsg_cb = (CS_SERVERMSG_FUNC) funcptr;
        else
            ctx->_servermsg_cb = (CS_SERVERMSG_FUNC) funcptr;
        break;
    }
    return CS_SUCCEED;
}

CS_RETCODE
ct_con_props(CS_CONNECTION * con, CS_INT action, CS_INT property, CS_VOID * buffer, CS_INT buflen, CS_INT * out_len)
{
    CS_INT intval = 0;
    size_t maxcp;
    TDSSOCKET *tds;
    TDSLOGIN *tds_login;
    char *set_buffer = NULL;

    tdsdump_log(TDS_DBG_FUNC, "ct_con_props() action = %s property = %d\n", CS_GET ? "CS_GET" : "CS_SET", property);

    tds = con->tds_socket;
    tds_login = con->tds_login;

    if (action == CS_SET) {
        if (property == CS_USERNAME || property == CS_PASSWORD || property == CS_APPNAME ||
            property == CS_HOSTNAME || property == CS_SERVERADDR) {
            if (buflen == CS_NULLTERM) {
                maxcp = strlen((char *) buffer);
                set_buffer = (char *) malloc(maxcp + 1);
                strcpy(set_buffer, (char *) buffer);
            } else if (buflen == CS_UNUSED) {
                return CS_SUCCEED;
            } else {
                set_buffer = (char *) malloc(buflen + 1);
                strncpy(set_buffer, (char *) buffer, buflen);
                set_buffer[buflen] = '\0';
            }
        }

        /*
         * XXX "login" properties shouldn't be set after
         * login.  I don't know if it should fail silently
         * or return an error.
         */
        switch (property) {
        case CS_USERNAME:
            tds_set_user(tds_login, set_buffer);
            break;
        case CS_PASSWORD:
            tds_set_passwd(tds_login, set_buffer);
            break;
        case CS_APPNAME:
            tds_set_app(tds_login, set_buffer);
            break;
        case CS_HOSTNAME:
            tds_set_host(tds_login, set_buffer);
            break;
        case CS_PORT:
            tds_set_port(tds_login, *((int *) buffer));
            break;
        case CS_SERVERADDR: {
            /* Format of this property: "[hostname] [port]" */
            char *host, *port;
            int portno;
            host= strtok(set_buffer, " ");
            port= strtok(NULL, " ");
            if (host == NULL  ||  port == NULL) {
                free (set_buffer);
                return CS_FAIL;
            }

            portno= (int)strtol(port, NULL, 10);
            tds_set_server_addr(tds_login, host);
            tds_set_port(tds_login, portno);
            break;
        }
        case CS_LOC_PROP:
            con->locale = (CS_LOCALE *) buffer;
            break;
        case CS_USERDATA:
            if (con->userdata)
                free(con->userdata);
            con->userdata = (void *) malloc(buflen + 1);
            tdsdump_log(TDS_DBG_INFO2, "setting userdata orig %p new %p\n", buffer, con->userdata);
            con->userdata_len = buflen;
            memcpy(con->userdata, buffer, buflen);
            break;
        case CS_BULK_LOGIN:
            memcpy(&intval, buffer, sizeof(intval));
            if (intval)
                tds_set_bulk(tds_login, 1);
            else
                tds_set_bulk(tds_login, 0);
            break;
        case CS_PACKETSIZE:
            memcpy(&intval, buffer, sizeof(intval));
            tds_set_packet(tds_login, (short) intval);
            break;
        case CS_TDS_VERSION:
            /*
             * FIXME
             * (a) We don't support all versions in tds/login.c -
             *     I tried to pick reasonable versions.
             * (b) Might need support outside of tds/login.c
             * (c) It's a "negotiated" property so probably
             *     needs tds_process_env_chg() support
             * (d) Minor - we don't check against context
             *     which should limit the acceptable values
             */
            if (*(int *) buffer == 0) {
                /* autodetect version */
                tds_set_version(tds_login, 0, 0);
            } else if (*(int *) buffer == CS_TDS_40) {
                tds_set_version(tds_login, 4, 2);
            } else if (*(int *) buffer == CS_TDS_42) {
                tds_set_version(tds_login, 4, 2);
            } else if (*(int *) buffer == CS_TDS_46) {
                tds_set_version(tds_login, 4, 6);
            } else if (*(int *) buffer == CS_TDS_495) {
                tds_set_version(tds_login, 4, 6);
            } else if (*(int *) buffer == CS_TDS_50) {
                tds_set_version(tds_login, 5, 0);
            } else if (*(int *) buffer == CS_TDS_70) {
                tds_set_version(tds_login, 7, 0);
            } else if (*(int *) buffer == CS_TDS_80) {
                tds_set_version(tds_login, 8, 0);
            } else {
                return CS_FAIL;
            }
            break;
        default:
            tdsdump_log(TDS_DBG_ERROR, "Unknown property %d\n", property);
            break;
        }
        if (set_buffer)
            free(set_buffer);
    } else if (action == CS_GET) {
        switch (property) {
        case CS_USERNAME:
            if (out_len)
                *out_len = tds_dstr_len(&tds_login->user_name);
            tds_strlcpy((char *) buffer, tds_dstr_cstr(&tds_login->user_name), buflen);
            break;
        case CS_PASSWORD:
            if (out_len)
                *out_len = tds_dstr_len(&tds_login->password);
            tds_strlcpy((char *) buffer, tds_dstr_cstr(&tds_login->password), buflen);
            break;
        case CS_APPNAME:
            if (out_len)
                *out_len = tds_dstr_len(&tds_login->app_name);
            tds_strlcpy((char *) buffer, tds_dstr_cstr(&tds_login->app_name), buflen);
            break;
        case CS_HOSTNAME:
            if (out_len)
                *out_len = tds_dstr_len(&tds_login->client_host_name);
            tds_strlcpy((char *) buffer, tds_dstr_cstr(&tds_login->client_host_name), buflen);
            break;
        case CS_SERVERNAME:
            if (out_len)
                *out_len = tds_dstr_len(&tds_login->server_name);
            tds_strlcpy((char *) buffer, tds_dstr_cstr(&tds_login->server_name), buflen);
            break;
        case CS_LOC_PROP:
            buffer = (CS_VOID *) con->locale;
            break;
        case CS_USERDATA:
            tdsdump_log(TDS_DBG_INFO2, "fetching userdata %p\n", con->userdata);
            maxcp = con->userdata_len;
            if (out_len)
                *out_len = maxcp;
            if (maxcp > buflen)
                maxcp = buflen;
            memcpy(buffer, con->userdata, maxcp);
            break;
        case CS_CON_STATUS:
            if (!(IS_TDSDEAD(tds)))
                intval |= CS_CONSTAT_CONNECTED;
            else
                intval &= ~CS_CONSTAT_CONNECTED;
            if (tds && tds->state == TDS_DEAD)
                intval |= CS_CONSTAT_DEAD;
            else
                intval &= ~CS_CONSTAT_DEAD;
            memcpy(buffer, &intval, sizeof(intval));
            break;
        case CS_BULK_LOGIN:
            if (tds_login->bulk_copy)
                intval = CS_FALSE;
            else
                intval = CS_TRUE;
            memcpy(buffer, &intval, sizeof(intval));
            break;
        case CS_PACKETSIZE:
            if (tds)
                intval = tds->env.block_size;
            else
                intval = tds_login->block_size;
            memcpy(buffer, &intval, sizeof(intval));
            if (out_len)
                *out_len = sizeof(intval);
            break;
        case CS_TDS_VERSION:
            switch (tds_version(tds, NULL)) {
            case 40:
                (*(int *) buffer = CS_TDS_40);
                break;
            case 42:
                (*(int *) buffer = CS_TDS_42);
                break;
            case 46:
                (*(int *) buffer = CS_TDS_46);
                break;
            case 40 + 95:
                (*(int *) buffer = CS_TDS_495);
                break;
            case 50:
                (*(int *) buffer = CS_TDS_50);
                break;
            case 70:
                (*(int *) buffer = CS_TDS_70);
                break;
            case 80:
                (*(int *) buffer = CS_TDS_80);
                break;
            default:
                return CS_FAIL;
            }
            break;
        case CS_PARENT_HANDLE:
            *(CS_CONTEXT **) buffer = con->ctx;
            break;

        default:
            tdsdump_log(TDS_DBG_ERROR, "Unknown property %d\n", property);
            break;
        }
    }
    return CS_SUCCEED;
}

static int
ctlib_query_timeout(void *param, unsigned int total_timeout)
{
    CS_CONNECTION* con = (CS_CONNECTION*) param;

    if (!con) {
        return TDS_INT_CONTINUE;
    } else if (con->tds_socket->state == TDS_QUERYING  &&  !con->cmds) {
        /* Timeout during login, presumably */
        return TDS_INT_CANCEL;
    }

    /* tds_set_state(con->tds_socket, TDS_PENDING); */
    tds_set_state(con->tds_socket, TDS_QUERYING);

    /* tds_process_cancel(con->tds_socket); */
    ct_cancel(con, NULL, CS_CANCEL_ATTN);

    return TDS_INT_CANCEL;
}

CS_RETCODE
ct_connect(CS_CONNECTION * con, CS_CHAR * servername, CS_INT snamelen)
{
    char *server;
    int needfree = 0;
    CS_CONTEXT *ctx;
    TDSCONNECTION *connection;

    tdsdump_log(TDS_DBG_FUNC, "ct_connect() servername = %s\n", servername ? servername : "NULL");

    if (!tds_dstr_isempty(&con->tds_login->server_addr)) {
        server = tds_dstr_cstr(&con->tds_login->server_addr);
    } else if (servername == NULL || snamelen == 0 || snamelen == CS_UNUSED) {
        server = NULL;
    } else if (snamelen == CS_NULLTERM) {
        server = (char *) servername;
    } else {
        server = (char *) malloc(snamelen + 1);
        needfree++;
        strncpy(server, servername, snamelen);
        server[snamelen] = '\0';
    }
    tds_set_server(con->tds_login, server);
    if (needfree)
        free(server);
    ctx = con->ctx;
    if (!(con->tds_socket = tds_alloc_socket(ctx->tds_ctx, 512)))
        return CS_FAIL;
    tds_set_parent(con->tds_socket, (void *) con);
    if (!(connection = tds_read_config_info(NULL, con->tds_login, ctx->tds_ctx->locale))) {
        tds_free_socket(con->tds_socket);
        con->tds_socket = NULL;
        return CS_FAIL;
    }

    /* Timeouts ... */
    con->tds_socket->query_timeout_param = con;
    con->tds_socket->query_timeout_func = ctlib_query_timeout;

    if (ctx->login_timeout > 0) {
        connection->connect_timeout = ctx->login_timeout;
    }

    if (ctx->query_timeout > 0) {
        connection->query_timeout = ctx->query_timeout;
    }

    if (con->locale && con->locale->charset) {
        tds_dstr_copy(&connection->client_charset, con->locale->charset);
    }
    /*     tds_dstr_copy(&connection->server_charset, "UCS-2LE"); */

    if (tds_connect(con->tds_socket, connection) == TDS_FAIL) {
        tds_free_socket(con->tds_socket);
        con->tds_socket = NULL;
        tds_free_connection(connection);
        tdsdump_log(TDS_DBG_FUNC, "leaving ct_connect() returning %d\n", CS_FAIL);
        return CS_FAIL;
    }
    tds_free_connection(connection);

    tdsdump_log(TDS_DBG_FUNC, "leaving ct_connect() returning %d\n", CS_SUCCEED);
    return CS_SUCCEED;
}

CS_RETCODE
ct_cmd_alloc(CS_CONNECTION * con, CS_COMMAND ** cmd)
{

    CS_COMMAND_LIST *command_list;
    CS_COMMAND_LIST *pcommand;

    tdsdump_log(TDS_DBG_FUNC, "ct_cmd_alloc()\n");

    *cmd = (CS_COMMAND *) malloc(sizeof(CS_COMMAND));
    if (!*cmd)
        return CS_FAIL;
    memset(*cmd, '\0', sizeof(CS_COMMAND));

    /* so we know who we belong to */
    (*cmd)->con = con;

    /* initialise command state */
    ct_set_command_state(*cmd, _CS_COMMAND_IDLE);

    command_list = malloc(sizeof(CS_COMMAND_LIST));
    memset(command_list, '\0', sizeof(CS_COMMAND_LIST));
    command_list->cmd = *cmd;
    command_list->next = NULL;

    if ( con->cmds == NULL ) {
        tdsdump_log(TDS_DBG_FUNC, "ct_cmd_alloc() : allocating command list to head\n");
        con->cmds = command_list;
    } else {
        pcommand = con->cmds;
        for (;;) {
            tdsdump_log(TDS_DBG_FUNC, "ct_cmd_alloc() : stepping thru existing commands\n");
            if (pcommand->next == NULL)
                break;
            pcommand = pcommand->next;
        }
        pcommand->next = command_list;
    }

    return CS_SUCCEED;
}

CS_RETCODE
ct_command(CS_COMMAND * cmd, CS_INT type, const CS_VOID * buffer, CS_INT buflen, CS_INT option)
{
    ssize_t query_len;
    ssize_t current_query_len;

    tdsdump_log(TDS_DBG_FUNC, "ct_command()\n");

    /* Unless we are in the process of building a CS_LANG_CMD   */
    /* type command, then clear everything, ready to start anew */

    if (cmd->command_state != _CS_COMMAND_BUILDING) {
        _ct_initialise_cmd(cmd);
        ct_set_command_state(cmd, _CS_COMMAND_IDLE);
    }

    switch (type) {
    case CS_LANG_CMD:
        switch (option) {
        case CS_MORE:   /* The text in buffer is only part of the language command to be executed. */
        case CS_END:    /* The text in buffer is the last part of the language command to be executed. */
        case CS_UNUSED: /* Equivalent to CS_END. */
            if (buflen == CS_NULLTERM) {
                query_len = strlen((const char *) buffer);
            } else {
                query_len = buflen;
            }
            /* small fix for no crash */
            if (query_len == CS_UNUSED) {
                cmd->query = NULL;
                return CS_FAIL;
            }
            if (cmd->command_state == _CS_COMMAND_IDLE) {
                cmd->query = (char *) malloc(query_len + 1);
                strncpy(cmd->query, (const char *) buffer, query_len);
                cmd->query[query_len] = '\0';
                if (option == CS_MORE) {
                    ct_set_command_state(cmd, _CS_COMMAND_BUILDING);
                } else {
                    ct_set_command_state(cmd, _CS_COMMAND_READY);
                }
            }
            if (cmd->command_state == _CS_COMMAND_BUILDING) {
                current_query_len = strlen(cmd->query);
                cmd->query = (char *) realloc(cmd->query, current_query_len + query_len + 1);
                strncat(cmd->query, (const char *) buffer, query_len);
                cmd->query[current_query_len + query_len] = '\0';
                if (option == CS_MORE) {
                    ct_set_command_state(cmd, _CS_COMMAND_BUILDING);
                } else {
                    ct_set_command_state(cmd, _CS_COMMAND_READY);
                }
            }

            break;
        default:
            return CS_FAIL;
        }
        break;

    case CS_RPC_CMD:

        /* Code changed for RPC functionality -SUHA */
        /* RPC code changes starts here */

        if (cmd == NULL)
            return CS_FAIL;

        cmd->rpc = (CSREMOTE_PROC *) malloc(sizeof(CSREMOTE_PROC));
        if (cmd->rpc == NULL)
            return CS_FAIL;

        memset(cmd->rpc, 0, sizeof(CSREMOTE_PROC));

        if (buflen == CS_NULLTERM) {
            cmd->rpc->name = strdup(buffer);
            if (cmd->rpc->name == NULL)
                return CS_FAIL;
        } else if (buflen > 0) {
            cmd->rpc->name = (char *) malloc(buflen + 1);
            if (cmd->rpc->name == NULL)
                return CS_FAIL;
            memset(cmd->rpc->name, 0, buflen + 1);
            strncpy(cmd->rpc->name, (const char *) buffer, buflen);
        } else {
            return CS_FAIL;
        }

        cmd->rpc->param_list = NULL;

        tdsdump_log(TDS_DBG_INFO1, "ct_command() added rpcname \"%s\"\n", cmd->rpc->name);

        /* FIXME: I don't know the value for RECOMPILE, NORECOMPILE options. Hence assigning zero  */
        switch (option) {
        case CS_RECOMPILE:  /* Recompile the stored procedure before executing it. */
            cmd->rpc->options = 0;
            break;
        case CS_NO_RECOMPILE:   /* Do not recompile the stored procedure before executing it. */
            cmd->rpc->options = 0;
            break;
        case CS_UNUSED: /* Equivalent to CS_NO_RECOMPILE. */
            cmd->rpc->options = 0;
            break;
        default:
            return CS_FAIL;
        }
        ct_set_command_state(cmd, _CS_COMMAND_READY);
        break;
        /* RPC code changes ends here */

    case CS_SEND_DATA_CMD:
        switch (option) {
        case CS_COLUMN_DATA:    /* The data will be used for a text or image column update. */
            cmd->send_data_started = 0;
            break;
        case CS_BULK_DATA:  /* For internal Sybase use only. The data will be used for a bulk copy operation. */
        default:
            return CS_FAIL;
        }
        ct_set_command_state(cmd, _CS_COMMAND_READY);
        break;

    case CS_SEND_BULK_CMD:
        switch (option) {
        case CS_BULK_INIT:  /* For internal Sybase use only. Initialize a bulk copy operation. */
        case CS_BULK_CONT:  /* For internal Sybase use only. Continue a bulk copy operation. */
        default:
            return CS_FAIL;
        }
        ct_set_command_state(cmd, _CS_COMMAND_READY);
        break;

    default:
        return CS_FAIL;
    }

    cmd->command_type = type;


    return CS_SUCCEED;
}

static void
_ct_initialise_cmd(CS_COMMAND *cmd)
{
    if (cmd->query) {
        free(cmd->query);
        cmd->query = NULL;
    }

    if (cmd->input_params) {
        param_clear(cmd->input_params);
        cmd->input_params = NULL;
    }
    ct_set_command_state(cmd, _CS_COMMAND_IDLE);

    rpc_clear(cmd->rpc);
    cmd->rpc = NULL;
}

CS_RETCODE
ct_send(CS_COMMAND * cmd)
{
    TDSSOCKET *tds;
    CS_RETCODE ret;
    /* CSREMOTE_PROC **rpc; */
    TDSPARAMINFO *pparam_info;
    TDSCURSOR *cursor;
    TDSDYNAMIC *tdsdyn;

    tdsdump_log(TDS_DBG_FUNC, "ct_send() command_type = %d\n", cmd->command_type);

    if (!cmd->con || !cmd->con->tds_socket)
        return CS_FAIL;

    tds = cmd->con->tds_socket;

    if (cmd->cancel_state == _CS_CANCEL_PENDING) {
        _ct_cancel_cleanup(cmd);
        return CS_CANCELED;
    }

    if (cmd->command_state == _CS_COMMAND_IDLE) {
        tdsdump_log(TDS_DBG_FUNC, "ct_send() command_state = IDLE\n");
        _ctclient_msg(cmd->con, "ct_send", 1, 1, 1, 16843163, "");
        return CS_FAIL;
    }

    cmd->results_state = _CS_RES_NONE;

    if (cmd->command_type == CS_DYNAMIC_CMD) {

        if (cmd->dyn == NULL)
            return CS_FAIL;

        switch (cmd->dynamic_cmd) {
        case CS_PREPARE:
            if (tds_submit_prepare(tds, cmd->dyn->stmt, cmd->dyn->id, NULL, NULL) == TDS_FAIL)
                return CS_FAIL;
            ct_set_command_state(cmd, _CS_COMMAND_SENT);
            return CS_SUCCEED;
            break;
        case CS_EXECUTE:
            pparam_info = paraminfoalloc(tds, cmd->dyn->param_list);
            tdsdyn = tds_lookup_dynamic(tds, cmd->dyn->id);
            if (!tdsdyn) {
                tdsdump_log(TDS_DBG_INFO1, "ct_send(CS_EXECUTE) no tdsdyn!\n");
                return CS_FAIL;
            }
            tds_free_input_params(tdsdyn);
            tdsdyn->params = pparam_info;
            if (tds_submit_execute(tds, tdsdyn) == TDS_FAIL)
                return CS_FAIL;
            ct_set_command_state(cmd, _CS_COMMAND_SENT);
            return CS_SUCCEED;
            break;
        case CS_DESCRIBE_INPUT:
            tdsdump_log(TDS_DBG_INFO1, "ct_send(CS_DESCRIBE_INPUT)\n");
            ct_set_command_state(cmd, _CS_COMMAND_SENT);
            cmd->results_state = _CS_RES_DESCRIBE_RESULT;
            if (tds->cur_dyn)
                tds->current_results = tds->cur_dyn->res_info;
            else
                tds->current_results = tds->param_info;
            break;

        case CS_DESCRIBE_OUTPUT:
            tdsdump_log(TDS_DBG_INFO1, "ct_send(CS_DESCRIBE_OUTPUT)\n");
            ct_set_command_state(cmd, _CS_COMMAND_SENT);
            cmd->results_state = _CS_RES_DESCRIBE_RESULT;
            tds->current_results = tds->res_info;
            break;

        case CS_DEALLOC:
            tdsdyn = tds_lookup_dynamic(tds, cmd->dyn->id);
            if (!tdsdyn) {
                tdsdump_log(TDS_DBG_INFO1, "ct_send(CS_DEALLOC) no tdsdyn!\n");
                return CS_FAIL;
            }
            if (tds_submit_unprepare(tds, tdsdyn) == TDS_FAIL)
                return CS_FAIL;
            else {
                ct_set_command_state(cmd, _CS_COMMAND_SENT);
                return CS_SUCCEED;
            }
            break;

        default:
            return CS_FAIL;
        }
    }

    if (cmd->command_type == CS_RPC_CMD) {
        /* sanity */
        if (cmd == NULL || cmd->rpc == NULL /* ct_command should allocate pointer */
            || cmd->rpc->name == NULL) {    /* can't be ready without a name      */
            return CS_FAIL;
        }

        /* rpc = &(cmd->rpc); */
        pparam_info = paraminfoalloc(tds, cmd->rpc->param_list);
        ret = tds_submit_rpc(tds, cmd->rpc->name, pparam_info);

        tds_free_param_results(pparam_info);

        ct_set_command_state(cmd, _CS_COMMAND_SENT);

        if (ret == TDS_FAIL) {
            return CS_FAIL;
        }

        return CS_SUCCEED;
    }

    /* RPC Code changes ends here */

    if (cmd->command_type == CS_LANG_CMD) {
        ret = CS_FAIL;
        if (cmd->input_params) {
            pparam_info = paraminfoalloc(tds, cmd->input_params);
            ret = tds_submit_query_params(tds, cmd->query, pparam_info);
            tds_free_param_results(pparam_info);
        } else {
            ret = tds_submit_query(tds, cmd->query);
        }

        ct_set_command_state(cmd, _CS_COMMAND_SENT);

        if (ret == TDS_FAIL) {
            tdsdump_log(TDS_DBG_WARN, "ct_send() failed\n");
            return CS_FAIL;
        }
        tdsdump_log(TDS_DBG_INFO2, "ct_send() succeeded\n");
        return CS_SUCCEED;
    }

    /* Code added for CURSOR support */

    if (cmd->command_type == CS_CUR_CMD) {
        /* sanity */
        /*
         * ct_cursor declare should allocate cursor pointer
         * cursor stmt cannot be NULL
         * cursor name cannot be NULL
         */

        int something_to_send = 0;

        tdsdump_log(TDS_DBG_FUNC, "ct_send() : CS_CUR_CMD\n");

        cursor = cmd->cursor;
        if (!cursor) {
            tdsdump_log(TDS_DBG_FUNC, "ct_send() : cursor not present\n");
            return CS_FAIL;
        }

        if (cmd == NULL ) {
            tdsdump_log(TDS_DBG_FUNC, "ct_send() : cmd is null\n");
            return CS_FAIL;
        }
        if (cursor->query == NULL ) {
            tdsdump_log(TDS_DBG_FUNC, "ct_send() : cursor->query is null\n");
            return CS_FAIL;
        }
        if ( cursor->cursor_name == NULL ) {
            tdsdump_log(TDS_DBG_FUNC, "ct_send() : cursor->name is null\n");
            return CS_FAIL;
        }

        if (cursor->status.declare == _CS_CURS_TYPE_REQUESTED) {
            ret =  tds_cursor_declare(tds, cursor, &something_to_send);
            if (ret == CS_SUCCEED){
                cursor->status.declare = _CS_CURS_TYPE_SENT; /* Cursor is declared */
                if (something_to_send == 0) {
                    cmd->results_state = _CS_RES_END_RESULTS;
                }
            }
            else {
                tdsdump_log(TDS_DBG_WARN, "ct_send(): cursor declare failed \n");
                return CS_FAIL;
            }
        }

        if (cursor->status.cursor_row == _CS_CURS_TYPE_REQUESTED &&
            cursor->status.declare == _CS_CURS_TYPE_SENT) {

            ret = tds_cursor_setrows(tds, cursor, &something_to_send);
            if (ret == CS_SUCCEED){
                cursor->status.cursor_row = _CS_CURS_TYPE_SENT; /* Cursor rows set */
                if (something_to_send == 0) {
                    cmd->results_state = _CS_RES_END_RESULTS;
                }
            }
            else {
                tdsdump_log(TDS_DBG_WARN, "ct_send(): cursor set rows failed\n");
                return CS_FAIL;
            }
        }

        if (cursor->status.open == _CS_CURS_TYPE_REQUESTED &&
            cursor->status.declare == _CS_CURS_TYPE_SENT) {

            ret = tds_cursor_open(tds, cursor, &something_to_send);
            if (ret == CS_SUCCEED){
                cursor->status.open = _CS_CURS_TYPE_SENT;
                cmd->results_state = _CS_RES_INIT;
            }
            else {
                tdsdump_log(TDS_DBG_WARN, "ct_send(): cursor open failed\n");
                return CS_FAIL;
            }
        }

        if (something_to_send) {
            tdsdump_log(TDS_DBG_WARN, "ct_send(): sending cursor commands\n");
            tds_flush_packet(tds);
            tds_set_state(tds, TDS_PENDING);
            something_to_send = 0;

            ct_set_command_state(cmd, _CS_COMMAND_SENT);

            return CS_SUCCEED;
        }

        if (cursor->status.close == _CS_CURS_TYPE_REQUESTED){
            if (cursor->status.dealloc == TDS_CURSOR_STATE_REQUESTED) {
                /* FIXME what happen if tds_cursor_dealloc return TDS_FAIL ?? */
                ret = tds_cursor_close(tds, cursor);
                cmd->cursor = NULL;
                /* 0.95
                tds_release_cursor(tds, cursor);
                cmd->cursor = cursor = NULL;
                */
            } else {
                ret = tds_cursor_close(tds, cursor);
                cursor->status.close = _CS_CURS_TYPE_SENT;
            }
        }

        if (cursor && cursor->status.dealloc == _CS_CURS_TYPE_REQUESTED) {
            /* FIXME what happen if tds_cursor_dealloc return TDS_FAIL ?? */
            ret = tds_cursor_dealloc(tds, cursor);
            cmd->cursor = NULL;
            tds_free_all_results(tds);
        }

        ct_set_command_state(cmd, _CS_COMMAND_SENT);
        return CS_SUCCEED;
    }

    if (cmd->command_type == CS_SEND_DATA_CMD) {
        tds_flush_packet(tds);
        tds_set_state(tds, TDS_PENDING);
        ct_set_command_state(cmd, _CS_COMMAND_SENT);
    }

    return CS_SUCCEED;
}


CS_RETCODE
ct_results(CS_COMMAND * cmd, CS_INT * result_type)
{
    TDSSOCKET *tds;
    CS_CONTEXT *context;

    int tdsret;
    CS_INT res_type;
    CS_INT done_flags;

    tdsdump_log(TDS_DBG_FUNC, "ct_results()\n");

    if (cmd->cancel_state == _CS_CANCEL_PENDING) {
        _ct_cancel_cleanup(cmd);
        return CS_CANCELED;
    }

    if (!cmd->con || !cmd->con->tds_socket)
        return CS_FAIL;

    cmd->bind_count = CS_UNUSED;

    context = cmd->con->ctx;

    tds = cmd->con->tds_socket;
    cmd->row_prefetched = 0;

    /*
     * depending on the current results state, we may
     * not need to call tds_process_tokens...
     */

    switch (cmd->results_state) {
    case _CS_RES_CMD_SUCCEED:
        *result_type = CS_CMD_SUCCEED;
        cmd->results_state = _CS_RES_CMD_DONE;
        return CS_SUCCEED;
    case _CS_RES_CMD_DONE:
        *result_type = CS_CMD_DONE;
        cmd->results_state = _CS_RES_INIT;
        return CS_SUCCEED;
    case _CS_RES_END_RESULTS:
        *result_type = CS_CMD_DONE;
        cmd->results_state = _CS_RES_INIT;
        return CS_END_RESULTS;
    case _CS_RES_DESCRIBE_RESULT:
        *result_type = CS_DESCRIBE_RESULT;
        cmd->results_state = _CS_RES_CMD_DONE;
        return CS_SUCCEED;
    case _CS_RES_NONE:              /* first time in after ct_send */
        cmd->results_state = _CS_RES_INIT;
        break;
    default:
        break;
    }

    /*
     * see what "result" tokens we have. a "result" in ct-lib terms also
     * includes row data. Some result types always get reported back  to
     * the calling program, others are only reported back if the relevant
     * config flag is set.
     */

    for (;;) {

        tdsret = tds_process_tokens(tds, &res_type, &done_flags, TDS_TOKEN_RESULTS);

        tdsdump_log(TDS_DBG_FUNC, "ct_results() process_result_tokens returned %d (type %d) \n",
                tdsret, res_type);

        switch (tdsret) {

        case TDS_SUCCEED:

            cmd->curr_result_type = res_type;

            switch (res_type) {

            case CS_COMPUTEFMT_RESULT:
            case CS_ROWFMT_RESULT:

                /*
                 * set results state to indicate that we
                 * have a result set (empty for the moment)
                 * If the CS_EXPOSE_FMTS  property has been
                 * set in ct_config(), we need to return an
                 * appropraite format result, otherwise just
                 * carry on and get the next token.....
                 */

                cmd->results_state = _CS_RES_RESULTSET_EMPTY;

                if (context->config.cs_expose_formats) {
                    *result_type = res_type;
                    return CS_SUCCEED;
                }
                break;

            case CS_ROW_RESULT:

                /*
                 * we've hit a data row. pass back that fact
                 * to the calling program. set results state
                 * to show that the result set has rows...
                 */

                cmd->results_state = _CS_RES_RESULTSET_ROWS;
                if (cmd->command_type == CS_CUR_CMD) {
                    *result_type = CS_CURSOR_RESULT;
                } else {
                    *result_type = CS_ROW_RESULT;
                }
                return CS_SUCCEED;
                break;

            case CS_COMPUTE_RESULT:

                /*
                 * we've hit a compute data row. We have to get hold of this
                 * data now, as it's necessary  to tie this data back to its
                 * result format...the user may call ct_res_info() & friends
                 * after getting back a compute "result".
                 *
                 * but first, if we've hit this compute row without having
                 * hit a data row first, we need to return a  CS_ROW_RESULT
                 * before letting them have the compute row...
                 */

                if (cmd->results_state == _CS_RES_RESULTSET_EMPTY) {
                    *result_type = CS_ROW_RESULT;
                    tds->current_results = tds->res_info;
                    cmd->results_state = _CS_RES_RESULTSET_ROWS;
                    return CS_SUCCEED;
                }

                tdsret = tds_process_tokens(tds, &res_type, NULL, TDS_STOPAT_ROWFMT|TDS_RETURN_DONE|TDS_RETURN_ROW|TDS_RETURN_COMPUTE);

                /* set results state to show that the result set has rows... */

                cmd->results_state = _CS_RES_RESULTSET_ROWS;

                *result_type = res_type;
                if (tdsret == TDS_SUCCEED && (res_type == TDS_ROW_RESULT || res_type == TDS_COMPUTE_RESULT)) {
                    if (res_type == TDS_COMPUTE_RESULT) {
                        cmd->row_prefetched = 1;
                        return CS_SUCCEED;
                    } else {
                        /* this couldn't really happen, but... */
                        return CS_FAIL;
                    }
                } else
                    return CS_FAIL;
                break;

            case TDS_DONE_RESULT:

                /*
                 * A done token signifies the end of a logical
                 * command. There are three possibilities...
                 * 1. Simple command with no result set, i.e.
                 *    update, delete, insert
                 * 2. Command with result set but no rows
                 * 3. Command with result set and rows
                 * in these cases we need to:
                 * 1. return CS_CMD_FAIL/SUCCED depending on
                 *    the status returned in done_flags
                 * 2. "manufacture" a CS_ROW_RESULT return,
                 *    and set the results state to DONE
                 * 3. return with CS_CMD_DONE and reset the
                 *    results_state
                 */

                tdsdump_log(TDS_DBG_FUNC, "ct_results() results state = %d\n",cmd->results_state);
                tdsdump_log(TDS_DBG_FUNC, "ct_results() command type  = %d\n",cmd->command_type);
                tdsdump_log(TDS_DBG_FUNC, "ct_results() dynamic cmd   = %d\n",cmd->dynamic_cmd);

                if ((cmd->command_type == CS_DYNAMIC_CMD) &&
                    (cmd->dynamic_cmd == CS_PREPARE || cmd->dynamic_cmd == CS_DEALLOC)) {
                    *result_type = CS_CMD_SUCCEED;
                    cmd->results_state = _CS_RES_CMD_DONE;
                    return CS_SUCCEED;
                }

                switch (cmd->results_state) {

                case _CS_RES_INIT:
                case _CS_RES_STATUS:
                    if (done_flags & TDS_DONE_ERROR)
                        *result_type = CS_CMD_FAIL;
                    else
                        *result_type = CS_CMD_SUCCEED;
                    cmd->results_state = _CS_RES_CMD_DONE;
                    break;

                case _CS_RES_RESULTSET_EMPTY:
                    if (cmd->command_type == CS_CUR_CMD) {
                        *result_type = CS_CURSOR_RESULT;
                        cmd->results_state = _CS_RES_RESULTSET_ROWS;
                    } else {
                        *result_type = CS_ROW_RESULT;
                        cmd->results_state = _CS_RES_CMD_DONE;
                    }
                    break;

                case _CS_RES_RESULTSET_ROWS:
                    *result_type = CS_CMD_DONE;
                    cmd->results_state = _CS_RES_INIT;
                    break;

                }
                return CS_SUCCEED;
                break;

            case TDS_DONEINPROC_RESULT:

                /*
                 * A doneinproc token may signify the end of a
                 * logical command if the command had a result
                 * set. Otherwise it is ignored....
                 */

                switch (cmd->results_state) {
                case _CS_RES_INIT:   /* command had no result set */
                    break;
                case _CS_RES_RESULTSET_EMPTY:
                    if (cmd->command_type == CS_CUR_CMD) {
                        *result_type = CS_CURSOR_RESULT;
                    } else {
                        *result_type = CS_ROW_RESULT;
                    }
                    cmd->results_state = _CS_RES_CMD_DONE;
                    return CS_SUCCEED;
                    break;
                case _CS_RES_RESULTSET_ROWS:
                    *result_type = CS_CMD_DONE;
                    cmd->results_state = _CS_RES_INIT;
                    return CS_SUCCEED;
                    break;
                }
                break;

            case TDS_DONEPROC_RESULT:

                /*
                 * A DONEPROC result means the end of a logical
                 * command only if it was one of the commands
                 * directly sent from ct_send, not as a result
                 * of a nested stored procedure call. We know
                 * if this is the case if a STATUS_RESULT was
                 * received immediately prior to the DONE_PROC
                 */

                if (cmd->results_state == _CS_RES_STATUS ||
                    cmd->results_state == _CS_RES_INIT /* ssikorsk */
                    ) {
                    if (done_flags & TDS_DONE_ERROR)
                        *result_type = CS_CMD_FAIL;
                    else
                        *result_type = CS_CMD_SUCCEED;
                    cmd->results_state = _CS_RES_CMD_DONE;
                    return CS_SUCCEED;
                } else {
                    if (cmd->command_type == CS_DYNAMIC_CMD) {
                        *result_type = CS_CMD_SUCCEED;
                        cmd->results_state = _CS_RES_CMD_DONE;
                        return CS_SUCCEED;
                    }
                }

                break;

            case TDS_PARAM_RESULT:
                cmd->row_prefetched = 1;
                *result_type = res_type;
                return CS_SUCCEED;
                break;

            case TDS_STATUS_RESULT:
                _ct_process_return_status(tds);
                cmd->row_prefetched = 1;
                *result_type = res_type;
                cmd->results_state = _CS_RES_STATUS;
                return CS_SUCCEED;
                break;

            case TDS_DESCRIBE_RESULT:
                if (cmd->dynamic_cmd == CS_DESCRIBE_INPUT ||
                    cmd->dynamic_cmd == CS_DESCRIBE_OUTPUT) {
                    *result_type = res_type;
                    return CS_SUCCEED;
                }
                break;
            default:
                *result_type = res_type;
                return CS_SUCCEED;
                break;
            }

            break;

        case TDS_NO_MORE_RESULTS:

            /* some commands can be re-sent once completed */
            /* so mark the command state as 'ready'...     */

            if (cmd->command_type == CS_LANG_CMD ||
                cmd->command_type == CS_RPC_CMD ||
                cmd->command_type == CS_CUR_CMD ||
                cmd->command_type == CS_DYNAMIC_CMD) {
                ct_set_command_state(cmd, _CS_COMMAND_READY);
            }
            /* if we have just completed processing a dynamic deallocate */
            /* get rid of our dynamic statement structure...             */

            if (cmd->command_type == CS_DYNAMIC_CMD &&
                cmd->dynamic_cmd  == CS_DEALLOC) {
                _ct_deallocate_dynamic(cmd->con, cmd->dyn);
                cmd->dyn = NULL;
            }
            return CS_END_RESULTS;
            break;

        case TDS_CANCELLED:
            cmd->cancel_state = _CS_CANCEL_NOCANCEL;
            return CS_CANCELED;
            break;

        case TDS_FAIL:
        default:
            return CS_FAIL;
            break;

        }  /* switch (tdsret) */
    }      /* for (;;)        */
}


CS_RETCODE
ct_bind(CS_COMMAND * cmd, CS_INT item, CS_DATAFMT * datafmt, CS_VOID * buffer, CS_INT * copied, CS_SMALLINT * indicator)
{
    TDSCOLUMN *colinfo;
    TDSRESULTINFO *resinfo;
    TDSSOCKET *tds;
    CS_CONNECTION *con = cmd->con;
    CS_INT bind_count;

    tdsdump_log(TDS_DBG_FUNC, "ct_bind() datafmt count = %d column_number = %d\n", datafmt->count, item);

    if (!con || !con->tds_socket)
        return CS_FAIL;

    tds = con->tds_socket;
    resinfo = tds->current_results;

    /* check item value */
    if (!resinfo || item <= 0 || item > resinfo->num_cols)
        return CS_FAIL;

    colinfo = resinfo->columns[item - 1];

    /* check whether the request is for array binding and ensure that user */
    /* supplies the same datafmt->count to the subsequent ct_bind calls    */

    bind_count = (datafmt->count == 0) ? 1 : datafmt->count;

    /* first bind for this result set */

    if (cmd->bind_count == CS_UNUSED) {
        cmd->bind_count = bind_count;
    } else {
        /* all subsequent binds for this result set - the bind counts must be the same */
        if (cmd->bind_count != bind_count) {
            _ctclient_msg(con, "ct_bind", 1, 1, 1, 137, "%d, %d", bind_count, cmd->bind_count);
            return CS_FAIL;
        }
    }

    /* bind the column_varaddr to the address of the buffer */

    colinfo = resinfo->columns[item - 1];
    colinfo->column_varaddr = (char *) buffer;
    colinfo->column_bindtype = datafmt->datatype;
    colinfo->column_bindfmt = datafmt->format;
    colinfo->column_bindlen = datafmt->maxlength;
    if (indicator) {
        colinfo->column_nullbind = indicator;
    }
    if (copied) {
        colinfo->column_lenbind = copied;
    }
    return CS_SUCCEED;
}

CS_RETCODE
ct_fetch(CS_COMMAND * cmd, CS_INT type, CS_INT offset, CS_INT option, CS_INT * rows_read)
{
    TDS_INT ret_type;
    TDS_INT ret;
    TDS_INT marker;
    TDS_INT temp_count;
    TDSSOCKET *tds;

    tdsdump_log(TDS_DBG_FUNC, "ct_fetch()\n");

    if (!cmd->con || !cmd->con->tds_socket)
        return CS_FAIL;

    if (cmd->command_state == _CS_COMMAND_IDLE) {
        _ctclient_msg(cmd->con, "ct_fetch", 1, 1, 1, 16843163, "");
        return CS_FAIL;
    }

    if (cmd->cancel_state == _CS_CANCEL_PENDING) {
        _ct_cancel_cleanup(cmd);
        return CS_CANCELED;
    }

    tds = cmd->con->tds_socket;

    /* We'll call a special function for fetches from a cursor            */
    /* the processing is too incompatible to patch into a single function */

    if (cmd->command_type == CS_CUR_CMD) {
        return _ct_fetch_cursor(cmd, type, offset, option, rows_read);
    }

    if (rows_read)
        *rows_read = 0;

    /* taking a copy of the cmd->bind_count value. */
    temp_count = cmd->bind_count;

    if ( cmd->bind_count == CS_UNUSED )
        cmd->bind_count = 1;

    /* compute rows and parameter results have been pre-fetched by ct_results() */

    if (cmd->row_prefetched) {
        cmd->row_prefetched = 0;
        cmd->get_data_item = 0;
        cmd->get_data_bytes_returned = 0;
        if (_ct_bind_data(cmd->con->ctx, tds->current_results, tds->current_results, 0))
            return CS_ROW_FAIL;
        if (rows_read)
            *rows_read = 1;
        return CS_SUCCEED;
    }

    if (cmd->results_state == _CS_RES_CMD_DONE)
        return CS_END_DATA;
    if (cmd->curr_result_type == CS_COMPUTE_RESULT)
        return CS_END_DATA;
    if (cmd->curr_result_type == CS_CMD_FAIL)
        return CS_CMD_FAIL;


    marker = tds_peek(tds);
    if ((cmd->curr_result_type == CS_ROW_RESULT && marker != TDS_ROW_TOKEN) ||
        (cmd->curr_result_type == CS_STATUS_RESULT && marker != TDS_RETURNSTATUS_TOKEN) )
        return CS_END_DATA;

    /* Array Binding Code changes start here */

    for (temp_count = 0; temp_count < cmd->bind_count; temp_count++) {

        ret = tds_process_tokens(tds, &ret_type, NULL, TDS_STOPAT_ROWFMT|TDS_STOPAT_DONE|TDS_RETURN_ROW|TDS_RETURN_COMPUTE);

        tdsdump_log(TDS_DBG_FUNC, "inside ct_fetch()process_row_tokens returned %d\n", ret);

        switch (ret) {
            case TDS_SUCCEED:
                if (ret_type == TDS_ROW_RESULT || ret_type == TDS_COMPUTE_RESULT) {
                    cmd->get_data_item = 0;
                    cmd->get_data_bytes_returned = 0;
                    if (_ct_bind_data(cmd->con->ctx, tds->current_results, tds->current_results, temp_count))
                        return CS_ROW_FAIL;
                    if (rows_read)
                        *rows_read = *rows_read + 1;
                    break;
                }
            case TDS_NO_MORE_RESULTS:
                return CS_END_DATA;
                break;

            case TDS_CANCELLED:
                cmd->cancel_state = _CS_CANCEL_NOCANCEL;
                return CS_CANCELED;
                break;

            default:
                return CS_FAIL;
                break;
        }

        /* have we reached the end of the rows ? */

        marker = tds_peek(tds);

        if (cmd->curr_result_type == CS_ROW_RESULT && marker != TDS_ROW_TOKEN)
            break;

    }

    /* Array Binding Code changes end here */

    return CS_SUCCEED;
}

static CS_RETCODE
_ct_fetch_cursor(CS_COMMAND * cmd, CS_INT type, CS_INT offset, CS_INT option, CS_INT * rows_read)
{
    TDSSOCKET * tds;
    TDSCURSOR *cursor;
    TDS_INT restype;
    TDS_INT ret;
    TDS_INT temp_count;
    TDS_INT done_flags;
    TDS_INT rows_this_fetch = 0;

    tdsdump_log(TDS_DBG_FUNC, "_ct_fetch_cursor()\n");

    if (!cmd->con || !cmd->con->tds_socket)
        return CS_FAIL;

    tds = cmd->con->tds_socket;

    if (rows_read)
        *rows_read = 0;

    /* taking a copy of the cmd->bind_count value. */
    temp_count = cmd->bind_count;

    if ( cmd->bind_count == CS_UNUSED )
        cmd->bind_count = 1;

    cursor = cmd->cursor;
    if (!cursor) {
        tdsdump_log(TDS_DBG_FUNC, "ct_fetch_cursor() : cursor not present\n");
        return CS_FAIL;
    }

    /* currently we are placing this restriction on cursor fetches.  */
    /* the alternatives are too awful to contemplate at the moment   */
    /* i.e. buffering all the rows from the fetch internally...      */

    if (cmd->bind_count < cursor->cursor_rows) {
        tdsdump_log(TDS_DBG_WARN, "_ct_fetch_cursor(): bind count must equal cursor rows \n");
        return CS_FAIL;
    }

    /* if ( tds_cursor_fetch(tds, cursor) == CS_SUCCEED) { ssikorsk */
    if (tds_cursor_fetch(tds, cursor, TDS_CURSOR_FETCH_NEXT, 0) == CS_SUCCEED) {
        cursor->status.fetch = _CS_CURS_TYPE_SENT;
    }
    else {
        tdsdump_log(TDS_DBG_WARN, "ct_fetch(): cursor fetch failed\n");
        return CS_FAIL;
    }

    tds->cur_cursor = cursor;

    while ((tds_process_tokens(tds, &restype, &done_flags, TDS_TOKEN_RESULTS)) == TDS_SUCCEED) {
        switch (restype) {
            case CS_ROWFMT_RESULT:
                break;
            case CS_ROW_RESULT:
                for (temp_count = 0; temp_count < cmd->bind_count; temp_count++) {

                    ret = tds_process_tokens(tds, &restype, NULL, TDS_STOPAT_ROWFMT|TDS_STOPAT_DONE|TDS_RETURN_ROW|TDS_RETURN_COMPUTE);

                    tdsdump_log(TDS_DBG_FUNC, "_ct_fetch_cursor() tds_process_tokens returned %d\n", ret);

                    if (ret == TDS_SUCCEED && (restype == TDS_ROW_RESULT || restype == TDS_COMPUTE_RESULT)) {
                        cmd->get_data_item = 0;
                        cmd->get_data_bytes_returned = 0;
                        if (restype == TDS_ROW_RESULT) {
                            if (_ct_bind_data(cmd->con->ctx, tds->current_results, tds->current_results, temp_count))
                                return CS_ROW_FAIL;
                            if (rows_read)
                                *rows_read = *rows_read + 1;
                            rows_this_fetch++;
                        }
                    } else {
                        if (ret != TDS_FAIL) {
                            break;
                        } else {
                            return CS_FAIL;
                        }
                    }
                }
                break;
            case TDS_DONE_RESULT:
                break;
        }
    }
    if (rows_this_fetch)
        return CS_SUCCEED;
    else {
        cmd->results_state = _CS_RES_CMD_SUCCEED;
        return CS_END_DATA;
    }

}


int
_ct_bind_data(CS_CONTEXT *ctx, TDSRESULTINFO * resinfo, TDSRESULTINFO *bindinfo, CS_INT offset)
{
    int i;
    TDSCOLUMN *curcol;
    TDSCOLUMN *bindcol;
    unsigned char *src;
    unsigned char *dest, *temp_add;
    int result = 0;
    TDS_INT srctype, srclen, /* desttype, */ len;
    CS_DATAFMT srcfmt, destfmt;
    TDS_INT *datalen = NULL;
    TDS_SMALLINT *nullind = NULL;

    tdsdump_log(TDS_DBG_FUNC, "_ct_bind_data()\n");

    for (i = 0; i < resinfo->num_cols; i++) {
        datalen = NULL;
        nullind = NULL;

        curcol = resinfo->columns[i];
        bindcol = bindinfo->columns[i];

        tdsdump_log(TDS_DBG_FUNC, "_ct_bind_data(): column_type: %d column_len: %d\n",
            curcol->column_type,
            curcol->column_cur_size
        );

        if (curcol->column_hidden)
            continue;


        srctype = curcol->column_type;
        /* desttype = _ct_get_server_type(bindcol->column_bindtype); */

        /* retrieve the initial bound column_varaddress */
        /* and increment it if offset specified         */

        temp_add = (unsigned char *) bindcol->column_varaddr;
        dest = temp_add + (offset * bindcol->column_bindlen);

        if (bindcol->column_nullbind) {
            nullind = bindcol->column_nullbind;
            nullind += offset;
        }
        if (bindcol->column_lenbind) {
            datalen = bindcol->column_lenbind;
            datalen += offset;
        }

        if (dest) {

            if (curcol->column_cur_size < 0) {
                if (nullind)
                    *nullind = -1;
                if (datalen)
                    *datalen = 0;
            } else {

                srctype = _ct_get_client_type(ctx, curcol->column_type, curcol->column_usertype, curcol->column_size);

                src = &(resinfo->current_row[curcol->column_offset]);
                if (is_blob_type(curcol->column_type))
                    src = (unsigned char *) ((TDSBLOB *) src)->textvalue;

                srclen = curcol->column_cur_size;
                srcfmt.datatype = srctype;
                srcfmt.maxlength = srclen;

                destfmt.datatype = bindcol->column_bindtype;
                /*destfmt.maxlength = bindcol->column_cur_size;*/ /*ssikorsk*/
                destfmt.maxlength = bindcol->column_bindlen; /* original */
                destfmt.format = bindcol->column_bindfmt;

                /* if convert return FAIL mark error but process other columns */
                if ((result= cs_convert(ctx, &srcfmt, (CS_VOID *) src, &destfmt, (CS_VOID *) dest, &len) != CS_SUCCEED)) {
                    tdsdump_log(TDS_DBG_FUNC, "cs_convert-result = %d\n", result);
                    result = 1;
                    len = 0;
                    tdsdump_log(TDS_DBG_INFO1, "\n  convert failed for %d \n", srcfmt.datatype);
                }

                if (nullind)
                    *nullind = 0;
                if (datalen) {
                    *datalen = len;
                }
            }
        } else {
            if (datalen)
                *datalen = 0;
        }
    }
    return result;
}

CS_RETCODE
ct_cmd_drop(CS_COMMAND * cmd)
{
    CS_RETCODE ret;
    ret = _ct_cmd_drop(cmd, 1);
    return ret;
}

static CS_RETCODE
_ct_cmd_drop(CS_COMMAND * cmd, CS_INT free_conn_ref)
{
    CS_COMMAND_LIST *victim = NULL;
    CS_COMMAND_LIST *prev = NULL;
    CS_COMMAND_LIST *next = NULL;
    CS_CONNECTION *con;

    tdsdump_log(TDS_DBG_FUNC, "ct_cmd_drop()\n");
    if (cmd) {
        if (cmd->query)
            free(cmd->query);
        if (cmd->input_params)
            param_clear(cmd->input_params);
        if (cmd->userdata)
            free(cmd->userdata);
        if (cmd->rpc) {
            if (cmd->rpc->param_list)
                param_clear(cmd->rpc->param_list);
            free(cmd->rpc->name);
            free(cmd->rpc);
        }
        if (cmd->iodesc)
            free(cmd->iodesc);

        /* now remove this command from the list of commands in the connection */

        if (free_conn_ref && cmd->con) {
            con = cmd->con;
            victim = con->cmds;

            for (;;) {
                if (victim->cmd == cmd)
                    break;
                prev = victim;
                victim = victim->next;
                if (victim == NULL) {
                    tdsdump_log(TDS_DBG_FUNC, "ct_cmd_drop() : cannot find command entry in list \n");
                    return CS_FAIL;
                }
            }

            tdsdump_log(TDS_DBG_FUNC, "ct_cmd_drop() : command entry found in list\n");

            next = victim->next;
            free(victim);

            tdsdump_log(TDS_DBG_FUNC, "ct_cmd_drop() : relinking list\n");

            if (prev)
                prev->next = next;
            else
                con->cmds = next;

            tdsdump_log(TDS_DBG_FUNC, "ct_cmd_drop() : relinked list\n");
        }

        free(cmd);
    }
    return CS_SUCCEED;
}

CS_RETCODE
ct_close(CS_CONNECTION * con, CS_INT option)
{
    tdsdump_log(TDS_DBG_FUNC, "ct_close()\n");
    tds_free_socket(con->tds_socket);
    con->tds_socket = NULL;
    return CS_SUCCEED;
}

CS_RETCODE
ct_con_drop(CS_CONNECTION * con)
{
    CS_COMMAND_LIST *currptr;
    CS_COMMAND_LIST *freeptr;

    tdsdump_log(TDS_DBG_FUNC, "ct_con_drop()\n");
    if (con) {
        if (con->userdata)
            free(con->userdata);
        if (con->tds_login)
            tds_free_login(con->tds_login);
        if (con->cmds) {
            currptr = con->cmds;
            while (currptr != NULL) {
                freeptr = currptr;
                if (currptr->cmd)
                    currptr->cmd->con = NULL;
                currptr = currptr->next;
                free(freeptr);
            }
        }
        free(con);
    }
    return CS_SUCCEED;
}

int
_ct_get_client_type(CS_CONTEXT *ctx, int datatype, int usertype, int size)
{
    tdsdump_log(TDS_DBG_FUNC, "_ct_get_client_type(type %d, user %d, size %d)\n", datatype, usertype, size);
    switch (datatype) {
    case SYBBIT:
    case SYBBITN:
        return CS_BIT_TYPE;
        break;
    case SYBCHAR:
        return CS_VARCHAR_TYPE;
        break;
    case SYBVARCHAR:
        return CS_VARCHAR_TYPE;
        break;
    case SYBINT8:
    case SYBUINT8:
        return CS_LONG_TYPE;
        break;
    case SYBINT4:
    case SYBUINT4:
        return CS_INT_TYPE;
        break;
    case SYBINT2:
    case SYBUINT2:
        return CS_SMALLINT_TYPE;
        break;
    case SYBINT1:
        return CS_TINYINT_TYPE;
        break;
    case SYBINTN:
        switch (size) {
        case 8:
            return CS_LONG_TYPE;
        case 4:
            return CS_INT_TYPE;
        case 2:
            return CS_SMALLINT_TYPE;
        case 1:
            return CS_TINYINT_TYPE;
        default:
            _csclient_msg(ctx, "_ct_get_client_type", 2, 1, 16, 28, "%d", size);
        }
        break;
    case SYBREAL:
        return CS_REAL_TYPE;
        break;
    case SYBFLT8:
        return CS_FLOAT_TYPE;
        break;
    case SYBFLTN:
        if (size == 4) {
            return CS_REAL_TYPE;
        } else if (size == 8) {
            return CS_FLOAT_TYPE;
        } else {
            _csclient_msg(ctx, "_ct_get_client_type", 2, 1, 16, 29, "%d", size);
        }
    case SYBMONEY:
        return CS_MONEY_TYPE;
        break;
    case SYBMONEY4:
        return CS_MONEY4_TYPE;
        break;
    case SYBMONEYN:
        if (size == 4) {
            return CS_MONEY4_TYPE;
        } else if (size == 8) {
            return CS_MONEY_TYPE;
        } else {
            _csclient_msg(ctx, "_ct_get_client_type", 2, 1, 16, 30, "%d", size);
        }
    case SYBDATETIME:
        return CS_DATETIME_TYPE;
        break;
    case SYBDATETIME4:
        return CS_DATETIME4_TYPE;
        break;
    case SYBDATETIMN:
        if (size == 4) {
            return CS_DATETIME4_TYPE;
        } else if (size == 8) {
            return CS_DATETIME_TYPE;
        } else {
            _csclient_msg(ctx, "_ct_get_client_type", 2, 1, 16, 31, "%d", size);
        }
        break;
    case SYBNUMERIC:
        return CS_NUMERIC_TYPE;
        break;
    case SYBDECIMAL:
        return CS_DECIMAL_TYPE;
        break;
    case SYBBINARY:
        return CS_BINARY_TYPE;
        break;
    case SYBIMAGE:
        return CS_IMAGE_TYPE;
        break;
    case SYBVARBINARY:
        return CS_VARBINARY_TYPE;
        break;
    case SYBTEXT:
        return CS_TEXT_TYPE;
        break;
    case SYBUNIQUE:
        return CS_UNIQUE_TYPE;
        break;
    case SYBLONGBINARY:
        if (usertype == USER_UNICHAR_TYPE || usertype == USER_UNIVARCHAR_TYPE)
            return CS_UNICHAR_TYPE;
        /*return CS_CHAR_TYPE;*/
        return CS_LONGBINARY_TYPE;
        break;
    }

    return CS_FAIL;
}

int
_ct_get_server_type(int datatype)
{
    tdsdump_log(TDS_DBG_FUNC, "_ct_get_server_type(%d)\n", datatype);
    switch (datatype) {
    case CS_IMAGE_TYPE:
        return SYBIMAGE;
        break;
    case CS_BINARY_TYPE:
        return SYBBINARY;
        break;
    case CS_BIT_TYPE:
        return SYBBIT;
        break;
    case CS_CHAR_TYPE:
        return SYBCHAR;
        break;
    case CS_LONG_TYPE:
        return SYBINT8;
        break;
    case CS_INT_TYPE:
        return SYBINT4;
        break;
    case CS_SMALLINT_TYPE:
        return SYBINT2;
        break;
    case CS_TINYINT_TYPE:
        return SYBINT1;
        break;
    case CS_REAL_TYPE:
        return SYBREAL;
        break;
    case CS_FLOAT_TYPE:
        return SYBFLT8;
        break;
    case CS_MONEY_TYPE:
        return SYBMONEY;
        break;
    case CS_MONEY4_TYPE:
        return SYBMONEY4;
        break;
    case CS_DATETIME_TYPE:
        return SYBDATETIME;
        break;
    case CS_DATETIME4_TYPE:
        return SYBDATETIME4;
        break;
    case CS_NUMERIC_TYPE:
        return SYBNUMERIC;
        break;
    case CS_DECIMAL_TYPE:
        return SYBDECIMAL;
        break;
    case CS_VARBINARY_TYPE:
        return SYBVARBINARY;
        break;
    case CS_TEXT_TYPE:
        return SYBTEXT;
        break;
    case CS_UNIQUE_TYPE:
        return SYBUNIQUE;
        break;
    case CS_LONGBINARY_TYPE:    /* vicm */
        return SYBLONGBINARY;
        break;
    case CS_UNICHAR_TYPE:
        return SYBNVARCHAR;
    case CS_LONGCHAR_TYPE:
        return SYBVARCHAR;
    case CS_NLONGCHAR_TYPE:
        return SYBNVARCHAR;
    case CS_VARCHAR_TYPE:
        return SYBVARCHAR;
    case CS_NVARCHAR_TYPE:
        return SYBNVARCHAR;
    default:
        /*
        CS_SENSITIVITY_TYPE
        CS_BOUNDARY_TYPE
        CS_VOID_TYPE
        */
        return -1;
        break;
    }
}

CS_RETCODE
ct_cancel(CS_CONNECTION * conn, CS_COMMAND * cmd, CS_INT type)
{
    CS_RETCODE ret;
    CS_COMMAND_LIST *cmds;
    CS_COMMAND *conn_cmd;
    CS_CONNECTION *cmd_conn;

    tdsdump_log(TDS_DBG_FUNC, "ct_cancel()\n");

    /*
     * Comments taken from Sybase ct-library reference manual
     * ------------------------------------------------------
     *
     * "To cancel current results, an application calls ct_cancel
     *  with type as CT_CANCEL CURRENT. This is equivalent to
     *  calling ct_fetch until it returns CS_END_DATA"
     *
     * "For CS_CANCEL_CURRENT cancels, cmd must be supplied"
     * "For CS_CANCEL_CURRENT cancels, connection must be NULL"
     */

    if (type == CS_CANCEL_CURRENT) {


        tdsdump_log(TDS_DBG_FUNC, "CS_CANCEL_CURRENT\n");
        if (conn || !cmd)
            return CS_FAIL;


        if (!_ct_fetchable_results(cmd)) {
            tdsdump_log(TDS_DBG_FUNC, "ct_cancel() no fetchable results - return()\n");
            return CS_SUCCEED;
        }

        tdsdump_log(TDS_DBG_FUNC, "ct_cancel() - fetching results()\n");
        do {
            ret = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, NULL);
        } while ((ret == CS_SUCCEED) || (ret == CS_ROW_FAIL));

        if (cmd->con && cmd->con->tds_socket)
            tds_free_all_results(cmd->con->tds_socket);

        if (ret == CS_END_DATA) {
            return CS_SUCCEED;
        }
        return CS_FAIL;
    }

    /*
     * Comments taken from Sybase ct-library reference manual
     * ------------------------------------------------------
     *
     * "To cancel the current command and all results generated
     *  by it, call ct_cancel with type as CS_CANCEL_ATTN or
     *  CS_CANCEL_ALL. Both of these calls tell client library
     *  to :
     *  * Send an attention to the server
     *  * discard any results already generated by the command
     *  Both types of cancel return CS_SUCCEED immediately, w/o
     *  sending an attention, if no command is in progress.
     *  If an application has not yet called ct_send to send an
     *  initiated command, a CS_CANCEL_ATTN has no effect.
     *
     *  If a command has been sent and ct_results has not been
     *  called, a ct_cancel (CS_CANCEL_ATTN) has no effect."
     *
     * "For CS_CANCEL_ATTN cancels, one of connection or cmd
     *  must be NULL. if cmd is supplied and connection is NULL
     *  the cancel operation applies only to the command pend-
     *  ing for this command structure. If cmd is NULL and
     *  connection is supplied, the cancel operation applies to
     *  all commands pending for this connection.
     */

    if (type == CS_CANCEL_ATTN) {
        if ((conn && cmd) || (!conn && !cmd)) {
            return CS_FAIL;
        }
        if (cmd) {
            tdsdump_log(TDS_DBG_FUNC, "CS_CANCEL_ATTN with cmd\n");
            cmd_conn = cmd->con;
            switch (cmd->command_state) {
                case _CS_COMMAND_IDLE:
                case _CS_COMMAND_READY:
                    tdsdump_log(TDS_DBG_FUNC, "ct_cancel() command state READY/IDLE\n");
                    break;
                case _CS_COMMAND_SENT:
                    tdsdump_log(TDS_DBG_FUNC, "ct_cancel() command state SENT results_state %d\n", cmd->results_state);
                    if (cmd->results_state != _CS_RES_NONE) {
                        tdsdump_log(TDS_DBG_FUNC, "ct_cancel() sending a cancel \n");
                        tds_send_cancel(cmd_conn->tds_socket);
                        cmd->cancel_state = _CS_CANCEL_PENDING;
                    }
                    break;
            }
        }
        if (conn) {
            tdsdump_log(TDS_DBG_FUNC, "CS_CANCEL_ATTN with connection\n");
            for (cmds = conn->cmds; cmds != NULL; cmds = cmds->next) {
                conn_cmd = cmds->cmd;
                switch (conn_cmd->command_state) {
                    case _CS_COMMAND_IDLE:
                    case _CS_COMMAND_READY:
                        tdsdump_log(TDS_DBG_FUNC, "ct_cancel() command state READY/IDLE\n");
                        break;
                    case _CS_COMMAND_SENT:
                        tdsdump_log(TDS_DBG_FUNC, "ct_cancel() command state SENT\n");
                        if (conn_cmd->results_state != _CS_RES_NONE) {
                            tdsdump_log(TDS_DBG_FUNC, "ct_cancel() sending a cancel \n");
                            tds_send_cancel(conn->tds_socket);
                            conn_cmd->cancel_state = _CS_CANCEL_PENDING;
                        }
                    break;
                }
            }
        }

        return CS_SUCCEED;
    }

    /*
     * Comments taken from Sybase ct-library reference manual
     * ------------------------------------------------------
     *
     * "To cancel the current command and all results generated
     *  by it, call ct_cancel with type as CS_CANCEL_ATTN or
     *  CS_CANCEL_ALL. Both of these calls tell client library
     *  to :
     *  * Send an attention to the server
     *  * discard any results already generated by the command
     *  Both types of cancel return CS_SUCCEED immediately, w/o
     *  sending an attention, if no command is in progress.
     *
     *  If an application has not yet called ct_send to send an
     *  initiated command, a CS_CANCEL_ALL cancel discards the
     *  initiated command without sending an attention to the
     *  server.
     *
     *  CS_CANCEL_ALL leaves the command structure in a 'clean'
     *  state, available for use in another operation.
     *
     * "For CS_CANCEL_ALL cancels, one of connection or cmd
     *  must be NULL. if cmd is supplied and connection is NULL
     *  the cancel operation applies only to the command pend-
     *  ing for this command structure. If cmd is NULL and
     *  connection is supplied, the cancel operation applies to
     *  all commands pending for this connection.
     */

    if (type == CS_CANCEL_ALL) {

        if ((conn && cmd) || (!conn && !cmd)) {
            return CS_FAIL;
        }
        if (cmd) {
            tdsdump_log(TDS_DBG_FUNC, "CS_CANCEL_ALL with cmd\n");
            cmd_conn = cmd->con;
            switch (cmd->command_state) {
                case _CS_COMMAND_IDLE:
                case _CS_COMMAND_BUILDING:
                case _CS_COMMAND_READY:
                    tdsdump_log(TDS_DBG_FUNC, "ct_cancel() command state READY/IDLE\n");
                    _ct_initialise_cmd(cmd);
                    break;
                case _CS_COMMAND_SENT:
                    tdsdump_log(TDS_DBG_FUNC, "ct_cancel() command state SENT\n");
                    tdsdump_log(TDS_DBG_FUNC, "ct_cancel() sending a cancel \n");
                    tds_send_cancel(cmd_conn->tds_socket);
                    tds_process_cancel(cmd_conn->tds_socket);
                    _ct_initialise_cmd(cmd);
                    break;
            }
        }
        if (conn) {
            tdsdump_log(TDS_DBG_FUNC, "CS_CANCEL_ALL with connection\n");
            for (cmds = conn->cmds; cmds != NULL; cmds = cmds->next) {
                tdsdump_log(TDS_DBG_FUNC, "ct_cancel() cancelling a command for a connection\n");
                conn_cmd = cmds->cmd;
                switch (conn_cmd->command_state) {
                    case _CS_COMMAND_IDLE:
                    case _CS_COMMAND_BUILDING:
                    case _CS_COMMAND_READY:
                        tdsdump_log(TDS_DBG_FUNC, "ct_cancel() command state SENT\n");
                        _ct_initialise_cmd(conn_cmd);
                        break;
                    case _CS_COMMAND_SENT:
                        tdsdump_log(TDS_DBG_FUNC, "ct_cancel() command state SENT\n");
                        tdsdump_log(TDS_DBG_FUNC, "ct_cancel() sending a cancel \n");
                        tds_send_cancel(conn->tds_socket);
                        tds_process_cancel(conn->tds_socket);
                        _ct_initialise_cmd(conn_cmd);
                    break;
                }
            }
        }

        return CS_SUCCEED;
    }
    return CS_FAIL;
}

static CS_RETCODE
_ct_cancel_cleanup(CS_COMMAND * cmd)
{
    CS_CONNECTION * con;

    con = cmd->con;

    if (con && !IS_TDSDEAD(con->tds_socket))
        tds_process_cancel(con->tds_socket);

    cmd->cancel_state = _CS_CANCEL_NOCANCEL;

    return CS_SUCCEED;

}

CS_RETCODE
ct_describe(CS_COMMAND * cmd, CS_INT item, CS_DATAFMT * datafmt)
{
    TDSSOCKET *tds;
    TDSRESULTINFO *resinfo;
    TDSCOLUMN *curcol;
    int len;

    tdsdump_log(TDS_DBG_FUNC, "ct_describe()\n");

    if (!cmd->con || !cmd->con->tds_socket)
        return CS_FAIL;

    tds = cmd->con->tds_socket;
    resinfo = tds->current_results;;

    if (item < 1 || item > resinfo->num_cols)
        return CS_FAIL;

    curcol = resinfo->columns[item - 1];
    len = curcol->column_namelen;
    if (len >= CS_MAX_NAME)
        len = CS_MAX_NAME - 1;
    strncpy(datafmt->name, curcol->column_name, len);
    /* name is always null terminated */
    datafmt->name[len] = 0;
    datafmt->namelen = len;
    /* need to turn the SYBxxx into a CS_xxx_TYPE */
    datafmt->datatype = _ct_get_client_type(cmd->con->ctx, curcol->column_type, curcol->column_usertype, curcol->column_size);
    tdsdump_log(TDS_DBG_INFO1, "ct_describe() datafmt->datatype = %d server type %d\n", datafmt->datatype,
            curcol->column_type);
    /* FIXME is ok this value for numeric/decimal? */
    datafmt->maxlength = curcol->column_size;
    datafmt->usertype = curcol->column_usertype;
    datafmt->precision = curcol->column_prec;
    datafmt->scale = curcol->column_scale;
    /* Added by ssikorsk */
    datafmt->format = curcol->column_bindfmt;

    /*
     * There are other options that can be returned, but these are the
     * only two being noted via the TDS layer.
     */
    datafmt->status = 0;
    if (curcol->column_nullable)
        datafmt->status |= CS_CANBENULL;
    if (curcol->column_identity)
        datafmt->status |= CS_IDENTITY;
    if (strcmp(datafmt->name, "txts") == 0)
        datafmt->status |= CS_TIMESTAMP;

    datafmt->count = 1;
    datafmt->locale = NULL;

    return CS_SUCCEED;
}

CS_RETCODE
ct_res_info(CS_COMMAND * cmd, CS_INT type, CS_VOID * buffer, CS_INT buflen, CS_INT * out_len)
{
    TDSSOCKET *tds;
    TDSRESULTINFO *resinfo;
    TDSCOLUMN *curcol;
    CS_INT int_val;
    int i;

    tdsdump_log(TDS_DBG_FUNC, "ct_res_info()\n");

    if (!cmd->con || !cmd->con->tds_socket)
        return CS_FAIL;

    tds = cmd->con->tds_socket;
    resinfo = tds->current_results;

    switch (type) {
    case CS_NUMDATA:
        int_val = 0;
        if (resinfo) {
            for (i = 0; i < resinfo->num_cols; i++) {
                curcol = resinfo->columns[i];
                if (!curcol->column_hidden) {
                    int_val++;
                }
            }
        }
        tdsdump_log(TDS_DBG_FUNC, "ct_res_info(): Number of columns is %d\n", int_val);
        memcpy(buffer, &int_val, sizeof(CS_INT));
        break;
    case CS_ROW_COUNT:
        int_val = tds->rows_affected;
        tdsdump_log(TDS_DBG_FUNC, "ct_res_info(): Number of rows is %d\n", int_val);
        memcpy(buffer, &int_val, sizeof(CS_INT));
        break;
    default:
        _csclient_msg(cmd->con->ctx, "ct_res_info", 2, 1, 16, 32, "%d", type);
        return CS_FAIL;
        break;
    }
    return CS_SUCCEED;

}

CS_RETCODE
ct_config(CS_CONTEXT * ctx, CS_INT action, CS_INT property, CS_VOID * buffer, CS_INT buflen, CS_INT * outlen)
{
    CS_RETCODE ret = CS_SUCCEED;
    CS_INT *buf = (CS_INT *) buffer;

    tdsdump_log(TDS_DBG_FUNC, "ct_config() action = %s property = %d\n",
            CS_GET ? "CS_GET" : CS_SET ? "CS_SET" : CS_SUPPORTED ? "CS_SUPPORTED" : "CS_CLEAR", property);

    switch (property) {
    case CS_EXPOSE_FMTS:
        switch (action) {
        case CS_SUPPORTED:
            *buf = CS_TRUE;
            break;
        case CS_SET:
            if (*buf != CS_TRUE && *buf != CS_FALSE)
                ret = CS_FAIL;
            else
                ctx->config.cs_expose_formats = *buf;
            break;
        case CS_GET:
            if (buf)
                *buf = ctx->config.cs_expose_formats;
            else
                ret = CS_FAIL;
            break;
        case CS_CLEAR:
            ctx->config.cs_expose_formats = CS_FALSE;
            break;
        default:
            ret = CS_FAIL;
        }
        break;
    case CS_VER_STRING: {
        ret = CS_FAIL;
        switch (action) {
            case CS_GET: {
                if (buffer && buflen > 0 && outlen) {
                    const TDS_COMPILETIME_SETTINGS *settings= tds_get_compiletime_settings();
                    *outlen= snprintf((char*)buffer, buflen, "%s (%s, default tds version=%s)",
                        settings->freetds_version,
                        (settings->threadsafe ? "threadsafe" : "non-threadsafe"),
                        settings->tdsver
                    );
                    ((char*)buffer)[buflen - 1]= 0;
                    if (*outlen < 0)
                        *outlen = (CS_INT) strlen((char*) buffer);
                    ret = CS_SUCCEED;
                }
                break;
                default:
                    ret = CS_FAIL;
                    break;
                }
            }
        }
        break;
    case CS_VERSION:
        ret = CS_FAIL;
        switch (action) {
            case CS_GET: {
                if (buffer && buflen > 0 && outlen) {
                    const TDS_COMPILETIME_SETTINGS *settings= tds_get_compiletime_settings();
                    *outlen= snprintf(buffer, buflen, "%s", settings->freetds_version);
                    ((char*)buffer)[buflen - 1]= 0;
                    if (*outlen < 0)
                        *outlen = (CS_INT) strlen((char*) buffer);
                    ret = CS_SUCCEED;
                }
                break;
            default:
                ret = CS_FAIL;
                break;
            }
        }
        break;
    case CS_TIMEOUT:
        switch (action) {
        case CS_SET:
            ctx->query_timeout = *((unsigned int*)buf);
            break;
        case CS_GET:
            *((unsigned int*)buf) = ctx->query_timeout;
            break;
        case CS_CLEAR:
            ctx->query_timeout = -1;
            break;
        default:
            ret = CS_FAIL;
            break;
        }
        break;
    case CS_LOGIN_TIMEOUT:
        switch (action) {
        case CS_SET:
            ctx->login_timeout = *((unsigned int*)buf);
            break;
        case CS_GET:
            *((unsigned int*)buf) = ctx->login_timeout;
            break;
        case CS_CLEAR:
            ctx->login_timeout = -1;
            break;
        default:
            ret = CS_FAIL;
            break;
        }
        break;
    default:
        ret = CS_SUCCEED;
        break;
    }

    return ret;
}

CS_RETCODE
ct_cmd_props(CS_COMMAND * cmd, CS_INT action, CS_INT property, CS_VOID * buffer, CS_INT buflen, CS_INT * outlen)
{
    /* TDSSOCKET *tds; */
    TDSCURSOR *cursor;
    int maxcp;

    if (!cmd->con || !cmd->con->tds_socket)
        return CS_FAIL;

    /* tds = cmd->con->tds_socket; */

    tdsdump_log(TDS_DBG_FUNC, "ct_cmd_props() action = %s property = %d\n", CS_GET ? "CS_GET" : "CS_SET", property);
    if (action == CS_SET) {
        switch (property) {
        case CS_USERDATA:
            if (cmd->userdata)
                free(cmd->userdata);
            cmd->userdata = (void *) malloc(buflen + 1);
            tdsdump_log(TDS_DBG_INFO2, "setting userdata orig %p new %p\n", buffer, cmd->userdata);
            cmd->userdata_len = buflen;
            memcpy(cmd->userdata, buffer, buflen);
            break;
        default:
            break;
        }
    }
    if (action == CS_GET) {
        switch (property) {

        case CS_PARENT_HANDLE:
            *(CS_CONNECTION **) buffer = cmd->con;
            break;

        case CS_CUR_STATUS:
        case CS_CUR_ID:
        case CS_CUR_NAME:
        case CS_CUR_ROWCOUNT:

            cursor = cmd->cursor;

            if (!cursor) {
                tdsdump_log(TDS_DBG_FUNC, "ct_cmd_props() : cannot find cursor\n");
                if (property == CS_CUR_STATUS) {
                    *(CS_INT *)buffer = (CS_INT) CS_CURSTAT_NONE;
                    if (outlen) *outlen = sizeof(CS_INT);
                    return CS_SUCCEED;
                } else {
                    return CS_FAIL;
                }
            }

            if (property == CS_CUR_STATUS) {
                *(CS_INT *)buffer = cursor->srv_status;
                if (outlen) *outlen = sizeof(CS_INT);
            }
            if (property == CS_CUR_ID) {
                *(CS_INT *)buffer = cursor->cursor_id;
                if (outlen) *outlen = sizeof(CS_INT);
            }
            if (property == CS_CUR_NAME) {
                size_t len = strlen(cursor->cursor_name);
                if ((CS_INT)len >= buflen)
                    return CS_FAIL;
                strcpy(buffer, cursor->cursor_name);
                if (outlen) *outlen = (CS_INT)len;
            }
            if (property == CS_CUR_ROWCOUNT) {
                *(CS_INT *)buffer = cursor->cursor_rows;
                if (outlen) *outlen = sizeof(CS_INT);
            }
            break;

        case CS_USERDATA:
            tdsdump_log(TDS_DBG_INFO2, "fetching userdata %p\n", cmd->userdata);
            maxcp = cmd->userdata_len;
            if (outlen) *outlen = maxcp;
            if (maxcp > buflen)
                maxcp = buflen;
            memcpy(buffer, cmd->userdata, maxcp);
            break;
        default:
            break;
        }
    }
    return CS_SUCCEED;
}

CS_RETCODE
ct_compute_info(CS_COMMAND * cmd, CS_INT type, CS_INT colnum, CS_VOID * buffer, CS_INT buflen, CS_INT * outlen)
{
    TDSSOCKET *tds;
    TDSRESULTINFO *resinfo;
    TDSCOLUMN *curcol;
    CS_INT int_val;
    CS_SMALLINT *dest_by_col_ptr;
    TDS_SMALLINT *src_by_col_ptr;
    int i;

    tdsdump_log(TDS_DBG_FUNC, "ct_compute_info() type = %d, colnum = %d\n", type, colnum);

    if (!cmd->con || !cmd->con->tds_socket)
        return CS_FAIL;

    tds = cmd->con->tds_socket;
    resinfo = tds->current_results;

    switch (type) {
    case CS_BYLIST_LEN:
        if (!resinfo) {
            int_val = 0;
        } else {
            int_val = resinfo->by_cols;
        }
        memcpy(buffer, &int_val, sizeof(CS_INT));
        if (outlen)
            *outlen = sizeof(CS_INT);
        break;
    case CS_COMP_BYLIST:
        if (buflen < (CS_INT)(resinfo->by_cols * sizeof(CS_SMALLINT))) {
            return CS_FAIL;
        } else {
            dest_by_col_ptr = (CS_SMALLINT *) buffer;
            src_by_col_ptr = resinfo->bycolumns;
            for (i = 0; i < resinfo->by_cols; i++) {
                *dest_by_col_ptr = *src_by_col_ptr;
                dest_by_col_ptr++;
                src_by_col_ptr++;
            }
            if (outlen)
                *outlen = (resinfo->by_cols * sizeof(CS_SMALLINT));
        }
        break;
    case CS_COMP_COLID:
        if (!resinfo) {
            int_val = 0;
        } else {
            curcol = resinfo->columns[colnum - 1];
            int_val = curcol->column_operand;
        }
        memcpy(buffer, &int_val, sizeof(CS_INT));
        if (outlen)
            *outlen = sizeof(CS_INT);
        break;
    case CS_COMP_ID:
        if (!resinfo) {
            int_val = 0;
        } else {
            int_val = resinfo->computeid;
        }
        memcpy(buffer, &int_val, sizeof(CS_INT));
        if (outlen)
            *outlen = sizeof(CS_INT);
        break;
    case CS_COMP_OP:
        if (!resinfo) {
            int_val = 0;
        } else {
            curcol = resinfo->columns[colnum - 1];
            int_val = curcol->column_operator;
        }
        memcpy(buffer, &int_val, sizeof(CS_INT));
        if (outlen)
            *outlen = sizeof(CS_INT);
        break;
    default:
        _csclient_msg(cmd->con->ctx, "ct_compute_info", 2, 1, 16, 32, "%d", type);
        return CS_FAIL;
        break;
    }
    return CS_SUCCEED;
}

CS_RETCODE
ct_get_data(CS_COMMAND * cmd, CS_INT item, CS_VOID * buffer, CS_INT buflen, CS_INT * outlen)
{
    TDSRESULTINFO *resinfo;
    TDSCOLUMN *curcol;
    unsigned char *src;
    TDS_INT srclen;

    tdsdump_log(TDS_DBG_FUNC, "ct_get_data() item = %d buflen = %d\n", item, buflen);

    if (cmd->cancel_state == _CS_CANCEL_PENDING) {
        _ct_cancel_cleanup(cmd);
        return CS_CANCELED;
    }

    /* basic validations... */

    if (!cmd || !cmd->con || !cmd->con->tds_socket || !(resinfo = cmd->con->tds_socket->current_results))
        return CS_FAIL;
    if (item < 1 || item > resinfo->num_cols)
        return CS_FAIL;
    if (buffer == NULL)
        return CS_FAIL;
    if (buflen == CS_UNUSED)
        return CS_FAIL;

    /* This is a new column we are being asked to return */

    if (item != cmd->get_data_item) {
        TDSBLOB *blob = NULL;
        CS_INT table_namelen, column_namelen;

        /* allocare needed descriptor if needed */
        if (cmd->iodesc)
            free(cmd->iodesc);
        cmd->iodesc = calloc(1, sizeof(CS_IODESC));
        if (!cmd->iodesc)
            return CS_FAIL;

        /* reset these values */
        cmd->get_data_item = item;
        cmd->get_data_bytes_returned = 0;

        /* get at the source data and length */
        curcol = resinfo->columns[item - 1];

        src = &(resinfo->current_row[curcol->column_offset]);
        if (is_blob_type(curcol->column_type)) {
            blob = (TDSBLOB *) src;
            src = (unsigned char *) blob->textvalue;
        }

        /* now populate the io_desc structure for this data item */

        cmd->iodesc->iotype = CS_IODATA;
        cmd->iodesc->datatype = curcol->column_type;
        cmd->iodesc->locale = cmd->con->locale;
        cmd->iodesc->usertype = curcol->column_usertype;
        cmd->iodesc->total_txtlen = curcol->column_cur_size;
        cmd->iodesc->offset = curcol->column_offset;
        cmd->iodesc->log_on_update = CS_FALSE;

        /* TODO quote needed ?? */
        /* avoid possible buffer overflow */
        table_namelen = curcol->table_namelen;
        if (table_namelen + 2 > sizeof(cmd->iodesc->name))
            table_namelen = sizeof(cmd->iodesc->name) - 2;
        column_namelen = curcol->column_namelen;
        if (table_namelen + column_namelen + 2 > sizeof(cmd->iodesc->name))
            column_namelen = sizeof(cmd->iodesc->name) - 2 - table_namelen;

        /* Old code ...
        sprintf(cmd->iodesc->name, "%*.*s.%*.*s",
            (int) table_namelen, (int) table_namelen, curcol->table_name,
            (int) column_namelen, (int) column_namelen, curcol->column_name);

        cmd->iodesc->namelen = strlen(cmd->iodesc->name);
        */

        /* New code ... */
        if (table_namelen) {
            memcpy(cmd->iodesc->name, curcol->table_name, table_namelen);
            cmd->iodesc->namelen = table_namelen;
        } else {
            cmd->iodesc->namelen = 0;
        }

        cmd->iodesc->name[cmd->iodesc->namelen] = '.';
        ++cmd->iodesc->namelen;

        if (column_namelen) {
            memcpy(cmd->iodesc->name + cmd->iodesc->namelen, curcol->column_name, column_namelen);
            cmd->iodesc->namelen += column_namelen;
        }

        cmd->iodesc->name[cmd->iodesc->namelen] = '\0';

        /* End of new code ... */

        if (blob) {
            memcpy(cmd->iodesc->timestamp, blob->timestamp, CS_TS_SIZE);
            cmd->iodesc->timestamplen = CS_TS_SIZE;
            memcpy(cmd->iodesc->textptr, blob->textptr, CS_TP_SIZE);
            cmd->iodesc->textptrlen = CS_TP_SIZE;
        }
    } else {
        /* get at the source data */
        curcol = resinfo->columns[item - 1];
        src = &(resinfo->current_row[curcol->column_offset]);
        if (is_blob_type(curcol->column_type))
            src = (unsigned char *) ((TDSBLOB *) src)->textvalue;

    }

    /*
     * and adjust the data and length based on
     * what we may have already returned
     */

    srclen = curcol->column_cur_size;
    if (srclen < 0) {
        /* this is NULL */
        if (outlen)
            *outlen = srclen;
        if (item < resinfo->num_cols)
            return CS_END_ITEM;
        return CS_END_DATA;
    }

    src += cmd->get_data_bytes_returned;
    srclen -= cmd->get_data_bytes_returned;

    /* if we have enough buffer to cope with all the data */

    if (buflen >= srclen) {
        memcpy(buffer, src, srclen);
        cmd->get_data_bytes_returned += srclen;
        if (outlen)
            *outlen = srclen;
        if (item < resinfo->num_cols)
            return CS_END_ITEM;
        return CS_END_DATA;

    }
    memcpy(buffer, src, buflen);
    cmd->get_data_bytes_returned += buflen;
    if (outlen)
        *outlen = buflen;
    return CS_SUCCEED;
}

CS_RETCODE
ct_send_data(CS_COMMAND * cmd, CS_VOID * buffer, CS_INT buflen)
{
    TDSSOCKET *tds;
    char writetext_cmd[512];

    char textptr_string[35];    /* 16 * 2 + 2 (0x) + 1 */
    char timestamp_string[19];  /* 8 * 2 + 2 (0x) + 1 */
    char *c;
    int s;
    char hex2[3];

    tdsdump_log(TDS_DBG_FUNC, "ct_send_data()\n");

    if (!cmd->con || !cmd->con->tds_socket)
        return CS_FAIL;

    tds = cmd->con->tds_socket;

    /* basic validations */

    if (cmd->command_type != CS_SEND_DATA_CMD)
        return CS_FAIL;

    if (!cmd->iodesc)
        return CS_FAIL;

    /* first ct_send_data for this column */

    if (!cmd->send_data_started) {

        /* turn the timestamp and textptr into character format */

        c = textptr_string;

        for (s = 0; s < cmd->iodesc->textptrlen; s++) {
            sprintf(hex2, "%02x", cmd->iodesc->textptr[s]);
            *c++ = hex2[0];
            *c++ = hex2[1];
        }
        *c = '\0';

        c = timestamp_string;

        for (s = 0; s < cmd->iodesc->timestamplen; s++) {
            sprintf(hex2, "%02x", cmd->iodesc->timestamp[s]);
            *c++ = hex2[0];
            *c++ = hex2[1];
        }
        *c = '\0';

        /* submit the "writetext bulk" command */

        sprintf(writetext_cmd, "writetext bulk %s 0x%s timestamp = 0x%s%s",
            cmd->iodesc->name,
            textptr_string, timestamp_string, ((cmd->iodesc->log_on_update == CS_TRUE) ? " with log" : "")
            );

        if (tds_submit_query(tds, writetext_cmd) != TDS_SUCCEED) {
            return CS_FAIL;
        }

        /* FIXME in this case processing all results can bring state to IDLE... not threading safe */
        /* read the end token */
        if (tds_process_simple_query(tds) != TDS_SUCCEED)
            return CS_FAIL;

        if (tds_set_state(tds, TDS_QUERYING) != TDS_QUERYING)
            return CS_FAIL;

        cmd->send_data_started = 1;
        tds->out_flag = TDS_BULK;
        tds_put_int(tds, cmd->iodesc->total_txtlen);
    }

    tds->out_flag = TDS_BULK;
    tds_put_n(tds, buffer, buflen);

    return CS_SUCCEED;
}

CS_RETCODE
ct_data_info(CS_COMMAND * cmd, CS_INT action, CS_INT colnum, CS_IODESC * iodesc)
{
    TDSSOCKET *tds;
    TDSRESULTINFO *resinfo;

    if (!cmd->con || !cmd->con->tds_socket)
        return CS_FAIL;

    tds = cmd->con->tds_socket;
    resinfo = tds->current_results;

    tdsdump_log(TDS_DBG_FUNC, "ct_data_info() colnum %d\n", colnum);

    switch (action) {
    case CS_SET:

        if (cmd->iodesc)
            free(cmd->iodesc);
        cmd->iodesc = malloc(sizeof(CS_IODESC));

        cmd->iodesc->iotype = CS_IODATA;
        
        /* ssikorsk  */
        /* cmd->iodesc->datatype = iodesc->datatype; */
        switch (iodesc->datatype) {
            case CS_IMAGE_TYPE:
                cmd->iodesc->datatype = SYBIMAGE;
                break;
            case CS_TEXT_TYPE:
                cmd->iodesc->datatype = SYBTEXT;
                break;
            default:
                cmd->iodesc->datatype = iodesc->datatype;
                break;
        };
        
        cmd->iodesc->locale = cmd->con->locale;
        cmd->iodesc->usertype = iodesc->usertype;
        cmd->iodesc->total_txtlen = iodesc->total_txtlen;
        cmd->iodesc->offset = iodesc->offset;
        cmd->iodesc->log_on_update = iodesc->log_on_update;
        strcpy(cmd->iodesc->name, iodesc->name);
        cmd->iodesc->namelen = iodesc->namelen;
        memcpy(cmd->iodesc->timestamp, iodesc->timestamp, CS_TS_SIZE);
        cmd->iodesc->timestamplen = CS_TS_SIZE;
        memcpy(cmd->iodesc->textptr, iodesc->textptr, CS_TP_SIZE);
        cmd->iodesc->textptrlen = CS_TP_SIZE;
        break;

    case CS_GET:

        if (colnum < 1 || colnum > resinfo->num_cols)
            return CS_FAIL;
        if (colnum != cmd->get_data_item)
            return CS_FAIL;

        iodesc->iotype = cmd->iodesc->iotype;
        
        /* ssikorsk  */
        /* iodesc->datatype = cmd->iodesc->datatype; */
        switch (cmd->iodesc->datatype) {
            case SYBIMAGE:
                iodesc->datatype = CS_IMAGE_TYPE;
                break;
            case SYBTEXT:
                iodesc->datatype = CS_TEXT_TYPE;
                break;
            default:
                iodesc->datatype = cmd->iodesc->datatype;
                break;
        };

        iodesc->locale = cmd->iodesc->locale;
        iodesc->usertype = cmd->iodesc->usertype;
        iodesc->total_txtlen = cmd->iodesc->total_txtlen;
        iodesc->offset = cmd->iodesc->offset;
        iodesc->log_on_update = CS_FALSE;
        strcpy(iodesc->name, cmd->iodesc->name);
        iodesc->namelen = cmd->iodesc->namelen;
        memcpy(iodesc->timestamp, cmd->iodesc->timestamp, cmd->iodesc->timestamplen);
        iodesc->timestamplen = cmd->iodesc->timestamplen;
        memcpy(iodesc->textptr, cmd->iodesc->textptr, cmd->iodesc->textptrlen);
        iodesc->textptrlen = cmd->iodesc->textptrlen;
        break;

    default:
        return CS_FAIL;
    }

    return CS_SUCCEED;
}

CS_RETCODE
ct_capability(CS_CONNECTION * con, CS_INT action, CS_INT type, CS_INT capability, CS_VOID * value)
{
    TDSLOGIN *login;
    int idx = 0;
    unsigned char bitmask = 0;
    unsigned char *mask;

    tdsdump_log(TDS_DBG_FUNC, "ct_capability()\n");
    login = (TDSLOGIN *) con->tds_login;
    mask = login->capabilities;

    if (type == CS_CAP_RESPONSE) {
        switch (capability) {
        case CS_DATA_NOBOUNDARY:
            idx = 13;
            bitmask = 0x01;
            break;
        case CS_RES_NOTDSDEBUG:
            idx = 13;
            bitmask = 0x02;
            break;
        case CS_RES_NOSTRIPBLANKS:
            idx = 13;
            bitmask = 0x04;
            break;
        case CS_DATA_NOINT8:
            idx = 13;
            bitmask = 0x08;
            break;
        case CS_DATA_NOINTN:
            idx = 14;
            bitmask = 0x01;
            break;
        case CS_DATA_NODATETIMEN:
            idx = 14;
            bitmask = 0x02;
            break;
        case CS_DATA_NOMONEYN:
            idx = 14;
            bitmask = 0x04;
            break;
        case CS_CON_NOOOB:
            idx = 14;
            bitmask = 0x08;
            break;
        case CS_CON_NOINBAND:
            idx = 14;
            bitmask = 0x10;
            break;
        case CS_PROTO_NOTEXT:
            idx = 14;
            bitmask = 0x20;
            break;
        case CS_PROTO_NOBULK:
            idx = 14;
            bitmask = 0x40;
            break;
        case CS_DATA_NOSENSITIVITY:
            idx = 14;
            bitmask = 0x80;
            break;
        case CS_DATA_NOFLT4:
            idx = 15;
            bitmask = 0x01;
            break;
        case CS_DATA_NOFLT8:
            idx = 15;
            bitmask = 0x02;
            break;
        case CS_DATA_NONUM:
            idx = 15;
            bitmask = 0x04;
            break;
        case CS_DATA_NOTEXT:
            idx = 15;
            bitmask = 0x08;
            break;
        case CS_DATA_NOIMAGE:
            idx = 15;
            bitmask = 0x10;
            break;
        case CS_DATA_NODEC:
            idx = 15;
            bitmask = 0x20;
            break;
        case CS_DATA_NOLCHAR:
            idx = 15;
            bitmask = 0x40;
            break;
        case CS_DATA_NOLBIN:
            idx = 15;
            bitmask = 0x80;
            break;
        case CS_DATA_NOCHAR:
            idx = 16;
            bitmask = 0x01;
            break;
        case CS_DATA_NOVCHAR:
            idx = 16;
            bitmask = 0x02;
            break;
        case CS_DATA_NOBIN:
            idx = 16;
            bitmask = 0x04;
            break;
        case CS_DATA_NOVBIN:
            idx = 16;
            bitmask = 0x08;
            break;
        case CS_DATA_NOMNY8:
            idx = 16;
            bitmask = 0x10;
            break;
        case CS_DATA_NOMNY4:
            idx = 16;
            bitmask = 0x20;
            break;
        case CS_DATA_NODATE8:
            idx = 16;
            bitmask = 0x40;
            break;
        case CS_DATA_NODATE4:
            idx = 16;
            bitmask = 0x80;
            break;
        case CS_RES_NOMSG:
            idx = 17;
            bitmask = 0x02;
            break;
        case CS_RES_NOEED:
            idx = 17;
            bitmask = 0x04;
            break;
        case CS_RES_NOPARAM:
            idx = 17;
            bitmask = 0x08;
            break;
        case CS_DATA_NOINT1:
            idx = 17;
            bitmask = 0x10;
            break;
        case CS_DATA_NOINT2:
            idx = 17;
            bitmask = 0x20;
            break;
        case CS_DATA_NOINT4:
            idx = 17;
            bitmask = 0x40;
            break;
        case CS_DATA_NOBIT:
            idx = 17;
            bitmask = 0x80;
            break;
        default:
            tdsdump_log(TDS_DBG_SEVERE, "ct_capability -- attempt to set/get a non-existant capability\n");
            return CS_FAIL;
        }           /* end capability */

        assert(13 <= idx && idx <= 17);
        assert(bitmask);

        switch (action) {
        case CS_SET:
            /* Having established the offset and the bitmask, we can now turn the capability on or off */
            switch (*(CS_BOOL *) value) {
            case CS_TRUE:
                mask[idx] |= bitmask;
                break;
            case CS_FALSE:
                mask[idx] &= ~bitmask;
                break;
            default:
                tdsdump_log(TDS_DBG_SEVERE, "ct_capability -- unknown value\n");
                return CS_FAIL;
            }
            break;
        case CS_GET:
            *(CS_BOOL *) value = (mask[idx] & bitmask) ? CS_TRUE : CS_FALSE;
            break;
        default:
            tdsdump_log(TDS_DBG_SEVERE, "ct_capability -- unknown action\n");
            return CS_FAIL;
        }
        return CS_SUCCEED;
    } /* End handling CS_CAP_RESPONSE (returned) */

    /*
     * Begin handling CS_CAP_REQUEST
     * These capabilities describe the types of requests that a server can support.
     */
    switch (capability) {
    case CS_PROTO_DYNPROC:
        *(CS_BOOL *) value = mask[2] & 0x01 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_FLTN:
        *(CS_BOOL *) value = mask[2] & 0x02 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_BITN:
        *(CS_BOOL *) value = mask[2] & 0x04 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_INT8:
        *(CS_BOOL *) value = mask[2] & 0x08 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_VOID:
        *(CS_BOOL *) value = mask[2] & 0x10 ? CS_TRUE : CS_FALSE;
        break;
    case CS_CON_INBAND:
        *(CS_BOOL *) value = mask[3] & 0x01 ? CS_TRUE : CS_FALSE;
        break;
    case CS_CON_LOGICAL:
        *(CS_BOOL *) value = mask[3] & 0x02 ? CS_TRUE : CS_FALSE;
        break;
    case CS_PROTO_TEXT:
        *(CS_BOOL *) value = mask[3] & 0x04 ? CS_TRUE : CS_FALSE;
        break;
    case CS_PROTO_BULK:
        *(CS_BOOL *) value = mask[3] & 0x08 ? CS_TRUE : CS_FALSE;
        break;
    case CS_REQ_URGNOTIF:
        *(CS_BOOL *) value = mask[3] & 0x10 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_SENSITIVITY:
        *(CS_BOOL *) value = mask[3] & 0x20 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_BOUNDARY:
        *(CS_BOOL *) value = mask[3] & 0x40 ? CS_TRUE : CS_FALSE;
        break;
    case CS_PROTO_DYNAMIC:
        *(CS_BOOL *) value = mask[3] & 0x80 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_MONEYN:
        *(CS_BOOL *) value = mask[4] & 0x01 ? CS_TRUE : CS_FALSE;
        break;
    case CS_CSR_PREV:
        *(CS_BOOL *) value = mask[4] & 0x02 ? CS_TRUE : CS_FALSE;
        break;
    case CS_CSR_FIRST:
        *(CS_BOOL *) value = mask[4] & 0x04 ? CS_TRUE : CS_FALSE;
        break;
    case CS_CSR_LAST:
        *(CS_BOOL *) value = mask[4] & 0x08 ? CS_TRUE : CS_FALSE;
        break;
    case CS_CSR_ABS:
        *(CS_BOOL *) value = mask[4] & 0x10 ? CS_TRUE : CS_FALSE;
        break;
    case CS_CSR_REL:
        *(CS_BOOL *) value = mask[4] & 0x20 ? CS_TRUE : CS_FALSE;
        break;
    case CS_CSR_MULTI:
        *(CS_BOOL *) value = mask[4] & 0x40 ? CS_TRUE : CS_FALSE;
        break;
    case CS_CON_OOB:
        *(CS_BOOL *) value = mask[4] & 0x80 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_NUM:
        *(CS_BOOL *) value = mask[5] & 0x01 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_TEXT:
        *(CS_BOOL *) value = mask[5] & 0x02 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_IMAGE:
        *(CS_BOOL *) value = mask[5] & 0x04 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_DEC:
        *(CS_BOOL *) value = mask[5] & 0x08 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_LCHAR:
        *(CS_BOOL *) value = mask[5] & 0x10 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_LBIN:
        *(CS_BOOL *) value = mask[5] & 0x20 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_INTN:
        *(CS_BOOL *) value = mask[5] & 0x40 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_DATETIMEN:
        *(CS_BOOL *) value = mask[5] & 0x80 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_BIN:
        *(CS_BOOL *) value = mask[6] & 0x01 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_VBIN:
        *(CS_BOOL *) value = mask[6] & 0x02 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_MNY8:
        *(CS_BOOL *) value = mask[6] & 0x04 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_MNY4:
        *(CS_BOOL *) value = mask[6] & 0x08 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_DATE8:
        *(CS_BOOL *) value = mask[6] & 0x10 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_DATE4:
        *(CS_BOOL *) value = mask[6] & 0x20 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_FLT4:
        *(CS_BOOL *) value = mask[6] & 0x40 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_FLT8:
        *(CS_BOOL *) value = mask[6] & 0x80 ? CS_TRUE : CS_FALSE;
        break;
    case CS_REQ_MSG:
        *(CS_BOOL *) value = mask[7] & 0x01 ? CS_TRUE : CS_FALSE;
        break;
    case CS_REQ_PARAM:
        *(CS_BOOL *) value = mask[7] & 0x02 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_INT1:
        *(CS_BOOL *) value = mask[7] & 0x04 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_INT2:
        *(CS_BOOL *) value = mask[7] & 0x08 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_INT4:
        *(CS_BOOL *) value = mask[7] & 0x10 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_BIT:
        *(CS_BOOL *) value = mask[7] & 0x20 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_CHAR:
        *(CS_BOOL *) value = mask[7] & 0x40 ? CS_TRUE : CS_FALSE;
        break;
    case CS_DATA_VCHAR:
        *(CS_BOOL *) value = mask[7] & 0x80 ? CS_TRUE : CS_FALSE;
        break;
    case CS_REQ_LANG:
        *(CS_BOOL *) value = mask[8] & 0x02 ? CS_TRUE : CS_FALSE;
        break;
    case CS_REQ_RPC:
        *(CS_BOOL *) value = mask[8] & 0x04 ? CS_TRUE : CS_FALSE;
        break;
    case CS_REQ_NOTIF:
        *(CS_BOOL *) value = mask[8] & 0x08 ? CS_TRUE : CS_FALSE;
        break;
    case CS_REQ_MSTMT:
        *(CS_BOOL *) value = mask[8] & 0x10 ? CS_TRUE : CS_FALSE;
        break;
    case CS_REQ_BCP:
        *(CS_BOOL *) value = mask[8] & 0x20 ? CS_TRUE : CS_FALSE;
        break;
    case CS_REQ_CURSOR:
        *(CS_BOOL *) value = mask[8] & 0x40 ? CS_TRUE : CS_FALSE;
        break;
    case CS_REQ_DYN:
        *(CS_BOOL *) value = mask[8] & 0x80 ? CS_TRUE : CS_FALSE;
        break;
    default:
        tdsdump_log(TDS_DBG_SEVERE, "ct_capability -- attempt to get a non-existant capability\n");
        return CS_FAIL;
        break;
    }           /* end capability */

    assert(*(CS_BOOL *) value);

    /* CS_CAP_RESPONSE is read-only */
    if (type == CS_CAP_REQUEST && action == CS_GET) {
        tdsdump_log(TDS_DBG_INFO1, "ct_capability returns success, value %s\n",
            *(CS_BOOL *)value == CS_TRUE ? "TRUE" : "FALSE");
        return CS_SUCCEED;
    }

    tdsdump_log(TDS_DBG_SEVERE, "ct_capability -- attempt to set a read-only capability (type %d, action %d)\n",
        type, action);
    return CS_FAIL;
}               /* end ct_capability( */


CS_RETCODE
ct_dynamic(CS_COMMAND * cmd, CS_INT type, CS_CHAR * id, CS_INT idlen, CS_CHAR * buffer, CS_INT buflen)
{
    size_t query_len;
    CS_CONNECTION *con;
    CS_DYNAMIC *dyn;

    if (!cmd->con)
        return CS_FAIL;

    cmd->command_type = CS_DYNAMIC_CMD;
    cmd->dynamic_cmd = type;

    tdsdump_log(TDS_DBG_FUNC, "ct_dynamic(%d)\n", type);
    con = cmd->con;

    switch (type) {
    case CS_PREPARE:

        dyn = _ct_allocate_dynamic(con, id, idlen);

        if (dyn == NULL) {
            return CS_FAIL;
        }

        /* now the query */
        if (buflen == CS_NULLTERM) {
            query_len = strlen(buffer);
        } else {
            query_len = buflen;
        }
        dyn->stmt = (char *) malloc(query_len + 1);
        strncpy(dyn->stmt, (char *) buffer, query_len);
        dyn->stmt[query_len] = '\0';

        cmd->dyn = dyn;

        break;
    case CS_DEALLOC:
        cmd->dyn = _ct_locate_dynamic(con, id, idlen);
        if (cmd->dyn == NULL)
            return CS_FAIL;
        break;
    case CS_DESCRIBE_INPUT:
    case CS_DESCRIBE_OUTPUT:
        cmd->dyn = _ct_locate_dynamic(con, id, idlen);
        if (cmd->dyn == NULL)
            return CS_FAIL;
        break;
    case CS_EXECUTE:
        cmd->dyn = _ct_locate_dynamic(con, id, idlen);
        if (cmd->dyn == NULL)
            return CS_FAIL;

        tdsdump_log(TDS_DBG_FUNC, "ct_dynamic() calling param_clear\n");
        param_clear(cmd->dyn->param_list);
        cmd->dyn->param_list = NULL;
        break;
    }

    ct_set_command_state(cmd, _CS_COMMAND_READY);
    return CS_SUCCEED;
}

CS_RETCODE
ct_param(CS_COMMAND * cmd, CS_DATAFMT * datafmt, CS_VOID * data, CS_INT datalen, CS_SMALLINT indicator)
{
    CSREMOTE_PROC *rpc;
    CS_DYNAMIC *dyn;
    CS_PARAM **pparam;
    CS_PARAM *param;
    int /* bool */ promote = (datafmt->datatype == CS_VARCHAR_TYPE  ||
                              datafmt->datatype == CS_LONGCHAR_TYPE);


    tdsdump_log(TDS_DBG_FUNC, "ct_param()\n");
    tdsdump_log(TDS_DBG_INFO1, "ct_param() data addr = %p data length = %d\n", data, datalen);

    if (cmd == NULL  ||  cmd->con == NULL)
        return CS_FAIL;

    if (promote  &&  IS_TDS7_PLUS(cmd->con->tds_socket)) {
        /* Actually promote only if non-ASCII characters are present */
        CS_INT i;

        promote = 0;
        for (i = 0;  i < datalen;  ++i) {
            if (((const char*)data)[i] & 0x80) {
                promote = 1;
                break;
            }
        }

        if ( !promote ) {
            /* pure ASCII, leave as is */
        } else if (datafmt->datatype == CS_VARCHAR_TYPE) {
            datafmt->datatype = CS_NVARCHAR_TYPE;
        } else if (datafmt->datatype == CS_LONGCHAR_TYPE) {
            datafmt->datatype = CS_NLONGCHAR_TYPE;
        }
    }


    switch (cmd->command_type) {
    case CS_RPC_CMD:
        if (cmd->rpc == NULL) {
            fprintf(stdout, "RPC is NULL ct_param\n");
            return CS_FAIL;
        }

        param = (CSREMOTE_PROC_PARAM *) malloc(sizeof(CSREMOTE_PROC_PARAM));
        if (!param)
            return CS_FAIL;
        memset(param, 0, sizeof(CSREMOTE_PROC_PARAM));

        if (CS_SUCCEED != _ct_fill_param(cmd->command_type, param, datafmt, data, &datalen, &indicator, 1)) {
            tdsdump_log(TDS_DBG_INFO1, "ct_param() failed to add rpc param\n");
            tdsdump_log(TDS_DBG_INFO1, "ct_param() failed to add input value\n");
            free(param);
            return CS_FAIL;
        }

        rpc = cmd->rpc;
        pparam = &rpc->param_list;
        while (*pparam) {
            pparam = &(*pparam)->next;
        }

        *pparam = param;
        tdsdump_log(TDS_DBG_INFO1, " ct_param() added rpc parameter %s \n", (*param).name);
        return CS_SUCCEED;
        break;

    case CS_LANG_CMD:
        /* only accept CS_INPUTVALUE as the status */
        if (CS_INPUTVALUE != datafmt->status) {
            tdsdump_log(TDS_DBG_ERROR, "illegal datafmt->status(%d) passed to ct_param()\n", datafmt->status);
            return CS_FAIL;
        }

        param = (CSREMOTE_PROC_PARAM *) malloc(sizeof(CSREMOTE_PROC_PARAM));
        memset(param, 0, sizeof(CSREMOTE_PROC_PARAM));

        if (CS_SUCCEED != _ct_fill_param(cmd->command_type, param, datafmt, data, &datalen, &indicator, 1)) {
            free(param);
            return CS_FAIL;
        }

        if (NULL == cmd->input_params)
            cmd->input_params = param;
        else {
            pparam = &cmd->input_params;
            while ((*pparam)->next)
                pparam = &(*pparam)->next;
            (*pparam)->next = param;
        }
        tdsdump_log(TDS_DBG_INFO1, "ct_param() added input value\n");
        return CS_SUCCEED;
        break;

    case CS_DYNAMIC_CMD:
        if (cmd->dyn == NULL) {
            tdsdump_log(TDS_DBG_INFO1, "cmd->dyn is NULL ct_param\n");
            return CS_FAIL;
        }

        param = (CS_DYNAMIC_PARAM *) malloc(sizeof(CS_DYNAMIC_PARAM));
        if (!param)
            return CS_FAIL;
        memset(param, 0, sizeof(CS_DYNAMIC_PARAM));

        if (CS_SUCCEED != _ct_fill_param(cmd->command_type, param, datafmt, data, &datalen, &indicator, 1)) {
            tdsdump_log(TDS_DBG_INFO1, "ct_param() failed to add CS_DYNAMIC param\n");
            free(param);
            return CS_FAIL;
        }

        dyn = cmd->dyn;
        pparam = &dyn->param_list;
        while (*pparam) {
            pparam = &(*pparam)->next;
        }

        *pparam = param;
        return CS_SUCCEED;
        break;
    }
    return CS_FAIL;
}

CS_RETCODE
ct_setparam(CS_COMMAND * cmd, CS_DATAFMT * datafmt, CS_VOID * data, CS_INT * datalen, CS_SMALLINT * indicator)
{
    CSREMOTE_PROC *rpc;
    CS_PARAM **pparam;
    CS_PARAM *param;
    CS_DYNAMIC *dyn;

    tdsdump_log(TDS_DBG_FUNC, "ct_setparam() command type = %d, data type = %d\n", cmd->command_type, datafmt->datatype);

    /* Code changed for RPC functionality - SUHA */
    /* RPC code changes starts here */

    if (cmd == NULL)
        return CS_FAIL;

    switch (cmd->command_type) {

    case CS_RPC_CMD:

        if (cmd->rpc == NULL) {
            fprintf(stdout, "RPC is NULL ct_param\n");
            return CS_FAIL;
        }

        param = (CSREMOTE_PROC_PARAM *) malloc(sizeof(CSREMOTE_PROC_PARAM));
        memset(param, 0, sizeof(CSREMOTE_PROC_PARAM));

        if (CS_SUCCEED != _ct_fill_param(cmd->command_type, param, datafmt, data, datalen, indicator, 0)) {
            tdsdump_log(TDS_DBG_INFO1, "ct_setparam() failed to add rpc param\n");
            tdsdump_log(TDS_DBG_INFO1, "ct_setparam() failed to add input value\n");
            free(param);
            return CS_FAIL;
        }

        rpc = cmd->rpc;
        pparam = &rpc->param_list;
        tdsdump_log(TDS_DBG_INFO1, " ct_setparam() reached here\n");
        if (*pparam != NULL) {
            while ((*pparam)->next != NULL) {
                pparam = &(*pparam)->next;
            }

            pparam = &(*pparam)->next;
        }
        *pparam = param;
        param->next = NULL;
        tdsdump_log(TDS_DBG_INFO1, " ct_setparam() added parameter %s \n", (*param).name);
        return CS_SUCCEED;
        break;

    case CS_DYNAMIC_CMD :

        if (cmd->dyn == NULL) {
            fprintf(stdout, "cmd->dyn is NULL ct_param\n");
            return CS_FAIL;
        }

        param = (CS_DYNAMIC_PARAM *) malloc(sizeof(CS_DYNAMIC_PARAM));
        memset(param, 0, sizeof(CS_DYNAMIC_PARAM));

        if (CS_SUCCEED != _ct_fill_param(cmd->command_type, param, datafmt, data, datalen, indicator, 0)) {
            tdsdump_log(TDS_DBG_INFO1, "ct_setparam() failed to add dynamic param\n");
            free(param);
            return CS_FAIL;
        }

        dyn = cmd->dyn;
        pparam = &dyn->param_list;
        if (*pparam != NULL) {
            while ((*pparam)->next != NULL) {
                pparam = &(*pparam)->next;
            }

            pparam = &(*pparam)->next;
        }
        *pparam = param;
        param->next = NULL;
        tdsdump_log(TDS_DBG_INFO1, "ct_setparam() added dynamic parameter\n");
        return CS_SUCCEED;
        break;

    case CS_LANG_CMD:

        /* only accept CS_INPUTVALUE as the status */
        if (CS_INPUTVALUE != datafmt->status) {
            tdsdump_log(TDS_DBG_ERROR, "illegal datafmt->status(%d) passed to ct_setparam()\n", datafmt->status);
            return CS_FAIL;
        }

        param = (CSREMOTE_PROC_PARAM *) malloc(sizeof(CSREMOTE_PROC_PARAM));
        memset(param, 0, sizeof(CSREMOTE_PROC_PARAM));

        if (CS_SUCCEED != _ct_fill_param(cmd->command_type, param, datafmt, data, datalen, indicator, 0)) {
            tdsdump_log(TDS_DBG_INFO1, "ct_setparam() failed to add language param\n");
            free(param);
            return CS_FAIL;
        }

        if (NULL == cmd->input_params)
            cmd->input_params = param;
        else {
            pparam = &cmd->input_params;
            while ((*pparam)->next)
                pparam = &(*pparam)->next;
            (*pparam)->next = param;
        }
        tdsdump_log(TDS_DBG_INFO1, "ct_setparam() added language parameter\n");
        return CS_SUCCEED;
        break;
    }
    return CS_FAIL;
}

CS_RETCODE
ct_options(CS_CONNECTION * con, CS_INT action, CS_INT option, CS_VOID * param, CS_INT paramlen, CS_INT * outlen)
{
    TDS_OPTION_CMD tds_command = 0;
    TDS_OPTION tds_option = 0;
    TDS_OPTION_ARG tds_argument;
    TDS_INT tds_argsize = 0;
    TDSSOCKET *tds;

    const char *action_string = NULL;
    int i;

    /* boolean options can all be treated the same way */
    static const struct TDS_BOOL_OPTION_MAP
    {
        CS_INT option;
        TDS_OPTION tds_option;
    } tds_bool_option_map[] = {
          { CS_OPT_ANSINULL,       TDS_OPT_ANSINULL       }
        , { CS_OPT_CHAINXACTS,     TDS_OPT_CHAINXACTS     }
        , { CS_OPT_CURCLOSEONXACT, TDS_OPT_CURCLOSEONXACT }
        , { CS_OPT_FIPSFLAG,       TDS_OPT_FIPSFLAG       }
        , { CS_OPT_FORCEPLAN,      TDS_OPT_FORCEPLAN      }
        , { CS_OPT_FORMATONLY,     TDS_OPT_FORMATONLY     }
        , { CS_OPT_GETDATA,        TDS_OPT_GETDATA        }
        , { CS_OPT_NOCOUNT,        TDS_OPT_NOCOUNT        }
        , { CS_OPT_NOEXEC,         TDS_OPT_NOEXEC         }
        , { CS_OPT_PARSEONLY,      TDS_OPT_PARSEONLY      }
        , { CS_OPT_QUOTED_IDENT,   TDS_OPT_QUOTED_IDENT   }
        , { CS_OPT_RESTREES,       TDS_OPT_RESTREES       }
        , { CS_OPT_SHOWPLAN,       TDS_OPT_SHOWPLAN       }
        , { CS_OPT_STATS_IO,       TDS_OPT_STAT_IO        }
        , { CS_OPT_STATS_TIME,     TDS_OPT_STAT_TIME      }
    };

    if (param == NULL)
        return CS_FAIL;

    tds = con->tds_socket;

    /*
     * Set the tds command
     */
    switch (action) {
    case CS_GET:
        tds_command = TDS_OPT_LIST; /* will be acknowledged by TDS_OPT_INFO */
        action_string = "CS_GET";
        tds_argsize = 0;
        break;
    case CS_SET:
        tds_command = TDS_OPT_SET;
        action_string = "CS_SET";
        break;
    case CS_CLEAR:
        tds_command = TDS_OPT_DEFAULT;
        action_string = "CS_CLEAR";
        tds_argsize = 0;
        break;
    default:
        tdsdump_log(TDS_DBG_FUNC, "ct_options: invalid action = %d\n", action);
        return CS_FAIL;
    }

    assert(tds_command && action_string);

    tdsdump_log(TDS_DBG_FUNC, "ct_options: %s, option = %d\n", action_string, option);

    /*
     * Set the tds option
     *      The following TDS options apparently cannot be set with this function.
     *      TDS_OPT_CHARSET
     *      TDS_OPT_CURREAD
     *      TDS_OPT_IDENTITYOFF
     *      TDS_OPT_IDENTITYON
     *      TDS_OPT_CURWRITE
     *      TDS_OPT_NATLANG
     *      TDS_OPT_ROWCOUNT
     *      TDS_OPT_TEXTSIZE
     */

    /*
     * First, take care of the easy cases, the booleans.
     */
    for (i = 0; i < TDS_VECTOR_SIZE(tds_bool_option_map); i++) {
        if (tds_bool_option_map[i].option == option) {
            tds_option = tds_bool_option_map[i].tds_option;
            break;
        }
    }

    if (tds_option != 0) {  /* found a boolean */
        if (action == CS_SET) {
            switch (*(CS_BOOL *) param) {
            case CS_TRUE:
                tds_argument.ti = 1;
                break;
            case CS_FALSE:
                tds_argument.ti = 0;
                break;
            default:
                return CS_FAIL;
            }
            tds_argsize = 1;
        }
        if (action == CS_GET) {
            tds_argsize = 0;
        }
        goto SEND_OPTION;
    }

    /*
     * Non-booleans are more complicated.
     */
    switch (option) {
    case CS_OPT_ANSIPERM:
    case CS_OPT_STR_RTRUNC:
        /* no documented tds option */
        switch (*(CS_BOOL *) param) {
        case CS_TRUE:
        case CS_FALSE:
            break;  /* end valid choices */
        default:
            if (action == CS_SET)
                return CS_FAIL;
        }
        break;
    case CS_OPT_ARITHABORT:
        switch (*(CS_BOOL *) param) {
        case CS_TRUE:
            tds_option = TDS_OPT_ARITHABORTON;
            break;
        case CS_FALSE:
            tds_option = TDS_OPT_ARITHABORTOFF;
            break;
        default:
            if (action == CS_SET)
                return CS_FAIL;
            tds_option = TDS_OPT_ARITHABORTON;
        }
        tds_argument.ti = TDS_OPT_ARITHOVERFLOW | TDS_OPT_NUMERICTRUNC;
        tds_argsize = (action == CS_SET) ? 1 : 0;
        break;
    case CS_OPT_ARITHIGNORE:
        switch (*(CS_BOOL *) param) {
        case CS_TRUE:
            tds_option = TDS_OPT_ARITHIGNOREON;
            break;
        case CS_FALSE:
            tds_option = TDS_OPT_ARITHIGNOREOFF;
            break;
        default:
            if (action == CS_SET)
                return CS_FAIL;
        }
        tds_argument.i = TDS_OPT_ARITHOVERFLOW | TDS_OPT_NUMERICTRUNC;
        tds_argsize = (action == CS_SET) ? 4 : 0;
        break;
    case CS_OPT_AUTHOFF:
        tds_option = TDS_OPT_AUTHOFF;
        tds_argument.c = (TDS_CHAR *) param;
        tds_argsize = (action == CS_SET) ? paramlen : 0;
        break;
    case CS_OPT_AUTHON:
        tds_option = TDS_OPT_AUTHON;
        tds_argument.c = (TDS_CHAR *) param;
        tds_argsize = (action == CS_SET) ? paramlen : 0;
        break;

    case CS_OPT_DATEFIRST:
        tds_option = TDS_OPT_DATEFIRST;
        switch (*(CS_INT *) param) {
        case CS_OPT_SUNDAY:
            tds_argument.ti = TDS_OPT_SUNDAY;
            break;
        case CS_OPT_MONDAY:
            tds_argument.ti = TDS_OPT_MONDAY;
            break;
        case CS_OPT_TUESDAY:
            tds_argument.ti = TDS_OPT_TUESDAY;
            break;
        case CS_OPT_WEDNESDAY:
            tds_argument.ti = TDS_OPT_WEDNESDAY;
            break;
        case CS_OPT_THURSDAY:
            tds_argument.ti = TDS_OPT_THURSDAY;
            break;
        case CS_OPT_FRIDAY:
            tds_argument.ti = TDS_OPT_FRIDAY;
            break;
        case CS_OPT_SATURDAY:
            tds_argument.ti = TDS_OPT_SATURDAY;
            break;
        default:
            if (action == CS_SET)
                return CS_FAIL;
        }
        tds_argsize = (action == CS_SET) ? 1 : 0;
        break;
    case CS_OPT_DATEFORMAT:
        tds_option = TDS_OPT_DATEFORMAT;
        switch (*(CS_INT *) param) {
        case CS_OPT_FMTMDY:
            tds_argument.ti = TDS_OPT_FMTMDY;
            break;
        case CS_OPT_FMTDMY:
            tds_argument.ti = TDS_OPT_FMTDMY;
            break;
        case CS_OPT_FMTYMD:
            tds_argument.ti = TDS_OPT_FMTYMD;
            break;
        case CS_OPT_FMTYDM:
            tds_argument.ti = TDS_OPT_FMTYDM;
            break;
        case CS_OPT_FMTMYD:
            tds_argument.ti = TDS_OPT_FMTMYD;
            break;
        case CS_OPT_FMTDYM:
            tds_argument.ti = TDS_OPT_FMTDYM;
            break;
        default:
            if (action == CS_SET)
                return CS_FAIL;
        }
        tds_argsize = (action == CS_SET) ? 1 : 0;
        break;
    case CS_OPT_ISOLATION:
        tds_option = TDS_OPT_ISOLATION;
        switch (*(char *) param) {
        case CS_OPT_LEVEL0: /* CS_OPT_LEVEL0 requires SQL Server version 11.0 or later or Adaptive Server. */
            /* no documented value */
            tds_option = 0;
            tds_argument.ti = 0;
            break;
        case CS_OPT_LEVEL1:
            tds_argument.ti = TDS_OPT_LEVEL1;
        case CS_OPT_LEVEL3:
            tds_argument.ti = TDS_OPT_LEVEL3;
            break;
        default:
            if (action == CS_SET)
                return CS_FAIL;
        }
        tds_argsize = (action == CS_SET) ? 1 : 0;
        break;
    case CS_OPT_TRUNCIGNORE:
        tds_option = TDS_OPT_TRUNCABORT;    /* note inverted sense */
        switch (*(CS_BOOL *) param) {
        case CS_TRUE:
        case CS_FALSE:
            break;
        default:
            if (action == CS_SET)
                return CS_FAIL;
        }
        tds_argument.ti = !*(char *) param;
        tds_argsize = (action == CS_SET) ? 1 : 0;
        break;
    default:
        return CS_FAIL; /* invalid option */
    }

SEND_OPTION:

    tdsdump_log(TDS_DBG_FUNC, "\ttds_submit_optioncmd will be action(%s) option(%d) arg(%x) arglen(%d)\n",
                action_string, tds_option, tds_argsize == 1 ? tds_argument.ti : (tds_argsize == 4 ? tds_argument.i : 0), tds_argsize);

    if (tds_submit_optioncmd(tds, tds_command, tds_option, &tds_argument, tds_argsize) == TDS_FAIL) {
        return CS_FAIL;
    }

    if (action == CS_GET) {
        switch (option) {
        case CS_OPT_ANSINULL :
        case CS_OPT_CHAINXACTS :
        case CS_OPT_CURCLOSEONXACT :
        case CS_OPT_NOCOUNT :
        case CS_OPT_QUOTED_IDENT :
            *(CS_BOOL *)param = tds->option_value;
            break;
        case CS_OPT_DATEFIRST:
            switch (tds->option_value) {
            case TDS_OPT_SUNDAY: *(CS_INT *)param = CS_OPT_SUNDAY; break;
            case TDS_OPT_MONDAY: *(CS_INT *)param = CS_OPT_MONDAY; break;
            case TDS_OPT_TUESDAY: *(CS_INT *)param = CS_OPT_TUESDAY; break;
            case TDS_OPT_WEDNESDAY: *(CS_INT *)param = CS_OPT_WEDNESDAY; break;
            case TDS_OPT_THURSDAY: *(CS_INT *)param = CS_OPT_THURSDAY; break;
            case TDS_OPT_FRIDAY: *(CS_INT *)param = CS_OPT_FRIDAY; break;
            case TDS_OPT_SATURDAY: *(CS_INT *)param = CS_OPT_SATURDAY; break;
            default: return CS_FAIL;
            }
            break;
        case CS_OPT_DATEFORMAT:
            switch (tds->option_value) {
            case TDS_OPT_FMTDMY: *(CS_INT *)param = CS_OPT_FMTDMY; break;
            case TDS_OPT_FMTDYM: *(CS_INT *)param = CS_OPT_FMTDYM; break;
            case TDS_OPT_FMTMDY: *(CS_INT *)param = CS_OPT_FMTMDY; break;
            case TDS_OPT_FMTMYD: *(CS_INT *)param = CS_OPT_FMTMYD; break;
            case TDS_OPT_FMTYMD: *(CS_INT *)param = CS_OPT_FMTYMD; break;
            case TDS_OPT_FMTYDM: *(CS_INT *)param = CS_OPT_FMTYDM; break;
            default: return CS_FAIL;
            }
            break;
        case CS_OPT_ARITHABORT :
        case CS_OPT_ARITHIGNORE :
            *(CS_BOOL *)param = tds->option_value;
            break;
        case CS_OPT_TRUNCIGNORE :
            break;
        }
    }

    return CS_SUCCEED;
}               /* end ct_options() */

CS_RETCODE
ct_poll(CS_CONTEXT * ctx, CS_CONNECTION * connection, CS_INT milliseconds, CS_CONNECTION ** compconn, CS_COMMAND ** compcmd,
    CS_INT * compid, CS_INT * compstatus)
{
    tdsdump_log(TDS_DBG_FUNC, "UNIMPLEMENTED ct_poll()\n");
    return CS_FAIL;
}

static
int str_icmp(char* s1, char* s2, int len)
{
    int i;

    for (i = 0; i < len; ++i) {
        char c1 = s1[i];
        char c2 = s2[i];

        if (c1 >= 'a' && c1 <= 'z') {
            c1 -= 'a'-'A';
        }

        if (c2 >= 'a' && c2 <= 'z') {
            c2 -= 'a'-'A';
        }

        if (c1 == c2) {
            if (c1 == '\0') {
                return 0;
            }
        } else {
            if (c1 < c2) {
                return -1;
            } else {
                return 1;
            }
        }
    }

    return 0;
}

static
char* get_next_tok(char* str, char* delimiter, char **ptrptr)
{
    char* result = NULL;
    *ptrptr = NULL;

    if (str && delimiter) {
        size_t str_len = strlen(str);

        size_t pos = strspn(str, delimiter);

        if (pos == 0) {
            *ptrptr = strpbrk(str, delimiter);
            return str;
        } else {
            if (pos != str_len) {
                result = str + pos;
                *ptrptr = strpbrk(result, delimiter);
            }
        }
    }

    return result;
}

CS_RETCODE
ct_cursor(CS_COMMAND * cmd, CS_INT type, CS_CHAR * name, CS_INT namelen, CS_CHAR * text, CS_INT tlen, CS_INT option)
{
    TDSSOCKET *tds;
    TDSCURSOR *cursor;

    if (!cmd->con || !cmd->con->tds_socket)
        return CS_FAIL;

    tds = cmd->con->tds_socket;
    cmd->command_type = CS_CUR_CMD;

    tdsdump_log(TDS_DBG_FUNC, "ct_cursor() : type = %d \n", type);

    switch (type) {
    case CS_CURSOR_DECLARE:

        cursor = tds_alloc_cursor(tds, name, namelen == CS_NULLTERM ? strlen(name) + 1 : namelen,
                        text, tlen == CS_NULLTERM ? strlen(text) + 1 : tlen);
        if (!cursor)
            return CS_FAIL;

        cursor->cursor_rows = 1;
        cursor->options = option;
        cursor->status.declare    = _CS_CURS_TYPE_REQUESTED;
        cursor->status.cursor_row = _CS_CURS_TYPE_UNACTIONED;
        cursor->status.open       = _CS_CURS_TYPE_UNACTIONED;
        cursor->status.fetch      = _CS_CURS_TYPE_UNACTIONED;
        cursor->status.close      = _CS_CURS_TYPE_UNACTIONED;
        cursor->status.dealloc    = _CS_CURS_TYPE_UNACTIONED;

        /* ssikorsk */
        if (option == CS_UNUSED || (option & CS_END) != 0) {
            /* Try to figure out type of the cursor. */
            char delimiter[] = "\n\t,.[]() ";
            int state = 0;
            char* savept = NULL;
            char* s = text;

            char* tok = get_next_tok(s, delimiter, &savept);
            while (tok != NULL) {
                s = savept;

                if (str_icmp(tok, "FOR", 3) == 0) {
                    state = 1;
                } else if (str_icmp(tok, "UPDATE", 6) == 0) {
                    if (state == 1) {
                        state = 2;
                        break;
                    }
                } else {
                    state = 0;
                }

                tok = get_next_tok(s, delimiter, &savept);
            }

            if (state == 2) {
                /* FOR UPDATE */
                cursor->type = 0x4; /* Forward-only cursor. */
            } else {
                /* readonly */
                cursor->type = 0x1; /* Keyset-driven cursor. Default value. */
            }
        } else if ((option & CS_FOR_UPDATE) != 0) {
            cursor->type = 0x4; /* Forward-only cursor. */
        } else {
            cursor->type = 0x1; /* Keyset-driven cursor. Default value. */
        }

        cursor->concurrency = 0x2000 | 0x4; /* Optimistic. Checks timestamps and, when not available, values. */

        cmd->cursor = cursor;
        ct_set_command_state(cmd, _CS_COMMAND_READY);
        return CS_SUCCEED;
        break;

    case CS_CURSOR_ROWS:

        cursor = cmd->cursor;
        if (!cursor) {
            tdsdump_log(TDS_DBG_FUNC, "ct_cursor() : cursor not present\n");
            return CS_FAIL;
        }

        if (cursor->status.declare == _CS_CURS_TYPE_REQUESTED ||
            cursor->status.declare == _CS_CURS_TYPE_SENT) {

            cursor->cursor_rows = option;
            cursor->status.cursor_row = _CS_CURS_TYPE_REQUESTED;

            ct_set_command_state(cmd, _CS_COMMAND_READY);
            return CS_SUCCEED;
        }
        else {
            cursor->status.cursor_row  = _CS_CURS_TYPE_UNACTIONED;
            tdsdump_log(TDS_DBG_FUNC, "ct_cursor() : cursor not declared\n");
            return CS_FAIL;
        }
        break;

    case CS_CURSOR_OPEN:

        cursor = cmd->cursor;
        if (!cursor) {
            tdsdump_log(TDS_DBG_FUNC, "ct_cursor() : cursor not present\n");
            return CS_FAIL;
        }

        if (cursor->status.declare == _CS_CURS_TYPE_REQUESTED ||
            cursor->status.declare == _CS_CURS_TYPE_SENT ) {

            cursor->status.open  = _CS_CURS_TYPE_REQUESTED;

            return CS_SUCCEED;
            ct_set_command_state(cmd, _CS_COMMAND_READY);
        }
        else {
            cursor->status.open = _CS_CURS_TYPE_UNACTIONED;
            tdsdump_log(TDS_DBG_FUNC, "ct_cursor() : cursor not declared\n");
            return CS_FAIL;
        }

        break;

    case CS_CURSOR_CLOSE:

        cursor = cmd->cursor;
        if (!cursor) {
            tdsdump_log(TDS_DBG_FUNC, "ct_cursor() : cursor not present\n");
            return CS_FAIL;
        }

        cursor->status.cursor_row = _CS_CURS_TYPE_UNACTIONED;
        cursor->status.open       = _CS_CURS_TYPE_UNACTIONED;
        cursor->status.fetch      = _CS_CURS_TYPE_UNACTIONED;
        cursor->status.close      = _CS_CURS_TYPE_REQUESTED;
        if (option == CS_DEALLOC) {
            cursor->status.dealloc   = _CS_CURS_TYPE_REQUESTED;
        }
        ct_set_command_state(cmd, _CS_COMMAND_READY);
        return CS_SUCCEED;

    case CS_CURSOR_DEALLOC:

        cursor = cmd->cursor;
        if (!cursor) {
            tdsdump_log(TDS_DBG_FUNC, "ct_cursor() : cursor not present\n");
            return CS_FAIL;
        }
        cursor->status.dealloc   = _CS_CURS_TYPE_REQUESTED;
        ct_set_command_state(cmd, _CS_COMMAND_READY);
        return CS_SUCCEED;

    case CS_IMPLICIT_CURSOR:
        tdsdump_log(TDS_DBG_INFO1, "CS_IMPLICIT_CURSOR: Option not implemented\n");
        return CS_FAIL;
    case CS_CURSOR_OPTION:
        tdsdump_log(TDS_DBG_INFO1, "CS_CURSOR_OPTION: Option not implemented\n");
        return CS_FAIL;
    case CS_CURSOR_UPDATE:
        tdsdump_log(TDS_DBG_INFO1, "CS_CURSOR_UPDATE: Option not implemented\n");
        return CS_FAIL;
    case CS_CURSOR_DELETE:
        tdsdump_log(TDS_DBG_INFO1, "CS_CURSOR_DELETE: Option not implemented\n");
        return CS_FAIL;

    }

    return CS_FAIL;
}

static int
_ct_fetchable_results(CS_COMMAND * cmd)
{
    switch (cmd->curr_result_type) {
    case CS_COMPUTE_RESULT:
    case CS_CURSOR_RESULT:
    case CS_PARAM_RESULT:
    case CS_ROW_RESULT:
    case CS_STATUS_RESULT:
        return 1;
    }
    return 0;
}

static int
_ct_process_return_status(TDSSOCKET * tds)
{
    TDSRESULTINFO *info;
    TDSCOLUMN *curcol;
    TDS_INT saved_status;

    enum
    { num_cols = 1 };

    assert(tds);
    saved_status = tds->ret_status;
    tds_free_all_results(tds);

    /* allocate the columns structure */
    tds->current_results = tds->res_info = tds_alloc_results(num_cols);

    if (!tds->res_info)
        return TDS_FAIL;

    info = tds->res_info;

    curcol = info->columns[0];

    tds_set_column_type(tds, curcol, SYBINT4);

    tdsdump_log(TDS_DBG_INFO1, "generating return status row. type = %d(%s), varint_size %d\n",
            curcol->column_type, tds_prtype(curcol->column_type), curcol->column_varint_size);

    tds_add_row_column_size(info, curcol);

    info->current_row = tds_alloc_row(info);

    if (!info->current_row)
        return TDS_FAIL;

    assert(0 <= curcol->column_offset && curcol->column_offset < info->row_size);

    *(TDS_INT *) (info->current_row + curcol->column_offset) = saved_status;

    return TDS_SUCCEED;
}

/* Code added for RPC functionality  - SUHA */
/* RPC code changes starts here */

static const unsigned char *
paramrowalloc(TDSPARAMINFO * params, TDSCOLUMN * curcol, int param_num, void *value, int size)
{
    const unsigned char *row = tds_alloc_param_row(params, curcol);

    tdsdump_log(TDS_DBG_INFO1, "paramrowalloc, size = %d, offset = %d, row_size = %d\n",
                size, curcol->column_offset,
                params->row_size);
    if (!row)
        return NULL;

    if (value) {
        /* TODO check for BLOB and numeric */
        if (size > curcol->column_size)
            size = curcol->column_size;

        if (is_blob_type(curcol->column_type)  &&  size > 0) {
            TDSBLOB *blob = (TDSBLOB*)&params->current_row[curcol->column_offset];
            blob->textvalue = (TDS_CHAR *) malloc(size);
            memcpy(blob->textvalue, value, size);
        }
        else {
            memcpy(&params->current_row[curcol->column_offset], value, size);
        }
        curcol->column_cur_size = size;
    } else {
        tdsdump_log(TDS_DBG_FUNC, "paramrowalloc(): setting parameter #%d to NULL\n", param_num);
        curcol->column_cur_size = -1;
    }

    return row;
}

/**
 * Allocate memory and copy the rpc information into a TDSPARAMINFO structure.
 */
static TDSPARAMINFO *
paraminfoalloc(TDSSOCKET * tds, CS_PARAM * first_param)
{
    int i;
    CS_PARAM *p;
    TDSCOLUMN *pcol;
    TDSPARAMINFO *params = NULL;

    TDS_SERVER_TYPE temp_type;
    CS_BYTE *temp_value;
    CS_INT temp_datalen;
    int param_is_null;


    /* sanity */
    if (first_param == NULL)
        return NULL;

    for (i = 0, p = first_param; p != NULL; p = p->next, i++) {
        const unsigned char *prow;

        if (!(params = tds_alloc_param_result(params))) {
            _csclient_msg(((CS_CONNECTION*)tds->parent)->ctx, "paraminfoalloc", 2, 1, 17, 33, "");
            return NULL;
        }

        /* The parameteter data has been passed by reference */
        /* i.e. using ct_setparam rather than ct_param       */

        if (p->param_by_value == 0) {

            param_is_null = 0;
            temp_datalen = 0;
            temp_value = NULL;
            temp_type = (TDS_SERVER_TYPE) p->type;

            /* here's one way of passing a null parameter */

            if (*(p->ind) == -1) {
                temp_value = NULL;
                temp_datalen = 0;
                param_is_null = 1;
            } else {

                /* and here's another... */
                if ((*(p->datalen) == 0 || *(p->datalen) == CS_UNUSED) && p->value == NULL) {
                    temp_value = NULL;
                    temp_datalen = 0;
                    param_is_null = 1;
                } else {

                    /* datafmt.datalen is ignored for fixed length types */

                    if (is_fixed_type(temp_type)) {
                        temp_datalen = tds_get_size_by_type(temp_type);
                    } else {
                        temp_datalen = (*(p->datalen) == CS_UNUSED) ? 0 : *(p->datalen);
                    }

                    if (temp_datalen && p->value) {
                        temp_value = p->value;
                    } else {
                        temp_value = NULL;
                        temp_datalen = 0;
                        param_is_null = 1;
                    }
                }
            }

            if (param_is_null) {
                switch (temp_type) {
                case SYBINT1:
                case SYBINT2:
                case SYBINT4:
                /* TODO check if supported ?? */
                case SYBINT8:
                    temp_type = SYBINTN;
                    break;
                case SYBDATETIME:
                case SYBDATETIME4:
                    temp_type = SYBDATETIMN;
                    break;
                case SYBFLT8:
                    temp_type = SYBFLTN;
                    break;
                case SYBBIT:
                    temp_type = SYBBITN;
                    break;
                case SYBMONEY:
                case SYBMONEY4:
                    temp_type = SYBMONEYN;
                    break;
                default:
                    break;
                }
            }
        } else {
            temp_type = (TDS_SERVER_TYPE) p->type;
            temp_value = p->value;
            temp_datalen = *(p->datalen);
        }

        pcol = params->columns[i];

        /* meta data */
        pcol->column_namelen = 0;
        if (p->name) {
            tds_strlcpy(pcol->column_name, p->name, sizeof(pcol->column_name));
            pcol->column_namelen = (TDS_SMALLINT) strlen(pcol->column_name);
        }

        if (p->status == CS_RETURN)
            pcol->column_output = 1;
        else
            pcol->column_output = 0;

        tds_set_param_type(tds, pcol, temp_type);

        if (temp_datalen == CS_NULLTERM && temp_value)
            temp_datalen = strlen((const char*) temp_value);

        pcol->column_prec = p->precision;
        pcol->column_scale = p->scale;
        if (pcol->column_varint_size) {
            if (p->maxlen < 0)
                return NULL;
            pcol->on_server.column_size = pcol->column_size = p->maxlen;
            pcol->column_cur_size = temp_datalen;
            if (temp_datalen > 0 && temp_datalen > p->maxlen)
                pcol->on_server.column_size = pcol->column_size = temp_datalen;
        } else {
            pcol->column_cur_size = pcol->column_size;
        }

        /* actual data */
        tdsdump_log(TDS_DBG_FUNC, "paraminfoalloc: status = %d, maxlen %d \n", p->status, p->maxlen);
        tdsdump_log(TDS_DBG_FUNC,
                "paraminfoalloc: name = %*.*s, varint size %d column_type %d size %d, %d column_cur_size %d column_output = %d\n",
                pcol->column_namelen, pcol->column_namelen, pcol->column_name,
                pcol->column_varint_size, pcol->column_type,
                pcol->on_server.column_size, pcol->column_size,
                pcol->column_cur_size, pcol->column_output);
        prow = paramrowalloc(params, pcol, i, temp_value, temp_datalen);
        if (!prow) {
            _csclient_msg(((CS_CONNECTION*)tds->parent)->ctx, "paraminfoalloc", 2, 1, 17, 33, "");
            return NULL;
        }
    }

    return params;

}

static void
rpc_clear(CSREMOTE_PROC * rpc)
{
    if (rpc == NULL)
        return;

    param_clear(rpc->param_list);

    assert(rpc->name);
    free(rpc->name);
    free(rpc);
}

/**
 * recursively erase the parameter list
 */
static void
param_clear(CS_PARAM * pparam)
{
    if (pparam == NULL)
        return;

    if (pparam->next) {
        param_clear(pparam->next);
        pparam->next = NULL;
    }

    /* free self after clearing children */

    if (pparam->name)
        free(pparam->name);
    if (pparam->param_by_value)
        free(pparam->value);

    /*
     * DO NOT free datalen or ind, they are just pointer
     * to client data or private structure
     */

    free(pparam);
}

/* RPC Code changes ends here */


static int
_ct_fill_param(CS_INT cmd_type, CS_PARAM * param, CS_DATAFMT * datafmt, CS_VOID * data, CS_INT * datalen, CS_SMALLINT * indicator, CS_BYTE byvalue)
{
    int param_is_null = 0;

    if (cmd_type == CS_DYNAMIC_CMD) {
        param->name = NULL;
    } else {
        if (datafmt->namelen == CS_NULLTERM) {
            param->name = strdup(datafmt->name);
            if (param->name == NULL)
                return CS_FAIL;
        } else if (datafmt->namelen > 0) {
            param->name = malloc(datafmt->namelen + 1);
            if (param->name == NULL)
                return CS_FAIL;
            memset(param->name, 0, datafmt->namelen + 1);
            strncpy(param->name, datafmt->name, datafmt->namelen);
        } else {
            param->name = NULL;
        }
    }

    param->status = datafmt->status;
    tdsdump_log(TDS_DBG_INFO1, " _ct_fill_param() status = %d \n", param->status);

    /*
     * translate datafmt.datatype, e.g. CS_SMALLINT_TYPE
     * to Server type, e.g. SYBINT2
     */
    param->type = _ct_get_server_type(datafmt->datatype);

    if (is_numeric_type(param->type)) {
        param->scale = datafmt->scale;
        param->precision = datafmt->precision;
        if (param->scale < 0 || param->precision < 0
            || param->precision > 77 || param->scale > param->precision)
            return CS_FAIL;
    }

    param->maxlen = datafmt->maxlength;

    if (is_fixed_type(param->type)) {
        param->maxlen = tds_get_size_by_type(param->type);
    }

    param->param_by_value = byvalue;

    if (byvalue) {
        param->datalen = &param->datalen_value;
        *(param->datalen) = *datalen;

        param->ind = &param->indicator_value;
        *(param->ind) = *indicator;

        /* here's one way of passing a null parameter */

        if (*indicator == -1) {
            param->value = NULL;
            *(param->datalen) = 0;
            param_is_null = 1;
        } else {

            /* and here's another... */
            if ((*datalen == 0 || *datalen == CS_UNUSED) && data == NULL) {
                param->value = NULL;
                *(param->datalen) = 0;
                param_is_null = 1;
            } else {

                /* datafmt.datalen is ignored for fixed length types */

                if (is_fixed_type(param->type)) {
                    *(param->datalen) = tds_get_size_by_type(param->type);
                } else {
                    *(param->datalen) = (*datalen == CS_UNUSED) ? 0 : *datalen;
                }

                if (data) {
                    if (*(param->datalen) == CS_NULLTERM) {
                        *(param->datalen) = strlen((char*)data);
                        tdsdump_log(TDS_DBG_INFO1, " _ct_fill_param() about to strdup string %u bytes long\n",
                                (unsigned int) *(param->datalen));
                    } else if (*(param->datalen) < 0) {
                        return CS_FAIL;
                    }
                    param->value = malloc(*(param->datalen) ? *(param->datalen)
                                          : 1);
                    if (param->value == NULL)
                        return CS_FAIL;
                    memcpy(param->value, data, *(param->datalen));
                    param->param_by_value = 1;
                } else {
                    param->value = NULL;
                    *(param->datalen) = 0;
                    param_is_null = 1;
                }
            }
        }

        if (param_is_null) {
            switch (param->type) {
            case SYBINT1:
            case SYBINT2:
            case SYBINT4:
            /* TODO check if supported ?? */
            case SYBINT8:
                param->type = SYBINTN;
                break;
            case SYBDATETIME:
            case SYBDATETIME4:
                param->type = SYBDATETIMN;
                break;
            case SYBFLT8:
                param->type = SYBFLTN;
                break;
            case SYBBIT:
                param->type = SYBBITN;
                break;
            case SYBMONEY:
            case SYBMONEY4:
                param->type = SYBMONEYN;
                break;
            default:
                break;
            }
        }
    } else {        /* not by value, i.e. by reference */
        param->datalen = datalen;
        param->ind = indicator;
        param->value = data;
    }
    return CS_SUCCEED;
}

/* Code added for ct_diag implementation */
/* Code changes start here - CT_DIAG - 02*/

CS_RETCODE
ct_diag(CS_CONNECTION * conn, CS_INT operation, CS_INT type, CS_INT idx, CS_VOID * buffer)
{
    switch (operation) {
    case CS_INIT:
        if (conn->ctx->cs_errhandletype == _CS_ERRHAND_CB) {
            /* contrary to the manual page you don't seem to */
            /* be able to turn on inline message handling    */
            /* using cs_diag, once a callback is installed!  */
            return CS_FAIL;
        }

        conn->ctx->cs_errhandletype = _CS_ERRHAND_INLINE;

        if (conn->ctx->cs_diag_msglimit_client == 0)
            conn->ctx->cs_diag_msglimit_client = CS_NO_LIMIT;

        if (conn->ctx->cs_diag_msglimit_server == 0)
            conn->ctx->cs_diag_msglimit_server = CS_NO_LIMIT;

        if (conn->ctx->cs_diag_msglimit_total == 0)
            conn->ctx->cs_diag_msglimit_total = CS_NO_LIMIT;

        conn->ctx->_clientmsg_cb = (CS_CLIENTMSG_FUNC) ct_diag_storeclientmsg;
        conn->ctx->_servermsg_cb = (CS_SERVERMSG_FUNC) ct_diag_storeservermsg;

        break;

    case CS_MSGLIMIT:
        if (conn->ctx->cs_errhandletype != _CS_ERRHAND_INLINE)
            return CS_FAIL;

        if (type == CS_CLIENTMSG_TYPE)
            conn->ctx->cs_diag_msglimit_client = *(CS_INT *) buffer;

        if (type == CS_SERVERMSG_TYPE)
            conn->ctx->cs_diag_msglimit_server = *(CS_INT *) buffer;

        if (type == CS_ALLMSG_TYPE)
            conn->ctx->cs_diag_msglimit_total = *(CS_INT *) buffer;

        break;

    case CS_CLEAR:
        if (conn->ctx->cs_errhandletype != _CS_ERRHAND_INLINE)
            return CS_FAIL;
        return _ct_diag_clearmsg(conn->ctx, type);
        break;

    case CS_GET:
        if (conn->ctx->cs_errhandletype != _CS_ERRHAND_INLINE)
            return CS_FAIL;

        if (buffer == NULL)
            return CS_FAIL;

        if (type == CS_CLIENTMSG_TYPE) {
            if (idx == 0
                || (conn->ctx->cs_diag_msglimit_client != CS_NO_LIMIT && idx > conn->ctx->cs_diag_msglimit_client))
                return CS_FAIL;

            return (ct_diag_getclientmsg(conn->ctx, idx, (CS_CLIENTMSG *) buffer));
        }

        if (type == CS_SERVERMSG_TYPE) {
            if (idx == 0
                || (conn->ctx->cs_diag_msglimit_server != CS_NO_LIMIT && idx > conn->ctx->cs_diag_msglimit_server))
                return CS_FAIL;
            return (ct_diag_getservermsg(conn->ctx, idx, (CS_SERVERMSG *) buffer));
        }

        break;

    case CS_STATUS:
        if (conn->ctx->cs_errhandletype != _CS_ERRHAND_INLINE)
            return CS_FAIL;
        if (buffer == NULL)
            return CS_FAIL;

        return (ct_diag_countmsg(conn->ctx, type, (CS_INT *) buffer));
        break;
    }
    return CS_SUCCEED;
}

static CS_INT
ct_diag_storeclientmsg(CS_CONTEXT * context, CS_CONNECTION * conn, CS_CLIENTMSG * message)
{
    struct cs_diag_msg_client **curptr;
    struct cs_diag_msg_svr **scurptr;

    CS_INT msg_count = 0;

    curptr = &(conn->ctx->clientstore);

    scurptr = &(conn->ctx->svrstore);

    /* if we already have a list of messages, */
    /* go to the end of the list...           */

    while (*curptr != NULL) {
        msg_count++;
        curptr = &((*curptr)->next);
    }

    /* messages over and above the agreed limit */
    /* are simply discarded...                  */

    if (conn->ctx->cs_diag_msglimit_client != CS_NO_LIMIT && msg_count >= conn->ctx->cs_diag_msglimit_client) {
        return CS_FAIL;
    }

    /* messages over and above the agreed TOTAL limit */
    /* are simply discarded */

    if (conn->ctx->cs_diag_msglimit_total != CS_NO_LIMIT) {
        while (*scurptr != NULL) {
            msg_count++;
            scurptr = &((*scurptr)->next);
        }
        if (msg_count >= conn->ctx->cs_diag_msglimit_total) {
            return CS_FAIL;
        }
    }

    *curptr = (struct cs_diag_msg_client *) malloc(sizeof(struct cs_diag_msg_client));
    if (*curptr == NULL) {
        return CS_FAIL;
    } else {
        (*curptr)->next = NULL;
        (*curptr)->clientmsg = malloc(sizeof(CS_CLIENTMSG));
        if ((*curptr)->clientmsg == NULL) {
            return CS_FAIL;
        } else {
            memcpy((*curptr)->clientmsg, message, sizeof(CS_CLIENTMSG));
        }
    }

    return CS_SUCCEED;
}

static CS_INT
ct_diag_storeservermsg(CS_CONTEXT * context, CS_CONNECTION * conn, CS_SERVERMSG * message)
{
    struct cs_diag_msg_svr **curptr;
    struct cs_diag_msg_client **ccurptr;

    CS_INT msg_count = 0;

    curptr = &(conn->ctx->svrstore);
    ccurptr = &(conn->ctx->clientstore);

    /* if we already have a list of messages, */
    /* go to the end of the list...           */

    while (*curptr != NULL) {
        msg_count++;
        curptr = &((*curptr)->next);
    }

    /* messages over and above the agreed limit */
    /* are simply discarded...                  */

    if (conn->ctx->cs_diag_msglimit_server != CS_NO_LIMIT && msg_count >= conn->ctx->cs_diag_msglimit_server) {
        return CS_FAIL;
    }

    /* messages over and above the agreed TOTAL limit */
    /* are simply discarded...                  */

    if (conn->ctx->cs_diag_msglimit_total != CS_NO_LIMIT) {
        while (*ccurptr != NULL) {
            msg_count++;
            ccurptr = &((*ccurptr)->next);
        }
        if (msg_count >= conn->ctx->cs_diag_msglimit_total) {
            return CS_FAIL;
        }
    }

    *curptr = (struct cs_diag_msg_svr *) malloc(sizeof(struct cs_diag_msg_svr));
    if (*curptr == NULL) {
        return CS_FAIL;
    } else {
        (*curptr)->next = NULL;
        (*curptr)->servermsg = malloc(sizeof(CS_SERVERMSG));
        if ((*curptr)->servermsg == NULL) {
            return CS_FAIL;
        } else {
            memcpy((*curptr)->servermsg, message, sizeof(CS_SERVERMSG));
        }
    }

    return CS_SUCCEED;
}

static CS_INT
ct_diag_getclientmsg(CS_CONTEXT * context, CS_INT idx, CS_CLIENTMSG * message)
{
    struct cs_diag_msg_client *curptr;
    CS_INT msg_count = 0, msg_found = 0;

    curptr = context->clientstore;

    /* if we already have a list of messages, */
    /* go to the end of the list...           */

    while (curptr != NULL) {
        msg_count++;
        if (msg_count == idx) {
            msg_found++;
            break;
        }
        curptr = curptr->next;
    }

    if (msg_found) {
        memcpy(message, curptr->clientmsg, sizeof(CS_CLIENTMSG));
        return CS_SUCCEED;
    }
    return CS_NOMSG;
}

static CS_INT
ct_diag_getservermsg(CS_CONTEXT * context, CS_INT idx, CS_SERVERMSG * message)
{
    struct cs_diag_msg_svr *curptr;
    CS_INT msg_count = 0, msg_found = 0;

    curptr = context->svrstore;

    /* if we already have a list of messages, */
    /* go to the end of the list...           */

    while (curptr != NULL) {
        msg_count++;
        if (msg_count == idx) {
            msg_found++;
            break;
        }
        curptr = curptr->next;
    }

    if (msg_found) {
        memcpy(message, curptr->servermsg, sizeof(CS_SERVERMSG));
        return CS_SUCCEED;
    } else {
        return CS_NOMSG;
    }
}

CS_INT
_ct_diag_clearmsg(CS_CONTEXT * context, CS_INT type)
{
    struct cs_diag_msg_client *curptr, *freeptr;
    struct cs_diag_msg_svr *scurptr, *sfreeptr;

    if (type == CS_CLIENTMSG_TYPE || type == CS_ALLMSG_TYPE) {
        curptr = context->clientstore;
        context->clientstore = NULL;

        while (curptr != NULL) {
            freeptr = curptr;
            curptr = freeptr->next;
            if (freeptr->clientmsg)
                free(freeptr->clientmsg);
            free(freeptr);
        }
    }

    if (type == CS_SERVERMSG_TYPE || type == CS_ALLMSG_TYPE) {
        scurptr = context->svrstore;
        context->svrstore = NULL;

        while (scurptr != NULL) {
            sfreeptr = scurptr;
            scurptr = sfreeptr->next;
            if (sfreeptr->servermsg)
                free(sfreeptr->servermsg);
            free(sfreeptr);
        }
    }
    return CS_SUCCEED;
}

static CS_INT
ct_diag_countmsg(CS_CONTEXT * context, CS_INT type, CS_INT * count)
{
    struct cs_diag_msg_client *curptr;
    struct cs_diag_msg_svr *scurptr;

    CS_INT msg_count = 0;

    if (type == CS_CLIENTMSG_TYPE || type == CS_ALLMSG_TYPE) {
        curptr = context->clientstore;

        while (curptr != NULL) {
            msg_count++;
            curptr = curptr->next;
        }
    }

    if (type == CS_SERVERMSG_TYPE || type == CS_ALLMSG_TYPE) {
        scurptr = context->svrstore;

        while (scurptr != NULL) {
            msg_count++;
            scurptr = scurptr->next;
        }
    }
    *count = msg_count;

    return CS_SUCCEED;
}

/* Code changes ends here - CT_DIAG - 02*/

static CS_DYNAMIC *
_ct_allocate_dynamic(CS_CONNECTION * con, char *id, int idlen)
{
    CS_DYNAMIC *dyn;
    CS_DYNAMIC **pdyn;
    size_t id_len;

    dyn = (CS_DYNAMIC *) malloc(sizeof(CS_DYNAMIC));

    if (idlen == CS_NULLTERM)
        id_len = strlen(id);
    else
        id_len = idlen;

    if (dyn != NULL) {
        memset(dyn, '\0', sizeof(CS_DYNAMIC));
        dyn->id = malloc(id_len + 1);
        strncpy(dyn->id, id, id_len);
        dyn->id[id_len] = '\0';

        if (con->dynlist == NULL) {
            tdsdump_log(TDS_DBG_INFO1, "_ct_allocate_dynamic() attaching dynamic command to head\n");
            con->dynlist = dyn;
        } else {
            pdyn = &con->dynlist;
            while (*pdyn) {
                pdyn = &(*pdyn)->next;
            }

            *pdyn = dyn;
        }
    }
    return (dyn);
}

static CS_DYNAMIC *
_ct_locate_dynamic(CS_CONNECTION * con, char *id, int idlen)
{
    CS_DYNAMIC *dyn;
    size_t id_len;

    if (idlen == CS_NULLTERM)
        id_len = strlen(id);
    else
        id_len = idlen;

    tdsdump_log(TDS_DBG_INFO1, "_ct_locate_dynamic() looking for %s\n", (char *) id);

    for (dyn = con->dynlist; dyn != NULL; dyn = dyn->next) {
        tdsdump_log(TDS_DBG_INFO1, "_ct_locate_dynamic() matching with %s\n", (char *) dyn->id);
        if (strncmp(dyn->id, id, id_len) == 0)
            break;
    }

    return (dyn);
}

static CS_INT
_ct_deallocate_dynamic(CS_CONNECTION * con, CS_DYNAMIC *dyn)
{
    CS_DYNAMIC_LIST *victim = NULL;
    CS_DYNAMIC_LIST *prev = NULL;
    CS_DYNAMIC_LIST *next = NULL;
    TDSDYNAMIC *tdsdyn;
    char myid[256];

    strcpy(myid,"");

    victim = con->dynlist;

    for (;;) {
        if (victim == dyn)
            break;
        prev = victim;
        victim = victim->next;
        if (victim == NULL) {
            tdsdump_log(TDS_DBG_FUNC, "ct_deallocate_dynamic() : cannot find entry in list\n");
            return CS_FAIL;
        }
    }

    tdsdump_log(TDS_DBG_FUNC, "ct_deallocate_dynamic() : command entry found in list\n");

    next = victim->next;

    if (victim->id) {
        strcpy(myid, victim->id);
        free(victim->id);
    }
    if (victim->stmt) free(victim->stmt);
    param_clear(victim->param_list);

    free(victim);

    tdsdump_log(TDS_DBG_FUNC, "ct_deallocate_dynamic() : relinking list\n");

    if (prev)
        prev->next = next;
    else
        con->dynlist = next;

    tdsdump_log(TDS_DBG_FUNC, "ct_deallocate_dynamic() : relinked list\n");

    if (strlen(myid) > 0) {
        tdsdyn = tds_lookup_dynamic(con->tds_socket, myid);
        if (tdsdyn) {
            tds_free_dynamic(con->tds_socket, tdsdyn);
        }
    }

    return CS_SUCCEED;

}
