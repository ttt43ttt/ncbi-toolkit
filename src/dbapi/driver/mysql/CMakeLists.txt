#############################################################################
# $Id: CMakeLists.txt 566367 2018-06-28 12:41:46Z gouriano $
#############################################################################

NCBI_requires(MySQL)
NCBI_add_library(ncbi_xdbapi_mysql)
NCBI_add_subdirectory(samples)

# Include projects from this directory
#include(CMakeLists.ncbi_xdbapi_mysql.lib.txt)

# Recurse subdirectories
#add_subdirectory(samples)
