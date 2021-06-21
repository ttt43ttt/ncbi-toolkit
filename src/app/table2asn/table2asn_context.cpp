/*  $Id: table2asn_context.cpp 577684 2019-01-08 13:00:57Z ivanov $
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
* Author:  Sergiy Gotvyanskyy, NCBI
*
* File Description:
*   Context structure holding all table2asn parameters
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/feat_ci.hpp>

#include <objmgr/bioseq_ci.hpp>
#include <objects/general/Dbtag.hpp>

#include <objects/submit/Seq_submit.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objtools/readers/source_mod_parser.hpp>
#include <objects/general/Date.hpp>

#include <objtools/readers/message_listener.hpp>
#include <objtools/edit/remote_updater.hpp>

#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objects/general/Int_fuzz.hpp>

#include <objects/submit/Submit_block.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>

#include <objects/submit/Contact_info.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Auth_list.hpp>

#include <objects/seqfeat/Gb_qual.hpp>

#include <objmgr/seqdesc_ci.hpp>

#include <objmgr/scope.hpp>
#include <objtools/edit/dblink_field.hpp>

#include "table2asn_context.hpp"

#include "visitors.hpp"
#include <objects/seqfeat/Feat_id.hpp>

#include <common/test_assert.h>  /* This header must go last */

BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

namespace
{

bool x_ApplyCreateDate(CSeq_entry& entry)
{
    CAutoAddDesc create_date_desc(entry.SetDescr(), CSeqdesc::e_Create_date);
    if (create_date_desc.IsNull())
    {   
        CRef<CDate> date(new CDate(CTime(CTime::eCurrent), CDate::ePrecision_day));
        create_date_desc.Set().SetCreate_date(*date);
        return false; // no need update_date    
    }
    else
        return true; // need update_date
}


void x_CorrectCollectionDates(CTable2AsnContext& context, objects::CBioSource& source)
{
    static CTimeFormat in_formats[2] = { "M-D-Y", "D-M-Y" };
    static CTimeFormat out_format("D-b-Y");

    if (!source.IsSetSubtype())
        return;

    size_t p = CTempString("dD").find_first_of(context.m_cleanup);

    for (auto subtype : source.SetSubtype())
    {
        if (subtype->IsSetSubtype() && subtype->GetSubtype() == CSubSource::eSubtype_collection_date)
        {
            string& col_date = subtype->SetName();
            if (CTime::ValidateString(col_date, in_formats[p]))
            {
                col_date = CTime(col_date, in_formats[p]).AsString(out_format);
            }
        }
    }

}

void x_CorrectCollectionDates(CTable2AsnContext& context, objects::CSeq_annot& annot)
{
    size_t p = context.m_cleanup.find_first_of("Dd");
    if (p == string::npos)
        return;

    if (!annot.IsFtable())
        return;

    for (auto feature : annot.SetData().SetFtable())
    {
        if (feature->IsSetData() && feature->GetData().IsBiosrc())
            x_CorrectCollectionDates(context, feature->SetData().SetBiosrc());
    }
}

template<class _T>
void x_CorrectCollectionDates(CTable2AsnContext& context, _T& seq_or_set)
{
    size_t p = context.m_cleanup.find_first_of("Dd");
    if (p == string::npos)
        return;

    if (seq_or_set.IsSetDescr())
    {
        CRef<CSeqdesc> biosource = CAutoAddDesc::LocateDesc(seq_or_set.SetDescr(), CSeqdesc::e_Source);
        if (biosource.NotEmpty())
            x_CorrectCollectionDates(context, biosource->SetSource());
    }

    if (seq_or_set.IsSetAnnot())
    {
        for (auto annot : seq_or_set.SetAnnot())
            x_CorrectCollectionDates(context, *annot);
    }
}

};


CTable2AsnContext::CTable2AsnContext():
    m_output(0),
    m_delay_genprodset(false),
    m_save_bioseq_set(false),
    m_disc_eucariote(false),
    m_RemoteTaxonomyLookup(false),
    m_RemotePubLookup(false),
    m_HandleAsSet(false),
    m_ClassValue(CBioseq_set::eClass_genbank),
    m_GenomicProductSet(false),
    m_SetIDFromFile(false),
    m_taxid(0),
    m_gapNmin(0),
    m_gap_Unknown_length(0),
    m_minimal_sequence_length(0),
    m_gap_type(-1),
    m_fcs_trim(false),
    m_split_log_files(false),
    m_postprocess_pubs(false),
    m_use_hypothetic_protein(false),
    m_eukariote(false),
    m_di_fasta(false),
    m_allow_accession(false),
    m_verbose(false),
    m_augustus_fix(false),
    m_make_flatfile(false),
    m_discrepancy(false),
    m_logger(0)
{
}

CTable2AsnContext::~CTable2AsnContext()
{
}

void CTable2AsnContext::AddUserTrack(CSeq_descr& SD, const string& type, const string& lbl, const string& data)
{
    if (data.empty())
        return;

    CRef<CUser_field> uf(new CUser_field);
    uf->SetLabel().SetStr(lbl);
    uf->SetNum(1);
    uf->SetData().SetStr(data);
    SetUserObject(SD, type).SetData().push_back(uf);
}

CSeq_descr& CTable2AsnContext::SetBioseqOrParentDescr(CBioseq& bioseq)
{
    if (bioseq.GetParentEntry() && bioseq.GetParentEntry()->GetParentEntry())
    {
        auto entry = bioseq.GetParentEntry()->GetParentEntry();
        if (entry->IsSet() && entry->GetSet().IsSetClass() &&
            entry->GetSet().GetClass() == CBioseq_set::eClass_nuc_prot)
            return entry->SetSet().SetDescr();
    }

    return bioseq.SetDescr();
}

CNcbiOstream & CTable2AsnContext::GetOstream(CTempString suffix, CTempString basename)
{
    auto& rec = m_outputs[suffix];
    if (rec.second.get() == 0)
    {
        if (rec.first.empty())
          rec.first = GenerateOutputFilename(suffix, basename);
        CFile(rec.first.c_str()).Remove(CFile::fIgnoreMissing);
        rec.second.reset(new CNcbiOfstream(rec.first.c_str()));
    }
    return *rec.second.get();
}

string CTable2AsnContext::GenerateOutputFilename(const CTempString& ext, CTempString basename) const
{
    string dir;
    string outputfile;
    string base;

    if (basename.empty())
        basename = m_output_filename;
    if (basename.empty())
        basename = m_current_file;

    CDirEntry::SplitPath(basename, &dir, &base, 0);
    if (basename == "-" || dir == "/dev") {
        CDirEntry::SplitPath(m_current_file, &dir, &base, 0);
        outputfile = m_ResultsDirectory.empty() ? dir : m_ResultsDirectory;
    }
    else {
        outputfile = m_ResultsDirectory.empty() ? dir : m_ResultsDirectory;
    }

    outputfile += base;
    outputfile += ext;

    return outputfile;
}

void CTable2AsnContext::ReleaseOutputs()
{
    m_outputs.clear(); // it will close all output files
}

CUser_object& CTable2AsnContext::SetUserObject(CSeq_descr& descr, const CTempString& type)
{
    CRef<CUser_object> user_obj;
    for (auto desc: descr.Set())
    {
        if (desc->IsUser() && desc->GetUser().IsSetType() &&
            desc->GetUser().GetType().IsStr() &&
            desc->GetUser().GetType().GetStr() == type)
        {
            return desc->SetUser();
        }
    }

    CRef<CObject_id> oi(new CObject_id);
    oi->SetStr(type);
    CRef<CUser_object> uo(new CUser_object);
    uo->SetType(*oi);

    CRef<CSeqdesc> user_desc(new CSeqdesc());
    user_desc->Select(CSeqdesc::e_User);
    user_desc->SetUser(*uo);

    descr.Set().push_back(user_desc);
    return *uo;
}

void CTable2AsnContext::ApplyUpdateDate(objects::CSeq_entry& entry) const
{
    CRef<CDate> date(new CDate(CTime(CTime::eCurrent), CDate::ePrecision_day));
    CAutoAddDesc date_desc(entry.SetDescr(), CSeqdesc::e_Update_date);
    date_desc.Set().SetUpdate_date(*date);
}

void CTable2AsnContext::ApplyAccession(objects::CSeq_entry& entry)
{
    if (m_accession.Empty())
        return;

    VisitAllBioseqs(entry, [this](CBioseq& bioseq)
    {
        CRef<CSeq_id> accession(new CSeq_id);
        accession->Assign(*this->m_accession);
        bioseq.SetId().push_back(accession);
    });
}

void CTable2AsnContext::UpdateSubmitObject(CRef<objects::CSeq_submit>& submit) const
{
    if (!m_HoldUntilPublish.IsEmpty())
    {
        submit->SetSub().SetHup(true);
        CRef<CDate> reldate(new CDate(m_HoldUntilPublish, CDate::ePrecision_day));
        submit->SetSub().SetReldate(*reldate);
    }

    string toolname = "table2asn " + CNcbiApplication::Instance()->GetVersion().Print();
    submit->SetSub().SetSubtype(CSubmit_block::eSubtype_new);
    submit->SetSub().SetTool(toolname);
}

CRef<CSerialObject> CTable2AsnContext::CreateSubmitFromTemplate(CRef<CSeq_entry>& object, CRef<CSeq_submit>& submit) const
{
    if (submit.NotEmpty())
    {
        UpdateSubmitObject(submit);
        return CRef<CSerialObject>(submit);
    }
    else
    if (m_submit_template.NotEmpty())
    {
        submit.Reset(new CSeq_submit);
        submit->Assign(*m_submit_template);

        submit->SetData().SetEntrys().clear();
        submit->SetData().SetEntrys().push_back(object);

        UpdateSubmitObject(submit);

        return CRef<CSerialObject>(submit);
    }

    return CRef<CSerialObject>(object);
}

CRef<CSerialObject> CTable2AsnContext::CreateSeqEntryFromTemplate(CRef<CSeq_entry> object) const
{
    if (m_submit_template.NotEmpty())
    {
        if (m_submit_template->IsSetSub() &&
            m_submit_template->GetSub().IsSetCit())
        {
            CRef<CPub> pub(new CPub);
            pub->SetSub().Assign(m_submit_template->GetSub().GetCit());

            CRef<CSeqdesc> pub_desc(new CSeqdesc);
            pub_desc->SetPub().SetPub().Set().push_back(pub);
            object->SetDescr().Set().push_back(pub_desc);
        }

        object->Parentize();
    }
    return CRef<CSerialObject>(object);
}

bool CTable2AsnContext::IsDBLink(const CSeqdesc& desc)
{
    if (desc.IsUser() && desc.GetUser().IsSetType() && desc.GetUser().GetType().IsStr() &&
        NStr::CompareNocase(desc.GetUser().GetType().GetStr().c_str(), "DBLink") == 0)
        return true;
    else
        return false;
}

void CTable2AsnContext::x_MergeSeqDescr(objects::CSeq_descr& dest, const objects::CSeq_descr& src, bool only_set) const
{
    for (auto src_desc: src.Get())
    {
        CRef<CSeqdesc> new_desc;
        switch (src_desc->Which())
        {
        case CSeqdesc::e_Molinfo:
        case CSeqdesc::e_Source:
            if (only_set)
               continue;
            break;
        case CSeqdesc::e_User:
            if (IsDBLink(*src_desc))
            {
               if (only_set)
                    continue;
            }
            else
            if (!only_set)
                continue;
            break;
        case CSeqdesc::e_Pub:
            if (!only_set)
                continue;
            break;
        default:
            if (only_set)
                continue;
            break;
        }

        switch (src_desc->Which())
        {
        case CSeqdesc::e_User:
            if (IsDBLink(*src_desc))
            {
                auto& user_obj = SetUserObject(dest, "DBLink");

                edit::CDBLink::MergeDBLink(user_obj, src_desc->GetUser());

                continue;
            }
            break;
        case CSeqdesc::e_Pub:
            break;
        default:
            new_desc = CAutoAddDesc::LocateDesc(dest, src_desc->Which());
        }

        if (new_desc.Empty())
        {
            new_desc.Reset(new CSeqdesc);
            dest.Set().push_back(new_desc);
        }
        new_desc->Assign(*src_desc);
    }
}

void CTable2AsnContext::MergeWithTemplate(CSeq_entry& entry) const
{
    if (m_entry_template.IsNull() || !m_entry_template->IsSetDescr())
        return;

    if (entry.IsSet())// && entry.GetSet().IsSetClass())
    {
        x_MergeSeqDescr(entry.SetDescr(), m_entry_template->GetDescr(), true);

        CSeq_entry_Base::TSet::TSeq_set& data = entry.SetSet().SetSeq_set();
        NON_CONST_ITERATE(CSeq_entry_Base::TSet::TSeq_set, it, data)
        {
            MergeWithTemplate(**it);
        }
    }
    else
    if (entry.IsSeq())
    {
        if (entry.GetParentEntry() == 0)
           x_MergeSeqDescr(entry.SetDescr(), m_entry_template->GetDescr(), true);
        x_MergeSeqDescr(entry.SetDescr(), m_entry_template->GetDescr(), false);
    }
}

void CTable2AsnContext::SetSeqId(CSeq_entry& entry) const
{
    string base;
    CDirEntry::SplitPath(m_current_file, 0, &base, 0);
    CRef<CSeq_id> id(new CSeq_id(string("lcl|") + base));

    CBioseq* bioseq = 0;
    if (entry.IsSeq())
    {
        bioseq = &entry.SetSeq();
    }
    else
    if (entry.IsSet())
    {
        bioseq = &entry.SetSet().SetSeq_set().front()->SetSeq();
    }
    _ASSERT(bioseq);
    bioseq->SetId().clear();
    bioseq->SetId().push_back(id);
    // now it's good to rename features ....
}


//LCOV_EXCL_START
void CTable2AsnContext::CopyFeatureIdsToComments(CSeq_entry& entry) const
{
    CScope scope(*CObjectManager::GetInstance());
    CSeq_entry_Handle h_entry = scope.AddTopLevelSeqEntry(entry);
    for (CBioseq_CI bioseq_it(h_entry); bioseq_it; ++bioseq_it)
    {
        for (CFeat_CI feat_it(*bioseq_it, SAnnotSelector(CSeqFeatData::e_Rna) ); feat_it; ++feat_it)
        {
            ITERATE(CBioseq::TId, id_it, bioseq_it->GetBioseqCore()->GetId())
            {
                if (!(**id_it).IsGeneral()) continue;

                const string& dbtag = (**id_it).GetGeneral().GetDb();
                if (NStr::Compare(dbtag, "TMSMART") == 0) continue;
                if (NStr::Compare(dbtag, "NCBIFILE") == 0) continue;

                CSeq_feat& feature = (CSeq_feat&) feat_it->GetOriginalFeature();

                if (!feature.IsSetComment())
                    feature.SetComment("");

                string& comment = feature.SetComment();
                if (!comment.empty())
                    comment += "; ";
                (**id_it).GetLabel(&comment);
           }
        }
    }
}
//LCOV_EXCL_STOP

//LCOV_EXCL_START
void CTable2AsnContext::SmartFeatureAnnotation(CSeq_entry& entry) const
{
    CScope scope(*CObjectManager::GetInstance());
    CSeq_entry_Handle h_entry = scope.AddTopLevelSeqEntry(entry);

    size_t numgene = 0;

    std::vector<CSeq_feat*> cds;
    std::vector<CSeq_feat*> rnas;
    for (CFeat_CI feat_it(h_entry); feat_it; ++feat_it)
    {
        if (!feat_it->IsSetData())
            continue;

        switch (feat_it->GetData().Which())
        {
        case CSeqFeatData::e_Gene:
            numgene++;
            break;
        case CSeqFeatData::e_Cdregion:
            cds.push_back((CSeq_feat*) &feat_it->GetOriginalFeature());
            break;
        case CSeqFeatData::e_Rna:
            rnas.push_back((CSeq_feat*) &feat_it->GetOriginalFeature());
            break;
        default:
            break;
        }
    }
    if (numgene == 0)
        return;

}
//LCOV_EXCL_STOP

void CTable2AsnContext::MakeGenomeCenterId(CSeq_entry& entry)
{
    if (m_genome_center_id.empty())
        return;

    VisitAllBioseqs(entry, [this](CBioseq& bioseq)
    {
        if (m_genome_center_id.empty()) return;

        CTempString db = m_genome_center_id;

        NON_CONST_ITERATE(CBioseq::TId, id_it, bioseq.SetId())
        {
            CRef<CSeq_id> seq_id(*id_it);
            if (seq_id.Empty()) continue;

            const CObject_id* obj_id;
            switch (seq_id->Which())
            {
            case CSeq_id::e_Local:
                obj_id = &seq_id->GetLocal();
                break;
                //        case CSeq_id::e_General:
                //            obj_id = &seq_id->GetGeneral().GetTag();
                //            break;
            default:
                continue;
            }
            if (obj_id->IsId())
                seq_id->SetGeneral().SetTag().SetId(obj_id->GetId());
            else
            {
                string id = obj_id->GetStr();
                seq_id->SetGeneral().SetTag().SetStr(id);
            }

            seq_id->SetGeneral().SetDb(db);
        }
    });
};


void CTable2AsnContext::RenameProteinIdsQuals(CSeq_feat& feature)
{
    if (!feature.IsSetQual())
        return;

    CSeq_feat::TQual& quals = feature.SetQual();
    for (CSeq_feat::TQual::iterator it = quals.begin(); it != quals.end(); it++)
    {
        auto& qual = (**it);
        if (qual.CanGetVal())
        {
            const auto& qual_name = qual.GetQual();
            const auto& qual_value = qual.GetVal();
            //discussion of rw-451: always rename, never delete, regardless of 
            // whether in original data or not
            //
            if (qual_name == "transcript_id") {
                qual.SetQual("orig_transcript_id");
                continue;
            }
            if (qual_name == "protein_id") {
                qual.SetQual("orig_protein_id");
                continue;
            }
        }
    }
    if (quals.empty())
        feature.ResetQual();
}

void CTable2AsnContext::RemoveProteinIdsQuals(CSeq_feat& feature)
{
    if (!feature.IsSetQual())
        return;

    CSeq_feat::TQual& quals = feature.SetQual();
    for (CSeq_feat::TQual::iterator it = quals.begin(); it != quals.end();) // no ++ iterator
    {
        if ((**it).GetQual() == "protein_id" ||
            (**it).GetQual() == "transcript_id")
        {
            it = quals.erase(it);
        }
        else
        {
            it++;
        }
    }
    if (quals.empty())
        feature.ResetQual();
}

bool CTable2AsnContext::ApplyCreateUpdateDates(objects::CSeq_entry& entry) const
{
    bool need_update = false;
    switch(entry.Which())
    {
    case CSeq_entry::e_Seq:
        need_update |= x_ApplyCreateDate(entry);
        if (need_update)
        {
            if (entry.GetParentEntry() == 0)
                ApplyUpdateDate(entry);
            else
                CAutoAddDesc::EraseDesc(entry.SetDescr(), CSeqdesc::e_Update_date);
        }
        break;
    case CSeq_entry::e_Set:
        {
            if (entry.GetSet().IsSetClass() &&
                entry.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot)
            {
                ApplyUpdateDate(entry);
            }
            else
            {
                NON_CONST_ITERATE(CSeq_entry::TSet::TSeq_set, it, entry.SetSet().SetSeq_set())
                {
                    need_update |= ApplyCreateUpdateDates(**it);
                }
                if (need_update)
                    ApplyUpdateDate(entry);
            }
        }
        break;
    default:
        break;
    }
    return need_update;
}

void CTable2AsnContext::ApplyFileTracks(objects::CSeq_entry& entry) const
{
  if (!m_ft_url.empty()) 
    AddUserTrack(entry.SetDescr(), "FileTrack", "Map-FileTrackURL", m_ft_url);
  if (!m_ft_url_mod.empty()) 
    AddUserTrack(entry.SetDescr(), "FileTrack", "BaseModification-FileTrackURL", m_ft_url_mod);

}

CRef<COrg_ref> CTable2AsnContext::GetOrgRef(CSeq_descr& descr)
{
    NON_CONST_ITERATE(CSeq_descr_Base::Tdata, it, descr.Set())
    {
        if ((**it).IsSource())
        {
            CBioSource& source = (**it).SetSource();
            if (source.IsSetOrg())
            {
                return Ref(&source.SetOrg());
            }
        }
        if ((**it).IsOrg())
        {
            return Ref(&(**it).SetOrg());
        }
    }
    return CRef<COrg_ref>();
}

bool CTable2AsnContext::GetOrgName(string& name, const CSeq_entry& entry)
{
    if (entry.IsSet() && entry.GetSet().IsSetDescr())
    {
        ITERATE(CSeq_descr_Base::Tdata, it, entry.GetSet().GetDescr().Get())
        {
            if ((**it).IsSource())
            {
                const CBioSource& source = (**it).GetSource();
                if (source.IsSetTaxname())
                {
                    name = source.GetTaxname();
                    return true;
                }
                if (source.IsSetOrgname())
                {
                    if (source.GetOrgname().GetFlatName(name))
                        return true;
                }
                if (source.IsSetOrg() && source.GetOrg().IsSetOrgname())
                {
                    if (source.GetOrg().GetOrgname().GetFlatName(name))
                        return true;
                }
            }
            if ((**it).IsOrg())
            {
                if ((**it).GetOrg().IsSetOrgname())
                {
                    if ((**it).GetOrg().GetOrgname().GetFlatName(name))
                        return true;
                }
            }
        }
    }
    else
    if (entry.IsSeq())
    {
    }
    return false;
}


void CTable2AsnContext::UpdateTaxonFromTable(objects::CBioseq& bioseq)
{  
    if (bioseq.IsSetDescr() && bioseq.GetDescr().IsSet())
    {
        CRef<COrg_ref> org_ref = GetOrgRef(bioseq.SetDescr());
        if (org_ref.NotEmpty())
            org_ref->UpdateFromTable();
    }
}

bool AssignLocalIdIfEmpty(ncbi::objects::CSeq_feat& feature, int& id)
{
    if (feature.IsSetId())
        return true;
    else
    {
        feature.SetId().SetLocal().SetId(id++);
        return false;
    }
}

void CTable2AsnContext::CorrectCollectionDates(objects::CSeq_entry& entry)
{
    size_t p = m_cleanup.find_first_of("Dd");
    if (p == string::npos)
        return;


    VisitAllSetandSeq(entry, 
        [this](CBioseq_set& bioseq_set)->bool
        {
            x_CorrectCollectionDates(*this, bioseq_set);
            return true;
    },
        [this](CBioseq& bioseq)
        {
            x_CorrectCollectionDates(*this, bioseq);
        }
    );
}

void CTable2AsnContext::ApplyComments(objects::CSeq_entry& entry)
{
    if (m_Comment.empty())
        return;

    VisitAllSetandSeq(entry,
        [this](CBioseq_set& bioseq_set)->bool
        {
            if (bioseq_set.IsSetClass() && bioseq_set.GetClass() == CBioseq_set::eClass_genbank)
            {
                return true; // let's go deeper               
            }

            CRef<CSeqdesc> comment_desc(new CSeqdesc());
            comment_desc->SetComment(m_Comment);
            bioseq_set.SetDescr().Set().push_back(comment_desc);

            return false; // stop going deeper
        },
        [this](CBioseq& bioseq)
        {
            CRef<CSeqdesc> comment_desc(new CSeqdesc());
            comment_desc->SetComment(m_Comment);
            bioseq.SetDescr().Set().push_back(comment_desc);
        }
    );
}

END_NCBI_SCOPE
