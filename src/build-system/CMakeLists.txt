#############################################################################
# $Id: CMakeLists.txt 567665 2018-07-23 12:58:02Z gouriano $
#############################################################################
# 
#

# Include projects from this directory

NCBI_add_subdirectory(project_tree_builder)

if(OFF)
# Recurse subdirectories
add_subdirectory(project_tree_builder)
#add_subdirectory(helpers)
endif()
