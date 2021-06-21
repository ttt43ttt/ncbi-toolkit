#############################################################################
# $Id: CMakeLists.txt 564454 2018-05-29 13:35:44Z gouriano $
#############################################################################

NCBI_add_library(genome_collection gencoll_client)
NCBI_add_subdirectory(gc_cli)

# Include projects from this directory
#include(CMakeLists.genome_collection.asn.txt)
#include(CMakeLists.gencoll_client.asn.txt)

# Recurse subdirectories
#add_subdirectory(gc_cli )
