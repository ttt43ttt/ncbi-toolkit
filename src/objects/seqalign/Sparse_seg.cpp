/* $Id: Sparse_seg.cpp 513588 2016-09-13 13:46:29Z ivanov $
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
 * Author:  Kamen Todorov
 *
 * File Description:
 *   Sparse-seg
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using the following specifications:
 *   'seqalign.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>

// generated includes
#include <objects/seqalign/Sparse_seg.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CSparse_seg::~CSparse_seg(void)
{
}


void CSparse_seg::Validate(bool full_test) const
{
    ITERATE(TRows, aln_it, GetRows()) {
        (*aln_it)->Validate(full_test);
    }
}


CSparse_seg::TDim CSparse_seg::CheckNumRows(void) const
{
    size_t dim = GetRows().size();
    _SEQALIGN_ASSERT(IsSetRow_scores() ? GetRow_scores().size() == dim : true);
    _SEQALIGN_ASSERT(dim < kMax_Int);
    return TDim(dim+1); // extra 1 is for the consensus sequence (first-id)
}


const CSeq_id& CSparse_seg::GetSeq_id(TDim row) const
{
    if ( row == 0 ) {
        // consensus sequence - first-id
        if ( !GetRows().empty() ) {
            const CSparse_align& align = *GetRows()[0];
            return align.GetFirst_id();
        }
    }
    else {
        if ( size_t(row) <= GetRows().size() ) {
            const CSparse_align& align = *GetRows()[row-1];
            return align.GetSecond_id();
        }
    }
    NCBI_THROW(CSeqalignException, eInvalidRowNumber,
               "CSparse_seg::GetSeq_id(): "
               "can not get seq-id for the row requested.");
}


ENa_strand CSparse_seg::GetSeqStrand(TDim row) const
{
    if ( row == 0 ) {
        // consensus sequence - always plus
        return eNa_strand_plus;
    }
    else {
        if ( size_t(row) <= GetRows().size() ) {
            const CSparse_align& align = *GetRows()[row-1];
            if ( align.IsSetSecond_strands() ) {
                // return strand for the first 
                return align.GetSecond_strands()[0];
            }
            else {
                // unset strand - plus
                return eNa_strand_plus;
            }
        }
    }
    NCBI_THROW(CSeqalignException, eInvalidRowNumber,
               "CSparse_seg::GetSeqStrand(): "
               "can not get strand for the row requested.");
}


TSeqPos CSparse_seg::GetSeqStart(TDim row) const
{
    if ( row == 0 ) {
        // consensus sequence
        bool first_row = true;
        TSeqPos total_start = 0;
        ITERATE ( TRows, it, GetRows() ) {
            const CSparse_align& align = **it;
            TSeqPos start = align.GetFirst_starts().front();
            if ( first_row ) {
                first_row = false;
                total_start = start;
            }
            else if ( start < total_start ) {
                total_start = start;
            }
        }
        return total_start;
    }
    else {
        if ( size_t(row) <= GetRows().size() ) {
            const CSparse_align& align = *GetRows()[row-1];
            if ( !align.IsSetSecond_strands() ||
                 IsForward(align.GetSecond_strands()[0]) ) {
                // plus strand - first segment
                return align.GetSecond_starts().front();
            }
            else {
                // minus strand - last segment
                return align.GetSecond_starts().back();
            }
        }
    }
    NCBI_THROW(CSeqalignException, eInvalidRowNumber,
               "CSparse_seg::GetSeqStart(): "
               "can not get seq start for the row requested.");
}


TSeqPos CSparse_seg::GetSeqStop(TDim row) const
{
    if ( row == 0 ) {
        // consensus sequence
        bool first_row = true;
        TSeqPos total_stop = 0;
        ITERATE ( TRows, it, GetRows() ) {
            const CSparse_align& align = **it;
            TSeqPos stop = align.GetFirst_starts().back();
            stop += align.GetLens().back() - 1;
            if ( first_row ) {
                first_row = false;
                total_stop = stop;
            }
            else if ( stop > total_stop ) {
                total_stop = stop;
            }
        }
        return total_stop;
    }
    else {
        if ( size_t(row) <= GetRows().size() ) {
            const CSparse_align& align = *GetRows()[row-1];
            if ( !align.IsSetSecond_strands() ||
                 IsForward(align.GetSecond_strands()[0]) ) {
                // plus strand - last segment
                TSeqPos stop = align.GetSecond_starts().back();
                stop += align.GetLens().back() - 1;
                return stop;
            }
            else {
                // minus strand - first segment
                TSeqPos stop = align.GetSecond_starts().front();
                stop += align.GetLens().front() - 1;
                return stop;
            }
        }
    }
    NCBI_THROW(CSeqalignException, eInvalidRowNumber,
               "CSparse_seg::GetSeqStop(): "
               "can not get seq stop for the row requested.");
}


CRange<TSeqPos> CSparse_seg::GetSeqRange(TDim row) const
{
    if ( row == 0 ) {
        // consensus sequence
        bool first_row = true;
        TSeqPos total_start = 0, total_stop = 0;
        ITERATE ( TRows, it, GetRows() ) {
            const CSparse_align& align = **it;
            TSeqPos start = align.GetFirst_starts().front();
            TSeqPos stop = align.GetFirst_starts().back();
            stop += align.GetLens().back() - 1;
            if ( first_row ) {
                first_row = false;
                total_start = start;
                total_stop = stop;
            }
            else {
                if ( start < total_start ) {
                    total_start = start;
                }
                if ( stop > total_stop ) {
                    total_stop = stop;
                }
            }
        }
        return CRange<TSeqPos>(total_start, total_stop);
    }
    else {
        if ( size_t(row) <= GetRows().size() ) {
            const CSparse_align& align = *GetRows()[row-1];
            if ( !align.IsSetSecond_strands() ||
                 IsForward(align.GetSecond_strands()[0]) ) {
                // plus strand - (first,last) segments
                TSeqPos start = align.GetSecond_starts().front();
                TSeqPos stop = align.GetSecond_starts().back();
                stop += align.GetLens().back() - 1;
                return CRange<TSeqPos>(start, stop);
            }
            else {
                // minus strand - (last,first) segments
                TSeqPos start = align.GetSecond_starts().back();
                TSeqPos stop = align.GetSecond_starts().front();
                stop += align.GetLens().front() - 1;
                return CRange<TSeqPos>(start, stop);
            }
        }
    }
    NCBI_THROW(CSeqalignException, eInvalidRowNumber,
               "CSparse_seg::GetSeqRange(): "
               "can not get seq range for the row requested.");
}


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE
