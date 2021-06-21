#############################################################################
# $Id: CMakeLists.txt 566851 2018-07-09 15:25:50Z gouriano $
#############################################################################

NCBI_add_library(bamread)
NCBI_add_subdirectory(test)


# Include projects from this directory
#include(CMakeLists.bamread.lib.txt)

# Recurse subdirectories
#add_subdirectory(test )
