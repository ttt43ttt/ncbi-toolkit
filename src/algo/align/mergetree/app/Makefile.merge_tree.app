# $Id: Makefile.merge_tree.app 553554 2017-12-18 21:50:50Z fongah2 $

APP = merge_tree
SRC = merge_tree_app  


LIB = xmergetree xalgoalignutil xalnmgr tables scoremat \
      $(GENBANK_LIBS) $(SEQ_LIBS) 
LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)
