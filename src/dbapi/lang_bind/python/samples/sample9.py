#! /usr/bin/env python
 
# $Id: sample9.py 488869 2016-01-06 16:27:57Z ucko $
# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================
#
# File Name: sample9.py
#
# Author: Sergey Sikorskiy
#
# ===========================================================================

import os, sys, python_ncbi_dbapi

if os.environ['SYBASE'] == None :
        os.environ['SYBASE'] = "/netopt/Sybase/clients/current/"

pswd_file = open( './.dblogin', 'r' )
db_params_line = pswd_file.readline().strip()
db_params = db_params_line.split(':')
db_name = db_params[0]

conn = python_ncbi_dbapi.connect( 'ftds', 'MSSQL', db_name, db_params[1], db_params[2], db_params[3] )

cursor = conn.cursor()

if len( sys.argv ) < 2 :
        print("Please, provide Taxid")
        sys.exit( 1 )

taxid = sys.argv[1]

sql = "select * from Taxonomy where taxid = " + taxid

cursor.execute( sql )

for record in cursor.fetchall() :
        print(record)

print("SQL: %s\ntotal_row: %d" % (sql, cursor.rowcount))

if cursor.rowcount == 0 :
        print("No data existed in database %s.Taxonomy table with taxid = %s"
              % (db_name, taxid))
