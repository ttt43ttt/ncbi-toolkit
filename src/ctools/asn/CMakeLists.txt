#############################################################################
# $Id: CMakeLists.txt 564553 2018-05-30 14:04:12Z gouriano $
#############################################################################

NCBI_requires(NCBI_C)
NCBI_add_library(asn)

if(OFF)
# 
#

# Include projects from this directory
include(CMakeLists.asn.lib.txt)
endif()
