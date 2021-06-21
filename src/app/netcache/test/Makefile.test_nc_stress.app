# $Id: Makefile.test_nc_stress.app 507718 2016-07-21 12:47:39Z gouriano $

APP = test_nc_stress
SRC = test_nc_stress
LIB = xconnserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = gouriano
