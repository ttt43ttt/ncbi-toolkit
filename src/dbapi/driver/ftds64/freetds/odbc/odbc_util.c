/* FreeTDS - Library of routines accessing Sybase and Microsoft databases
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005  Brian Bruns
 * Copyright (C) 2005, 2006 Frediano Ziglio
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

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include <assert.h>

#include "tdsodbc.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

TDS_RCSID(var, "$Id: odbc_util.c 487448 2015-12-17 18:43:08Z ucko $");

/**
 * \ingroup odbc_api
 * \defgroup odbc_util ODBC utility
 * Functions called within \c ODBC driver.
 */

/**
 * \addtogroup odbc_util
 *  \@{ 
 */

static int
odbc_set_stmt(TDS_STMT * stmt, char **dest, const char *sql, ssize_t sql_len)
{
	char *p;

	assert(dest == &stmt->prepared_query || dest == &stmt->query);

	if (sql_len == SQL_NTS)
		sql_len = strlen(sql);
	else if (sql_len <= 0)
		return SQL_ERROR;

	/* TODO already NULL ?? */
	tds_free_param_results(stmt->params);
	stmt->params = NULL;
	stmt->param_num = 0;
	stmt->param_count = 0;
	stmt->prepared_query_is_func = 0;
	stmt->prepared_query_is_rpc = 0;
	stmt->prepared_pos = NULL;
	stmt->curr_param_row = 0;

	if (stmt->prepared_query)
		TDS_ZERO_FREE(stmt->prepared_query);

	if (stmt->query)
		TDS_ZERO_FREE(stmt->query);

	*dest = p = (char *) malloc(sql_len + 1);
	if (!p)
		return SQL_ERROR;

	if (sql) {
		memcpy(p, sql, sql_len);
		p[sql_len] = 0;
	} else {
		p[0] = 0;
	}

	return SQL_SUCCESS;
}

int
odbc_set_stmt_query(TDS_STMT * stmt, const char *sql, ssize_t sql_len)
{
	return odbc_set_stmt(stmt, &stmt->query, sql, sql_len);
}


int
odbc_set_stmt_prepared_query(TDS_STMT * stmt, const char *sql, ssize_t sql_len)
{
	return odbc_set_stmt(stmt, &stmt->prepared_query, sql, sql_len);
}


void
odbc_set_return_status(struct _hstmt *stmt)
{
	TDSSOCKET *tds = stmt->dbc->tds_socket;
	TDSCONTEXT *context = stmt->dbc->env->tds_ctx;

	/* TODO handle different type results (functions) on mssql2k */
	if (stmt->prepared_query_is_func && tds->has_status) {
		struct _drecord *drec;
		int len;

		if (stmt->apd->header.sql_desc_count < 1)
			return;
		drec = &stmt->apd->records[0];

		len = convert_tds2sql(context, SYBINT4, (TDS_CHAR *) & tds->ret_status, sizeof(TDS_INT),
				      drec->sql_desc_concise_type, drec->sql_desc_data_ptr, drec->sql_desc_octet_length, NULL);
		if (TDS_FAIL == len)
			return /* SQL_ERROR */ ;
		if (drec->sql_desc_indicator_ptr)
			*drec->sql_desc_indicator_ptr = 0;
		if (drec->sql_desc_octet_length_ptr)
			*drec->sql_desc_octet_length_ptr = len;
	}

}

void
odbc_set_return_params(struct _hstmt *stmt)
{
	TDSSOCKET *tds = stmt->dbc->tds_socket;
	TDSPARAMINFO *info = tds->current_results;
	TDSCONTEXT *context = stmt->dbc->env->tds_ctx;

	int i_begin = stmt->prepared_query_is_func ? 1 : 0;
	int i;
	int nparam = i_begin;

	/* I don't understand why but this happen -- freddy77 */
	/* TODO check why, put an assert ? */
	if (!info)
		return;

	for (i = 0; i < info->num_cols; ++i) {
		struct _drecord *drec_apd, *drec_ipd;
		TDSCOLUMN *colinfo = info->columns[i];
		TDS_CHAR *src;
		int srclen;
		SQLINTEGER len;
		int c_type;

		/* find next output parameter */
		for (;;) {
			drec_apd = NULL;
			/* TODO best way to stop */
			if (nparam >= stmt->apd->header.sql_desc_count || nparam >= stmt->ipd->header.sql_desc_count)
				return;
			drec_apd = &stmt->apd->records[nparam];
			drec_ipd = &stmt->ipd->records[nparam];
			if (stmt->ipd->records[nparam++].sql_desc_parameter_type != SQL_PARAM_INPUT)
				break;
		}

		/* null parameter ? */
		if (colinfo->column_cur_size < 0) {
			/* FIXME error if NULL */
			if (drec_apd->sql_desc_indicator_ptr)
				*drec_apd->sql_desc_indicator_ptr = SQL_NULL_DATA;
			continue;
		}

		src = (TDS_CHAR *) & info->current_row[colinfo->column_offset];
		if (is_blob_type(colinfo->column_type))
			src = ((TDSBLOB *) src)->textvalue;
		srclen = colinfo->column_cur_size;
		c_type = drec_apd->sql_desc_concise_type;
		if (c_type == SQL_C_DEFAULT)
			c_type = odbc_sql_to_c_type_default(drec_ipd->sql_desc_concise_type);
		/* 
		 * TODO why IPD ?? perhaps SQLBindParameter it's not correct ??
		 * Or tests are wrong ??
		 */
		len = convert_tds2sql(context, tds_get_conversion_type(colinfo->column_type, colinfo->column_size), src, srclen,
				      c_type, drec_apd->sql_desc_data_ptr, drec_apd->sql_desc_octet_length, drec_ipd);
		/* TODO error handling */
		if (len < 0)
			return /* SQL_ERROR */ ;
		if (drec_apd->sql_desc_indicator_ptr)
			*drec_apd->sql_desc_indicator_ptr = 0;
		if (drec_apd->sql_desc_octet_length_ptr)
			*drec_apd->sql_desc_octet_length_ptr = len;
	}
}

size_t
odbc_get_string_size(ssize_t size, SQLCHAR * str)
{
	if (str) {
		if (size == SQL_NTS)
			return strlen((const char *) str);
		if (size >= 0)
			return size;
	}
	/* SQL_NULL_DATA or any other strange value */
	return 0;
}

/**
 * Convert type from database to ODBC
 */
SQLSMALLINT
odbc_server_to_sql_type(int col_type, int col_size)
{
	/* FIXME finish */
	switch ((TDS_SERVER_TYPE) col_type) {
	case XSYBCHAR:
	case SYBCHAR:
		return SQL_CHAR;
	case XSYBVARCHAR:
	case SYBVARCHAR:
		return SQL_VARCHAR;
	case SYBTEXT:
		return SQL_LONGVARCHAR;
	case SYBBIT:
	case SYBBITN:
		return SQL_BIT;
#if (ODBCVER >= 0x0300)
    case SYB5INT8:
	case SYBINT8:
		/* TODO return numeric for odbc2 and convert bigint to numeric */
		return SQL_BIGINT;
#endif
	case SYBINT4:
		return SQL_INTEGER;
	case SYBINT2:
		return SQL_SMALLINT;
	case SYBINT1:
		return SQL_TINYINT;
	case SYBINTN:
		switch (col_size) {
		case 1:
			return SQL_TINYINT;
		case 2:
			return SQL_SMALLINT;
		case 4:
			return SQL_INTEGER;
#if (ODBCVER >= 0x0300)
		case 8:
			return SQL_BIGINT;
#endif
		}
		break;
	case SYBREAL:
		return SQL_REAL;
	case SYBFLT8:
		return SQL_DOUBLE;
	case SYBFLTN:
		switch (col_size) {
		case 4:
			return SQL_REAL;
		case 8:
			return SQL_DOUBLE;
		}
		break;
	case SYBMONEY:
		return SQL_DOUBLE;
	case SYBMONEY4:
		return SQL_DOUBLE;
	case SYBMONEYN:
		return SQL_DOUBLE;
	case SYBDATETIME:
	case SYBDATETIME4:
	case SYBDATETIMN:
#if (ODBCVER >= 0x0300)
		return SQL_TYPE_TIMESTAMP;
#else
		return SQL_TIMESTAMP;
#endif
	case XSYBBINARY:
	case SYBBINARY:
		return SQL_BINARY;
	case SYBLONGBINARY:
	case SYBIMAGE:
		return SQL_LONGVARBINARY;
	case XSYBVARBINARY:
	case SYBVARBINARY:
		return SQL_VARBINARY;
	case SYBNUMERIC:
	case SYBDECIMAL:
		return SQL_NUMERIC;
		/* TODO this is ODBC 3.5 */
	case SYBNTEXT:
	case SYBNVARCHAR:
	case XSYBNVARCHAR:
	case XSYBNCHAR:
		break;
#if (ODBCVER >= 0x0300)
	case SYBUNIQUE:
#ifdef SQL_GUID
		return SQL_GUID;
#else
		return SQL_CHAR;
#endif
	case SYBVARIANT:
		break;
#endif
		/* TODO what should I do with these types ?? */
	case SYBVOID:
	case SYBSINT1:
	case SYBUINT2:
	case SYBUINT4:
	case SYBUINT8:
		break;
	}
	return SQL_UNKNOWN_TYPE;
}

/**
 * Pass this an SQL_C_* type and get a SYB* type which most closely corresponds
 * to the SQL_C_* type.
 */
TDS_SERVER_TYPE
odbc_c_to_server_type(int c_type)
{
	switch (c_type) {
		/* FIXME this should be dependent on size of data !!! */
	case SQL_C_BINARY:
		return SYBBINARY;
		/* TODO what happen if varchar is more than 255 characters long */
	case SQL_C_CHAR:
		return SYBVARCHAR;
	case SQL_C_FLOAT:
		return SYBREAL;
	case SQL_C_DOUBLE:
		return SYBFLT8;
	case SQL_C_BIT:
		return SYBBIT;
#if (ODBCVER >= 0x0300)
	case SQL_C_SBIGINT:
	case SQL_C_UBIGINT:
		return SYBINT8;
#ifdef SQL_C_GUID
	case SQL_C_GUID:
		return SYBUNIQUE;
#endif
#endif
	case SQL_C_LONG:
	case SQL_C_SLONG:
	case SQL_C_ULONG:
		return SYBINT4;
	case SQL_C_SHORT:
	case SQL_C_SSHORT:
	case SQL_C_USHORT:
		return SYBINT2;
	case SQL_C_TINYINT:
	case SQL_C_STINYINT:
	case SQL_C_UTINYINT:
		return SYBINT1;
		/* ODBC date formats are completely differect from SQL one */
	case SQL_C_DATE:
	case SQL_C_TIME:
	case SQL_C_TIMESTAMP:
	case SQL_C_TYPE_DATE:
	case SQL_C_TYPE_TIME:
	case SQL_C_TYPE_TIMESTAMP:
		return SYBDATETIME;
		/* ODBC numeric/decimal formats are completely differect from tds one */
	case SQL_C_NUMERIC:
		return SYBNUMERIC;
		/* not supported */
	case SQL_C_INTERVAL_YEAR:
	case SQL_C_INTERVAL_MONTH:
	case SQL_C_INTERVAL_DAY:
	case SQL_C_INTERVAL_HOUR:
	case SQL_C_INTERVAL_MINUTE:
	case SQL_C_INTERVAL_SECOND:
	case SQL_C_INTERVAL_YEAR_TO_MONTH:
	case SQL_C_INTERVAL_DAY_TO_HOUR:
	case SQL_C_INTERVAL_DAY_TO_MINUTE:
	case SQL_C_INTERVAL_DAY_TO_SECOND:
	case SQL_C_INTERVAL_HOUR_TO_MINUTE:
	case SQL_C_INTERVAL_HOUR_TO_SECOND:
	case SQL_C_INTERVAL_MINUTE_TO_SECOND:
#ifdef SQL_C_WCHAR
	case SQL_C_WCHAR:
#endif
		break;
	}
    return (TDS_SERVER_TYPE) TDS_FAIL;
}

void
odbc_set_sql_type_info(TDSCOLUMN * col, struct _drecord *drec)
{
#define SET_INFO(type, prefix, suffix) \
	drec->sql_desc_literal_prefix = prefix; \
	drec->sql_desc_literal_suffix = suffix; \
	drec->sql_desc_type_name = type; \
	return;

	/* FIXME finish, support for N type (nvarchar) */
	switch (tds_get_conversion_type(col->column_type, col->column_size)) {
	case XSYBCHAR:
	case SYBCHAR:
		SET_INFO("char", "'", "'");
	case XSYBVARCHAR:
	case SYBVARCHAR:
		SET_INFO("varchar", "'", "'");
	case SYBTEXT:
		SET_INFO("text", "'", "'");
	case SYBBIT:
	case SYBBITN:
		SET_INFO("bit", "", "");
#if (ODBCVER >= 0x0300)
	case SYBINT8:
		/* TODO return numeric for odbc2 and convert bigint to numeric */
		SET_INFO("bigint", "", "");
#endif
	case SYBINT4:
		SET_INFO("int", "", "");
	case SYBINT2:
		SET_INFO("smallint", "", "");
	case SYBINT1:
		SET_INFO("tinyint", "", "");
	case SYBREAL:
		SET_INFO("real", "", "");
	case SYBFLT8:
		SET_INFO("float", "", "");
	case SYBMONEY:
	case SYBMONEY4:
		SET_INFO("money", "$", "");
	case SYBDATETIME:
	case SYBDATETIME4:
		SET_INFO("datetime", "'", "'");
	case SYBBINARY:
		/* handle TIMESTAMP using usertype */
		if (col->column_usertype == 80)
			SET_INFO("timestamp", "0x", "");
		SET_INFO("binary", "0x", "");
	case SYBIMAGE:
		SET_INFO("image", "0x", "");
	case SYBVARBINARY:
		SET_INFO("varbinary", "0x", "");
	case SYBNUMERIC:
		SET_INFO("numeric", "", "");
	case SYBDECIMAL:
		SET_INFO("decimal", "", "");
	case SYBINTN:
	case SYBDATETIMN:
	case SYBFLTN:
	case SYBMONEYN:
		assert(0);
	case SYBVOID:
	case SYBNTEXT:
	case SYBNVARCHAR:
	case XSYBNVARCHAR:
	case XSYBNCHAR:
		break;
#if (ODBCVER >= 0x0300)
	case SYBUNIQUE:
		/* FIXME for Sybase ?? */
		SET_INFO("uniqueidentifier", "'", "'");
	case SYBVARIANT:
		/* SET_INFO("sql_variant", "", ""); */
		break;
#endif
	}
	SET_INFO("", "", "");
#undef SET_INFO
}

SQLINTEGER
odbc_sql_to_displaysize(int sqltype, int column_size, int column_prec)
{
	SQLINTEGER size = 0;

	switch (sqltype) {
	case SQL_CHAR:
	case SQL_VARCHAR:
	case SQL_LONGVARCHAR:
		size = column_size;
		break;
	case SQL_BIGINT:
		size = 20;
		break;
	case SQL_INTEGER:
		size = 11;	/* -1000000000 */
		break;
	case SQL_SMALLINT:
		size = 6;	/* -10000 */
		break;
	case SQL_TINYINT:
		size = 3;	/* 255 */
		break;
	case SQL_DECIMAL:
	case SQL_NUMERIC:
		size = column_prec + 2;
		break;
	case SQL_DATE:
		/* FIXME check always yyyy-mm-dd ?? */
		size = 19;
		break;
	case SQL_TIME:
		/* FIXME check always hh:mm:ss[.fff] */
		size = 19;
		break;
	case SQL_TYPE_TIMESTAMP:
	case SQL_TIMESTAMP:
		/* TODO dependent on precision (decimal second digits) */
		/* FIXME check, always format yyyy-mm-dd hh:mm:ss[.fff] ?? */
		size = 24;
		/* spinellia@acm.org: int token.c it is 30 should we comply? */
		break;
	case SQL_FLOAT:
	case SQL_REAL:
	case SQL_DOUBLE:
		size = 24;	/* FIXME -- what should the correct size be? */
		break;
#ifdef SQL_GUID
	case SQL_GUID:
		size = 36;
		break;
#endif
	default:
		/* FIXME TODO finish, should support ALL types (interval, binary) */
		size = 40;
		tdsdump_log(TDS_DBG_INFO1, "odbc_sql_to_displaysize: unknown sql type %d\n", (int) sqltype);
		break;
	}
	return size;
}

int
odbc_sql_to_c_type_default(int sql_type)
{

	switch (sql_type) {

	case SQL_CHAR:
	case SQL_VARCHAR:
	case SQL_LONGVARCHAR:
		return SQL_C_CHAR;
		/* for compatibility numeric are converted to CHAR, not to structure */
	case SQL_DECIMAL:
	case SQL_NUMERIC:
		return SQL_C_CHAR;
#ifdef SQL_GUID
	case SQL_GUID:
		/* TODO return SQL_C_CHAR for Sybase ?? */
		return SQL_C_GUID;
#endif
	case SQL_BIT:
		return SQL_C_BIT;
	case SQL_TINYINT:
		return SQL_C_UTINYINT;
	case SQL_SMALLINT:
		return SQL_C_SSHORT;
	case SQL_INTEGER:
		return SQL_C_SLONG;
	case SQL_BIGINT:
		return SQL_C_SBIGINT;
	case SQL_REAL:
		return SQL_C_FLOAT;
	case SQL_FLOAT:
	case SQL_DOUBLE:
		return SQL_C_DOUBLE;
	case SQL_DATE:
	case SQL_TYPE_DATE:
		return SQL_C_TYPE_DATE;
	case SQL_TIME:
	case SQL_TYPE_TIME:
		return SQL_C_TYPE_TIME;
	case SQL_TIMESTAMP:
	case SQL_TYPE_TIMESTAMP:
		return SQL_C_TYPE_TIMESTAMP;
	case SQL_BINARY:
	case SQL_VARBINARY:
	case SQL_LONGVARBINARY:
		return SQL_C_BINARY;
		/* TODO interval types */
	default:
		return 0;
	}
}

TDS_SERVER_TYPE
odbc_sql_to_server_type(TDSSOCKET * tds, int sql_type)
{

	switch (sql_type) {

	case SQL_CHAR:
		return SYBCHAR;
	case SQL_VARCHAR:
		return SYBVARCHAR;
	case SQL_LONGVARCHAR:
		return SYBTEXT;
	case SQL_DECIMAL:
		return SYBDECIMAL;
	case SQL_NUMERIC:
		return SYBNUMERIC;
#ifdef SQL_GUID
	case SQL_GUID:
		if (IS_TDS7_PLUS(tds))
			return SYBUNIQUE;
		return 0;
#endif
	case SQL_BIT:
		if (IS_TDS7_PLUS(tds))
			return SYBBITN;
		return SYBBIT;
	case SQL_TINYINT:
		return SYBINT1;
	case SQL_SMALLINT:
		return SYBINT2;
	case SQL_INTEGER:
		return SYBINT4;
	case SQL_BIGINT:
		return SYBINT8;
	case SQL_REAL:
		return SYBREAL;
	case SQL_FLOAT:
	case SQL_DOUBLE:
		return SYBFLT8;
		/* ODBC version 2 */
	case SQL_DATE:
	case SQL_TIME:
	case SQL_TIMESTAMP:
		/* ODBC version 3 */
	case SQL_TYPE_DATE:
	case SQL_TYPE_TIME:
	case SQL_TYPE_TIMESTAMP:
		return SYBDATETIME;
	case SQL_BINARY:
		return SYBBINARY;
	case SQL_VARBINARY:
		return SYBVARBINARY;
	case SQL_LONGVARBINARY:
		return SYBIMAGE;
		/* TODO interval types */
	default:
        return (TDS_SERVER_TYPE) 0;
	}
}

/**
 * Copy a string to client setting size according to ODBC convenction
 * @param buffer    client buffer
 * @param cbBuffer  client buffer size (in bytes)
 * @param pcbBuffer pointer to SQLSMALLINT to hold string size
 * @param s         string to copy
 * @param len       len of string to copy. <0 null terminated
 */
SQLRETURN
odbc_set_string(SQLPOINTER buffer, SQLSMALLINT cbBuffer, SQLSMALLINT FAR * pcbBuffer, const char *s, ssize_t len)
{
	SQLRETURN result = SQL_SUCCESS;

	if (len < 0)
		len = strlen(s);

	if (pcbBuffer)
		*pcbBuffer = len;
	if (len >= cbBuffer) {
		len = cbBuffer - 1;
		result = SQL_SUCCESS_WITH_INFO;
	}
	if (buffer && len >= 0) {
		/* buffer can overlap, use memmove, thanks to Valgrind */
		memmove((char *) buffer, s, len);
		((char *) buffer)[len] = 0;
	}
	return result;
}

SQLRETURN
odbc_set_string_i(SQLPOINTER buffer, SQLINTEGER cbBuffer, SQLINTEGER FAR * pcbBuffer, const char *s, ssize_t len)
{
	SQLRETURN result = SQL_SUCCESS;

	if (len < 0)
		len = strlen(s);

	if (pcbBuffer)
		*pcbBuffer = len;
	if (len >= cbBuffer) {
		len = cbBuffer - 1;
		result = SQL_SUCCESS_WITH_INFO;
	}
	if (buffer && len >= 0) {
		/* buffer can overlap, use memmove, thanks to Valgrind */
		memmove((char *) buffer, s, len);
		((char *) buffer)[len] = 0;
	}
	return result;
}

/** Returns the version of the RDBMS in the ODBC format */
void
odbc_rdbms_version(TDSSOCKET * tds, char *pversion_string)
{
	sprintf(pversion_string, "%.02d.%.02d.%.04d", (int) ((tds->product_version & 0x7F000000) >> 24),
		(int) ((tds->product_version & 0x00FF0000) >> 16), (int) (tds->product_version & 0x0000FFFF));
}

/** Return length of parameter from parameter information */
SQLLEN
odbc_get_param_len(const struct _drecord *drec_apd, const struct _drecord *drec_ipd)
{
    SQLLEN len;
	int size;

	if (drec_apd->sql_desc_indicator_ptr && *drec_apd->sql_desc_indicator_ptr == SQL_NULL_DATA)
		len = SQL_NULL_DATA;
	else if (drec_apd->sql_desc_octet_length_ptr)
		len = *drec_apd->sql_desc_octet_length_ptr;
	else {
		len = 0;
		/* TODO add XML if defined */
		/* FIXME, other types available */
		if (drec_apd->sql_desc_concise_type == SQL_C_CHAR || drec_apd->sql_desc_concise_type == SQL_C_BINARY) {
			len = SQL_NTS;
		} else {
			int type = drec_apd->sql_desc_concise_type;

			if (type == SQL_C_DEFAULT)
				type = odbc_sql_to_c_type_default(drec_ipd->sql_desc_concise_type);
			type = odbc_c_to_server_type(type);

			/* FIXME check what happen to DATE/TIME types */
			size = tds_get_size_by_type(type);
			if (size > 0)
				len = size;
		}
	}
	return len;
}

#ifdef SQL_GUID
# define TYPE_NORMAL_SQL_GUID TYPE_NORMAL(SQL_GUID)
#else
# define TYPE_NORMAL_SQL_GUID
#endif
#define SQL_TYPES \
	TYPE_NORMAL(SQL_BIT) \
	TYPE_NORMAL(SQL_SMALLINT) \
	TYPE_NORMAL(SQL_TINYINT) \
	TYPE_NORMAL(SQL_INTEGER) \
	TYPE_NORMAL(SQL_BIGINT) \
\
	TYPE_NORMAL_SQL_GUID \
\
	TYPE_NORMAL(SQL_BINARY) \
	TYPE_NORMAL(SQL_VARBINARY) \
	TYPE_NORMAL(SQL_LONGVARBINARY) \
\
	TYPE_NORMAL(SQL_CHAR) \
	TYPE_NORMAL(SQL_VARCHAR) \
	TYPE_NORMAL(SQL_LONGVARCHAR) \
\
	TYPE_NORMAL(SQL_DECIMAL) \
	TYPE_NORMAL(SQL_NUMERIC) \
\
	TYPE_NORMAL(SQL_FLOAT) \
	TYPE_NORMAL(SQL_REAL) \
	TYPE_NORMAL(SQL_DOUBLE)\
\
	TYPE_VERBOSE_START(SQL_DATETIME) \
	TYPE_VERBOSE_DATE(SQL_DATETIME, SQL_CODE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP) \
	TYPE_VERBOSE_END(SQL_DATETIME)

SQLSMALLINT
odbc_get_concise_sql_type(SQLSMALLINT type, SQLSMALLINT interval)
{
#define TYPE_NORMAL(t) case t: return type;
#define TYPE_VERBOSE_START(t) \
	case t: switch (interval) {
#define TYPE_VERBOSE_DATE(t, interval, concise, old) \
	case interval: return concise;
#define TYPE_VERBOSE_END(t) \
	}

	switch (type) {
		SQL_TYPES;
	}
	return 0;
#undef TYPE_NORMAL
#undef TYPE_VERBOSE_START
#undef TYPE_VERBOSE_DATE
#undef TYPE_VERBOSE_END
}

/**
 * Set concise type and all cascading field.
 * @param concise_type concise type to set
 * @param drec         record to set. NULL to test error without setting
 * @param check_only   it <>0 (true) check only, do not set type
 */
SQLRETURN
odbc_set_concise_sql_type(SQLSMALLINT concise_type, struct _drecord * drec, int check_only)
{
	SQLSMALLINT type = concise_type, interval_code = 0;

#define TYPE_NORMAL(t) case t: break;
#define TYPE_VERBOSE_START(t)
#define TYPE_VERBOSE_DATE(t, interval, concise, old) \
	case old: concise_type = concise; \
	case concise: type = t; interval_code = interval; break;
#define TYPE_VERBOSE_END(t)

	switch (type) {
		SQL_TYPES;
	default:
		return SQL_ERROR;
	}
	if (!check_only) {
		drec->sql_desc_concise_type = concise_type;
		drec->sql_desc_type = type;
		drec->sql_desc_datetime_interval_code = interval_code;

		switch (drec->sql_desc_type) {
		case SQL_NUMERIC:
		case SQL_DECIMAL:
			drec->sql_desc_precision = 38;
			drec->sql_desc_scale = 0;
			break;
			/* TODO finish */
		}
	}
	return SQL_SUCCESS;
#undef TYPE_NORMAL
#undef TYPE_VERBOSE_START
#undef TYPE_VERBOSE_DATE
#undef TYPE_VERBOSE_END
}

#ifdef SQL_C_GUID
# define TYPE_NORMAL_SQL_C_GUID TYPE_NORMAL(SQL_C_GUID)
#else
# define TYPE_NORMAL_SQL_C_GUID
#endif
#define C_TYPES \
	TYPE_NORMAL(SQL_C_BIT) \
	TYPE_NORMAL(SQL_C_SHORT) \
	TYPE_NORMAL(SQL_C_TINYINT) \
	TYPE_NORMAL(SQL_C_UTINYINT) \
	TYPE_NORMAL(SQL_C_STINYINT) \
	TYPE_NORMAL(SQL_C_LONG) \
	TYPE_NORMAL(SQL_C_SBIGINT) \
	TYPE_NORMAL(SQL_C_UBIGINT) \
	TYPE_NORMAL(SQL_C_SSHORT) \
	TYPE_NORMAL(SQL_C_SLONG) \
	TYPE_NORMAL(SQL_C_USHORT) \
	TYPE_NORMAL(SQL_C_ULONG) \
\
	TYPE_NORMAL_SQL_C_GUID \
	TYPE_NORMAL(SQL_C_DEFAULT) \
\
	TYPE_NORMAL(SQL_C_BINARY) \
\
	TYPE_NORMAL(SQL_C_CHAR) \
\
	TYPE_NORMAL(SQL_C_NUMERIC) \
\
	TYPE_NORMAL(SQL_C_FLOAT) \
	TYPE_NORMAL(SQL_C_DOUBLE)\
\
	TYPE_VERBOSE_START(SQL_DATETIME) \
	TYPE_VERBOSE_DATE(SQL_DATETIME, SQL_CODE_DATE, SQL_C_TYPE_DATE, SQL_C_DATE) \
	TYPE_VERBOSE_DATE(SQL_DATETIME, SQL_CODE_TIME, SQL_C_TYPE_TIME, SQL_C_TIME) \
	TYPE_VERBOSE_DATE(SQL_DATETIME, SQL_CODE_TIMESTAMP, SQL_C_TYPE_TIMESTAMP, SQL_C_TIMESTAMP) \
	TYPE_VERBOSE_END(SQL_DATETIME) \
\
	TYPE_VERBOSE_START(SQL_INTERVAL) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_DAY, SQL_C_INTERVAL_DAY) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_DAY_TO_HOUR, SQL_C_INTERVAL_DAY_TO_HOUR) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_DAY_TO_MINUTE, SQL_C_INTERVAL_DAY_TO_MINUTE) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_DAY_TO_SECOND, SQL_C_INTERVAL_DAY_TO_SECOND) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_HOUR, SQL_C_INTERVAL_HOUR) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_HOUR_TO_MINUTE, SQL_C_INTERVAL_HOUR_TO_MINUTE) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_HOUR_TO_SECOND, SQL_C_INTERVAL_HOUR_TO_SECOND) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_MINUTE, SQL_C_INTERVAL_MINUTE) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_MINUTE_TO_SECOND, SQL_C_INTERVAL_MINUTE_TO_SECOND) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_MONTH, SQL_C_INTERVAL_MONTH) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_SECOND, SQL_C_INTERVAL_SECOND) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_YEAR, SQL_C_INTERVAL_YEAR) \
	TYPE_VERBOSE(SQL_INTERVAL, SQL_CODE_YEAR_TO_MONTH, SQL_C_INTERVAL_YEAR_TO_MONTH) \
	TYPE_VERBOSE_END(SQL_INTERVAL)

SQLSMALLINT
odbc_get_concise_c_type(SQLSMALLINT type, SQLSMALLINT interval)
{
#define TYPE_NORMAL(t) case t: return type;
#define TYPE_VERBOSE_START(t) \
	case t: switch (interval) {
#define TYPE_VERBOSE(t, interval, concise) \
	case interval: return concise;
#define TYPE_VERBOSE_DATE(t, interval, concise, old) \
	case interval: return concise;
#define TYPE_VERBOSE_END(t) \
	}

	switch (type) {
		C_TYPES;
	}
	return 0;
#undef TYPE_NORMAL
#undef TYPE_VERBOSE_START
#undef TYPE_VERBOSE
#undef TYPE_VERBOSE_DATE
#undef TYPE_VERBOSE_END
}

/**
 * Set concise type and all cascading field.
 * @param concise_type concise type to set
 * @param drec         record to set. NULL to test error without setting
 * @param check_only   it <>0 (true) check only, do not set type
 */
SQLRETURN
odbc_set_concise_c_type(SQLSMALLINT concise_type, struct _drecord * drec, int check_only)
{
	SQLSMALLINT type = concise_type, interval_code = 0;

#define TYPE_NORMAL(t) case t: break;
#define TYPE_VERBOSE_START(t)
#define TYPE_VERBOSE(t, interval, concise) \
	case concise: type = t; interval_code = interval; break;
#define TYPE_VERBOSE_DATE(t, interval, concise, old) \
	case concise: type = t; interval_code = interval; break; \
	case old: concise_type = concise; type = t; interval_code = interval; break;
#define TYPE_VERBOSE_END(t)

	switch (type) {
		C_TYPES;
	default:
		return SQL_ERROR;
	}
	if (!check_only) {
		drec->sql_desc_concise_type = concise_type;
		drec->sql_desc_type = type;
		drec->sql_desc_datetime_interval_code = interval_code;

		switch (drec->sql_desc_type) {
		case SQL_C_NUMERIC:
			drec->sql_desc_precision = 38;
			drec->sql_desc_scale = 0;
			break;
			/* TODO finish */
		}
	}
	return SQL_SUCCESS;
#undef TYPE_NORMAL
#undef TYPE_VERBOSE_START
#undef TYPE_VERBOSE
#undef TYPE_VERBOSE_DATE
#undef TYPE_VERBOSE_END
}

/** \@} */
