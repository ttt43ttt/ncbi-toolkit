/* FreeTDS - Library of routines accessing Sybase and Microsoft databases
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004  Brian Bruns
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

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include <stddef.h>

#include "cspublic.h"
#include "ctlib.h"
#include "tds.h"
#include "replacements.h"
/* #include "fortify.h" */


TDS_RCSID(var, "$Id: ctutil.c 487448 2015-12-17 18:43:08Z ucko $");

/*
 * test include consistency 
 * I don't think all compiler are able to compile this code... if not comment it
 */
#if ENABLE_EXTRA_CHECKS

#if defined(__llvm__)  \
    ||  (defined(__GNUC__)  \
         &&  (__GNUC__ >= 5  ||  (__GNUC__  == 4  &&  __GNUC_MINOR__ >= 6)))
#define COMPILE_CHECK(name,check) \
    _Static_assert(check,#name)
#elif defined(__GNUC__) && __GNUC__ >= 2
#define COMPILE_CHECK(name,check) \
    extern int name[(check)?1:-1] __attribute__ ((unused))
#else
#define COMPILE_CHECK(name,check) \
    extern int name[(check)?1:-1]
#endif

#define TEST_EQUAL(t,a,b) COMPILE_CHECK(t,a==b)

TEST_EQUAL(t01,CS_FAIL,TDS_FAIL);
TEST_EQUAL(t02,CS_SUCCEED,TDS_SUCCEED);
TEST_EQUAL(t03,CS_NULLTERM,TDS_NULLTERM);
TEST_EQUAL(t04,CS_CMD_SUCCEED,TDS_CMD_SUCCEED);
TEST_EQUAL(t05,CS_CMD_FAIL,TDS_CMD_FAIL);
TEST_EQUAL(t06,CS_CMD_DONE,TDS_CMD_DONE);
TEST_EQUAL(t07,CS_NO_COUNT,TDS_NO_COUNT);
TEST_EQUAL(t08,CS_COMPUTE_RESULT,TDS_COMPUTE_RESULT);
TEST_EQUAL(t09,CS_PARAM_RESULT,TDS_PARAM_RESULT);
TEST_EQUAL(t10,CS_ROW_RESULT,TDS_ROW_RESULT);
TEST_EQUAL(t11,CS_STATUS_RESULT,TDS_STATUS_RESULT);
TEST_EQUAL(t12,CS_COMPUTEFMT_RESULT,TDS_COMPUTEFMT_RESULT);
TEST_EQUAL(t13,CS_ROWFMT_RESULT,TDS_ROWFMT_RESULT);
TEST_EQUAL(t14,CS_MSG_RESULT,TDS_MSG_RESULT);
TEST_EQUAL(t15,CS_DESCRIBE_RESULT,TDS_DESCRIBE_RESULT);

#define TEST_ATTRIBUTE(t,sa,fa,sb,fb) \
    COMPILE_CHECK(t##a, sizeof(((sa*)0)->fa) == sizeof(((sb*)0)->fb)); \
    COMPILE_CHECK(t##b, offsetof(sa, fa) == offsetof(sa, fb));

TEST_ATTRIBUTE(t21,TDS_MONEY4,mny4,CS_MONEY4,mny4);
TEST_ATTRIBUTE(t22,TDS_OLD_MONEY,mnyhigh,CS_MONEY,mnyhigh);
TEST_ATTRIBUTE(t23,TDS_OLD_MONEY,mnylow,CS_MONEY,mnylow);
TEST_ATTRIBUTE(t24,TDS_DATETIME,dtdays,CS_DATETIME,dtdays);
TEST_ATTRIBUTE(t25,TDS_DATETIME,dttime,CS_DATETIME,dttime);
TEST_ATTRIBUTE(t26,TDS_DATETIME4,days,CS_DATETIME4,days);
TEST_ATTRIBUTE(t27,TDS_DATETIME4,minutes,CS_DATETIME4,minutes);
TEST_ATTRIBUTE(t28,TDS_NUMERIC,precision,CS_NUMERIC,precision);
TEST_ATTRIBUTE(t29,TDS_NUMERIC,scale,CS_NUMERIC,scale);
TEST_ATTRIBUTE(t30,TDS_NUMERIC,array,CS_NUMERIC,array);
TEST_ATTRIBUTE(t30,TDS_NUMERIC,precision,CS_DECIMAL,precision);
TEST_ATTRIBUTE(t31,TDS_NUMERIC,scale,CS_DECIMAL,scale);
TEST_ATTRIBUTE(t32,TDS_NUMERIC,array,CS_DECIMAL,array);
TEST_ATTRIBUTE(t33,TDS_VARBINARY,len,CS_VARBINARY,len);
TEST_ATTRIBUTE(t34,TDS_VARBINARY,array,CS_VARBINARY,array);
#endif

/* error handler */
int
_ct_handle_client_message(const TDSCONTEXT * ctx_tds, TDSSOCKET * tds, TDSMESSAGE * msg)
{
	CS_CLIENTMSG errmsg;
	CS_CONNECTION *con = NULL;
	CS_CONTEXT *ctx = NULL;
	int ret = (int) CS_SUCCEED;

	if (tds && tds->parent) {
		con = (CS_CONNECTION *) tds->parent;
	}

	memset(&errmsg, '\0', sizeof(errmsg));
	errmsg.msgnumber = msg->msgno;
        /* ssikorsk */
	errmsg.severity = msg->severity;
	strcpy(errmsg.msgstring, msg->message);
	errmsg.msgstringlen = strlen(msg->message);
	errmsg.osstring[0] = '\0';
	errmsg.osstringlen = 0;
	/* if there is no connection, attempt to call the context handler */
	if (!con) {
		ctx = (CS_CONTEXT *) ctx_tds->parent;
		if (ctx->_clientmsg_cb)
			ret = ctx->_clientmsg_cb(ctx, con, &errmsg);
	} else if (con->_clientmsg_cb)
		ret = con->_clientmsg_cb(con->ctx, con, &errmsg);
	else if (con->ctx->_clientmsg_cb)
		ret = con->ctx->_clientmsg_cb(con->ctx, con, &errmsg);
	return ret;
}

/* message handler */
int
_ct_handle_server_message(const TDSCONTEXT * ctx_tds, TDSSOCKET * tds, TDSMESSAGE * msg)
{
	CS_SERVERMSG errmsg;
	CS_CONNECTION *con = NULL;
	CS_CONTEXT *ctx = NULL;
	int ret = (int) CS_SUCCEED;

	if (tds && tds->parent) {
		con = (CS_CONNECTION *) tds->parent;
	}

	memset(&errmsg, '\0', sizeof(errmsg));
	errmsg.msgnumber = msg->msgno;
	tds_strlcpy(errmsg.text, msg->message, sizeof(errmsg.text));
    errmsg.textlen = (CS_INT) strlen(errmsg.text);
	errmsg.sqlstate[0] = 0;
	if (msg->sql_state)
		tds_strlcpy((char *) errmsg.sqlstate, msg->sql_state, sizeof(errmsg.sqlstate));
    errmsg.sqlstatelen = (CS_INT) strlen((char *) errmsg.sqlstate);
	errmsg.state = msg->state;
	errmsg.severity = msg->severity;
	errmsg.line = msg->line_number;
	if (msg->server) {
		errmsg.svrnlen = strlen(msg->server);
		tds_strlcpy(errmsg.svrname, msg->server, CS_MAX_NAME);
	}
	if (msg->proc_name) {
		errmsg.proclen = strlen(msg->proc_name);
		tds_strlcpy(errmsg.proc, msg->proc_name, CS_MAX_NAME);
	}
	/* if there is no connection, attempt to call the context handler */
	if (!con) {
		ctx = (CS_CONTEXT *) ctx_tds->parent;
		if (ctx->_servermsg_cb)
			ret = ctx->_servermsg_cb(ctx, con, &errmsg);
	} else if (con->_servermsg_cb) {
		ret = con->_servermsg_cb(con->ctx, con, &errmsg);
	} else if (con->ctx->_servermsg_cb) {
		ret = con->ctx->_servermsg_cb(con->ctx, con, &errmsg);
	}
	return ret;
}
