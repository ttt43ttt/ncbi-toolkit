#############################################################################
# $Id: CMakeLists.txt 564454 2018-05-29 13:35:44Z gouriano $
#############################################################################

NCBI_add_library(seq)
NCBI_add_subdirectory(test unit_test)

# Include projects from this directory
#include(CMakeLists.seq.asn.txt)

# Recurse subdirectories
#add_subdirectory(test )
#add_subdirectory(unit_test )
