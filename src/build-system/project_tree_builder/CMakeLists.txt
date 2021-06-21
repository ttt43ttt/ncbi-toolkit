#############################################################################
# $Id: CMakeLists.txt 564109 2018-05-21 17:19:24Z gouriano $
#############################################################################
#

NCBI_add_app(project_tree_builder)
NCBI_add_subdirectory(msbuild)

# Include projects from this directory
#include(CMakeLists.project_tree_builder.app.txt)

# Recurse subdirectories
#add_subdirectory(msbuild)
