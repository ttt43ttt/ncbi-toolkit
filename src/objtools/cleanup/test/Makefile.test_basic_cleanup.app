###############################
# $Id: Makefile.test_basic_cleanup.app 501626 2016-05-17 17:32:10Z kornbluh $
###############################

APP = test_basic_cleanup
SRC = test_basic_cleanup
LIB = xcleanup xobjutil valid taxon3 submit xconnect xregexp $(PCRE_LIB) \
      $(SOBJMGR_LIBS)

LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = bollin kans