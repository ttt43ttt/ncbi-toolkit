#################################
# $Id: Makefile.remote_blast_demo.app 553487 2017-12-18 14:23:38Z fongah2 $
#################################

WATCHERS = camacho madden

APP = remote_blast_demo
SRC = remote_blast_demo

# new_project.sh will copy everything in the following block to any
# Makefile.*_app generated from this sample project.  Do not change
# the lines reading "### BEGIN/END COPIED SETTINGS" in any way.

### BEGIN COPIED SETTINGS
LIB_ = $(BLAST_INPUT_LIBS) $(BLAST_LIBS) $(OBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC))
LIBS = $(BLAST_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

# These settings are necessary for optimized WorkShop builds, due to
# BLAST's own use of them.
CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS = $(FAST_LDFLAGS) 

REQUIRES = objects -Cygwin
### END COPIED SETTINGS

