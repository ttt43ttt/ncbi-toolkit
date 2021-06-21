#################################
# $Id: Makefile.agp_count.app 501626 2016-05-17 17:32:10Z kornbluh $
#################################

APP = agp_count
SRC = agp_count

LIB = $(OBJREAD_LIBS) seqset $(SEQ_LIBS) pub medline biblio general \
      xser xutil xncbi

WATCHERS = sapojnik
