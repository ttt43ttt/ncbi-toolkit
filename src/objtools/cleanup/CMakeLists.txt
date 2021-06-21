#############################################################################
# $Id: CMakeLists.txt 566390 2018-06-28 17:11:39Z gouriano $
#############################################################################

NCBI_add_library(cleanup)
NCBI_add_subdirectory(test)

# Include projects from this directory
#include(CMakeLists.cleanup.lib.txt)

# Recurse subdirectories
#add_subdirectory(test )
