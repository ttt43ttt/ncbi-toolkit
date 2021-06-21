#################################
# $Id: Makefile.test_annot_ci.app 505858 2016-06-29 16:55:21Z elisovdn $
# Author:  Aleksey Grichenko (grichenk@ncbi.nlm.nih.gov)
#################################

# Build seq-annot iterators test application "test_annot_ci"
#################################


APP = test_annot_ci
SRC = test_annot_ci
LIB = $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_annot_ci
CHECK_COPY = test_annot_ci.ini test_annot_entries.asn test_annot_res.asn

WATCHERS = grichenk
