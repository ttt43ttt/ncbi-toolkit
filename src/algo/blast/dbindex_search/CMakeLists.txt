#############################################################################
# $Id: CMakeLists.txt 566834 2018-07-09 12:39:41Z gouriano $
#############################################################################

NCBI_add_library(xalgoblastdbindex_search)
NCBI_add_subdirectory(srsearch)

# Include projects from this directory
#include(CMakeLists.xalgoblastdbindex_search.lib.txt)

# Recurse subdirectories
#add_subdirectory(srsearch)
