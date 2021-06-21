# $Id: Makefile.test_nc_stress_pubmed.app 507718 2016-07-21 12:47:39Z gouriano $

APP = test_nc_stress_pubmed
SRC = test_nc_stress_pubmed
LIB = xconnserv$(STATIC) xconnect$(STATIC) xutil$(STATIC) xncbi$(STATIC)

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = unix


WATCHERS = gouriano
