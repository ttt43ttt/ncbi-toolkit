#############################################################################
# $Id: CMakeLists.txt 564454 2018-05-29 13:35:44Z gouriano $
#############################################################################

NCBI_add_library(seqset)
NCBI_add_subdirectory(test)

# Include projects from this directory
#include(CMakeLists.seqset.asn.txt)

# Recurse subdirectories
#add_subdirectory(test )
