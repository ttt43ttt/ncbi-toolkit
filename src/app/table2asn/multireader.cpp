/*  $Id: multireader.cpp 574509 2018-11-14 16:02:58Z ivanov $
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
* Author:  Frank Ludwig, Sergiy Gotvyanskyy, NCBI
*
* File Description:
*   Reader for selected data file formats
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include "fasta_ex.hpp"

#include <objtools/readers/gff3_reader.hpp>
#include <objtools/readers/gtf_reader.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqfeat/Variation_ref.hpp>

#include <objects/seq/Seq_descr.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/general/Date.hpp>
#include <objects/biblio/Cit_sub.hpp>

#include <corelib/ncbistre.hpp>

#include <serial/iterator.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/objostrasn.hpp>
#include <serial/serial.hpp>
#include <objects/seq/Annot_id.hpp>
#include <objects/general/Dbtag.hpp>
#include <objtools/readers/readfeat.hpp>

#include <util/format_guess.hpp>

#include "multireader.hpp"
#include "table2asn_context.hpp"

#include <objtools/edit/feattable_edit.hpp>

#include <common/test_assert.h>  /* This header must go last */

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

namespace
{

    CRef<CSeq_id> GetSeqIdWithoutVersion(const CSeq_id& id)
    {
        const CTextseq_id* text_id = id.GetTextseq_Id();

        if (text_id && text_id->IsSetVersion())
        {
            CRef<CSeq_id> new_id(new CSeq_id);
            new_id->Assign(id);
            CTextseq_id* text_id = (CTextseq_id*)new_id->GetTextseq_Id();
            text_id->ResetVersion();
            return new_id;
        }

        return CRef<CSeq_id>(0);
    }

    struct SCSeqidCompare
    {
        inline
            bool operator()(const CSeq_id* left, const CSeq_id* right) const
        {
            return *left < *right;
        };
    };

    CRef<CSeq_id> GetIdByKind(const CSeq_id& id, const CBioseq::TId& ids)
    {
        ITERATE(CBioseq::TId, it, ids)
        {
            if ((**it).Which() == id.Which())
                return *it;
        }
        return CRef<CSeq_id>();
    }

	void x_ModifySeqIds(CSerialObject& obj, CConstRef<CSeq_id> match, CRef<CSeq_id> new_id)
	{
#if 0
		CTypeIterator<CSeq_id> visitor(obj);

		while (visitor)
		{
			CSeq_id& id = *visitor;
			if (match.Empty() || id.Compare(*match) == CSeq_id::e_YES)
			{
				id.Assign(*new_id);
			}
			++visitor;
		}
#else
        CTypeIterator<CSeq_loc> visitor(obj);

        CSeq_id& id = *new_id;
        while (visitor)
        {
            CSeq_loc& loc = *visitor;
            
            if (match.Empty() || loc.GetId()->Compare(*match) == CSeq_id::e_YES)
            {
                loc.SetId(id);
            }
            ++visitor;
        }
#endif
	}

}

CRef<CSerialObject> CMultiReader::xReadASN1(CObjectIStream& pObjIstrm)
{
    CRef<CSeq_entry> entry;
    CRef<CSeq_submit> submit;

    // guess object type
    string sType;
    try
    {
        sType = pObjIstrm.ReadFileHeader();
    }
    catch (const CEofException&)
    {
        sType.clear();
        // ignore EOF exception
    }

    // do the right thing depending on the input type
    if (sType == CBioseq_set::GetTypeInfo()->GetName()) {
        entry.Reset(new CSeq_entry);
        pObjIstrm.Read(ObjectInfo(entry->SetSet()), CObjectIStream::eNoFileHeader);
    }
    else
    if (sType == CSeq_submit::GetTypeInfo()->GetName()) {
        submit.Reset(new CSeq_submit);
        pObjIstrm.Read(ObjectInfo(*submit), CObjectIStream::eNoFileHeader);

        if (submit->GetData().GetEntrys().size() > 1)
        {
            entry.Reset(new CSeq_entry);
            entry->SetSet().SetSeq_set() = submit->GetData().GetEntrys();
        }
        else
            entry = *submit->SetData().SetEntrys().begin();
    }
    else
    if (sType == CSeq_entry::GetTypeInfo()->GetName()) {
        entry.Reset(new CSeq_entry);
        pObjIstrm.Read(ObjectInfo(*entry), CObjectIStream::eNoFileHeader);
    }
    else
    if (sType == CSeq_annot::GetTypeInfo()->GetName()) 
    {
        entry.Reset(new CSeq_entry);
        do
        {
            CRef<CSeq_annot> annot(new CSeq_annot);
            pObjIstrm.Read(ObjectInfo(*annot), CObjectIStream::eNoFileHeader);
            entry->SetSeq().SetAnnot().push_back(annot);
            try
            {
                sType = pObjIstrm.ReadFileHeader();
            }
            catch (const CEofException&)
            {
                sType.clear();
                // ignore EOF exception
            }
        } while (sType == CSeq_annot::GetTypeInfo()->GetName());
    }
    else
    {
        return CRef<CSerialObject>();
    }

    if (m_context.m_gapNmin > 0)
    {
        CGapsEditor gap_edit((CSeq_gap::EType)m_context.m_gap_type, m_context.m_gap_evidences, m_context.m_gapNmin, m_context.m_gap_Unknown_length);
        gap_edit.ConvertNs2Gaps(*entry);
    }

    if (submit.Empty())
        return entry;
    else
        return submit;
}

//  ----------------------------------------------------------------------------
CRef<CSeq_entry>
CMultiReader::xReadFasta(CNcbiIstream& instream)
    //  ----------------------------------------------------------------------------
{
    if (m_context.m_gapNmin > 0)
    {
        m_iFlags |= CFastaReader::fParseGaps
                 |  CFastaReader::fLetterGaps;
    }
    else
    {
        m_iFlags |= CFastaReader::fNoSplit;
//                 |  CFastaReader::fLeaveAsText;
    }

    m_iFlags |= CFastaReader::fAddMods
             |  CFastaReader::fValidate
             |  CFastaReader::fHyphensIgnoreAndWarn;

    if (m_context.m_allow_accession)
        m_iFlags |= CFastaReader::fParseRawID;


    m_iFlags |= CFastaReader::fAssumeNuc
             |  CFastaReader::fForceType;



    CStreamLineReader lr( instream );
#ifdef NEW_VERSION
    auto_ptr<CBaseFastaReader> pReader(new CBaseFastaReader(m_iFlags));
//    auto_ptr<CBaseFastaReader> pReader(new CFastaReaderEx(lr, m_iFlags));
#else
    auto_ptr<CFastaReaderEx> pReader(new CFastaReaderEx(m_context, lr, m_iFlags));
#endif
    if (!pReader.get()) {
        NCBI_THROW2(CObjReaderParseException, eFormat,
            "File format not supported", 0);
    }
#ifdef NEW_VERSION
    CRef<CSeq_entry> result = pReader->ReadSeqEntry(lr , m_context.m_logger);
#else
    if (m_context.m_gapNmin > 0)
    {
        pReader->SetMinGaps(m_context.m_gapNmin, m_context.m_gap_Unknown_length);
        if (m_context.m_gap_evidences.size() >0 || m_context.m_gap_type>=0)
            pReader->SetGapLinkageEvidences((CSeq_gap::EType)m_context.m_gap_type, m_context.m_gap_evidences);
    }

    int max_seqs = kMax_Int;
    CRef<CSeq_entry> result = m_context.m_di_fasta ? pReader->ReadDIFasta(m_context.m_logger) : pReader->ReadSet(max_seqs, m_context.m_logger);

    if (result.NotEmpty())
    {
        m_context.MakeGenomeCenterId(*result);
    }

    if (result->IsSet() && !m_context.m_HandleAsSet)
    {
        m_context.m_logger->PutError(*auto_ptr<CLineError>(
            CLineError::Create(ILineError::eProblem_GeneralParsingError, eDiag_Warning, "", 0,
            "File " + m_context.m_current_file + " contains multiple sequences")));
    }
    if (result->IsSet())
    {
        result->SetSet().SetClass(m_context.m_ClassValue);
    }

#endif
    return result;

}

CFormatGuess::EFormat CMultiReader::xGetFormat(CNcbiIstream& istr) const
    //  ----------------------------------------------------------------------------
{
    CFormatGuess FG(istr);
    FG.GetFormatHints().AddPreferredFormat(CFormatGuess::eBinaryASN);
    FG.GetFormatHints().AddPreferredFormat(CFormatGuess::eFasta);
    FG.GetFormatHints().AddPreferredFormat(CFormatGuess::eTextASN);
    FG.GetFormatHints().AddPreferredFormat(CFormatGuess::eGffAugustus);
    FG.GetFormatHints().AddPreferredFormat(CFormatGuess::eGff3);
    FG.GetFormatHints().AddPreferredFormat(CFormatGuess::eGff2);
    FG.GetFormatHints().AddPreferredFormat(CFormatGuess::eGtf);
    FG.GetFormatHints().AddPreferredFormat(CFormatGuess::eFiveColFeatureTable);
    FG.GetFormatHints().DisableAllNonpreferred();

    return FG.GuessFormat();
}

//  ----------------------------------------------------------------------------
void CMultiReader::WriteObject(
    const CSerialObject& object,
    ostream& ostr)
    //  ----------------------------------------------------------------------------
{
    ostr << MSerial_AsnText
         //<< MSerial_VerifyNo
         << object;
    ostr.flush();
}

CMultiReader::CMultiReader(CTable2AsnContext& context)
    :m_context(context)
{
}

void CMultiReader::ApplyAdditionalProperties(CSeq_entry& entry)
{
    switch(entry.Which())
    {
    case CSeq_entry::e_Seq:
        if (!m_context.m_OrganismName.empty() || m_context.m_taxid != 0)
        {
            CBioSource::TOrg& org(CAutoAddDesc(entry.SetDescr(), CSeqdesc::e_Source).Set().SetSource().SetOrg());
            // we should reset taxid in case new name is different
            if (org.IsSetTaxname() && org.GetTaxId() >0 && org.GetTaxname() != m_context.m_OrganismName)
            {
                org.SetTaxId(0);
            }

            if (!m_context.m_OrganismName.empty())
                org.SetTaxname(m_context.m_OrganismName);
            if (m_context.m_taxid != 0)
                org.SetTaxId(m_context.m_taxid);
        }
        break;

    case CSeq_entry::e_Set:
        {
            if (!entry.GetSet().IsSetClass())
                entry.SetSet().SetClass(CBioseq_set::eClass_genbank);

            NON_CONST_ITERATE(CBioseq_set_Base::TSeq_set, it, entry.SetSet().SetSeq_set())
            {
                ApplyAdditionalProperties(**it);
            }
        }
        break;
    default:
        break;
    }
}

void CMultiReader::LoadDescriptors(const string& ifname, CRef<CSeq_descr> & out_desc)
{
    out_desc.Reset(new CSeq_descr);

    unique_ptr<CObjectIStream> pObjIstrm = xCreateASNStream(ifname);

    // guess object type
    //const string sType = pObjIstrm->ReadFileHeader();

    // do the right thing depending on the input type
    while (true) {
        try {
            const string sType = pObjIstrm->ReadFileHeader();
            if (sType == CSeq_descr::GetTypeInfo()->GetName())
            {
                CRef<CSeq_descr> descr(new CSeq_descr);
                pObjIstrm->Read(ObjectInfo(*descr),
                    CObjectIStream::eNoFileHeader);
                out_desc->Set().insert(out_desc->Set().end(), descr->Get().begin(), descr->Get().end());
            }
            else if (sType == CSeqdesc::GetTypeInfo()->GetName())
            {
                CRef<CSeqdesc> desc(new CSeqdesc);
                pObjIstrm->Read(ObjectInfo(*desc),
                    CObjectIStream::eNoFileHeader);
                out_desc->Set().push_back(desc);
            }
            else if (sType == CPubdesc::GetTypeInfo()->GetName())
            {
                CRef<CSeqdesc> desc(new CSeqdesc);
                pObjIstrm->Read(ObjectInfo(desc->SetPub()),
                    CObjectIStream::eNoFileHeader);
                out_desc->Set().push_back(desc);
            }
            else
            {
                throw runtime_error("Descriptor file must contain "
                    "either Seq_descr or Seqdesc elements");
            }
        } catch (CException& ex) {
            if (!NStr::EqualNocase(ex.GetMsg(), "end of file")) {
                throw runtime_error("Unable to read descriptor from file:" + ex.GetMsg());
            }
            break;
        }
    }
}

void CMultiReader::LoadTemplate(CTable2AsnContext& context, const string& ifname)
{
#if 0
    // check if the location doesn't exist, and see if we can
    // consider it some kind of sequence identifier
    if( ! CDirEntry(ifname).IsFile() ) {
        // see if this is blatantly not a sequence identifier
        if( ! CRegexpUtil(sTemplateLocation).Exists("^[A-Za-z0-9_|]+(\\.[0-9]+)?$") ) {
            throw runtime_error("This is not a valid sequence identifier: " + sTemplateLocation);
        }

        // try to load from genbank
        CGBDataLoader::RegisterInObjectManager(*CObjectManager::GetInstance());
        CRef<CScope> pScope(new CScope(*CObjectManager::GetInstance()));
        pScope->AddDefaults();

        CRef<CSeq_id> pTemplateId( new CSeq_id(sTemplateLocation) );
        CBioseq_Handle bsh = pScope->GetBioseqHandle( *pTemplateId );

        if ( ! bsh ) {
            throw runtime_error("Invalid sequence identifier: " + sTemplateLocation);
        }
        CSeq_entry_Handle entry_h = bsh.GetParentEntry();

        context.m_entry_template->Assign( *entry_h.GetCompleteSeq_entry() );
        return;
    }
#endif

    unique_ptr<CObjectIStream> pObjIstrm = xCreateASNStream(ifname);

    // guess object type
    string sType = pObjIstrm->ReadFileHeader();

    // do the right thing depending on the input type
    if( sType == CSeq_entry::GetTypeInfo()->GetName() ) {
        context.m_entry_template.Reset( new CSeq_entry );
        pObjIstrm->Read(ObjectInfo(*context.m_entry_template), CObjectIStream::eNoFileHeader);
    } else if( sType == CBioseq::GetTypeInfo()->GetName() ) {
        CRef<CBioseq> pBioseq( new CBioseq );
        pObjIstrm->Read(ObjectInfo(*pBioseq), CObjectIStream::eNoFileHeader);
        context.m_entry_template.Reset( new CSeq_entry );
        context.m_entry_template->SetSeq( *pBioseq );
    } else if( sType == CSeq_submit::GetTypeInfo()->GetName() ) {

        context.m_submit_template.Reset( new CSeq_submit );
        pObjIstrm->Read(ObjectInfo(*context.m_submit_template), CObjectIStream::eNoFileHeader);
        if (!context.m_submit_template->GetData().IsEntrys()
            || context.m_submit_template->GetData().GetEntrys().size() != 1)
        {
            throw runtime_error("Seq-submit template must contain "
                "exactly one Seq-entry");
        }
    } else if( sType == CSubmit_block::GetTypeInfo()->GetName() ) {

        // a Submit-block
        CRef<CSubmit_block> submit_block(new CSubmit_block);
        pObjIstrm->Read(ObjectInfo(*submit_block),
            CObjectIStream::eNoFileHeader);

        // Build a Seq-submit containing this plus a bogus Seq-entry
        context.m_submit_template.Reset( new CSeq_submit );
        context.m_submit_template->SetSub(*submit_block);
        CRef<CSeq_entry> ent(new CSeq_entry);
        CRef<CSeq_id> dummy_id(new CSeq_id("lcl|dummy_id"));
        ent->SetSeq().SetId().push_back(dummy_id);
        ent->SetSeq().SetInst().SetRepr(CSeq_inst::eRepr_raw);
        ent->SetSeq().SetInst().SetMol(CSeq_inst::eMol_dna);
        context.m_submit_template->SetData().SetEntrys().push_back(ent);
    } else if ( sType == CSeqdesc::GetTypeInfo()->GetName()) {
        // it's OK
    } else {
        NCBI_USER_THROW_FMT("Template must be Seq-entry, Seq-submit, Bioseq or "
            "Submit-block.  Object seems to be of type: " << sType);
    }

    // for submit types, pull out the seq-entry inside and remember it
    if( context.m_submit_template.NotEmpty() && context.m_submit_template->IsEntrys() ) {
        context.m_entry_template = context.m_submit_template->SetData().SetEntrys().front();
    }

    // The template may contain a set rather than a seq.
    // That's OK if it contains only one na entry, which we'll use.
    if (context.m_entry_template.NotEmpty() && context.m_entry_template->IsSet()) 
    {
        CRef<CSeq_entry> tmp(new CSeq_entry);
        ITERATE(CBioseq_set::TSeq_set, ent_iter, context.m_entry_template->GetSet().GetSeq_set())
        {
            const CSeq_descr* descr = 0;
            if ((*ent_iter)->IsSetDescr())
            {
                descr = &(*ent_iter)->GetDescr();
            }
            if (descr)
            {
                //tmp->Assign(**ent_iter);
                tmp->SetSeq().SetInst();
                // Copy any descriptors from the set to the sequence
                ITERATE(CBioseq_set::TDescr::Tdata, desc_iter, descr->Get())
                {
                    switch ((*desc_iter)->Which())
                    {
                    case CSeqdesc::e_Pub:
                    case CSeqdesc::e_Source:
                       break;
                    default:
                       continue;
                    }
                    CRef<CSeqdesc> desc(new CSeqdesc);
                    desc->Assign(**desc_iter);
                    tmp->SetSeq().SetDescr().Set().push_back(desc);
                }
                break;
            }
        }

        if (tmp->IsSetDescr() && !tmp->GetDescr().Get().empty())
            context.m_entry_template = tmp;

    }

    // incorporate any Seqdesc's that follow in the file
    if (!pObjIstrm->EndOfData())
    {
        if (sType != CSeqdesc::GetTypeInfo()->GetName())
            sType = pObjIstrm->ReadFileHeader();

        while (sType == CSeqdesc::GetTypeInfo()->GetName()) {
            CRef<CSeqdesc> desc(new CSeqdesc);
            pObjIstrm->Read(ObjectInfo(*desc), CObjectIStream::eNoFileHeader);

            if  (context.m_entry_template.Empty())
                context.m_entry_template.Reset(new CSeq_entry);

            {
                if (desc->IsUser() && desc->GetUser().IsDBLink())
                {
                    CUser_object& user_obj = desc->SetUser();
                    if (user_obj.IsDBLink())
                    {
                        user_obj.SetData();
                    }
                }
            }

            context.m_entry_template->SetSeq().SetDescr().Set().push_back(desc);

            if (pObjIstrm->EndOfData())
                break;

            try {
                sType = pObjIstrm->ReadFileHeader();
            }
            catch(CEofException&) {
                break;
            }
        }
    }

#if 0
    if ( context.m_submit_template->IsEntrys() ) {
        // Take Seq-submit.sub.cit and put it in the Bioseq
        CRef<CPub> pub(new CPub);
        pub->SetSub().Assign(context.m_submit_template->GetSub().GetCit());
        CRef<CSeqdesc> pub_desc(new CSeqdesc);
        pub_desc->SetPub().SetPub().Set().push_back(pub);
        context.m_entry_template->SetSeq().SetDescr().Set().push_back(pub_desc);
    }
#endif

    if( context.m_entry_template.NotEmpty() && ! context.m_entry_template->IsSeq() ) {
        throw runtime_error("The Seq-entry must be a Bioseq not a Bioseq-set.");
    }

    if (context.m_submit_template.NotEmpty())
    {
        if (context.m_submit_template->IsSetSub() &&
            context.m_submit_template->GetSub().IsSetCit())
        {
        CRef<CDate> date(new CDate(CTime(CTime::eCurrent), CDate::ePrecision_day));
        context.m_submit_template->SetSub().SetCit().SetDate(*date);
        }
    }

#if 0
    if( args["output-type"].AsString() == "Seq-entry" ) {
        // force Seq-entry by throwing out the Seq-submit
        context.m_submit_template.Reset( new CSeq_submit );
    }
#endif
}

namespace
{
    class AllowedDuplicates: public set<CSeqdesc_Base::E_Choice>
    {
    public:
        AllowedDuplicates()
        {
            insert(CSeqdesc_Base::e_User);
        }
    };
    AllowedDuplicates m_allowed_duplicates;

    template<typename _which>
    struct LocateWhich
    {
        typename _which::E_Choice compare_to;
        bool operator() (_which l)  const
        {
            return l.Which() == compare_to;
        }
        bool operator() (const CRef<_which>& l)  const
        {
            return l->Which() == compare_to;
        }
    };
}

void CMultiReader::MergeDescriptors(CSeq_descr & dest, const CSeq_descr & source)
{
    ITERATE(CSeq_descr::Tdata, it, source.Get())
    {
        MergeDescriptors(dest, **it);
    }
}

void CMultiReader::MergeDescriptors(CSeq_descr & dest, const CSeqdesc & source)
{
    bool duplicates = (m_allowed_duplicates.find(source.Which()) == m_allowed_duplicates.end());

    CAutoAddDesc desc(dest, source.Which());
    desc.Set(duplicates).Assign(source);
}

void CMultiReader::ApplyDescriptors(CSeq_entry& entry, const CSeq_descr& source)
{
    MergeDescriptors(entry.SetDescr(), source);
}

namespace
{
    void CopyDescr(CSeq_entry& dest, const CSeq_entry& src)
    {
        if (src.IsSetDescr() && !src.GetDescr().Get().empty())
        {
            dest.SetDescr().Set().insert(dest.SetDescr().Set().end(),
                src.GetDescr().Get().begin(),
                src.GetDescr().Get().end());
        }
    }
    void CopyAnnot(CSeq_entry& dest, const CSeq_entry& src)
    {
        if (src.IsSetAnnot() && !src.GetAnnot().empty())
        {
            dest.SetAnnot().insert(dest.SetAnnot().end(),
                src.GetAnnot().begin(),
                src.GetAnnot().end());
        }
    }
};

CFormatGuess::EFormat CMultiReader::OpenFile(const string& filename, CRef<CSerialObject>& obj)
{
    unique_ptr<istream> istream(new CNcbiIfstream(filename.c_str()));
    CFormatGuess::EFormat format = xGetFormat(*istream);

    if (format == CFormatGuess::eTextASN) {
        m_obj_stream = xCreateASNStream(format, istream);
        obj = xReadASN1(*m_obj_stream);
    }
    else { // RW-616 - Assume FASTA
        m_iFlags = 0;
        m_iFlags |= CFastaReader::fNoUserObjs;

        obj = xReadFasta(*istream);
    }
    if (obj.Empty())
        NCBI_THROW2(CObjReaderParseException, eFormat,
            "File format not recognized", 0);

    obj = xApplyTemplate(obj);

    return format;
}

void CMultiReader::GetSeqEntry(CRef<objects::CSeq_entry>& entry, CRef<objects::CSeq_submit>& submit, CRef<CSerialObject> obj)
{
    if (obj->GetThisTypeInfo() == CSeq_submit::GetTypeInfo())
    {
        submit.Reset(static_cast<CSeq_submit*>(obj.GetPointer()));
        entry = submit->SetData().SetEntrys().front();
    }
    else
    if (obj->GetThisTypeInfo() == CSeq_entry::GetTypeInfo()) {
        entry.Reset(static_cast<CSeq_entry*>(obj.GetPointer()));
    }
}

CRef<CSerialObject> CMultiReader::xApplyTemplate(CRef<CSerialObject> obj)
{
    CRef<CSeq_entry> entry;
    CRef<CSeq_submit> submit;

    GetSeqEntry(entry, submit, obj);

    if (entry.NotEmpty()) // && 
    {
        if (submit.Empty())
        if (entry->IsSet() && entry->GetSet().GetSeq_set().size() < 2 &&
            entry->GetSet().GetSeq_set().front()->IsSeq())
        {
            CRef<CSeq_entry> seq = entry->SetSet().SetSeq_set().front();
            CopyDescr(*seq, *entry);
            CopyAnnot(*seq, *entry);
            entry = seq;
        }
        entry->ResetParentEntry();
        entry->Parentize();

        m_context.MergeWithTemplate(*entry);
    }

    if (submit.Empty())
        return entry;
    else
        return submit;
}

CRef<CSerialObject> CMultiReader::ReadNextEntry()
{
    if (m_obj_stream) 
        return xReadASN1(*m_obj_stream);
    else
        return CRef<CSerialObject>();
}

CRef<CSeq_entry> CMultiReader::xReadGFF3(CNcbiIstream& instream)
{
    int flags = 0;
    flags |= CGff3Reader::fGenbankMode;
    flags |= CGff3Reader::fRetainLocusIds;
    flags |= CGff3Reader::fGeneXrefs;
    flags |= CGff3Reader::fAllIdsAsLocal;

    CGff3Reader reader(flags, m_AnnotName, m_AnnotTitle);
    CStreamLineReader lr(instream);
    CRef<CSeq_entry> entry(new CSeq_entry);
    entry->SetSeq();
    reader.ReadSeqAnnots(entry->SetAnnot(), lr, m_context.m_logger);

    x_PostProcessAnnot(*entry);

    return entry;
}

void CMultiReader::x_PostProcessAnnot(objects::CSeq_entry& entry)
{
    unsigned int startingLocusTagNumber = 1;
    unsigned int startingFeatureId = 1;

    auto& annots = entry.SetAnnot();
   
    for (auto it = annots.begin(); it != annots.end(); ++it) {

        edit::CFeatTableEdit fte(
            **it, m_context.m_locus_tag_prefix, startingLocusTagNumber, startingFeatureId, m_context.m_logger);
        //fte.InferPartials();
        fte.GenerateMissingParentFeatures(m_context.m_eukariote);
        fte.GenerateLocusTags();
        fte.GenerateProteinAndTranscriptIds();
        //fte.InstantiateProducts();
        fte.EliminateBadQualifiers();
        fte.SubmitFixProducts();

        startingLocusTagNumber = fte.PendingLocusTagNumber();
        startingFeatureId = fte.PendingFeatureId();
    }
}

unique_ptr<CObjectIStream> CMultiReader::xCreateASNStream(const string& filename)
{
    unique_ptr<istream> instream(new CNcbiIfstream(filename.c_str()));
    return xCreateASNStream(CFormatGuess::eUnknown, instream);
}

unique_ptr<CObjectIStream> CMultiReader::xCreateASNStream(CFormatGuess::EFormat format, unique_ptr<istream>& instream)
{
    // guess format
    ESerialDataFormat eSerialDataFormat = eSerial_None;
    {{
        if (format == CFormatGuess::eUnknown)
            format = CFormatGuess::Format(*instream);

        switch(format) {
        case CFormatGuess::eBinaryASN:
            eSerialDataFormat = eSerial_AsnBinary;
            break;
        case CFormatGuess::eUnknown:
        case CFormatGuess::eTextASN:
            eSerialDataFormat = eSerial_AsnText;
            break;
        case CFormatGuess::eXml:
            eSerialDataFormat = eSerial_Xml;
            break;
        default:
            NCBI_USER_THROW_FMT(
                "Descriptor file seems to be in an unsupported format: "
                << CFormatGuess::GetFormatName(format) );
            break;
        }

        //instream.seekg(0);
    }}

    unique_ptr<CObjectIStream> pObjIstrm(
        CObjectIStream::Open(eSerialDataFormat, *instream, eTakeOwnership) );

    instream.release();

    return pObjIstrm;
}

CMultiReader::~CMultiReader()
{
}

class CAnnotationLoader
{
public:
    bool Init(CRef<CSeq_entry> entry)
    {
        if (entry && entry->IsSetAnnot())
        {
           m_entry = entry;
           m_annot_iteator = m_entry->SetAnnot().begin();
           return m_annot_iteator != m_entry->SetAnnot().end();
        }
        return false;
    }

    bool Init(const string& seqid_prefix, unique_ptr<istream>& instream, ILineErrorListener* logger)
    {
        m_seqid_prefix = seqid_prefix;
        m_line_reader = ILineReader::New(*instream, eTakeOwnership);
        instream.release();
        m_logger = logger;
        return true;
    }

    CRef<CSeq_annot> GetNextAnnot()
    {
        if (m_entry)
        {
            if (m_annot_iteator != m_entry->SetAnnot().end())
            {
                return *m_annot_iteator++;
            }
        }
        else
        {
            while (!m_line_reader->AtEOF()) {
                CRef<CSeq_annot> annot = CFeature_table_reader::ReadSequinFeatureTable(
                    *m_line_reader,
                    CFeature_table_reader::fReportBadKey |
                    CFeature_table_reader::fLeaveProteinIds |
                    CFeature_table_reader::fCreateGenesFromCDSs |
                    CFeature_table_reader::fAllIdsAsLocal,
                    m_logger, 0/*filter*/, m_seqid_prefix);

                if (annot.NotEmpty() && annot->IsSetData() && annot->GetData().IsFtable() &&
                    !annot->GetData().GetFtable().empty()) {
                    return annot;
                }
            }
        }
        return CRef<CSeq_annot>();
    }
private:
    CRef<CSeq_entry> m_entry;
    CBioseq::TAnnot::iterator m_annot_iteator;
    string m_seqid_prefix;
    CRef<ILineReader> m_line_reader;
    ILineErrorListener* m_logger;
};

bool CMultiReader::xGetAnnotLoader(CAnnotationLoader& loader, const string& filename)
{
    unique_ptr<istream> in(new CNcbiIfstream(filename.c_str()));

    CFormatGuess::EFormat uFormat = xGetFormat(*in);

    if (uFormat == CFormatGuess::eUnknown)
    {        
        string ext;
        CDirEntry::SplitPath(filename, 0, 0, &ext);
        NStr::ToLower(ext);
        if (ext == ".gff" || ext == ".gff3")
            uFormat = CFormatGuess::eGff3;
        else
        if (ext == ".gff2")
            uFormat = CFormatGuess::eGff2;
        else
        if (ext == ".gtf")
            uFormat = CFormatGuess::eGtf;
        else
        if (ext == ".tbl")
            uFormat = CFormatGuess::eFiveColFeatureTable;
        else
        if (ext == ".asn" || ext == ".sqn" || ext == ".sap")
            uFormat = CFormatGuess::eTextASN;

        if (uFormat != CFormatGuess::eUnknown)
        {
            LOG_POST("Presuming annotation format by filename suffix: " 
                 << CFormatGuess::GetFormatName(uFormat));
        }
    }
    else
    {
        LOG_POST("Recognized annotation format: " << CFormatGuess::GetFormatName(uFormat));
    }

    CRef<CSeq_entry> entry;
    switch (uFormat)
    {
    case CFormatGuess::eFiveColFeatureTable:
    {
        string seqid_prefix;
        if (!m_context.m_genome_center_id.empty())
            seqid_prefix = "gnl|" + m_context.m_genome_center_id + "|";
        return loader.Init(seqid_prefix, in, m_context.m_logger);
    }
    break;
    case CFormatGuess::eTextASN:
        {
        auto obj_stream = xCreateASNStream(uFormat, in);
        CRef<CSerialObject> obj = xReadASN1(*obj_stream);
        CRef<CSeq_submit> unused;
        GetSeqEntry(entry, unused, obj);
        }
        break;
    case CFormatGuess::eGff2:
    case CFormatGuess::eGff3:
        entry = xReadGFF3(*in);
        break;
    case CFormatGuess::eGtf:
    case CFormatGuess::eGffAugustus:
        entry = xReadGTF(*in);
        break;
    default:
        NCBI_THROW2(CObjReaderParseException, eFormat,
            "Annotation file format not recognized. Run format validator on your annotation file", 1);
    }
    if (entry)
    {
        loader.Init(entry);
        return true;
    }
    return false;
}

bool CMultiReader::LoadAnnot(objects::CSeq_entry& entry, const string& filename)
{
    CAnnotationLoader annot_loader;

    if (xGetAnnotLoader(annot_loader, filename))
    {
        CScope scope(*m_context.m_ObjMgr);
        scope.AddTopLevelSeqEntry(entry);

        CRef<CSeq_annot> annot_it;
        while ((annot_it = annot_loader.GetNextAnnot()).NotEmpty())
        {
            CRef<CSeq_id> annot_id;
            if (annot_it->IsSetId())
            {
                annot_id.Reset(new CSeq_id);
                const CAnnot_id& tmp_annot_id = *(*annot_it).GetId().front();
                if (tmp_annot_id.IsLocal())
                    annot_id->SetLocal().Assign(tmp_annot_id.GetLocal());
                else
                    if (tmp_annot_id.IsGeneral())
                    {
                        annot_id->SetGeneral().Assign(tmp_annot_id.GetGeneral());
                    }
                    else
                    {
                        //cerr << "Unknown id type:" << annot_id.Which() << endl;
                        continue;
                    }
            }
            else
                if (!annot_it->GetData().GetFtable().empty())
                {
                    // get a reference to CSeq_id instance, we'd need to update it recently
                    // 5 column feature reader has a single shared instance for all features 
                    // update one at once would change all the features
                    annot_id.Reset((CSeq_id*)annot_it->GetData().GetFtable().front()->GetLocation().GetId());
                }

            CBioseq::TId ids;
            CBioseq_Handle bioseq_h = scope.GetBioseqHandle(*annot_id);
            if (!bioseq_h && annot_id->IsLocal() && annot_id->GetLocal().IsStr())
            {
                CSeq_id::ParseIDs(ids, annot_id->GetLocal().GetStr());
                if (ids.size() == 1)
                {
                    bioseq_h = scope.GetBioseqHandle(*ids.front());
                }
                if (!bioseq_h && !m_context.m_genome_center_id.empty())
                {
                    string id = "gnl|" + m_context.m_genome_center_id + "|" + annot_id->GetLocal().GetStr();
                    ids.clear();
                    CSeq_id::ParseIDs(ids, id);
                    if (ids.size() == 1)
                    {
                        bioseq_h = scope.GetBioseqHandle(*ids.front());
                    }
                }
            }
            if (!bioseq_h)
            {
                CRef<CSeq_id> alt_id = GetSeqIdWithoutVersion(*annot_id);
                if (alt_id)
                {
                    bioseq_h = scope.GetBioseqHandle(*alt_id);
                }
            }
            if (bioseq_h)
            {
                // update ids
                CBioseq_EditHandle edit_handle = bioseq_h.GetEditHandle();
                CBioseq& bioseq = (CBioseq&)*edit_handle.GetBioseqCore();
                CRef<CSeq_id> matching_id = GetIdByKind(*annot_id, bioseq.GetId());

                if (matching_id.Empty())
                    matching_id = ids.front();

                if (matching_id && !annot_id->Equals(*matching_id))
                {
                    x_ModifySeqIds(*annot_it, annot_id, matching_id);                    
                }

                CRef<CSeq_annot> existing;
                for (auto feat_it : bioseq.SetAnnot())
                {
                    if (feat_it->IsFtable())
                    {
                        existing = feat_it;
                        break;
                    }
                }
                if (existing.Empty())
                    bioseq.SetAnnot().push_back(annot_it);
                else
                    existing->SetData().SetFtable().insert(existing->SetData().SetFtable().end(),
                        annot_it->SetData().SetFtable().begin(), annot_it->SetData().SetFtable().end());
            }
#ifdef _DEBUG
            else
            {
                cerr << MSerial_AsnText << "Found unmatched annot: " << *annot_id << endl;
            }
            if (false)
            {
                CNcbiOfstream debug_annot("annot.sqn");
                debug_annot << MSerial_AsnText
                    << MSerial_VerifyNo
                    << *annot_it;
            }
#endif
        }

        return true;
    }
    return false;
}

CRef<CSeq_entry> CMultiReader::xReadGTF(CNcbiIstream& instream)
{
    int flags = 0;
    flags |= CGtfReader::fGenbankMode;
    flags |= CGtfReader::fAllIdsAsLocal;
    flags |= CGtfReader::fGenerateChildXrefs;

    CGtfReader reader(flags, m_AnnotName, m_AnnotTitle);
    CStreamLineReader lr(instream);
    CRef<CSeq_entry> entry(new CSeq_entry);
    entry->SetSeq();
    reader.ReadSeqAnnots(entry->SetAnnot(), lr, m_context.m_logger);

    x_PostProcessAnnot(*entry);

    return entry;
}

END_NCBI_SCOPE
