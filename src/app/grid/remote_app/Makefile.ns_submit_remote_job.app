# $Id: Makefile.ns_submit_remote_job.app 466155 2015-04-29 14:36:10Z sadyrovr $

APP = ns_submit_remote_job
SRC = ns_submit_remote_job
LIB = xconnserv xthrserv xconnect xutil xncbi 

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = sadyrovr
