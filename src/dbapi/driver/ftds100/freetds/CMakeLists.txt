#############################################################################
# $Id: CMakeLists.txt 566367 2018-06-28 12:41:46Z gouriano $
#############################################################################

NCBI_disable_pch()
NCBI_add_subdirectory(
  tds ctlib dblib odbc #replacements
)

#add_subdirectory(ctlib)
#add_subdirectory(dblib)
#add_subdirectory(odbc)
#add_subdirectory(replacements)
#add_subdirectory(tds)
