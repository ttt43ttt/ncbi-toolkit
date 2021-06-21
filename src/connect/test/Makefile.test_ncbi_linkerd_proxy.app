# $Id: Makefile.test_ncbi_linkerd_proxy.app 570468 2018-09-10 13:45:50Z ivanov $

# Temporarily disable on Windows due to missing devops support.
REQUIRES = -MSWin

APP = test_ncbi_linkerd_proxy
SRC = test_ncbi_linkerd_proxy

LIB = xregexp $(PCRE_LIB) xconnect xncbi
LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = in-house-resources
CHECK_CMD =
CHECK_TIMEOUT = 600

WATCHERS = mcelhany
