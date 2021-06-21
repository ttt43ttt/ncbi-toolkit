#############################################################################
# $Id: CMakeLists.txt 564454 2018-05-29 13:35:44Z gouriano $
#############################################################################

NCBI_add_library(macro)
NCBI_add_subdirectory(unit_test)

# Include projects from this directory
#include(CMakeLists.macro.asn.txt)

# Recurse subdirectories
#add_subdirectory(unit_test )
