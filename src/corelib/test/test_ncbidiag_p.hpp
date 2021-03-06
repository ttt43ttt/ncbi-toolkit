#ifndef CORELIB___TESTNCBIDIAG_P__HPP
#define CORELIB___TESTNCBIDIAG_P__HPP

/*  $Id: test_ncbidiag_p.hpp 505891 2016-06-29 17:58:41Z gouriano $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Author:  Vyacheslav Kononenko
 *
 *
 */

/// @file ncbidiag_p.hpp
///
///   Test ncbidiag macros DIAG_COMPILE_INFO
///
///   More elaborate documentation could be found in:
///     http://ncbi.github.io/cxx-toolkit/pages/ch_log

#include <corelib/ncbidiag.hpp>

BEGIN_NCBI_SCOPE


inline
CNcbiDiag *MakeDiagInHeader()
{
    return new CNcbiDiag(DIAG_COMPILE_INFO);
}


END_NCBI_SCOPE

#endif  /* CORELIB___TESTNCBIDIAG_P__HPP */
