# $Id: Makefile.alnmrg.app 553798 2017-12-21 17:12:12Z fongah2 $


APP = alnmrg
SRC = alnmrg

LIB = xalnmgr $(OBJREAD_LIBS) ncbi_xloader_blastdb seqdb xobjutil blastdb \
      tables $(OBJMGR_LIBS) $(LMDB_LIB)

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

CHECK_CMD  = run_sybase_app.sh -run-script alnmrg.sh /CHECK_NAME=alnmrg.sh
CHECK_COPY = alnmrg.sh data

WATCHERS = grichenk
