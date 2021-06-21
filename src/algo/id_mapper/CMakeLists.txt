#############################################################################
# $Id: CMakeLists.txt 566834 2018-07-09 12:39:41Z gouriano $
#############################################################################

NCBI_add_library(id_mapper)
NCBI_add_subdirectory(unit_test)

# Include projects from this directory
#include(CMakeLists.id_mapper.lib.txt)

# Recurse subdirectories
#add_subdirectory(unit_test )
