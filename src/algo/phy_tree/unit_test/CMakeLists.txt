#############################################################################
# $Id: CMakeLists.txt 566834 2018-07-09 12:39:41Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_requires(Boost.Test.Included)
NCBI_add_app(phytree_calc_unit_test)


# Include projects from this directory
#include(CMakeLists.phytree_calc_unit_test.app.txt)

