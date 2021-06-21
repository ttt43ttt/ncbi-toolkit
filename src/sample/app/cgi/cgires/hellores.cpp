/*  $Id: hellores.cpp 103491 2007-05-04 17:18:18Z kazimird $
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
 * Authors:  Lewis Geer, Vsevolod Sandomirskiy, etc.
 *
 * File Description:  Defines the resource class, whose main purpose is to 
 *   aggregate (or hold) classes used to execute the program.  Classes
 *   aggregated include:
 *
 *   CNcbiRegistry: opens the *.ini file and reads the entries contained
 *       within.
 *
 *   CNcbiCommand: classes executed when particular CGI parameters are
 *       received.
 *
 */

#include <ncbi_pch.hpp>
#include "hellores.hpp"
#include "hellocmd.hpp"

#include <cgi/cgictx.hpp>
#include <corelib/ncbiutil.hpp>

BEGIN_NCBI_SCOPE

//
// class CHelloResource 
//

CHelloResource::CHelloResource( CNcbiRegistry& config )
    : CNcbiResource( config )
{}
    
CHelloResource::~CHelloResource()
{}

CNcbiCommand* CHelloResource::GetDefaultCommand( void ) const
{
    return new CHelloBasicCommand( const_cast<CHelloResource&>( *this ) );
    
    // CHelloBasicCommand is defined in hellocmd.hpp
}

END_NCBI_SCOPE
