#############################################################################
# $Id: CMakeLists.txt 564553 2018-05-30 14:04:12Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_add_app(cgitest test_multipart_cgi cgi_io_test test_cgi_entry_reader test_user_agent)

if(OFF)
##############################################################################
# 
#

# Include projects from this directory
include(CMakeLists.cgitest.app.txt)
include(CMakeLists.test_multipart_cgi.app.txt)
include(CMakeLists.cgi_io_test.app.txt)
include(CMakeLists.test_cgi_entry_reader.app.txt)
include(CMakeLists.test_user_agent.app.txt)
endif()
