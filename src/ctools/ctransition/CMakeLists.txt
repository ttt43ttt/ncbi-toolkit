#############################################################################
# $Id: CMakeLists.txt 564553 2018-05-30 14:04:12Z gouriano $
#############################################################################

NCBI_add_library(ctransition ctransition_nlmzip)
NCBI_add_subdirectory(test)

if(OFF)
include(CMakeLists.ctransition.lib.txt)

add_subdirectory(test)
endif()
