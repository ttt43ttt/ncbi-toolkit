#############################################################################
# $Id: CMakeLists.txt 566834 2018-07-09 12:39:41Z gouriano $
#############################################################################

NCBI_requires(Boost.Test.Included)
NCBI_project_tags(test)
NCBI_add_app(blastinput_unit_test)

# Include projects from this directory
#include(CMakeLists.blastinput_unit_test.app.txt)

