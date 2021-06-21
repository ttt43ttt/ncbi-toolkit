#################################
# $Id: Makefile.agp_renumber.app 501626 2016-05-17 17:32:10Z kornbluh $
#################################

REQUIRES = objects

APP = agp_renumber
SRC = agp_renumber

LIB = $(OBJREAD_LIBS) seqset $(SEQ_LIBS) pub medline biblio general \
      xser xutil xncbi

WATCHERS = sapojnik
