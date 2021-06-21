#############################################################################
# $Id: CMakeLists.txt 573635 2018-10-30 15:55:37Z ivanov $
#############################################################################


NCBI_add_library(ncbi_xdbapi_ftds100)
NCBI_add_subdirectory(samples)

if(OFF)
# Include projects from this directory
include(CMakeLists.ncbi_xdbapi_ftds100.lib.txt)

# Recurse subdirectories
add_subdirectory(samples)
endif()
