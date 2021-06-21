#############################################################################
# $Id: CMakeLists.txt 573635 2018-10-30 15:55:37Z ivanov $
#############################################################################


NCBI_add_app(ctl_sp_who_ftds100 ctl_lang_ftds100)

if(OFF)

add_definitions(-DFTDS_IN_USE)
include_directories(SYSTEM ${includedir}/dbapi/driver/ftds100)

# Include projects from this directory
include(CMakeLists.ctl_sp_who_ftds100.app.txt)
include(CMakeLists.ctl_lang_ftds100.app.txt)
endif()
