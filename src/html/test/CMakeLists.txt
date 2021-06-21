#############################################################################
# $Id: CMakeLists.txt 564553 2018-05-30 14:04:12Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_add_app(test_html)

if(OFF)
# 
#

# Include projects from this directory
include(CMakeLists.test_html.app.txt)
endif()
