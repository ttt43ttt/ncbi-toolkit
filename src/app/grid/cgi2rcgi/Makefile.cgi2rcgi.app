# $Id: Makefile.cgi2rcgi.app 466155 2015-04-29 14:36:10Z sadyrovr $

# Build test CGI application "cgi_tunnel2grid"
#################################

APP = cgi2rcgi
SRC = cgi2rcgi

LIB = xconnserv xthrserv xcgi xhtml xregexp $(PCRE_LIB) xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(PCRE_LIBS) $(ORIG_LIBS)

WATCHERS = sadyrovr
