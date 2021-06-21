#############################################################################
# $Id: CMakeLists.txt 565603 2018-06-14 13:44:37Z gouriano $
#############################################################################


NCBI_add_library(objmgr)
NCBI_add_subdirectory(test util split)

if(OFF)
# Include projects from this directory
include(CMakeLists.objmgr.lib.txt)

# Recurse subdirectories
add_subdirectory(test )
add_subdirectory(util)
add_subdirectory(split)
endif()
