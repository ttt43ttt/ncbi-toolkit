#################################
# $Id: Makefile.formatguess.app 556898 2018-02-06 16:33:09Z ivanov $
# Author:  Frank Ludwig
#################################

# Build application "formatguess"
#################################

APP = formatguess
SRC = formatguess
LIB  = xmlwrapp $(OBJREAD_LIBS) seqset $(SEQ_LIBS) pub medline biblio \
       general xser xconnect xutil xncbi
LIBS = $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

WATCHERS = ludwigf

REQUIRES = objects LIBXML LIBXSLT
