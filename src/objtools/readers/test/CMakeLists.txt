#############################################################################
# $Id: CMakeLists.txt 566390 2018-06-28 17:11:39Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_add_app(
  agp_count pacc test_source_mod_parser agp_val_test
  test_fasta_round_trip
)

# Include projects from this directory
#include(CMakeLists.agp_count.app.txt)
#include(CMakeLists.pacc.app.txt)
#include(CMakeLists.test_source_mod_parser.app.txt)
#include(CMakeLists.agp_val_test.app.txt)
#include(CMakeLists.test_fasta_round_trip.app.txt)

