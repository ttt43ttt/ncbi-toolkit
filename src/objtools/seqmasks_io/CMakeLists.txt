#############################################################################
# $Id: CMakeLists.txt 566390 2018-06-28 17:11:39Z gouriano $
#############################################################################

NCBI_add_library(seqmasks_io)
NCBI_add_subdirectory(unit_test)

# Include projects from this directory
#include(CMakeLists.seqmasks_io.lib.txt)

# Recurse subdirectories
#add_subdirectory(unit_test )
