#################################
# $Id: Makefile.tls.app 536927 2017-05-24 16:00:07Z ucko $
# Author:  Colleen Bollin
#################################

# Build application "tls"
#################################

APP = tls
SRC = tls

LIB = $(OBJEDIT_LIBS) $(XFORMAT_LIBS) xalnmgr xobjutil \
       xregexp $(PCRE_LIB) tables $(OBJMGR_LIBS)

LIBS = $(PCRE_LIBS) \
       $(NETWORK_LIBS) $(CMPRS_LIBS) $(ORIG_LIBS)

REQUIRES = objects

WATCHERS = bollin
