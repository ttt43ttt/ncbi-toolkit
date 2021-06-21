# $Id: Makefile.multipattern.app 563023 2018-05-01 14:58:19Z kachalos $
#
# Makefile:  for Multipattern Search Code Generator
# Author: Sema
#

###  BASIC PROJECT SETTINGS
APP = multipattern
SRC = multipattern

LIB = $(OBJMGR_LIBS)

LIBS = $(ORIG_LIBS)

CPPFLAGS= $(ORIG_CPPFLAGS) -I$(import_root)/../include

LDFLAGS = -L$(import_root)/../lib $(ORIG_LDFLAGS)

REQUIRES = objects

WATCHERS = kachalos
