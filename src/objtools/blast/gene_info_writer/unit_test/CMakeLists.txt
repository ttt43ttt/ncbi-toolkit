#############################################################################
# $Id: CMakeLists.txt 566390 2018-06-28 17:11:39Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_requires(Boost.Test.Included)
NCBI_add_app(gene_info_writer_unit_test)


# Include projects from this directory
#include(CMakeLists.gene_info_writer_unit_test.app.txt)

