#############################################################################
# $Id: CMakeLists.txt 566834 2018-07-09 12:39:41Z gouriano $
#############################################################################

NCBI_add_library(gnomon)
NCBI_add_subdirectory(demo test)

# Include projects from this directory
#include(CMakeLists.gnomon.asn.txt)

# Recurse subdirectories
#add_subdirectory(demo )
#add_subdirectory(test )
