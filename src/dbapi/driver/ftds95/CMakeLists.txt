#############################################################################
# $Id: CMakeLists.txt 566367 2018-06-28 12:41:46Z gouriano $
#############################################################################

NCBI_add_include_directories(${FTDS95_INCLUDE})
NCBI_add_subdirectory(freetds ctlib)

if(OFF)
# 
#

add_subdirectory(freetds)

if (SYBASE_FOUND)
  add_subdirectory(ctlib)
endif (SYBASE_FOUND)
endif()
