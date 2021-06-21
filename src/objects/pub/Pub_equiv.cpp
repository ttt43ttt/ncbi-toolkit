/* $Id: Pub_equiv.cpp 507570 2016-07-20 12:15:43Z bollin $
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
 * Author:  .......
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using specifications from the ASN data definition file
 *   'pub.asn'.
 */

// standard includes

// generated includes
#include <ncbi_pch.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CPub_equiv::~CPub_equiv(void)
{
}

// Appends a label to "label" based on content
bool CPub_equiv::GetLabel(string* label, TLabelFlags flags,
                          ELabelVersion version) const
{
    bool found = false;
    const CPub* pubs[5];
    int i;
    for(i = 0; i < 5; i++) {
        pubs[i] = 0;
    }
    i = 1;
    
    // Get five pubs in the set of pubs, giving preference to e_Muid, 
    // e_Pmid, and e_Gen.
    ITERATE (list<CRef<CPub> >, it, Get()) {
        switch ((**it).Which()) {
        case CPub::e_Muid:
            pubs[3] = *it;
            break;
        case CPub::e_Pmid:
            pubs[0] = *it;
            break;
        case CPub::e_Gen:
            if ((**it).GetGen().IsSetSerial_number()) {
                pubs[4] = *it;
                break;
            }
            // otherwise fall through
        default:
            if (i < 5) {
                if (!pubs[i]) {
                    pubs[i] = *it;
                }
                i++;
            }
            break;
        }
    }
    
    bool first = true;
    // Loop thru 5 pubs, ensuring each one exists
    for (i = 0; i < 5; i++) {
        if (!pubs[i]) {
            continue;
        }
        
        if (first) {
            first = false;
        } else {
            *label += " ";
        }
        // Append a label to "label"
        found |= pubs[i]->GetLabel(label, flags, version);
    }

    return found;
}


bool CPub_equiv::SameCitation(const CPub& pub) const
{
    ITERATE(CPub_equiv::Tdata, it1, Get()) {
        if ((*it1)->SameCitation(pub)) {
            return true;
        } else if ((*it1)->Which() == pub.Which()) {
            return false;
        }
    }
    return false;
}


bool CPub_equiv::SameCitation(const CPub_equiv& other) const
{
    ITERATE(CPub_equiv::Tdata, it1, Get()) {
        ITERATE(CPub_equiv::Tdata, it2, other.Get()) {
            if ((*it1)->SameCitation(**it2)) {
                return true;
            } else if ((*it1)->Which() == (*it2)->Which()) {
                return false;
            }
        }
    }
    return false;
}




END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 61, chars: 1880, CRC32: 176db75f */
