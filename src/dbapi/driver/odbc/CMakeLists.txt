#############################################################################
# $Id: CMakeLists.txt 566367 2018-06-28 12:41:46Z gouriano $
#############################################################################

NCBI_requires(ODBC)
NCBI_add_library(ncbi_xdbapi_odbc)
NCBI_add_subdirectory(samples)

# Include projects from this directory
#include(CMakeLists.ncbi_xdbapi_odbc.lib.txt)

# Recurse subdirectories
#add_subdirectory(samples)
