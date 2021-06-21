# $Id: Makefile.AccVerCache.app 569048 2018-08-15 16:56:17Z sadyrovr $

APP = AccVerCacheApp
SRC = AccVerCacheApp
LIB = psg_client psg_rpc psg_diag $(SEQ_LIBS) pub medline biblio general xser xconnserv xconnect xutil xncbi

LIBS = $(PSG_RPC_LIBS) $(ORIG_LIBS)
CPPFLAGS = $(PSG_RPC_INCLUDE) $(ORIG_CPPFLAGS)

WATCHERS = sadyrovr dmitrie1
