# $Id: Makefile.cn3d.app 553799 2017-12-21 17:12:21Z fongah2 $
# Author:  Paul Thiessen

# Build application "cn3d"
#################################

WATCHERS = thiessen lanczyck

REQUIRES = objects wxWidgets OpenGL PNG

APP = cn3d

SRC =	aaa_dummy_pch \
	alignment_manager \
	alignment_set \
	animation_controls \
	annotate_dialog \
	atom_set \
	block_multiple_alignment \
	bond \
	cdd_annot_dialog \
	cdd_book_ref_dialog \
	cdd_ibis_annot_dialog \
	cdd_ref_dialog \
	cdd_splash_dialog \
	chemical_graph \
	cn3d_app \
	cn3d_ba_interface \
	cn3d_blast \
	cn3d_cache \
	cn3d_colors \
	cn3d_glcanvas \
	cn3d_png \
	cn3d_pssm \
	cn3d_threader \
	cn3d_tools \
	command_processor \
	conservation_colorer \
	coord_set \
	data_manager \
	dist_select_dialog \
	file_messaging \
	messenger \
	molecule \
	molecule_identifier \
	multitext_dialog \
	object_3d \
	opengl_renderer \
	pattern_dialog \
	periodic_table \
	preferences_dialog \
	progress_meter \
	residue \
	sequence_display \
	sequence_set \
	sequence_viewer \
	sequence_viewer_widget \
	sequence_viewer_window \
	show_hide_dialog \
	show_hide_manager \
	structure_base \
	structure_set \
	structure_window \
	style_dialog \
	style_manager \
	taxonomy_tree \
	update_viewer \
	update_viewer_window \
	vector_math \
	viewer_base \
	viewer_window_base

LIB =   xbma_refiner_gui wx_tools xbma_refiner xcd_utils xstruct_util xstruct_dp xstruct_thread \
        xblast xalgowinmask xalgodustmask xalgoblastdbindex composition_adjustment seqmasks_io seqdb $(OBJREAD_LIBS) \
	xalnmgr tables xobjutil id1cli id1 ncbimime cdd cn3d mmdb blast_services xnetblastcli xnetblast scoremat \
	taxon1 blastdb xregexp entrez2cli xconnect entrez2 $(SOBJMGR_LIBS) $(Z_LIB) $(PCRE_LIB) $(LMDB_LIB)

CPPFLAGS = \
	$(ORIG_CPPFLAGS) \
	$(WXWIDGETS_INCLUDE) \
	$(Z_INCLUDE) $(PNG_INCLUDE) $(PCRE_INCLUDE)

CXXFLAGS = $(FAST_CXXFLAGS)

LDFLAGS = $(FAST_LDFLAGS)

LIBS = \
	$(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) \
	$(Z_LIBS) $(PNG_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) \
	$(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)


# for distribution on linux/gcc, do:
#   gcc  ....  -lpthread -Wl,-Bstatic -lstdc++ -Wl,-Bdynamic