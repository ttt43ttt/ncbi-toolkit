# $Id: Makefile.test_ncbi_ipv6.app 516469 2016-10-13 16:36:21Z lavr $

APP = test_ncbi_ipv6
SRC = test_ncbi_ipv6
LIB = connect

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

WATCHERS = lavr
