/* FreeTDS - Library of routines accessing Sybase and Microsoft databases
 * Copyright (C) 2004 Frediano Ziglio
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

#ifndef TDS_CHECKS_H
#define TDS_CHECKS_H

/* $Id: tds_checks.h 123513 2008-04-03 12:33:27Z ivanovp $ */

#if ENABLE_EXTRA_CHECKS
#define CHECK_STRUCT_EXTRA(func,s) func(s)
#else
#define CHECK_STRUCT_EXTRA(func,s)
#endif

#define CHECK_TDS_EXTRA(tds)              CHECK_STRUCT_EXTRA(tds_check_tds_extra,tds)
#define CHECK_CONTEXT_EXTRA(ctx)          CHECK_STRUCT_EXTRA(tds_check_context_extra,ctx)
#define CHECK_TDSENV_EXTRA(env)           CHECK_STRUCT_EXTRA(tds_check_env_extra,env)
#define CHECK_COLUMN_EXTRA(column)        CHECK_STRUCT_EXTRA(tds_check_column_extra,column)
#define CHECK_RESULTINFO_EXTRA(res_info)  CHECK_STRUCT_EXTRA(tds_check_resultinfo_extra,res_info)
#define CHECK_PARAMINFO_EXTRA(res_info)   CHECK_STRUCT_EXTRA(tds_check_resultinfo_extra,res_info)
#define CHECK_CURSOR_EXTRA(cursor)        CHECK_STRUCT_EXTRA(tds_check_cursor_extra,cursor)
#define CHECK_DYNAMIC_EXTRA(dynamic)      CHECK_STRUCT_EXTRA(tds_check_dynamic_extra,dynamic)

#if ENABLE_EXTRA_CHECKS
void tds_check_tds_extra(const TDSSOCKET * tds);
void tds_check_context_extra(const TDSCONTEXT * ctx);
void tds_check_env_extra(const TDSENV * env);
void tds_check_column_extra(const TDSCOLUMN * column);
void tds_check_resultinfo_extra(const TDSRESULTINFO * res_info);
void tds_check_cursor_extra(const TDSCURSOR * cursor);
void tds_check_dynamic_extra(const TDSDYNAMIC * dynamic);

int tds_get_cardinal_type(int datatype);
int tds_get_varint_size(TDSSOCKET * tds, int datatype);
#endif

#endif /* TDS_CHECKS_H */
