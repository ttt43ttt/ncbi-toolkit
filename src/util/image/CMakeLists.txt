#############################################################################
# $Id: CMakeLists.txt 563863 2018-05-16 14:50:58Z gouriano $
#############################################################################
# 
#

NCBI_add_library(image)
NCBI_add_subdirectory(test demo)

#include(CMakeLists.image.lib.txt)
# Recurse subdirectories
#add_subdirectory(test )
#add_subdirectory(demo )
