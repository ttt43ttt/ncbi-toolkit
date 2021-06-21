#############################################################################
# $Id: CMakeLists.txt 566390 2018-06-28 17:11:39Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_requires(Boost.Test.Included)
NCBI_add_app(blast_services_unit_test)


# Include projects from this directory
#include(CMakeLists.blast_services_unit_test.app.txt)

