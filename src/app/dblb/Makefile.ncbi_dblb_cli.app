# $Id: Makefile.ncbi_dblb_cli.app 537147 2017-05-26 13:07:29Z ucko $

APP = ncbi_dblb_cli
SRC = ncbi_dblb_cli

LIB  = $(SDBAPI_LIB) xconnect xregexp $(PCRE_LIB) xutil xncbi
LIBS = $(SDBAPI_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD      = test_ncbi_dblb_cli.sh
CHECK_COPY     = test_ncbi_dblb_cli.sh test_ncbi_dblb_cli.py
CHECK_REQUIRES = connext in-house-resources
