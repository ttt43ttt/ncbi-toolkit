#############################################################################
# $Id: CMakeLists.txt 565539 2018-06-13 16:19:50Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_add_app(hgvs2variation test_hgvs_parser test_hgvs_reader)
# 
#

# Include projects from this directory
#include(CMakeLists.hgvs2variation.app.txt)
#include(CMakeLists.test_hgvs_parser.app.txt)
#include(CMakeLists.test_hgvs_reader.app.txt)

