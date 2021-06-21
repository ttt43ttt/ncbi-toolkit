# $Id: Makefile.test_ncbi_download.app 530905 2017-03-19 01:19:36Z lavr $

APP = test_ncbi_download
SRC = test_ncbi_download
LIB = connssl connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

WATCHERS = lavr
