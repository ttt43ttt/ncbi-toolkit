#include "common.h"

#include <common/test_assert.h>

/* test SQL_C_DEFAULT with NCHAR type */

static char software_version[] = "$Id: wchar.c 487476 2015-12-17 19:48:39Z ucko $";
static void *no_unused_var_warn[] = { software_version, no_unused_var_warn };

int
main(int argc, char *argv[])
{
	char buf[102];
	SQLLEN ind;
	int failed = 0;

	odbc_use_version3 = 1;
	odbc_connect();

	CHKBindCol(1, SQL_C_DEFAULT, buf, 100, &ind, "S");
	odbc_command("SELECT CONVERT(NCHAR(10), 'Pippo 123')");

	/* get data */
	memset(buf, 0, sizeof(buf));
	CHKFetch("S");

	SQLMoreResults(odbc_stmt);
	SQLMoreResults(odbc_stmt);

	odbc_disconnect();

	if (strcmp(buf, "Pippo 123 ") != 0) {
		fprintf(stderr, "Wrong results '%s'\n", buf);
		failed = 1;
	}

	return failed ? 1 : 0;
}

