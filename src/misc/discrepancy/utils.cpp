/*  $Id: utils.cpp 545605 2017-09-07 19:36:37Z kachalos $
 * =========================================================================
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
 * =========================================================================
 *
 * Authors: Sema Kachalo
 *
 */

#include <ncbi_pch.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/seq_macros.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include "utils.hpp"

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(NDiscrepancy)
USING_SCOPE(objects);


bool IsAllCaps(const string& str)
{
    string up_str = str;
    up_str = NStr::ToUpper(up_str);
    if (up_str == str) return true;
    else return false;
}


bool IsAllLowerCase(const string& str)
{
    string low_str = str;
    low_str = NStr::ToLower(low_str);
    if (low_str == str) return true;
    else return false;
}


bool IsAllPunctuation(const string& str)
{
    for (unsigned i=0; i< str.size(); i++) {
        if (!ispunct(str[i])) return false;
    }
    return true;
}


bool IsWholeWord(const string& str, const string& phrase)
{
    if (str == phrase) return true;
    size_t pos = str.find(phrase);
    if (string::npos == pos) return false;
    size_t pos2 = pos+phrase.size();
    if ((!pos || !isalnum(str[pos-1])) && ((str.size() == pos2) || !isalnum(str[pos2]))) return true;
    else return false;
}


string GetTwoFieldSubfield(const string& str, unsigned subfield)
{
    string strtmp;
    if (str.empty() || subfield > 2)  return "";
    if (!subfield) return str;
    else {
        size_t pos = str.find(':');
        if (pos == string::npos) {
            if (subfield == 1) return str;
            else return kEmptyStr;
        }
        else {
            if (subfield == 1) return str.substr(0, pos);
            else {
                strtmp = CTempString(str).substr(pos+1).empty();
                if (!strtmp.empty()) return strtmp;
                else return "";
            }
        }
    }
}


static bool IsProdBiomol(int biomol)
{
    if (biomol == objects::CMolInfo::eBiomol_pre_RNA || 
        biomol == objects::CMolInfo::eBiomol_mRNA  || 
        biomol == objects::CMolInfo::eBiomol_rRNA  || 
        biomol == objects::CMolInfo::eBiomol_tRNA  || 
        biomol == objects::CMolInfo::eBiomol_ncRNA )
        return true;
    return false;
}


static bool x_IsGenProdSet(CConstRef<objects::CBioseq_set> bioseq_set)
{
    return (bioseq_set && bioseq_set->IsSetClass() && bioseq_set->GetClass() == objects::CBioseq_set::eClass_gen_prod_set);
}


bool IsmRNASequenceInGenProdSet(CConstRef<objects::CBioseq> bioseq, const vector<CConstRef<CBioseq_set> > &bioseq_set_stack)
{
    bool res = false;
    bool prod_mol = false;

    if (bioseq && bioseq->IsSetInst() && bioseq->GetInst().IsSetMol() && bioseq->GetInst().GetMol() == objects::CSeq_inst::eMol_rna)
    {
        FOR_EACH_SEQDESC_ON_BIOSEQ(desc,*bioseq)
        {
            if ((*desc)->IsMolinfo() && (*desc)->GetMolinfo().IsSetBiomol())
            {
                prod_mol = IsProdBiomol((*desc)->GetMolinfo().GetBiomol());
            }
        }
    }

    if (!bioseq_set_stack.empty() && prod_mol)
    {
        CConstRef<objects::CBioseq_set> bioseq_set = bioseq_set_stack.back();
        res = x_IsGenProdSet(bioseq_set);
        if (!res && bioseq_set_stack.size() > 1 && bioseq_set && bioseq_set->IsSetClass() && bioseq_set->GetClass() == objects::CBioseq_set::eClass_nuc_prot)
        {
            bioseq_set = bioseq_set_stack[bioseq_set_stack.size() - 2];
            res = x_IsGenProdSet(bioseq_set);
        }
    }
    return res;
}


void AddComment(CSeq_feat& feat, const string& comment)
{
    if (feat.IsSetComment() && !NStr::IsBlank(feat.GetComment()) && !NStr::EndsWith(feat.GetComment(), ";")) {
        feat.SetComment() += "; ";
    }
    feat.SetComment() += comment;
}


static string GetProductName(const CProt_ref& prot)
{
    return prot.IsSetName() && prot.GetName().size() > 0 ? prot.GetName().front() : "";
}


string GetProductName(const CSeq_feat& feat, objects::CScope& scope)
{
    if (feat.IsSetProduct()) {
        CBioseq_Handle prot_bsq = sequence::GetBioseqFromSeqLoc(feat.GetProduct(), scope);
        if (prot_bsq) {
            CFeat_CI prot_ci(prot_bsq, CSeqFeatData::e_Prot);
            if (prot_ci) {
                return GetProductName(prot_ci->GetOriginalFeature().GetData().GetProt());
            }
        }
    }
    else if (feat.IsSetXref()) {
        ITERATE (CSeq_feat::TXref, it, feat.GetXref()) {
            if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                return GetProductName((*it)->GetData().GetProt());
            }
        }
    }
    return kEmptyStr;
}


END_SCOPE(NDiscrepancy)
END_NCBI_SCOPE
