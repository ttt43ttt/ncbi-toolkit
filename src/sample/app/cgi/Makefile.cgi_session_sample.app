# $Id: Makefile.cgi_session_sample.app 536927 2017-05-24 16:00:07Z ucko $

# Build test CGI application "cgi_sample"
#################################

APP = cgi_session_sample.cgi
SRC = cgi_session_sample

# new_project.sh will copy everything in the following block to any
# Makefile.*_app generated from this sample project.  Do not change
# the lines reading "### BEGIN/END COPIED SETTINGS" in any way.

### BEGIN COPIED SETTINGS
LIB = xgridcgi xcgi xhtml xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

### END COPIED SETTINGS

WATCHERS = sadyrovr
