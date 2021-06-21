/* $Id: Blast_def_line_set.cpp 565529 2018-06-13 14:44:06Z rackerst $
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
 *   using the following specifications:
 *   'blastdb.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>

#include <corelib/ncbiutil.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/blastdb/defline_extra.hpp>
#include <objects/blastdb/Blast_def_line.hpp>

// generated includes
#include <objects/blastdb/Blast_def_line_set.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CBlast_def_line_set::~CBlast_def_line_set(void)
{
}

/// Compare two deflines for ordering purposes.
///
/// Given a CSeq_id, the ranking function produces an integer based on
/// the type of id involved.  Each defline is searched for the Seq-id
/// for which the ranking function produces the lowest number.  Then
/// the best ranks of each defline are compared.  The defline with the
/// lower rank is considered to come first in the ordering.
///
/// @param d1 A defline.
/// @param d2 Another one.
/// @param rank_func A ranking function.
/// @return true if d1 comes before d2 in the defined ordering.
static
bool s_DeflineCompare(const CRef<CBlast_def_line>& d1,
                      const CRef<CBlast_def_line>& d2,
                      int (*rank_func)(const CRef<CSeq_id>&))
{
    if (d1.Empty() || d2.Empty()) {
        return false;
    }
    
    bool rv = false;
    
    if (d1->CanGetSeqid() && d2->CanGetSeqid()) {
        
        // First, use Seq-id by-type ranking.
        
        CRef<CSeq_id> c1 = FindBestChoice(d1->GetSeqid(), rank_func);
        CRef<CSeq_id> c2 = FindBestChoice(d2->GetSeqid(), rank_func);
        
        int diff = rank_func(c1) - rank_func(c2);
        
        if (diff < 0) {
            return true;
        }
        
        if (diff > 0) {
            return false;
        }

        // Make sure the "N*" RefSeq accessions precede the "X*" accessions.
        if (c1->IsOther() && c2->IsOther()) {
            const string& acc1 = c1->GetOther().GetAccession();
            const string& acc2 = c2->GetOther().GetAccession();
            if (acc1.find("WP") == 0 && acc2.find("NP_") == 0)
		return true;
            else if (acc1.find("WP_") == 0 && acc2.find("YP_") == 0)
		return true;
            else if (acc1.find("YP_") == 0 && acc2.find("WP_") == 0)
		return false;
            else if (acc1.find("NP_") == 0 && acc2.find("WP_") == 0)
		return false;
            else if ((acc1.find("WP_") == 0 || acc1.find("NP_") == 0 || acc1.find("YP_") == 0 || acc1.find("AP_") == 0)
		&& acc2.find("XP_") == 0)
                return true;
            else if ((acc1.find("WP_") == 0 || acc1.find("NP_") == 0 || acc1.find("YP_") == 0)
		&& acc2.find("AP_") == 0)
                return true;
            else if (acc1.find("AP_") == 0 && 
		(acc2.find("NP_") == 0 || acc2.find("YP_") == 0 || acc2.find("WP_") == 0))
                return false;
            else if (acc1.find("YP_") == 0 && acc2.find("AP_") == 0)
                return true;
            else if (acc1.find("XP_") == 0 && 
		(acc2.find("NP_") == 0 || acc2.find("AP_") == 0 || acc2.find("YP_") == 0 || acc2.find("WP") == 0))
                return false;
	    else if (acc1.find("NM_") == 0 && acc2.find("XM_") ==0)
		return true;
	    else if (acc1.find("XM_") == 0 && acc2.find("NM_") ==0)
		return false;

        }

        // Second, use GI numerical ranking (least value first).  I
        // avoid the possibility of circular rankings by ranking GI
        // above non-GI here, although this should not happen unless
        // GIs are given a rank value equal to that of another type.
        // Finally, if no ranking is possible here, a string
        // comparison is done.
        
        const CSeq_id & cs1 = *d1->GetSeqid().front();
        const CSeq_id & cs2 = *d2->GetSeqid().front();
        
        if (cs1.IsGi()) {
            if (cs2.IsGi()) {
                rv = cs1.GetGi() < cs2.GetGi();
            } else {
                rv = true;
            }
        } else {
            if (cs2.IsGi()) {
                rv = false;
            } else {
                // Neither is a GI - this will typically only happen
                // for databases that have no GI.
                
                rv = cs1.AsFastaString() < cs2.AsFastaString();
            }
        }
    }
    
    return rv;
}

/// Ranking function for protein Blast-def-lines
inline
bool s_DeflineCompareAA(const CRef<CBlast_def_line>& d1, 
                        const CRef<CBlast_def_line>& d2)
{
    return s_DeflineCompare(d1, d2, CSeq_id::FastaAARank);
}

/// Ranking function for nucleotide Blast-def-lines
inline
bool s_DeflineCompareNA(const CRef<CBlast_def_line>& d1, 
                        const CRef<CBlast_def_line>& d2)
{
    return s_DeflineCompare(d1, d2, CSeq_id::FastaNARank);
}

void
CBlast_def_line_set::SortBySeqIdRank(bool is_protein)
{
    if (CanGet()) {
        Set().sort(is_protein ? s_DeflineCompareAA : s_DeflineCompareNA);
    }
}

void
CBlast_def_line_set::PutTargetGiFirst(TGi gi)
{
    if (gi <= ZERO_GI) {
        return;
    }

    CRef<CBlast_def_line> first_defline;

    NON_CONST_ITERATE(Tdata, defline, Set()) {
        ITERATE(CBlast_def_line::TSeqid, id, (*defline)->GetSeqid()) {
            if ((*id)->IsGi() && (*id)->GetGi() == gi) {
                first_defline = *defline;
                break;
            }
        }
        if (first_defline) {
            Set().erase(defline); // remove the item from the list...
            break;
        }
    }

    if (first_defline) {
        // ... and put it in the front
        Set().push_front(first_defline);
    }
}

void GetLinkoutTypes(vector<TLinkoutTypeString>& rv)
{
    rv.clear();
    // N.B.: only add those linkout types that are actively supported
    rv.push_back(make_pair(eFromType, string("eFromType")));
    rv.push_back(make_pair(eUnigene, string("eUnigene")));
    rv.push_back(make_pair(eStructure, string("eStructure")));
    rv.push_back(make_pair(eGeo, string("eGeo")));
    rv.push_back(make_pair(eGene, string("eGene")));
    rv.push_back(make_pair(eFromVerifiedMaterial, string("eFromVerifiedMaterial")));
    rv.push_back(make_pair(eMapviewer, string("eMapviewer")));
    rv.push_back(make_pair(eGenomicSeq, string("eGenomicSeq")));
    rv.push_back(make_pair(eBioAssay, string("eBioAssay")));
    rv.push_back(make_pair(eReprMicrobialGenomes, string("eReprMicrobialGenomes")));
    rv.push_back(make_pair(eGenomeDataViewer, string("eGenomeDataViewer")));
    rv.push_back(make_pair(eTranscript, string("eTranscript")));
}

END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 65, chars: 1918, CRC32: 1109a8b4 */
