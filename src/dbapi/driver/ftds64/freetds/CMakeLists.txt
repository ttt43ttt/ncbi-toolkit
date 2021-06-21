#############################################################################
# $Id: CMakeLists.txt 566367 2018-06-28 12:41:46Z gouriano $
#############################################################################

NCBI_disable_pch()
NCBI_add_subdirectory(
  tds ctlib #dblib odbc
)

# Include projects from this directory

# Recurse subdirectories
#add_subdirectory(tds)
#add_subdirectory(ctlib)
#add_subdirectory(dblib)
#add_subdirectory(odbc)
