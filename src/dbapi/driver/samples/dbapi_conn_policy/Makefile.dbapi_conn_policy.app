# $Id: Makefile.dbapi_conn_policy.app 554130 2017-12-28 17:20:38Z ucko $

APP = dbapi_conn_policy
SRC = dbapi_conn_policy

LIB  = dbapi_sample_base$(STATIC) $(DBAPI_CTLIB) $(DBAPI_ODBC) \
       ncbi_xdbapi_ftds ncbi_xdbapi_ftds64 $(FTDS64_LIB) \
       ncbi_xdbapi_ftds95 $(FTDS95_LIB) ncbi_xdbapi_ftds100 $(FTDS100_LIB) \
       dbapi_driver$(STATIC) $(XCONNEXT) xconnect xutil xncbi

LIBS = $(SYBASE_LIBS) $(SYBASE_DLLS) $(ODBC_LIBS) $(FTDS64_LIBS) \
       $(FTDS95_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_COPY = dbapi_conn_policy.ini

WATCHERS = ucko satskyse