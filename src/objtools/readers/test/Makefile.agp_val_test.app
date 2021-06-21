#################################
# $Id: Makefile.agp_val_test.app 501626 2016-05-17 17:32:10Z kornbluh $
#################################

REQUIRES = objects

APP = agp_val_test
SRC = agp_val_test

LIB = $(OBJREAD_LIBS) seqset $(SEQ_LIBS) pub medline biblio general \
      xser xutil xncbi

WATCHERS = sapojnik
