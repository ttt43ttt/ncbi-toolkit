#! /bin/sh
# $Id: test_ncbi_http_upload.sh 534927 2017-05-03 20:07:51Z lavr $

. ./ncbi_test_data

TEST_NCBI_HTTP_UPLOAD_TOKEN="$NCBI_TEST_DATA/http/test_ncbi_http_upload_token"
export TEST_NCBI_HTTP_UPLOAD_TOKEN

if [ ! -f "$TEST_NCBI_HTTP_UPLOAD_TOKEN" ]; then
  echo "NCBI_UNITTEST_SKIPPED"
  exit 0
fi

: ${CONN_DEBUG_PRINTOUT:=SOME};  export CONN_DEBUG_PRINTOUT

$CHECK_EXEC test_ncbi_http_upload $@
