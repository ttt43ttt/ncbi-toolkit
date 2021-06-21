#############################################################################
# $Id: CMakeLists.txt 564454 2018-05-29 13:35:44Z gouriano $
#############################################################################

NCBI_add_library(ncbimime)
NCBI_add_subdirectory(test)

# Include projects from this directory
#include(CMakeLists.ncbimime.asn.txt)

# Recurse subdirectories
#add_subdirectory(test )
