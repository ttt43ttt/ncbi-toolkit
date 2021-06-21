#################################
# $Id: Makefile.test_objmgr_mem.app 505858 2016-06-29 16:55:21Z elisovdn $
# Author:  Eugene Vasilchenko (vasilche@ncbi.nlm.nih.gov)
#################################

# Build object manager test application "test_objmgr_mem"
#################################


APP = test_objmgr_mem
SRC = test_objmgr_mem
LIB = $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_objmgr_mem

WATCHERS = vasilche
