#################################
# $Id: Makefile.test_checksum.app 573332 2018-10-25 17:26:22Z ivanov $

APP = test_checksum
SRC = test_checksum
LIB = xutil xncbi

CHECK_COPY = test_data

CHECK_CMD = test_checksum -selftest /CHECK_NAME=test_checksum

WATCHERS = vasilche ivanov
