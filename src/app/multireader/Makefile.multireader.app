#################################
# $Id: Makefile.multireader.app 572212 2018-10-10 14:46:05Z ivanov $
# Author:  Frank Ludwig
#################################

# Build application "multireader"
#################################

APP =  multireader
SRC =  multireader multifile_source multifile_destination
LIB =  xobjreadex $(OBJEDIT_LIBS) $(XFORMAT_LIBS) \
       xalgophytree biotree fastme xalnmgr tables \
       xobjutil xconnect xregexp $(PCRE_LIB) $(SOBJMGR_LIBS) 

LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects algo -Cygwin

# proj_tag gbench allows to include multireader into sviwer build tree 
PROJ_TAG = gbench

WATCHERS = ludwigf gotvyans
