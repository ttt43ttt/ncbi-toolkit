#############################################################################
# $Id: CMakeLists.txt 564101 2018-05-21 15:34:07Z gouriano $
#############################################################################
# 
#
NCBI_project_tags(test)
NCBI_add_library(we_cpp)
NCBI_add_app(test_serial)

# Include projects from this directory
#include(CMakeLists.test_serial.app.txt)
#include(CMakeLists.we_cpp.asn.txt)

