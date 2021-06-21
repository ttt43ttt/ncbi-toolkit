#############################################################################
# $Id: CMakeLists.txt 566390 2018-06-28 17:11:39Z gouriano $
#############################################################################

NCBI_add_library(gene_info)
NCBI_add_subdirectory(demo unit_test)

# Include projects from this directory
#include(CMakeLists.gene_info.lib.txt)

# Recurse subdirectories
#add_subdirectory(demo )
#add_subdirectory(unit_test )
