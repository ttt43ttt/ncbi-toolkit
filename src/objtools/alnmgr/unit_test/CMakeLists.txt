#############################################################################
# $Id: CMakeLists.txt 566390 2018-06-28 17:11:39Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_requires(Boost.Test.Included)
NCBI_add_app(unit_test_alnmgr unit_test_score_builder)


# Include projects from this directory
#include(CMakeLists.unit_test_alnmgr.app.txt)
#include(CMakeLists.unit_test_score_builder.app.txt)

# Recurse subdirectories
