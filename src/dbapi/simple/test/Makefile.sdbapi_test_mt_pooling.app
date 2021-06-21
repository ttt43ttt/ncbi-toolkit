# $Id: Makefile.sdbapi_test_mt_pooling.app 553128 2017-12-13 16:54:04Z satskyse $

APP = sdbapi_test_mt_pooling
SRC = sdbapi_test_mt_pooling

LIB  = $(SDBAPI_LIB) xconnect xutil test_mt xncbi
LIBS = $(SDBAPI_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD =
CHECK_REQUIRES = MT in-house-resources

WATCHERS = ucko satskyse
