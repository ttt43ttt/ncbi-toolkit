#############################################################################
# $Id: CMakeLists.txt 568619 2018-08-08 15:57:35Z dicuccio $
#############################################################################
# 
#

NCBI_project_tags(core)
NCBI_add_library(xregexp xregexp_template_tester)

if (OFF)
# Include projects from this directory
include(CMakeLists.xregexp.lib.txt)
include(CMakeLists.xregexp_template_tester.lib.txt)
endif()
