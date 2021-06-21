#############################################################################
# $Id: CMakeLists.txt 566390 2018-06-28 17:11:39Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_requires(Boost.Test.Included)
NCBI_add_app(writedb_unit_test writedb_lmdb_unit_test)

# Include projects from this directory
#include(CMakeLists.writedb_unit_test.app.txt)

