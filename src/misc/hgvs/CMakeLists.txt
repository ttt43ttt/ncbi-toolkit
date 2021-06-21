#############################################################################
# $Id: CMakeLists.txt 566386 2018-06-28 16:53:21Z gouriano $
#############################################################################


NCBI_requires(Boost.Spirit)
NCBI_add_definitions(BOOST_ERROR_CODE_HEADER_ONLY BOOST_SYSTEM_NO_DEPRECATED)
NCBI_add_library(hgvs_parser)
NCBI_add_subdirectory(test)

# 
#

# Include projects from this directory
#include(CMakeLists.hgvs_parser.lib.txt)

# Recurse subdirectories
#add_subdirectory(test )
