# $Id: Makefile.netcache_cgi_sample.app 536927 2017-05-24 16:00:07Z ucko $

APP = netcache_cgi_sample.cgi
SRC = netcache_cgi_sample

### BEGIN COPIED SETTINGS
LIB = xconnserv xthrserv xconnect xcgi xhtml xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)
### END COPIED SETTINGS

WATCHERS = sadyrovr
