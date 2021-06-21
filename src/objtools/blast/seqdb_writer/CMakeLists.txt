#############################################################################
# $Id: CMakeLists.txt 566390 2018-06-28 17:11:39Z gouriano $
#############################################################################

NCBI_requires(LMDB)
NCBI_add_library(writedb)
NCBI_add_subdirectory(unit_test)

# Include projects from this directory
#include(CMakeLists.writedb.lib.txt)

# Recurse subdirectories
#add_subdirectory(unit_test )
