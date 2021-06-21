#############################################################################
# $Id: CMakeLists.txt 564454 2018-05-29 13:35:44Z gouriano $
#############################################################################

NCBI_add_library(id1 id1cli)
NCBI_add_subdirectory(test)

# Include projects from this directory
#include(CMakeLists.id1cli.lib.txt)
#include(CMakeLists.id1.asn.txt)

# Recurse subdirectories
#add_subdirectory(test )
