#############################################################################
# $Id: CMakeLists.txt 564553 2018-05-30 14:04:12Z gouriano $
#############################################################################

NCBI_project_tags(demo)
NCBI_add_app(demo_html demo_html_template)

if(OFF)
# 
#

# Include projects from this directory
include(CMakeLists.demo_html.app.txt)
include(CMakeLists.demo_html_template.app.txt)
endif()
