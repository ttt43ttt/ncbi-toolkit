#################################
# $Id: Makefile.seqvec_bench.app 505858 2016-06-29 16:55:21Z elisovdn $
#################################


APP = seqvec_bench
SRC = seqvec_bench
LIB = test_mt $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = dicuccio
