# $Id: Makefile.bdbloader_unit_test.app 553487 2017-12-18 14:23:38Z fongah2 $

APP = bdbloader_unit_test
SRC = bdbloader_unit_test

CPPFLAGS = -DNCBI_MODULE=BLASTDB $(ORIG_CPPFLAGS) $(BLAST_THIRD_PARTY_INCLUDE) $(BOOST_INCLUDE) -I../

LIB_ = test_boost $(BLAST_INPUT_LIBS) $(BLAST_LIBS) $(OBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC))

LIBS =  $(BLAST_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)
LDFLAGS = $(FAST_LDFLAGS)

REQUIRES = objects Boost.Test.Included

CHECK_REQUIRES = MT in-house-resources
CHECK_CMD = bdbloader_unit_test
CHECK_COPY = data
CHECK_TIMEOUT = 600


WATCHERS = madden camacho
