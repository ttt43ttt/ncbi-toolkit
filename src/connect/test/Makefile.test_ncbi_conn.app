# $Id: Makefile.test_ncbi_conn.app 530905 2017-03-19 01:19:36Z lavr $

APP = test_ncbi_conn
SRC = test_ncbi_conn
LIB = xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_conn.sh
CHECK_COPY = test_ncbi_conn.sh ../../check/ncbi_test_data

WATCHERS = lavr satskyse
