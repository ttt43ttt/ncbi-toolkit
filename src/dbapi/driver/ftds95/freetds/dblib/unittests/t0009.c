/* 
 * Purpose: Test if dbbind can handle a varlen of 0 with a column bound as STRINGBIND and a database column of CHAR.
 * Functions: dbbind dbcmd dbnextrow dbnumcols dbopen dbresults dbsqlexec 
 */

#include "common.h"

#include <common/test_assert.h>

static char software_version[] = "$Id: t0009.c 487476 2015-12-17 19:48:39Z ucko $";
static void *no_unused_var_warn[] = { software_version, no_unused_var_warn };



int
main(int argc, char **argv)
{
	LOGINREC *login;
	DBPROCESS *dbproc;
	int i;
	char teststr[1024];
	DBINT testint;

	set_malloc_options();

	read_login_info(argc, argv);

	fprintf(stdout, "Starting %s\n", argv[0]);

	/* Fortify_EnterScope(); */
	dbinit();

	dberrhandle(syb_err_handler);
	dbmsghandle(syb_msg_handler);

	fprintf(stdout, "About to logon\n");

	login = dblogin();
	DBSETLPWD(login, PASSWORD);
	DBSETLUSER(login, USER);
	DBSETLAPP(login, "t0009");
	DBSETLHOST(login, "ntbox.dntis.ro");

	fprintf(stdout, "About to open\n");

	dbproc = dbopen(login, SERVER);
	if (strlen(DATABASE))
		dbuse(dbproc, DATABASE);
	dbloginfree(login);

#ifdef MICROSOFT_DBLIB
	dbsetopt(dbproc, DBBUFFER, "100");
#else
	dbsetopt(dbproc, DBBUFFER, "100", 0);
#endif

	fprintf(stdout, "creating table\n");
	sql_cmd(dbproc);
	dbsqlexec(dbproc);
	while (dbresults(dbproc) != NO_MORE_RESULTS) {
		/* nop */
	}

	fprintf(stdout, "insert\n");
	sql_cmd(dbproc);
	dbsqlexec(dbproc);
	while (dbresults(dbproc) != NO_MORE_RESULTS) {
		/* nop */
	}
	sql_cmd(dbproc);
	dbsqlexec(dbproc);
	while (dbresults(dbproc) != NO_MORE_RESULTS) {
		/* nop */
	}


	fprintf(stdout, "select\n");
	sql_cmd(dbproc);
	dbsqlexec(dbproc);

	if (dbresults(dbproc) != SUCCEED) {
		fprintf(stdout, "Was expecting a result set.");
		exit(1);
	}

	for (i = 1; i <= dbnumcols(dbproc); i++)
		printf("col %d is %s\n", i, dbcolname(dbproc, i));

	dbbind(dbproc, 1, INTBIND, -1, (BYTE *) & testint);
	dbbind(dbproc, 2, STRINGBIND, 0, (BYTE *) teststr);

	if (REG_ROW != dbnextrow(dbproc)) {
		fprintf(stderr, "dblib failed for %s\n", __FILE__);
		exit(1);
	}
	if (0 != strcmp("abcdef    ", teststr)) {
		fprintf(stderr, "Expected |%s|, found |%s|\n", "abcdef", teststr);
		fprintf(stderr, "dblib failed for %s\n", __FILE__);
		exit(1);
	}

	if (REG_ROW != dbnextrow(dbproc)) {
		fprintf(stderr, "dblib failed for %s\n", __FILE__);
		exit(1);
	}
	if (0 != strcmp("abc       ", teststr)) {
		fprintf(stderr, "Expected |%s|, found |%s|\n", "abc", teststr);
		fprintf(stderr, "dblib failed for %s\n", __FILE__);
		exit(1);
	}

	dbexit();

	printf("dblib passed for %s\n", __FILE__);
	return 0;
}
