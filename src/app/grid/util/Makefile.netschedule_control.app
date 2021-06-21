#################################
# $Id: Makefile.netschedule_control.app 466155 2015-04-29 14:36:10Z sadyrovr $
#################################

APP = netschedule_control
SRC = netschedule_control

LIB = xconnserv xthrserv xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = sadyrovr
