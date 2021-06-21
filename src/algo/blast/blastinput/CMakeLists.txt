#############################################################################
# $Id: CMakeLists.txt 566834 2018-07-09 12:39:41Z gouriano $
#############################################################################

NCBI_add_library(blastinput)
NCBI_add_subdirectory(unit_test demo)

# Include projects from this directory
#include(CMakeLists.blastinput.lib.txt)

# Recurse subdirectories
#add_subdirectory(unit_test )
#add_subdirectory(demo )
