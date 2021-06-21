# $Id: Makefile.psiblast_unit_test.app 553487 2017-12-18 14:23:38Z fongah2 $

APP = psiblast_unit_test
SRC = psiblast_unit_test

CPPFLAGS = -DNCBI_MODULE=BLAST $(ORIG_CPPFLAGS) $(BOOST_INCLUDE) -I$(srcdir)/../../api
LIB = seqalign_util test_boost $(BLAST_INPUT_LIBS) \
    $(BLAST_LIBS) xobjsimple $(OBJMGR_LIBS:ncbi_x%=ncbi_x%$(DLL)) 
LIBS = $(BLAST_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = MT in-house-resources
CHECK_CMD = psiblast_unit_test
CHECK_COPY = psiblast_unit_test.ini data

WATCHERS = boratyng madden camacho