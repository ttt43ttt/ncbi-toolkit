#############################################################################
# $Id: CMakeLists.txt 566367 2018-06-28 12:41:46Z gouriano $
#############################################################################


NCBI_add_app(ctl_sp_who_ftds95 ctl_lang_ftds95)

if(OFF)
add_definitions(-DFTDS_IN_USE)
include_directories(SYSTEM ${includedir}/dbapi/driver/ftds95)

# Include projects from this directory
include(CMakeLists.ctl_sp_who_ftds95.app.txt)
include(CMakeLists.ctl_lang_ftds95.app.txt)
endif()
