#############################################################################
# $Id: CMakeLists.txt 564553 2018-05-30 14:04:12Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_add_app(test_ctransition test_ctransition_nlmzip)

if(OFF)
include(CMakeLists.test_ctransition.app.txt)
endif()
