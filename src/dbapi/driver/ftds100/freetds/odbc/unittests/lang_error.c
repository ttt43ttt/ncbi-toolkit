#include "common.h"

#include <common/test_assert.h>

/* Test if SQLExecDirect return error if a error in row is returned */

static char software_version[] = "$Id: lang_error.c 554130 2017-12-28 17:20:38Z ucko $";
static void *no_unused_var_warn[] = { software_version, no_unused_var_warn };

int
main(int argc, char *argv[])
{
	odbc_connect();

	/* issue print statement and test message returned */
	odbc_command2("SELECT DATEADD(dd,-100000,getdate())", "E");

	odbc_disconnect();

	printf("Done.\n");
	return 0;
}