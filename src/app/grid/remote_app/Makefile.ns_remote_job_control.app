# $Id: Makefile.ns_remote_job_control.app 466155 2015-04-29 14:36:10Z sadyrovr $

APP = ns_remote_job_control
SRC = ns_remote_job_control info_collector renderer
LIB = xconnserv xthrserv xconnect xutil xncbi 

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = sadyrovr
