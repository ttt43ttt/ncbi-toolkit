#############################################################################
# $Id: CMakeLists.txt 573635 2018-10-30 15:55:37Z ivanov $
#############################################################################

NCBI_add_library(odbc100)
NCBI_add_subdirectory(unittests)

# Include projects from this directory
#include(CMakeLists.odbc100.lib.txt)

# Recurse subdirectories
#add_subdirectory(unittests)
