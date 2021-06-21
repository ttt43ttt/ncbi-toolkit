#############################################################################
# $Id: CMakeLists.txt 566367 2018-06-28 12:41:46Z gouriano $
#############################################################################


NCBI_add_subdirectory(python)

if(OFF)
# Include projects from this directory
if (${PYTHON_FOUND})
    add_subdirectory(python)
endif()
endif()
