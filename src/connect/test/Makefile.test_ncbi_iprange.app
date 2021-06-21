# $Id: Makefile.test_ncbi_iprange.app 537410 2017-05-31 16:20:20Z lavr $

APP = test_ncbi_iprange
SRC = test_ncbi_iprange
LIB = connect

LIBS = $(NETWORK_LIBS) $(C_LIBS)
LINK = $(C_LINK)
# LINK = purify $(C_LINK)

WATCHERS = lavr
