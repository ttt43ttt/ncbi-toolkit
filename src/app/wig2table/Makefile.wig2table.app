#################################
# $Id: Makefile.wig2table.app 398558 2013-05-07 11:21:36Z kornbluh $
# Author:  Eugene Vasilchenko
#################################

# Build application "wig2table"
#################################

APP = wig2table
SRC = wig2table

LIB  = xobjreadex $(OBJREAD_LIBS) xobjutil $(OBJMGR_LIBS)
LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects

WATCHERS = vasilche
