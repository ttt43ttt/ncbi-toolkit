#include "common.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif /* HAVE_SYS_WAIT_H */

#include <common/test_assert.h>

/*
 * test error on connection close 
 * With a trick we simulate a connection close then we try to 
 * prepare or execute a query. This should fail and return an error message.
 */

#if HAVE_FSTAT && defined(S_IFSOCK)

static int
close_last_socket(void)
{
	TDS_SYS_SOCKET max_socket = odbc_find_last_socket();
	TDS_SYS_SOCKET sockets[2];
#if defined(__APPLE__) && defined(SO_NOSIGPIPE)
        int on = 1;
#endif

	if (max_socket < 0) {
		fprintf(stderr, "Error finding last socket\n");
		return 0;
	}

	/* replace socket with a new one */
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
		return 0;

#if defined(__APPLE__)  &&  defined(SO_NOSIGPIPE)
        if (setsockopt(sockets[0], SOL_SOCKET, SO_NOSIGPIPE,
                       (const void *) &on, sizeof(on)))
                return 0;
#endif

	/* substitute socket */
	close(max_socket);
	dup2(sockets[0], max_socket);

	/* close connection */
	close(sockets[0]);
	close(sockets[1]);
	return 1;
}

static int
Test(int direct)
{
	SQLTCHAR buf[256];
	SQLTCHAR sqlstate[6];

	odbc_mark_sockets_opened();
	odbc_connect();

	if (!close_last_socket()) {
		fprintf(stderr, "Error closing connection\n");
		return 1;
	}

	/* force disconnection closing socket */
	if (direct) {
		CHKExecDirect(T("SELECT 1"), SQL_NTS, "E");
	} else {
		SQLSMALLINT cols;
		/* use prepare, force dialog with server */
		if (CHKPrepare(T("SELECT 1"), SQL_NTS, "SE") == SQL_SUCCESS)
			CHKNumResultCols(&cols, "E");
	}

	CHKGetDiagRec(SQL_HANDLE_STMT, odbc_stmt, 1, sqlstate, NULL, buf, ODBC_VECTOR_SIZE(buf), NULL, "SI");
	sqlstate[5] = 0;
	printf("state=%s err=%s\n", C(sqlstate), C(buf));
	
	odbc_disconnect();

	printf("Done.\n");
	return 0;
}

int
main(void)
{
	if (Test(0) || Test(1))
		return 1;
	return 0;
}

#else
int
main(void)
{
	printf("Not possible for this platform.\n");
	odbc_test_skipped();
	return 0;
}
#endif

