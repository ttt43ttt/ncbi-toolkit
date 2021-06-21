# $Id: Makefile.sub_cache_create.app 579394 2019-02-01 21:33:41Z fukanchi $

APP = sub_cache_create
SRC = sub_cache_create

LIB  = $(DATA_LOADERS_UTIL_LIB) $(SRAREAD_LIBS) $(OBJMGR_LIBS)
LIBS = $(DATA_LOADERS_UTIL_LIBS) $(CMPRS_LIBS) $(ORIG_LIBS)

CHECK_CMD = python sub_cache_create_unit_test.py --missing-gis remote.gis --lds remote.lds2 --reference-index reference.idx --extract test.gis --scratch-directory tmp /CHECK_NAME=sub_cache_create_unit_test
CHECK_COPY = sub_cache_create_unit_test.py
CHECK_REQUIRES = in-house-resources PYTHON internal

REQUIRES = BerkeleyDB SQLITE3

WATCHERS = mozese2
