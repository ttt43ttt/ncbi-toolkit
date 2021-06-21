#############################################################################
# $Id: CMakeLists.txt 566834 2018-07-09 12:39:41Z gouriano $
#############################################################################

NCBI_add_library(xalgoblastdbindex)
NCBI_add_subdirectory(makeindex)

# Include projects from this directory
#include(CMakeLists.xalgoblastdbindex.lib.txt)

# Recurse subdirectories
#add_subdirectory(makeindex)
