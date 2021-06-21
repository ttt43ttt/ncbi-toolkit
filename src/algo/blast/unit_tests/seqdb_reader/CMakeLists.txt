#############################################################################
# $Id: CMakeLists.txt 566834 2018-07-09 12:39:41Z gouriano $
#############################################################################

NCBI_requires(Boost.Test.Included)
NCBI_add_app(seqdb_unit_test seqdb_lmdb_unit_test)


# Include projects from this directory
#include(CMakeLists.seqdb_unit_test.app.txt)

