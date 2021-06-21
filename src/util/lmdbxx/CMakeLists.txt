#############################################################################
# $Id: CMakeLists.txt 564072 2018-05-21 13:16:53Z gouriano $
#############################################################################

NCBI_requires(LMDB)
NCBI_add_app(lmdbxx_sample)

# Include projects from this directory
# temporaty disabled until CXX-9344 is done
#include(CMakeLists.lmdbxx_sample.app.txt)
