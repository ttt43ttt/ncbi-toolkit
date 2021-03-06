/* FreeTDS - Library of routines accessing Sybase and Microsoft databases
 * Copyright (C) 1998-1999  Brian Bruns
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

#ifndef _tdsconvert_h_
#define _tdsconvert_h_

#ifdef __cplusplus
extern "C"
{
#if 0
}
#endif
#endif

static const char rcsid_tdsconvert_h[] = "$Id: tdsconvert.h 487448 2015-12-17 18:43:08Z ucko $";
static const void *const no_unused_tdsconvert_h_warn[] = { rcsid_tdsconvert_h, no_unused_tdsconvert_h_warn };

typedef union conv_result
{
	TDS_TINYINT ti;
	TDS_SMALLINT si;
	TDS_INT i;
	TDS_INT8 bi;
	TDS_FLOAT f;
	TDS_REAL r;
	TDS_CHAR *c;
	TDS_MONEY m;
	TDS_MONEY4 m4;
	TDS_DATETIME dt;
	TDS_DATETIME4 dt4;
	TDS_NUMERIC n;
	TDS_CHAR *ib;
	TDS_UNIQUE u;
}
CONV_RESULT;

/*
 * Failure return codes for tds_convert()
 */
#define TDS_CONVERT_FAIL	-1	/* unspecified failure */
#define TDS_CONVERT_NOAVAIL	-2	/* conversion does not exist */
#define TDS_CONVERT_SYNTAX	-3	/* syntax error in source field */
#define TDS_CONVERT_NOMEM	-4	/* insufficient memory */
#define TDS_CONVERT_OVERFLOW	-5	/* result too large */

struct tds_time
{
	int tm_year; /**< year (0=1900) */
	int tm_mon;  /**< month (0-11) */
	int tm_mday; /**< month day (1-31) */
	int tm_hour; /**< hours (0-23) */
	int tm_min;  /**< minutes (0-59) */
	int tm_sec;  /**< seconds (0-59) */
	int tm_ms;   /**< milliseconds (0-999) */
};

unsigned char tds_willconvert(int srctype, int desttype);

TDS_SERVER_TYPE tds_get_null_type(TDS_SERVER_TYPE srctype);
TDS_INT tds_convert(const TDSCONTEXT * context, int srctype, const TDS_CHAR * src, TDS_UINT srclen, int desttype, CONV_RESULT * cr);

size_t tds_strftime(char *buf, size_t maxsize, const char *format, const TDSDATEREC * timeptr);

#ifdef __cplusplus
#if 0
{
#endif
}
#endif

#endif /* _tdsconvert_h_ */
