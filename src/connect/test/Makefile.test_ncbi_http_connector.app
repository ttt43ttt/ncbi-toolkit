# $Id: Makefile.test_ncbi_http_connector.app 530905 2017-03-19 01:19:36Z lavr $

APP = test_ncbi_http_connector
SRC = test_ncbi_http_connector ncbi_conntest
LIB = connssl connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =

WATCHERS = lavr