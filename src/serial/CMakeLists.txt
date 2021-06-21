#############################################################################
# $Id: CMakeLists.txt 564101 2018-05-21 15:34:07Z gouriano $
#############################################################################
# 
#

NCBI_project_tags(core)
NCBI_add_library(serial cserial)
NCBI_add_subdirectory(datatool test soap)


if(OFF)
# Include projects from this directory
include(CMakeLists.serial.lib.txt)
include(CMakeLists.cserial.lib.txt)

# Recurse subdirectories
add_subdirectory(datatool)
add_subdirectory(test )
add_subdirectory(soap)
endif()
