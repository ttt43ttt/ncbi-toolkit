#################################
# $Id: Makefile.gff_deconcat.app 512626 2016-09-01 18:29:40Z foleyjp $
# Author:  Justin Foley
#################################

# Build application "gff_deconcat"
#################################

APP = gff_deconcat
SRC = gff_deconcat
LIB = $(OBJREAD_LIBS) xobjutil $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin


WATCHERS = foleyjp
