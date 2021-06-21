# $Id: Makefile.prt2fsm.app 569060 2018-08-15 18:14:53Z kachalos $
#
# Makefile:  for Multipattern Search Code Generator
# Author: Sema
#

###  BASIC PROJECT SETTINGS
APP = prt2fsm
SRC = prt2fsm

LIB = macro $(OBJMGR_LIBS)

LIBS = $(ORIG_LIBS)

CPPFLAGS= $(ORIG_CPPFLAGS) -I$(import_root)/../include

LDFLAGS = -L$(import_root)/../lib $(ORIG_LDFLAGS)

REQUIRES = objects

WATCHERS = kachalos
