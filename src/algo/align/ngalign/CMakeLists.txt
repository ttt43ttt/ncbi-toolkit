#############################################################################
# $Id: CMakeLists.txt 566834 2018-07-09 12:39:41Z gouriano $
#############################################################################

NCBI_project_watchers(boukn)
NCBI_add_library(xngalign)
NCBI_add_subdirectory(app)

# Include projects from this directory
#include(CMakeLists.xngalign.lib.txt)

#add_subdirectory(app)
