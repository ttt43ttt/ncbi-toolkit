# $Id: Makefile.test_ncbi_clog_templates.app 467289 2015-05-12 16:23:41Z ivanov $

APP = test_ncbi_clog_templates
SRC = test_ncbi_clog_templates

LIB = xregexp_template_tester xregexp $(PCRE_LIB) xncbi clog
LIBS = $(PCRE_LIBS) $(ORIG_LIBS)
//CPPFLAGS = $(PCRE_INCLUDE) $(ORIG_CPPFLAGS)

CHECK_CMD =
CHECK_COPY = clog-test-templates

WATCHERS = ivanov
