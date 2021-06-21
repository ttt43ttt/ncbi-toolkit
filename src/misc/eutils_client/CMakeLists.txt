#############################################################################
# $Id: CMakeLists.txt 565539 2018-06-13 16:19:50Z gouriano $
#############################################################################


NCBI_add_library(eutils_client)
NCBI_add_subdirectory(test unit_test)

if(OFF)
# Include projects from this directory
include(CMakeLists.eutils_client.lib.txt)

# Recurse subdirectories
add_subdirectory(test )
add_subdirectory(unit_test )
endif()
