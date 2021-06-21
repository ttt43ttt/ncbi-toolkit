#################################
# $Id: Makefile.test_edit_saver.app 505858 2016-06-29 16:55:21Z elisovdn $
# Author:  Maxim Didenko (didenko@ncbi.nlm.nih.gov)
#################################

APP = test_edit_saver
SRC = test_edit_saver
LIB = xobjutil ncbi_xloader_patcher $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_edit_saver -gi 45678
CHECK_CMD = test_edit_saver -gi 21225451 

WATCHERS = vasilche
