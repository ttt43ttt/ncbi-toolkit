#############################################################################
# $Id: CMakeLists.txt 566834 2018-07-09 12:39:41Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_add_subdirectory(blast_format blastdb seqdb_reader api)

# Include projects from this directory

# Recurse subdirectories
#add_subdirectory(blast_format)
#add_subdirectory(blastdb)
#add_subdirectory(seqdb_reader)
#add_subdirectory(api)
