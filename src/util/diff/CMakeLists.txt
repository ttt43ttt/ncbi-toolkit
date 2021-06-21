#############################################################################
# $Id: CMakeLists.txt 564072 2018-05-21 13:16:53Z gouriano $
#############################################################################
# 
#

NCBI_project_tags(core)
NCBI_add_library(xdiff)
NCBI_add_subdirectory(test)

# Include projects from this directory
#include(CMakeLists.xdiff.lib.txt)

# Recurse subdirectories
#add_subdirectory(test )
