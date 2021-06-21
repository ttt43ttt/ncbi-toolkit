# $Id: Makefile.unit_test_id_mapper.app 500512 2016-05-05 13:14:49Z gouriano $

APP = unit_test_id_mapper

SRC = unit_test_id_mapper

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xid_mapper gencoll_client $(OBJREAD_LIBS) $(XFORMAT_LIBS) \
      xalnmgr xobjutil tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD  = unit_test_id_mapper
CHECK_COPY = unit_test_id_mapper.ini
CHECK_TIMEOUT = 1800

WATCHERS = boukn meric
