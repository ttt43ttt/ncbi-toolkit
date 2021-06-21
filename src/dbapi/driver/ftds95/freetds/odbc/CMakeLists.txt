#############################################################################
# $Id: CMakeLists.txt 574544 2018-11-14 21:12:24Z dicuccio $
#############################################################################

NCBI_requires(ODBC)
NCBI_add_library(odbc95)
NCBI_add_subdirectory(unittests)

# Include projects from this directory
#include(CMakeLists.odbc95.lib.txt)

# Recurse subdirectories
#add_subdirectory(unittests)
