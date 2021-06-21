#############################################################################
# $Id: CMakeLists.txt 566367 2018-06-28 12:41:46Z gouriano $
#############################################################################

NCBI_add_include_directories(${FTDS100_INCLUDE})
NCBI_add_subdirectory(freetds ctlib)

#add_subdirectory(freetds)
#add_subdirectory(ctlib)
