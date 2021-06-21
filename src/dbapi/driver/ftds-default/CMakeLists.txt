#############################################################################
# $Id: CMakeLists.txt 566367 2018-06-28 12:41:46Z gouriano $
#############################################################################


NCBI_requires(FreeTDS)
NCBI_add_library(ncbi_xdbapi_ftds)

# Include projects from this directory
#include(CMakeLists.ncbi_xdbapi_ftds.lib.txt)
