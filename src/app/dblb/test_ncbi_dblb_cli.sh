#! /bin/sh
# $Id: test_ncbi_dblb_cli.sh 454575 2014-12-16 18:12:59Z ucko $

# Fail immediately if any command fails.
set -e

# Run this unconditionally on all platforms
$CHECK_EXEC ncbi_dblb_cli lookup -service MAPVIEW

# Run more tests on linux hosts
case $CHECK_SIGNATURE in
    *linux* )
        $CHECK_EXEC test_ncbi_dblb_cli.py
        ;;
esac
