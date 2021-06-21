# $Id: Makefile.ctl_lang_ftds95.app 573637 2018-10-30 15:57:48Z ivanov $

APP = ctl_lang_ftds95
SRC = ctl_lang_ftds95 dbapi_driver_sample_base_ftds95

LIB  = ncbi_xdbapi_ftds95$(STATIC) $(FTDS95_CTLIB_LIB) \
       dbapi_driver$(STATIC) $(XCONNEXT) xconnect xncbi
LIBS = $(FTDS95_CTLIB_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS) $(DL_LIBS)

CPPFLAGS = -DFTDS_IN_USE -I$(includedir)/dbapi/driver/ftds95 \
           $(FTDS95_INCLUDE) $(ORIG_CPPFLAGS)

CHECK_REQUIRES = connext in-house-resources
# CHECK_CMD = run_sybase_app.sh ctl_lang_ftds95
CHECK_CMD = run_sybase_app.sh ctl_lang_ftds95 -S DBAPI_MS2017_TEST_LB /CHECK_NAME=ctl_lang_ftds95-MS2017
CHECK_CMD = run_sybase_app.sh ctl_lang_ftds95 -S DBAPI_SYB160_TEST -v 50 /CHECK_NAME=ctl_lang_ftds95-SYB16-2K
CHECK_CMD = run_sybase_app.sh ctl_lang_ftds95 -S DBAPI_DEV16_16K -v 50 /CHECK_NAME=ctl_lang_ftds95-SYB16-16K

WATCHERS = ucko satskyse