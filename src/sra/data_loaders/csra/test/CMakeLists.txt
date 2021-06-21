#############################################################################
# $Id: CMakeLists.txt 566851 2018-07-09 15:25:50Z gouriano $
#############################################################################

NCBI_add_app(test_csra_loader test_csra_loader_mt test_sra)

# Include projects from this directory
#include(CMakeLists.test_sra.app.txt)
#include(CMakeLists.test_csra_loader.app.txt)
#include(CMakeLists.test_csra_loader_mt.app.txt)

