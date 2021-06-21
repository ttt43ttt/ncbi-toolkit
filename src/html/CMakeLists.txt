#############################################################################
# $Id: CMakeLists.txt 564553 2018-05-30 14:04:12Z gouriano $
#############################################################################

NCBI_project_tags(web)
NCBI_add_library(html)
NCBI_add_subdirectory(test demo)

if(OFF)
# 
#

# Include projects from this directory
include(CMakeLists.html.lib.txt)

# Recurse subdirectories
add_subdirectory(test )
add_subdirectory(demo )
endif()
