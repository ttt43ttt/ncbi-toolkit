#ifndef COMMON_h
#define COMMON_h

static char rcsid_common_h[] = "$Id: common.h 487476 2015-12-17 19:48:39Z ucko $";
static void *no_unused_common_h_warn[] = { rcsid_common_h, no_unused_common_h_warn };

#include <config.h>

#include <stdarg.h>
#include <stdio.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include <freetds/tds.h>
#include <freetds/data.h>

#ifndef FREETDS_SRCDIR
#define FREETDS_SRCDIR FREETDS_TOPDIR "/src/tds/unittests"
#endif

extern char PASSWORD[512];
extern char USER[512];
extern char SERVER[512];
extern char DATABASE[512];
extern char CHARSET[512];

extern TDSCONTEXT *test_context;

int try_tds_login(TDSLOGIN ** login, TDSSOCKET ** tds, const char *appname, int verbose);
int try_tds_logout(TDSLOGIN * login, TDSSOCKET * tds, int verbose);

int run_query(TDSSOCKET * tds, const char *query);

extern int utf8_max_len;

int get_unichar(const char **psrc);
char *to_utf8(const char *src, char *dest);

#endif
