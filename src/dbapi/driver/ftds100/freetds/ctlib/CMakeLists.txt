#############################################################################
# $Id: CMakeLists.txt 573635 2018-10-30 15:55:37Z ivanov $
#############################################################################

NCBI_add_library(ctlib100)
NCBI_add_subdirectory(unittests)

if(OFF)
# Include projects from this directory
include(CMakeLists.ctlib100.lib.txt)

# Recurse subdirectories
add_subdirectory(unittests)
endif()
