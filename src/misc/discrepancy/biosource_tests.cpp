/*  $Id: biosource_tests.cpp 569684 2018-08-27 18:34:03Z ivanov $
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
 * Authors: Colleen Bollin, based on similar discrepancy tests
 *
 */

#include <ncbi_pch.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/general/Dbtag.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_vector.hpp>
#include <objects/macro/Source_qual.hpp>
#include <objects/taxon3/taxon3.hpp>
#include <util/xregexp/regexp.hpp>

#include "discrepancy_core.hpp"

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(NDiscrepancy)
USING_SCOPE(objects);

DISCREPANCY_MODULE(biosource_tests);

static unsigned int AutofixBiosrc(TReportObjectList& list, CScope& scope, bool (*call)(CBioSource& src))
{
    unsigned int n = 0;
    NON_CONST_ITERATE (TReportObjectList, it, list) {
        if ((*it)->CanAutofix()) {
            const CSeq_feat* sf = dynamic_cast<const CSeq_feat*>(dynamic_cast<CDiscrepancyObject*>((*it).GetNCPointer())->GetObject().GetPointer());
            const CSeqdesc* csd = dynamic_cast<const CSeqdesc*>(dynamic_cast<CDiscrepancyObject*>((*it).GetNCPointer())->GetObject().GetPointer());
            if (sf) {
                if (sf->IsSetData() && sf->GetData().IsBiosrc()) {
                    CRef<CSeq_feat> new_feat(new CSeq_feat());
                    new_feat->Assign(*sf);
                    if (call(new_feat->SetData().SetBiosrc())) {
                        CSeq_feat_EditHandle feh(scope.GetSeq_featHandle(*sf));
                        feh.Replace(*new_feat);
                        n++;
                        dynamic_cast<CDiscrepancyObject*>((*it).GetNCPointer())->SetFixed();
                    }
                }
            }
            else if (csd) {
                if (csd->IsSource()) {
                    CSeqdesc* sd = const_cast<CSeqdesc*>(csd);
                    if (call(sd->SetSource())) {
                        n++;
                        dynamic_cast<CDiscrepancyObject*>((*it).GetNCPointer())->SetFixed();
                    }
                }
            }
        }
    }
    return n;
}


// MAP_CHROMOSOME_CONFLICT

const string kMapChromosomeConflict = "[n] source[s] on eukaryotic sequence[s] [has] map but not chromosome";


DISCREPANCY_CASE(MAP_CHROMOSOME_CONFLICT, CSeqdesc, eDisc | eOncaller | eSmart, "Eukaryotic sequences with a map source qualifier should also have a chromosome source qualifier")
{
    if (!obj.IsSource() || !context.IsEukaryotic() || !obj.GetSource().IsSetSubtype()) {
        return;
    }

    bool has_map = false;
    bool has_chromosome = false;

    ITERATE (CBioSource::TSubtype, it, obj.GetSource().GetSubtype()) {
        if ((*it)->IsSetSubtype()) {
            if ((*it)->GetSubtype() == CSubSource::eSubtype_map) {
                has_map = true;
            } else if ((*it)->GetSubtype() == CSubSource::eSubtype_chromosome) {
                has_chromosome = true;
                break;
            }
        } 
    }

    if (has_map && !has_chromosome) {
        m_Objs[kMapChromosomeConflict].Add(*context.SeqdescObj(obj), false).Fatal();
    }
}


//  ----------------------------------------------------------------------------
DISCREPANCY_SUMMARIZE(MAP_CHROMOSOME_CONFLICT)
//  ----------------------------------------------------------------------------
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// INFLUENZA_DATE_MISMATCH

DISCREPANCY_CASE(INFLUENZA_DATE_MISMATCH, CBioSource, eOncaller, "Influenza Strain/Collection Date Mismatch")
{
    if (!obj.IsSetOrg() || !obj.IsSetSubtype() || !obj.GetOrg().IsSetOrgname() || !obj.GetOrg().GetOrgname().IsSetMod() || !obj.GetOrg().IsSetTaxname() || !NStr::StartsWith(obj.GetOrg().GetTaxname(), "Influenza ")) {
        return;
    }
    int strain_year = 0;
    int collection_year = 0;
    ITERATE (COrgName::TMod, it, obj.GetOrg().GetOrgname().GetMod()) {
        if ((*it)->IsSetSubtype() && (*it)->GetSubtype() == COrgMod::eSubtype_strain) {
            string s = (*it)->GetSubname();
            size_t pos = s.rfind('/');
            if (pos == string::npos) {
                return;
            }
            ++pos;
            while (isspace(s.c_str()[pos])) {
                ++pos;
            }
            size_t len = 0;
            while (isdigit(s.c_str()[pos + len])) {
                len++;
            }
            if (!len) {
                return;
            }
            strain_year = NStr::StringToInt(s.substr(pos, len));
            break;
        }
    }
    ITERATE (CBioSource::TSubtype, it, obj.GetSubtype()) {
        if ((*it)->IsSetSubtype() && (*it)->GetSubtype() == CSubSource::eSubtype_collection_date) {
            try {
                CRef<CDate> date = CSubSource::DateFromCollectionDate((*it)->GetName());
                if (date->IsStd() && date->GetStd().IsSetYear()) {
                    collection_year = date->GetStd().GetYear();
                }
            }
            catch (...) {}
            break;
        }
    }
    if (strain_year != collection_year) {
        m_Objs["[n] influenza strain[s] conflict with collection date"].Add(*context.FeatOrDescObj());
    }
}


DISCREPANCY_SUMMARIZE(INFLUENZA_DATE_MISMATCH)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// INFLUENZA_QUALS

DISCREPANCY_CASE(INFLUENZA_QUALS, CBioSource, eOncaller, "Influenza must have strain, host, isolation_source, country, collection_date")
{
    if (!obj.IsSetOrg() || !obj.GetOrg().IsSetTaxname() || !NStr::StartsWith(obj.GetOrg().GetTaxname(), "Influenza ")) {
        return;
    }
    bool found_strain = false;
    bool found_host = false;
    bool found_country = false;
    bool found_collection_date = false;

    if (obj.IsSetSubtype()) {
        ITERATE (CBioSource::TSubtype, it, obj.GetSubtype()) {
            if ((*it)->IsSetSubtype()) {
                switch ((*it)->GetSubtype()) {
                    case CSubSource::eSubtype_country:
                        found_country = true;
                        break;
                    case CSubSource::eSubtype_collection_date:
                        found_collection_date = true;
                        break;
                }
            }
        }
    }
    if (obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        ITERATE (COrgName::TMod, it, obj.GetOrg().GetOrgname().GetMod()) {
            if ((*it)->IsSetSubtype()) {
                switch ((*it)->GetSubtype()) {
                    case COrgMod::eSubtype_strain:
                        found_strain = true;
                        break;
                    case COrgMod::eSubtype_nat_host:
                        found_host = true;
                        break;
                }
            }
        }
    }
    if (!found_strain) {
        m_Objs["[n] Influenza biosource[s] [does] not have strain"].Add(*context.FeatOrDescObj());
    }
    if (!found_host) {
        m_Objs["[n] Influenza biosource[s] [does] not have host"].Add(*context.FeatOrDescObj());
    }
    if (!found_country) {
        m_Objs["[n] Influenza biosource[s] [does] not have country"].Add(*context.FeatOrDescObj());
    }
    if (!found_collection_date) {
        m_Objs["[n] Influenza biosource[s] [does] not have collection-date"].Add(*context.FeatOrDescObj());
    }
}


DISCREPANCY_SUMMARIZE(INFLUENZA_QUALS)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// INFLUENZA_SEROTYPE

DISCREPANCY_CASE(INFLUENZA_SEROTYPE, CBioSource, eOncaller, "Influenza A virus must have serotype")
{
    if (!obj.IsSetOrg() || !obj.GetOrg().IsSetTaxname() || !NStr::StartsWith(obj.GetOrg().GetTaxname(), "Influenza A virus ")) {
        return;
    }
    if (obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        ITERATE (COrgName::TMod, it, obj.GetOrg().GetOrgname().GetMod()) {
            if ((*it)->IsSetSubtype() && (*it)->GetSubtype() == COrgMod::eSubtype_serotype) {
                return;
            }
        }
    }
    m_Objs["[n] Influenza A virus biosource[s] [does] not have serotype"].Add(*context.FeatOrDescObj());
}


DISCREPANCY_SUMMARIZE(INFLUENZA_SEROTYPE)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// INFLUENZA_SEROTYPE_FORMAT

DISCREPANCY_CASE(INFLUENZA_SEROTYPE_FORMAT, CBioSource, eOncaller, "Influenza A virus serotype must match /^H[1-9]\\d*$|^N[1-9]\\d*$|^H[1-9]\\d*N[1-9]\\d*$|^mixed$/")
{
    if (!obj.IsSetOrg() || !obj.GetOrg().IsSetTaxname() || !NStr::StartsWith(obj.GetOrg().GetTaxname(), "Influenza A virus ")) {
        return;
    }
    static CRegexp rx("^H[1-9]\\d*$|^N[1-9]\\d*$|^H[1-9]\\d*N[1-9]\\d*$|^mixed$");
    if (obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        ITERATE (COrgName::TMod, it, obj.GetOrg().GetOrgname().GetMod()) {
            if ((*it)->IsSetSubtype() && (*it)->GetSubtype() == COrgMod::eSubtype_serotype && !rx.IsMatch((*it)->GetSubname())) {
                m_Objs["[n] Influenza A virus serotype[s] [has] incorrect format"].Add(*context.FeatOrDescObj());
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(INFLUENZA_SEROTYPE_FORMAT)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// UNCULTURED_NOTES

static bool HasUnculturedNotes(const CBioSource& src)
{
    if (!src.IsSetSubtype()) {
        return false;
    }
    ITERATE (CBioSource::TSubtype, it, src.GetSubtype()) {
        if ((*it)->IsSetSubtype() &&
            (*it)->GetSubtype() == CSubSource::eSubtype_other &&
            (*it)->IsSetName() &&
            CSubSource::HasCultureNotes((*it)->GetName())) {
            return true;
        }
    }
    return false;
};


const string kUnculturedNotes = "[n] bio-source[s] [has] uncultured note[s]";

DISCREPANCY_CASE(UNCULTURED_NOTES, CBioSource, eOncaller, "Uncultured Notes")
{
    if (HasUnculturedNotes(obj)) {
        m_Objs[kUnculturedNotes].Add(*context.FeatOrDescObj(), false).Fatal();
    }
}


DISCREPANCY_SUMMARIZE(UNCULTURED_NOTES)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// MISSING_VIRAL_QUALS

const string kMissingViralQualsTop = "[n] virus organism[s] [is] missing required qualifiers";
const string kMissingViralQualsCollectionDate = "[n] virus organism[s] [is] missing suggested qualifier collection date";
const string kMissingViralQualsCountry = "[n] virus organism[s] [is] missing suggested qualifier country";
const string kMissingViralQualsSpecificHost = "[n] virus organism[s] [is] missing suggested qualifier specific-host";

DISCREPANCY_CASE(MISSING_VIRAL_QUALS, CBioSource, eOncaller, "Viruses should specify collection-date, country, and specific-host")
{
    if (!CDiscrepancyContext::HasLineage(obj, context.GetLineage(), "Viruses")) {
        return;
    }
    bool has_collection_date = false;
    bool has_country = false;
    bool has_specific_host = false;
    if (obj.IsSetSubtype()) {
        ITERATE (CBioSource::TSubtype, it, obj.GetSubtype()) {
            if ((*it)->IsSetSubtype()) {
                if ((*it)->GetSubtype() == CSubSource::eSubtype_collection_date) {
                    has_collection_date = true;
                } else if ((*it)->GetSubtype() == CSubSource::eSubtype_country) {
                    has_country = true;
                }
                if (has_collection_date && has_country) {
                    break;
                }
            }
        }
    }
    if (obj.IsSetOrg() && obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        ITERATE (COrgName::TMod, it, obj.GetOrg().GetOrgname().GetMod()) {
            if ((*it)->IsSetSubtype() && (*it)->GetSubtype() == COrgMod::eSubtype_nat_host) {
                has_specific_host = true;
            }
        }
    }
    if (!has_collection_date || !has_country || !has_specific_host) {
        if (!has_collection_date) {
            m_Objs[kMissingViralQualsTop][kMissingViralQualsCollectionDate].Ext().Add(*context.FeatOrDescObj(), false);
        }
        if (!has_country) {
            m_Objs[kMissingViralQualsTop][kMissingViralQualsCountry].Ext().Add(*context.FeatOrDescObj(), false);
        }
        if (!has_specific_host) {
            m_Objs[kMissingViralQualsTop][kMissingViralQualsSpecificHost].Ext().Add(*context.FeatOrDescObj(), false);
        }
    }
}


DISCREPANCY_SUMMARIZE(MISSING_VIRAL_QUALS)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// ATCC_CULTURE_CONFLICT
bool HasCultureCollectionForATCCStrain(const COrgName::TMod& mods, const string& strain)
{
    if (NStr::IsBlank(strain)) {
        return true;
    }

    bool found = false;

    ITERATE (COrgName::TMod, m, mods) {
        if ((*m)->IsSetSubtype() &&
            (*m)->GetSubtype() == COrgMod::eSubtype_culture_collection &&
            (*m)->IsSetSubname() &&
            NStr::StartsWith((*m)->GetSubname(), "ATCC:")) {
            string cmp = (*m)->GetSubname().substr(5);
            NStr::TruncateSpacesInPlace(cmp);
            size_t pos = NStr::Find(cmp, ";");
            if (pos != string::npos) {
                cmp = cmp.substr(0, pos);
            }
            if (NStr::Equal(cmp, strain)) {
                found = true;
                break;
            }
        }
    }

    return found;
}


bool HasStrainForATCCCultureCollection(const COrgName::TMod& mods, const string& culture_collection)
{
    if (NStr::IsBlank(culture_collection)) {
        return true;
    }

    bool found = false;

    ITERATE (COrgName::TMod, m, mods) {
        if ((*m)->IsSetSubtype() &&
            (*m)->GetSubtype() == COrgMod::eSubtype_strain &&
            (*m)->IsSetSubname() &&
            NStr::StartsWith((*m)->GetSubname(), "ATCC ")) {
            string cmp = (*m)->GetSubname().substr(5);
            NStr::TruncateSpacesInPlace(cmp);
            size_t pos = NStr::Find(cmp, ";");
            if (pos != string::npos) {
                cmp = cmp.substr(0, pos);
            }
            if (NStr::Equal(cmp, culture_collection)) {
                found = true;
                break;
            }
        }
    }

    return found;
}


const string kATCCCultureConflict = "[n] biosource[s] [has] conflicting ATCC strain and culture collection values";

//  ----------------------------------------------------------------------------
DISCREPANCY_CASE(ATCC_CULTURE_CONFLICT, CBioSource, eDisc | eOncaller, "ATCC strain should also appear in culture collection")
//  ----------------------------------------------------------------------------
{
    if (!obj.IsSetOrg() || !obj.GetOrg().IsSetOrgname() || !obj.GetOrg().GetOrgname().IsSetMod()) {
        return;
    }

    bool report = false;

    ITERATE (COrgName::TMod, m, obj.GetOrg().GetOrgname().GetMod()) {
        if ((*m)->IsSetSubtype() && 
            (*m)->IsSetSubname()) {
            if ((*m)->GetSubtype() == COrgMod::eSubtype_strain &&
                NStr::StartsWith((*m)->GetSubname(), "ATCC ") &&
                !HasCultureCollectionForATCCStrain(obj.GetOrg().GetOrgname().GetMod(), (*m)->GetSubname().substr(5))) {
                report = true;
                break;
            } else if ((*m)->GetSubtype() == COrgMod::eSubtype_culture_collection &&
                NStr::StartsWith((*m)->GetSubname(), "ATCC:") &&
                !HasStrainForATCCCultureCollection(obj.GetOrg().GetOrgname().GetMod(), (*m)->GetSubname().substr(5))) {
                report = true;
                break;
            }
        }
    }
    if (report) {
        m_Objs[kATCCCultureConflict].Add(*context.FeatOrDescObj());
    }
}


//  ----------------------------------------------------------------------------
DISCREPANCY_SUMMARIZE(ATCC_CULTURE_CONFLICT)
//  ----------------------------------------------------------------------------
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


static bool SetCultureCollectionFromStrain(CBioSource& src)
{
    if (!src.IsSetOrg() || !src.GetOrg().IsSetOrgMod() || !src.GetOrg().GetOrgname().IsSetMod()) {
        return false;
    }

    vector<string> add;
    ITERATE (COrgName::TMod, m, src.GetOrg().GetOrgname().GetMod()) {
        if ((*m)->IsSetSubtype() &&
            (*m)->IsSetSubname()) {
            if ((*m)->GetSubtype() == COrgMod::eSubtype_strain &&
                NStr::StartsWith((*m)->GetSubname(), "ATCC ") &&
                !HasCultureCollectionForATCCStrain(src.GetOrg().GetOrgname().GetMod(), (*m)->GetSubname().substr(5))) {
                add.push_back("ATCC:" + (*m)->GetSubname());
            }
        }
    }
    if (!add.empty()) {
        ITERATE (vector<string>, s, add) {
            CRef<COrgMod> m(new COrgMod(COrgMod::eSubtype_culture_collection, *s));
            src.SetOrg().SetOrgname().SetMod().push_back(m);
        }
        return true;
    }
    return false;
}


DISCREPANCY_AUTOFIX(ATCC_CULTURE_CONFLICT)
{
    TReportObjectList list = item->GetDetails();
    unsigned int n = AutofixBiosrc(list, scope, SetCultureCollectionFromStrain);
    return CRef<CAutofixReport>(n ? new CAutofixReport("ATCC_CULTURE_CONFLICT: Set culture collection for [n] source[s]", n) : 0);
}


// BACTERIA_SHOULD_NOT_HAVE_ISOLATE

const string kAmplifiedWithSpeciesSpecificPrimers = "amplified with species-specific primers";

DISCREPANCY_CASE(BACTERIA_SHOULD_NOT_HAVE_ISOLATE, CBioSource, eDisc | eOncaller | eSmart, "Bacterial sources should not have isolate")
{
    // only looking for bacteria and archaea
    if (!CDiscrepancyContext::HasLineage(obj, context.GetLineage(), "Bacteria") && !CDiscrepancyContext::HasLineage(obj, context.GetLineage(), "Archaea")) {
        return;
    }

    bool has_bad_isolate = false;
    bool is_metagenomic = false;
    bool is_env_sample = false;

    if (obj.IsSetSubtype()) {
        for (auto s: obj.GetSubtype()) {
            if (s->IsSetSubtype()) {
                if (s->GetSubtype() == CSubSource::eSubtype_environmental_sample) {
                    is_env_sample = true;
                    if (is_metagenomic && is_env_sample) {
                        return;
                    }
                }
                if (s->GetSubtype() == CSubSource::eSubtype_metagenomic) {
                    is_metagenomic = true;
                    if (is_metagenomic && is_env_sample) {
                        return;
                    }
                }
                if (s->GetSubtype() == CSubSource::eSubtype_other && s->IsSetName() && NStr::Equal(s->GetName(), kAmplifiedWithSpeciesSpecificPrimers)) {
                    return;
                }
            }
        }
    }

    if (obj.IsSetOrg() && obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        for (auto m: obj.GetOrg().GetOrgname().GetMod()) {
            if (m->IsSetSubtype()) {
                if (m->GetSubtype() == CSubSource::eSubtype_other && m->IsSetSubname() && NStr::Equal(m->GetSubname(), kAmplifiedWithSpeciesSpecificPrimers)) {
                    return;
                }
                if (m->GetSubtype() == COrgMod::eSubtype_isolate && m->IsSetSubname() && 
                        !NStr::StartsWith(m->GetSubname(), "DGGE gel band") &&
                        !NStr::StartsWith(m->GetSubname(), "TGGE gel band") &&
                        !NStr::StartsWith(m->GetSubname(), "SSCP gel band")) {
                    has_bad_isolate = true;
                }
            }
        }
    }
    if (has_bad_isolate) {
        m_Objs["[n] bacterial biosource[s] [has] isolate"].Add(*context.FeatOrDescObj());
    }
}


DISCREPANCY_SUMMARIZE(BACTERIA_SHOULD_NOT_HAVE_ISOLATE)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// MAG_SHOULD_NOT_HAVE_STRAIN

DISCREPANCY_CASE(MAG_SHOULD_NOT_HAVE_STRAIN, CBioSource, eDisc | eSmart, "Organism assembled from metagenome reads should not have strain")
{
    if (!CDiscrepancyContext::HasLineage(obj, context.GetLineage(), "Bacteria") && !CDiscrepancyContext::HasLineage(obj, context.GetLineage(), "Archaea")) {
        return;
    }

    bool is_metagenomic = false;
    bool is_env_sample = false;

    if (obj.IsSetSubtype()) {
        for (auto s : obj.GetSubtype()) {
            if (s->IsSetSubtype()) {
                if (s->GetSubtype() == CSubSource::eSubtype_environmental_sample) {
                    is_env_sample = true;
                }
                if (s->GetSubtype() == CSubSource::eSubtype_metagenomic) {
                    is_metagenomic = true;
                }
            }
        }
    }
    if (!is_metagenomic || !is_env_sample) {
        return;
    }

    if (obj.IsSetOrg() && obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        for (auto m : obj.GetOrg().GetOrgname().GetMod()) {
            if (m->IsSetSubtype() && m->GetSubtype() == COrgMod::eSubtype_strain) {
                m_Objs["[n] organism[s] assembled from metagenome [has] strain"].Add(*context.FeatOrDescObj());
                return;
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(MAG_SHOULD_NOT_HAVE_STRAIN)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// MAG_MISSING_ISOLATE

DISCREPANCY_CASE(MAG_MISSING_ISOLATE, CBioSource, eDisc | eSmart, "Organism assembled from metagenome reads should have isolate")
{
    if (!CDiscrepancyContext::HasLineage(obj, context.GetLineage(), "Bacteria") && !CDiscrepancyContext::HasLineage(obj, context.GetLineage(), "Archaea")) {
        return;
    }

    bool is_metagenomic = false;
    bool is_env_sample = false;
    bool has_isolate = false;

    if (obj.IsSetSubtype()) {
        for (auto s : obj.GetSubtype()) {
            if (s->IsSetSubtype()) {
                if (s->GetSubtype() == CSubSource::eSubtype_environmental_sample) {
                    is_env_sample = true;
                }
                if (s->GetSubtype() == CSubSource::eSubtype_metagenomic) {
                    is_metagenomic = true;
                }
            }
        }
    }
    if (!is_metagenomic || !is_env_sample) {
        return;
    }

    if (obj.IsSetOrg() && obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        for (auto m : obj.GetOrg().GetOrgname().GetMod()) {
            if (m->IsSetSubtype() && m->GetSubtype() == COrgMod::eSubtype_isolate) {
                has_isolate = true;
                break;
            }
        }
    }
    if (!has_isolate) {
        m_Objs["[n] organism[s] assembled from metagenome [is] missing isolate"].Add(*context.FeatOrDescObj());
    }
}


DISCREPANCY_SUMMARIZE(MAG_MISSING_ISOLATE)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// MULTISRC

DISCREPANCY_CASE(MULTISRC, CBioSource, eDisc | eOncaller, "Comma or semicolon appears in strain or isolate")
{
    if (!obj.IsSetOrg() || !obj.GetOrg().IsSetOrgname() || !obj.GetOrg().GetOrgname().IsSetMod()) {
        return;
    }
    bool report = false;
    ITERATE (COrgName::TMod, m, obj.GetOrg().GetOrgname().GetMod()) {
        if ((*m)->IsSetSubtype() &&
            ((*m)->GetSubtype() == COrgMod::eSubtype_isolate || (*m)->GetSubtype() == COrgMod::eSubtype_strain) &&
            (*m)->IsSetSubname() &&
            (NStr::Find((*m)->GetSubname(), ",") != string::npos ||
             NStr::Find((*m)->GetSubname(), ";") != string::npos)) {
            report = true;
            break;
        }
    }
    if (report) {
        m_Objs["[n] organism[s] [has] comma or semicolon in strain or isolate"].Add(*context.FeatOrDescObj());
    }
}


DISCREPANCY_SUMMARIZE(MULTISRC)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// MULTIPLE_CULTURE_COLLECTION

DISCREPANCY_CASE(MULTIPLE_CULTURE_COLLECTION, CBioSource, eOncaller, "Multiple culture-collection quals")
{
    if (!obj.IsSetOrg() || !obj.GetOrg().IsSetOrgname() || !obj.GetOrg().GetOrgname().IsSetMod()) {
        return;
    }
    bool found = false;
    ITERATE (COrgName::TMod, m, obj.GetOrg().GetOrgname().GetMod()) {
        if ((*m)->IsSetSubtype() && (*m)->GetSubtype() == COrgMod::eSubtype_culture_collection) {
            if (found) {
                m_Objs["[n] organism[s] [has] multiple culture-collection qualifiers"].Add(*context.FeatOrDescObj());
                return;
            }
            found = true;
        }
    }
}


DISCREPANCY_SUMMARIZE(MULTIPLE_CULTURE_COLLECTION)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// REQUIRED_STRAIN

DISCREPANCY_CASE(REQUIRED_STRAIN, CBioSource, eDisc | eSubmitter | eSmart, "Bacteria should have strain")
{
    // only looking for bacteria and archaea
    if (!CDiscrepancyContext::HasLineage(obj, context.GetLineage(), "Bacteria") && !CDiscrepancyContext::HasLineage(obj, context.GetLineage(), "Archaea")) {
        return;
    }

    bool is_metagenomic = false;
    bool is_env_sample = false;
    if (obj.IsSetSubtype()) {
        for (auto s : obj.GetSubtype()) {
            if (s->IsSetSubtype()) {
                if (s->GetSubtype() == CSubSource::eSubtype_environmental_sample) {
                    is_env_sample = true;
                    if (is_metagenomic && is_env_sample) {
                        return;
                    }
                }
                if (s->GetSubtype() == CSubSource::eSubtype_metagenomic) {
                    is_metagenomic = true;
                    if (is_metagenomic && is_env_sample) {
                        return;
                    }
                }
            }
        }
    }

    if (obj.IsSetOrg() && obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        ITERATE(COrgName::TMod, m, obj.GetOrg().GetOrgname().GetMod()) {
            if ((*m)->IsSetSubtype() && (*m)->GetSubtype() == COrgMod::eSubtype_strain) {
                return;
            }
        }
    }
    m_Objs["[n] biosource[s] [is] missing required strain value"].Add(*context.FeatOrDescObj());
}


DISCREPANCY_SUMMARIZE(REQUIRED_STRAIN)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// STRAIN_CULTURE_COLLECTION_MISMATCH

static bool MatchExceptSpaceColon(const string& a, const string& b)
{
    size_t i = 0;
    size_t j = 0;
    while (i < a.length() && j < b.length()) {
        while (i < a.length() && (a[i] == ':' || a[i] == ' ')) i++;
        while (j < b.length() && (b[j] == ':' || b[j] == ' ')) j++;
        if (i == a.length()) {
            return j == b.length();
        }
        if (j == b.length() || a[i] != b[j]) {
            return false;
        }
        i++;
        j++;
    }
    return true;
}


DISCREPANCY_CASE(STRAIN_CULTURE_COLLECTION_MISMATCH, CBioSource, eOncaller | eSmart, "Strain and culture-collection values conflict")
{
    if (!obj.IsSetOrg() || !obj.GetOrg().IsSetOrgname() || !obj.GetOrg().GetOrgname().IsSetMod()) {
        return;
    }
    const COrgName::TMod& mod = obj.GetOrg().GetOrgname().GetMod();
    bool conflict = false;
    ITERATE (COrgName::TMod, m, mod) {
        if ((*m)->IsSetSubtype() && (*m)->GetSubtype() == COrgMod::eSubtype_strain) {
            COrgName::TMod::const_iterator j = m;
            for (++j; j != mod.end(); ++j) {
                if ((*j)->IsSetSubtype() && (*j)->GetSubtype() == COrgMod::eSubtype_culture_collection) {
                    if(MatchExceptSpaceColon((*m)->GetSubname(), (*j)->GetSubname())) {
                        return;
                    }
                    else {
                        conflict = true;
                    }
                }
            }
        }
    }
    if (conflict) {
        m_Objs["[n] organism[s] [has] conflicting strain and culture-collection values"].Add(*context.FeatOrDescObj());
    }
}


DISCREPANCY_SUMMARIZE(STRAIN_CULTURE_COLLECTION_MISMATCH)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// SP_NOT_UNCULTURED

DISCREPANCY_CASE(SP_NOT_UNCULTURED, CBioSource, eOncaller, "Organism ending in sp. needs tax consult")
{
    if (!obj.IsSetOrg() || !obj.GetOrg().CanGetTaxname()) {
        return;
    }
    const string& s = obj.GetOrg().GetTaxname();
    if (s.length() > 4 && s.substr(s.length() - 4) == " sp." && s.substr(0, 11) != "uncultured ") {
        m_Objs["[n] biosource[s] [has] taxname[s] that end[S] with \' sp.\' but [does] not start with \'uncultured\'"].Add(*context.FeatOrDescObj());
    }
}


DISCREPANCY_SUMMARIZE(SP_NOT_UNCULTURED)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// FIND_STRAND_TRNAS

DISCREPANCY_CASE(FIND_STRAND_TRNAS, COverlappingFeatures, eDisc, "Find tRNAs on the same strand")
{
    const CBioSource* biosrc = context.GetCurrentBiosource();
    if (!biosrc || !biosrc->IsSetGenome()) {
        return;
    }
    CBioSource::EGenome genome = static_cast<CBioSource::EGenome>(biosrc->GetGenome());
    if (genome != CBioSource::eGenome_mitochondrion && genome != CBioSource::eGenome_chloroplast && genome != CBioSource::eGenome_plastid) {
        return;
    }
    bool strand_plus = false;
    bool strand_minus = false;
    for (auto feat : context.FeatTRNAs()) {
        if (feat->GetLocation().GetStrand() == eNa_strand_minus) {
            strand_minus = true;
        }
        else {
            strand_plus = true;
        }
        if (strand_plus && strand_minus) {
            return;
        }
    }
    for (auto feat : context.FeatTRNAs()) {
        m_Objs[strand_plus ? "[n] tRNA[s] on plus strand" : "[n] tRNA[s] on minus strand"].Add(*context.DiscrObj(*feat), false);
    }
}


DISCREPANCY_SUMMARIZE(FIND_STRAND_TRNAS)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// REQUIRED_CLONE

bool HasAmplifiedWithSpeciesSpecificPrimerNote(const CBioSource& src)
{
    if (src.IsSetSubtype()) {
        ITERATE(CBioSource::TSubtype, s, src.GetSubtype()) {
            if ((*s)->IsSetSubtype() && (*s)->GetSubtype() == CSubSource::eSubtype_other &&
                (*s)->IsSetName() && NStr::Equal((*s)->GetName(), kAmplifiedWithSpeciesSpecificPrimers)) {
                return true;
            }
        }
    }

    if (src.IsSetOrg() && src.GetOrg().IsSetOrgname() && src.GetOrg().GetOrgname().IsSetMod()) {
        ITERATE(COrgName::TMod, m, src.GetOrg().GetOrgname().GetMod()) {
            if ((*m)->IsSetSubtype() && (*m)->GetSubtype() == CSubSource::eSubtype_other &&
                (*m)->IsSetSubname() && NStr::Equal((*m)->GetSubname(), kAmplifiedWithSpeciesSpecificPrimers)) {
                return true;
            }
        }
    }

    return false;
}


static bool IsMissingRequiredClone(const CBioSource& biosource)
{
    if (HasAmplifiedWithSpeciesSpecificPrimerNote(biosource)) {
        return false;
    }

    bool needs_clone = biosource.IsSetOrg() && biosource.GetOrg().IsSetTaxname() &&
                       NStr::StartsWith(biosource.GetOrg().GetTaxname(), "uncultured", NStr::eNocase);

    bool has_clone = false;

    if (biosource.IsSetSubtype()) {

        ITERATE (CBioSource::TSubtype, subtype_it, biosource.GetSubtype())
        {
            if ((*subtype_it)->IsSetSubtype()) {

                CSubSource::TSubtype subtype = (*subtype_it)->GetSubtype();

                if (subtype == CSubSource::eSubtype_environmental_sample) {
                    needs_clone = true;
                }
                else if (subtype == CSubSource::eSubtype_clone) {
                    has_clone = true;
                }
            }
        }
    }

    if (needs_clone && !has_clone) {
        
        // look for gel band isolate
        bool has_gel_band_isolate = false;
        if (biosource.IsSetOrg() && biosource.GetOrg().IsSetOrgname() && biosource.GetOrg().GetOrgname().IsSetMod()) {

            ITERATE (COrgName::TMod, mod_it, biosource.GetOrg().GetOrgname().GetMod()) {

                if ((*mod_it)->IsSetSubtype() && (*mod_it)->GetSubtype() == COrgMod::eSubtype_isolate) {
                    if ((*mod_it)->IsSetSubname() && NStr::FindNoCase((*mod_it)->GetSubname(), "gel band") != NPOS) {
                        has_gel_band_isolate = true;
                        break;
                    }
                }
            }
        }

        if (has_gel_band_isolate) {
            needs_clone = false;
        }
    }

    return (needs_clone && !has_clone);
}

const string kMissingRequiredClone = "[n] biosource[s] [is] missing required clone value";

//  ----------------------------------------------------------------------------
DISCREPANCY_CASE(REQUIRED_CLONE, CBioSource, eOncaller, "Uncultured or environmental sources should have clone")
//  ----------------------------------------------------------------------------
{
    if (IsMissingRequiredClone(obj)) {
        m_Objs[kMissingRequiredClone].Add(*context.FeatOrDescObj());
    }
}

//  ----------------------------------------------------------------------------
DISCREPANCY_SUMMARIZE(REQUIRED_CLONE)
//  ----------------------------------------------------------------------------
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// STRAIN_TAXNAME_MISMATCH

DISCREPANCY_CASE(STRAIN_TAXNAME_MISMATCH, CBioSource, eDisc | eOncaller, "BioSources with the same strain should have the same taxname")
{
    if (obj.IsSetOrg() && obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        for (auto om: obj.GetOrg().GetOrgname().GetMod()) {
            if (om->IsSetSubtype() && om->GetSubtype() == COrgMod::eSubtype_strain && om->IsSetSubname()) {
                const string strain = om->GetSubname();
                if (!strain.empty()) {
                    m_Objs[strain][obj.GetOrg().IsSetTaxname() ? obj.GetOrg().GetTaxname() : ""].Add(*context.FeatOrDescObj());
                }
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(STRAIN_TAXNAME_MISMATCH)
{
    CReportNode rep, rep1;
    static const string root = "[n] biosources have strain/taxname conflicts";
    for (auto it: m_Objs.GetMap()) {
        if (it.second->GetMap().size() > 1) {
            for (auto mm: it.second->GetMap()) {
                for (auto obj : mm.second->GetObjects()) {
                    string label = "[n] biosources have strain " + it.first + " but do not have the same taxnames";
                    rep["[n] biosources have strain/taxname conflicts"][label].Ext().Add(*obj);
                    rep1[label].Add(*obj);
                }
            }
        }
    }
    m_ReportItems = rep1.GetMap().size() > 1 ? rep.Export(*this)->GetSubitems() : rep1.Export(*this)->GetSubitems();
}


// SPECVOUCHER_TAXNAME_MISMATCH

DISCREPANCY_CASE(SPECVOUCHER_TAXNAME_MISMATCH, CBioSource, eOncaller | eSmart, "BioSources with the same specimen voucher should have the same taxname")
{
    if (obj.IsSetOrg() && obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        for (auto om: obj.GetOrg().GetOrgname().GetMod()) {
            if (om->IsSetSubtype() && om->GetSubtype() == COrgMod::eSubtype_specimen_voucher && om->IsSetSubname()) {
                const string strain = om->GetSubname();
                if (!strain.empty()) {
                    m_Objs[strain][obj.GetOrg().IsSetTaxname() ? obj.GetOrg().GetTaxname() : ""].Add(*context.FeatOrDescObj());
                }
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(SPECVOUCHER_TAXNAME_MISMATCH)
{
    CReportNode rep, rep1;
    for (auto it: m_Objs.GetMap()) {
        if (it.second->GetMap().size() > 1) {
            for (auto mm: it.second->GetMap()) {
                for (auto obj: mm.second->GetObjects()) {
                    string label = "[n] biosources have specimen voucher " + it.first + " but do not have the same taxnames";
                    rep["[n] biosources have specimen voucher/taxname conflicts"][label].Ext().Add(*obj);
                    rep1[label].Add(*obj);
                }
            }
        }
    }
    m_ReportItems = rep1.GetMap().size() > 1 ? rep.Export(*this)->GetSubitems() : rep1.Export(*this)->GetSubitems();
}


// CULTURE_TAXNAME_MISMATCH

DISCREPANCY_CASE(CULTURE_TAXNAME_MISMATCH, CBioSource, eOncaller, "Test BioSources with the same culture collection but different taxname")
{
    if (obj.IsSetOrg() && obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        for (auto om : obj.GetOrg().GetOrgname().GetMod()) {
            if (om->IsSetSubtype() && om->GetSubtype() == COrgMod::eSubtype_culture_collection && om->IsSetSubname()) {
                const string strain = om->GetSubname();
                if (!strain.empty()) {
                    m_Objs[strain][obj.GetOrg().IsSetTaxname() ? obj.GetOrg().GetTaxname() : ""].Add(*context.FeatOrDescObj());
                }
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(CULTURE_TAXNAME_MISMATCH)
{
    CReportNode rep, rep1;
    for (auto it : m_Objs.GetMap()) {
        if (it.second->GetMap().size() > 1) {
            for (auto mm : it.second->GetMap()) {
                for (auto obj : mm.second->GetObjects()) {
                    string label = "[n] biosources have culture collection " + it.first + " but do not have the same taxnames";
                    rep["[n] biosources have culture collection/taxname conflicts"][label].Ext().Add(*obj);
                    rep1[label].Add(*obj);
                }
            }
        }
    }
    m_ReportItems = rep1.GetMap().size() > 1 ? rep.Export(*this)->GetSubitems() : rep1.Export(*this)->GetSubitems();
}


// BIOMATERIAL_TAXNAME_MISMATCH

DISCREPANCY_CASE(BIOMATERIAL_TAXNAME_MISMATCH, CBioSource, eOncaller | eSmart, "Test BioSources with the same biomaterial but different taxname")
{
    if (obj.IsSetOrg() && obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
        for (auto om : obj.GetOrg().GetOrgname().GetMod()) {
            if (om->IsSetSubtype() && om->GetSubtype() == COrgMod::eSubtype_bio_material && om->IsSetSubname()) {
                const string strain = om->GetSubname();
                if (!strain.empty()) {
                    m_Objs[strain][obj.GetOrg().IsSetTaxname() ? obj.GetOrg().GetTaxname() : ""].Add(*context.FeatOrDescObj());
                }
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(BIOMATERIAL_TAXNAME_MISMATCH)
{
    {
        CReportNode rep, rep1;
        for (auto it : m_Objs.GetMap()) {
            if (it.second->GetMap().size() > 1) {
                for (auto mm : it.second->GetMap()) {
                    for (auto obj : mm.second->GetObjects()) {
                        string label = "[n] biosources have biomaterial " + it.first + " but do not have the same taxnames";
                        rep["[n] biosources have biomaterial/taxname conflicts"][label].Ext().Add(*obj);
                        rep1[label].Add(*obj);
                    }
                }
            }
        }
        m_ReportItems = rep1.GetMap().size() > 1 ? rep.Export(*this)->GetSubitems() : rep1.Export(*this)->GetSubitems();
    }
}


// ORGANELLE_ITS
const string kSuspectITS = "[n] Bioseq[s] [has] suspect rRNA / ITS on organelle";


DISCREPANCY_CASE(ORGANELLE_ITS, CBioSource, eOncaller, "Test Bioseqs for suspect rRNA / ITS on organelle")
{
    if (!obj.IsSetGenome() || !(
            obj.GetGenome() == CBioSource::eGenome_apicoplast ||
            obj.GetGenome() == CBioSource::eGenome_chloroplast ||
            obj.GetGenome() == CBioSource::eGenome_chromatophore ||
            obj.GetGenome() == CBioSource::eGenome_chromoplast ||
            obj.GetGenome() == CBioSource::eGenome_cyanelle ||
            obj.GetGenome() == CBioSource::eGenome_hydrogenosome ||
            obj.GetGenome() == CBioSource::eGenome_kinetoplast ||
            obj.GetGenome() == CBioSource::eGenome_leucoplast ||
            obj.GetGenome() == CBioSource::eGenome_mitochondrion ||
            obj.GetGenome() == CBioSource::eGenome_plastid ||
            obj.GetGenome() == CBioSource::eGenome_proplastid
        )) {
        return;
    }
    CConstRef<CBioseq> bioseq = context.GetCurrentBioseq();
    if (!bioseq || !bioseq->IsSetAnnot()) {
        return;
    }

    const CSeq_annot* annot = nullptr;
    for (auto& annot_it: bioseq->GetAnnot()) {
        if (annot_it->IsFtable()) {
            annot = annot_it;
            break;
        }
    }

    if (annot) {
        for (auto& feat: annot->GetData().GetFtable()) {
            if (feat->IsSetData() && feat->GetData().IsRna()) {
                const CRNA_ref& rna = feat->GetData().GetRna();
                if (rna.IsSetType() && (rna.GetType() == CRNA_ref::eType_rRNA || rna.GetType() == CRNA_ref::eType_miscRNA)) {
                    static vector<string> suspectable_products = {
                        "18S ribosomal RNA",
                        "5.8S ribosomal RNA",
                        "25S ribosomal RNA",
                        "28S ribosomal RNA",
                        "internal transcribed spacer 1",
                        "internal transcribed spacer 2"
                    };
                    const string& product = rna.GetRnaProductName();
                    // The Owls Are Not What They Seem!
                    // if (NStr::FindNoCase(suspectable_products, product) != nullptr) {
                    if (!product.empty()) {
                        for (auto& pattern: suspectable_products) {
                            if (NStr::FindNoCase(product, pattern) != NPOS) {
                                m_Objs[kSuspectITS].Add(*context.BioseqObj());
                                return;
                            }
                        }
                    }
                    if (feat->IsSetComment()) {
                        const string& comment = feat->GetComment();
                        // The Owls Are Not What They Seem!
                        // if (!comment.empty() && NStr::FindNoCase(suspectable_products, comment) != nullptr) {
                        if (!comment.empty()) {
                            for (auto& pattern: suspectable_products) {
                                if (NStr::FindNoCase(comment, pattern) != NPOS) {
                                    m_Objs[kSuspectITS].Add(*context.BioseqObj());
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(ORGANELLE_ITS)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// INCONSISTENT_BIOSOURCE
typedef list<string> TInconsistecyDescriptionList;

template<class T, typename R> class CCompareValues
{
    typedef bool (T::*TIsSetFn)() const;
    typedef int (T::*TGetIntFn)() const;
    typedef const R& (T::*TGetRFn)() const;

public:
    static bool IsEqualInt(const T& first, const T& second, TIsSetFn is_set_fn, TGetIntFn get_fn, int not_set)
    {
        int first_val = (first.*is_set_fn)() ? (first.*get_fn)() : not_set,
            second_val = (second.*is_set_fn)() ? (second.*get_fn)() : not_set;

        return first_val == second_val;
    }

    static bool IsEqualVal(const T& first, const T& second, TIsSetFn is_set_fn, TGetRFn get_fn, const R& empty_val)
    {
        const R& first_val = (first.*is_set_fn)() ? (first.*get_fn)() : empty_val,
               & second_val = (second.*is_set_fn)() ? (second.*get_fn)() : empty_val;

        return first_val == second_val;
    }
};

static bool IsSameSubtype(const CBioSource::TSubtype& first, const CBioSource::TSubtype& second)
{
    if (first.size() != second.size()) {
        return false;
    }

    for (CBioSource::TSubtype::const_iterator it_first = first.cbegin(), it_second = second.cbegin();
         it_first != first.cend();
         ++it_first, ++it_second) {

        if (!CCompareValues<CSubSource, int>::IsEqualInt(**it_first, **it_second, &CSubSource::IsSetSubtype, &CSubSource::GetSubtype, 0)) {
            return false;
        }

        if (!CCompareValues<CSubSource, string>::IsEqualVal(**it_first, **it_second, &CSubSource::IsSetName, &CSubSource::GetName, "")) {
            return false;
        }

        if (!CCompareValues<CSubSource, string>::IsEqualVal(**it_first, **it_second, &CSubSource::IsSetAttrib, &CSubSource::GetAttrib, "")) {
            return false;
        }
    }

    return true;
}

static bool IsSameDb(const COrg_ref::TDb& first, const COrg_ref::TDb& second)
{
    if (first.size() != second.size()) {
        return false;
    }

    for (COrg_ref::TDb::const_iterator it_first = first.cbegin(), it_second = second.cbegin();
         it_first != first.cend();
         ++it_first, ++it_second) {

        if (!(*it_first)->Equals(**it_second)) {
            return false;
        }
    }

    return true;
}

static void GetOrgnameDifferences(const COrgName& first, const COrgName& second, TInconsistecyDescriptionList& diffs)
{
    bool first_name_set = first.IsSetName(),
         second_name_set = second.IsSetName();

    if (first_name_set != second_name_set || (first_name_set && first.GetName().Which() != second.GetName().Which())) {
        diffs.push_back("orgname choices differ");
    }

    if (!CCompareValues<COrgName, int>::IsEqualInt(first, second, &COrgName::IsSetGcode, &COrgName::GetGcode, -1)) {
        diffs.push_back("genetic codes differ");
    }

    if (!CCompareValues<COrgName, int>::IsEqualInt(first, second, &COrgName::IsSetMgcode, &COrgName::GetMgcode, -1)) {
        diffs.push_back("mitochondrial genetic codes differ");
    }

    if (!CCompareValues<COrgName, string>::IsEqualVal(first, second, &COrgName::IsSetAttrib, &COrgName::GetAttrib, "")) {
        diffs.push_back("attributes differ");
    }

    if (!CCompareValues<COrgName, string>::IsEqualVal(first, second, &COrgName::IsSetLineage, &COrgName::GetLineage, "")) {
        diffs.push_back("lineages differ");
    }

    if (!CCompareValues<COrgName, string>::IsEqualVal(first, second, &COrgName::IsSetDiv, &COrgName::GetDiv, "")) {
        diffs.push_back("divisions differ");
    }

    bool first_mod_set = first.IsSetMod(),
         second_mod_set = second.IsSetMod();

    COrgName::TMod::const_iterator it_first, it_second;
    if (first_mod_set) {
        it_first = first.GetMod().cbegin();
    }
    if (second_mod_set) {
        it_second = second.GetMod().cbegin();
    }
    if (first_mod_set && second_mod_set) {
        COrgName::TMod::const_iterator end_first = first.GetMod().cend(),
            end_second = second.GetMod().cend();

        for (; it_first != end_first && it_second != end_second; ++it_first, ++it_second) {

            const string& qual = (*it_first)->IsSetSubtype() ? COrgMod::ENUM_METHOD_NAME(ESubtype)()->FindName((*it_first)->GetSubtype(), true) : "Unknown source qualifier";

            if (!CCompareValues<COrgMod, int>::IsEqualInt(**it_first, **it_second, &COrgMod::IsSetSubtype, &COrgMod::GetSubtype, -1)) {
                diffs.push_back("missing " + qual + " modifier");
            }

            if (!CCompareValues<COrgMod, string>::IsEqualVal(**it_first, **it_second, &COrgMod::IsSetAttrib, &COrgMod::GetAttrib, "")) {
                diffs.push_back(qual + " modifier attrib values differ");
            }

            if (!CCompareValues<COrgMod, string>::IsEqualVal(**it_first, **it_second, &COrgMod::IsSetSubname, &COrgMod::GetSubname, "")) {
                diffs.push_back("different " + qual + " values");
            }
        }

        if (it_first == end_first) {
            first_mod_set = false;
        }
        if (it_second == end_second) {
            second_mod_set = false;
        }
    }

    if (first_mod_set && !second_mod_set) {
        const string& qual = (*it_first)->IsSetSubtype() ? ENUM_METHOD_NAME(ESource_qual)()->FindName((*it_first)->GetSubtype(), true) : "Unknown source qualifier";
        diffs.push_back("missing " + qual + " modifier");
    }
    else if (!first_mod_set && second_mod_set) {
        const string& qual = (*it_second)->IsSetSubtype() ? ENUM_METHOD_NAME(ESource_qual)()->FindName((*it_second)->GetSubtype(), true) : "Unknown source qualifier";
        diffs.push_back("missing " + qual + " modifier");
    }
}

static void GetOrgrefDifferences(const COrg_ref& first_org, const COrg_ref& second_org, TInconsistecyDescriptionList& diffs)
{
    if (!CCompareValues<COrg_ref, string>::IsEqualVal(first_org, second_org, &COrg_ref::IsSetTaxname, &COrg_ref::GetTaxname, "")) {
        diffs.push_back("taxnames differ");
    }

    if (!CCompareValues<COrg_ref, string>::IsEqualVal(first_org, second_org, &COrg_ref::IsSetCommon, &COrg_ref::GetCommon, "")) {
        diffs.push_back("common names differ");
    }

    if (!CCompareValues<COrg_ref, COrg_ref::TSyn>::IsEqualVal(first_org, second_org, &COrg_ref::IsSetSyn, &COrg_ref::GetSyn, COrg_ref::TSyn())) {
        diffs.push_back("synonyms differ");
    }

    bool first_db_set = first_org.IsSetDb(),
         second_db_set = second_org.IsSetDb();

    if (first_db_set != second_db_set || (first_db_set && !IsSameDb(first_org.GetDb(), second_org.GetDb()))) {
        diffs.push_back("dbxrefs differ");
    }

    bool first_orgname_set = first_org.IsSetOrgname(),
         second_orgname_set = second_org.IsSetOrgname();

    if (first_orgname_set != second_orgname_set) {
        diffs.push_back("one Orgname is missing");
    }
    else if (first_orgname_set && second_orgname_set) {
        GetOrgnameDifferences(first_org.GetOrgname(), second_org.GetOrgname(), diffs);
    }
}


static void GetBiosourceDifferences(const CBioSource& first_biosrc, const CBioSource& second_biosrc, TInconsistecyDescriptionList& diffs)
{
    if (!CCompareValues<CBioSource, int>::IsEqualInt(first_biosrc, second_biosrc, &CBioSource::IsSetOrigin, &CBioSource::GetOrigin, CBioSource::eOrigin_unknown)) {
        diffs.push_back("origins differ");
    }

    if (first_biosrc.IsSetIs_focus() != second_biosrc.IsSetIs_focus()) {
        diffs.push_back("focus differs");
    }

    if (!CCompareValues<CBioSource, int>::IsEqualInt(first_biosrc, second_biosrc, &CBioSource::IsSetGenome, &CBioSource::GetGenome, CBioSource::eGenome_unknown)) {
        diffs.push_back("locations differ");
    }

    static const CBioSource::TSubtype empty_subtype;

    const CBioSource::TSubtype& first_subtype = first_biosrc.IsSetSubtype() ? first_biosrc.GetSubtype() : empty_subtype,
                              & second_subtype = second_biosrc.IsSetSubtype() ? second_biosrc.GetSubtype() : empty_subtype;
    if (!IsSameSubtype(first_subtype, second_subtype)) {
        diffs.push_back("subsource qualifiers differ");
    }

    bool first_org_set = first_biosrc.IsSetOrg(),
         second_org_set = second_biosrc.IsSetOrg();

    if (first_org_set != second_org_set) {
        diffs.push_back("one OrgRef is missing");
    }
    else if (first_org_set && second_org_set) {
        GetOrgrefDifferences(first_biosrc.GetOrg(), second_biosrc.GetOrg(), diffs);
    }
}

static const string kBioSource = "BioSource";


DISCREPANCY_CASE(INCONSISTENT_BIOSOURCE, CBioSource, eDisc | eSmart, "Inconsistent BioSource")
{
    CConstRef<CBioseq> bioseq = context.GetCurrentBioseq();
    if (!bioseq->IsNa() || context.GetCurrentSeqdesc().Empty()) {
        return;
    }
    CSeqdesc* seqdesc = const_cast<CSeqdesc*>(context.GetCurrentSeqdesc().GetPointer());
    context.SeqdescObj(*seqdesc);    // this will cache the seqdesc name and allow using CConstRef<CBioseq>(0) in context.NewSeqdescObj()
    m_Objs[kBioSource].Add(*context.BioseqObj(false, seqdesc), true);
}


typedef pair<CRef<CReportObj>, const CSeqdesc*> TBioseqSeqdesc;
typedef list<pair<const CBioSource*, list<TBioseqSeqdesc>>> TItemsByBioSource;

static void GetItemsByBiosource(TReportObjectList& objs, TItemsByBioSource& items)
{
    for (auto obj: objs) {
        CDiscrepancyObject* dobj = dynamic_cast<CDiscrepancyObject*>(&*obj);
        if (dobj) {
            const CSeqdesc* cur_seqdesc = dynamic_cast<const CSeqdesc*>(dobj->GetMoreInfo().GetPointer());
            const CBioSource& cur_biosrc = cur_seqdesc->GetSource();

            bool found = false;
            for (auto& item: items) {
                if (item.first->Equals(cur_biosrc)) {
                    found = true;
                    item.second.push_back(make_pair(CRef<CReportObj>(dobj), cur_seqdesc));
                    break;
                }
            }

            if (!found) {
                items.push_back(make_pair(&cur_biosrc, list<TBioseqSeqdesc>()));
                items.back().second.push_back(make_pair(CRef<CReportObj>(dobj), cur_seqdesc));
            }
        }
    }
}


static const string kInconsistentBiosources = "[n/2] inconsistent contig source[s]";

DISCREPANCY_SUMMARIZE(INCONSISTENT_BIOSOURCE)
{
    if (m_Objs.empty()) {
        return;
    }

    TItemsByBioSource items;
    GetItemsByBiosource(m_Objs[kBioSource].GetObjects(), items);

    if (items.size() > 1) {
        m_Objs.GetMap().erase(kBioSource);
        TItemsByBioSource::iterator first = items.begin(),
                                    second = first;
        ++second;
        TInconsistecyDescriptionList diffs;
        GetBiosourceDifferences(*first->first, *second->first, diffs);
        string diff_str = NStr::Join(diffs, ", ");

        string subtype = kInconsistentBiosources;
        if (!diff_str.empty()) {
            subtype += " (" + diff_str + ")";
        }

        size_t subcat_index = 0;
        static size_t MAX_NUM_LEN = 10;

        for (auto& item: items) {
            string subcat_num = NStr::SizetToString(subcat_index);
            subcat_num = string(MAX_NUM_LEN - subcat_num.size(), '0') + subcat_num;
            string subcat = "[*" + subcat_num + "*][n/2] contig[s] [has] identical sources that do not match another contig source";
            ++subcat_index;
            for (auto bioseq_desc: item.second) {
                m_Objs[subtype][subcat].Add(*context.SeqdescObj(*bioseq_desc.second), false).Ext();
                m_Objs[subtype][subcat].Add(*bioseq_desc.first, false).Ext();
            }
        }
        m_ReportItems = m_Objs.Export(*this)->GetSubitems();
    }
}


// TAX_LOOKUP_MISMATCH
static const string kTaxnameMismatch = "[n] tax name[s] [does] not match taxonomy lookup";
static const string kOgRefs = "OrgRef";


DISCREPANCY_CASE(TAX_LOOKUP_MISMATCH, CBioSource, eDisc, "Find Tax Lookup Mismatches")
{
    if (obj.IsSetOrg()) {
        m_Objs[kOgRefs].Add(*context.FeatOrDescObj(false, const_cast<COrg_ref*>(&obj.GetOrg())));
    }
}

static void GetOrgRefs(TReportObjectList& objs, vector<CRef<COrg_ref>>& org_refs)

{
    for (auto obj: objs) {
        CDiscrepancyObject* dobj = dynamic_cast<CDiscrepancyObject*>(obj.GetPointer());
        if (dobj) {
            const COrg_ref* org_ref = dynamic_cast<const COrg_ref*>(dobj->GetMoreInfo().GetPointer());
            CRef<COrg_ref> new_org_ref(new COrg_ref);
            new_org_ref->Assign(*org_ref);
            org_refs.push_back(new_org_ref);
        }
    }
}


static const CDbtag* GetTaxonTag(const COrg_ref& org)
{
    const CDbtag* ret = nullptr;
    if (org.IsSetDb()) {
        ITERATE (COrg_ref::TDb, db, org.GetDb()) {
            if ((*db)->IsSetDb() && NStr::EqualNocase((*db)->GetDb(), "taxon")) {
                ret = *db;
                break;
            }
        }
    }

    return ret;
}


static bool IsOrgDiffers(const COrg_ref& first, const COrg_ref& second)
{
    bool first_set = first.IsSetTaxname(),
         second_set = second.IsSetTaxname();
    if (first_set != second_set || (first_set && first.GetTaxname() != second.GetTaxname())) {
        return true;
    }

    const CDbtag* first_db_tag = GetTaxonTag(first);
    const CDbtag* second_db_tag = GetTaxonTag(second);

    if (first_db_tag == nullptr || second_db_tag == nullptr) {
        return true;
    }

    return !first_db_tag->Equals(*second_db_tag);
}


static CRef<CTaxon3_reply> GetOrgRefs(vector<CRef<COrg_ref>>& orgs)
{
    CTaxon3 taxon3;
    taxon3.Init();
    CRef<CTaxon3_reply> reply = taxon3.SendOrgRefList(orgs);
    return reply;
}


static void GetMismatchOrMissingOrgRefReport(CDiscrepancyContext& context, CReportNode& objs_node, const string& subitem, bool is_mismatch)
{
    vector<CRef<COrg_ref>> org_refs;
    GetOrgRefs(objs_node[kOgRefs].GetObjects(), org_refs);

    if (!org_refs.empty()) {

        CRef<CTaxon3_reply> reply = GetOrgRefs(org_refs);
        if (reply) {

            vector<CRef<COrg_ref>>::const_iterator org_ref = org_refs.begin();

            TReportObjectList& objs = objs_node[kOgRefs].GetObjects();
            TReportObjectList::iterator obj_it = objs.begin();

            for (auto item: reply->GetReply()) {

                bool report = false;
                if (is_mismatch) {
                    report = item->IsData() && IsOrgDiffers(**org_ref, item->GetData().GetOrg());
                }
                else {
                    report = !item->IsData() || item->IsError();
                }

                if (report) {
                    objs_node[subitem].Add(**obj_it);
                }
                ++org_ref;
                ++obj_it;
            }
        }
    }
    objs_node.GetMap().erase(kOgRefs);
}


DISCREPANCY_SUMMARIZE(TAX_LOOKUP_MISMATCH)
{
    if (m_Objs.empty()) {
        return;
    }
    GetMismatchOrMissingOrgRefReport(context, m_Objs, kTaxnameMismatch, true);
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// TAX_LOOKUP_MISSING
static const string kTaxlookupMissing = "[n] tax name[s] [is] missing in taxonomy lookup";

DISCREPANCY_CASE(TAX_LOOKUP_MISSING, CBioSource, eDisc, "Find Missing Tax Lookup")
{
    if (obj.IsSetOrg()) {
        m_Objs[kOgRefs].Add(*context.FeatOrDescObj(false, const_cast<COrg_ref*>(&obj.GetOrg())));
    }
}


DISCREPANCY_SUMMARIZE(TAX_LOOKUP_MISSING)
{
    if (m_Objs.empty()) {
        return;
    }

    GetMismatchOrMissingOrgRefReport(context, m_Objs, kTaxlookupMissing, false);
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// UNNECESSARY_ENVIRONMENTAL

DISCREPANCY_CASE(UNNECESSARY_ENVIRONMENTAL, CBioSource, eOncaller, "Unnecessary environmental qualifier present")
{
    if (!obj.IsSetSubtype()) {
        return;
    }
    bool found = false;
    ITERATE (CBioSource::TSubtype, subtype, obj.GetSubtype()) {
        if ((*subtype)->IsSetSubtype()) {
            CSubSource::TSubtype st = (*subtype)->GetSubtype();
            if (st == CSubSource::eSubtype_metagenomic) {
                return;
            }
            else if (st == CSubSource::eSubtype_other && NStr::FindNoCase((*subtype)->GetName(), "amplified with species-specific primers") != NPOS) {
                return;
            }
            else if (st == CSubSource::eSubtype_environmental_sample) {
                found = true;
            }
        }
    }
    if (!found) {
        return;
    }
    if (obj.IsSetOrg()) {
        if (obj.GetOrg().IsSetTaxname()) {
            const string& s = obj.GetOrg().GetTaxname();
            if (NStr::FindNoCase(s, "uncultured") != NPOS
                    || NStr::FindNoCase(s, "enrichment culture") != NPOS || NStr::FindNoCase(s, "metagenome") != NPOS
                    || NStr::FindNoCase(s, "environmental") != NPOS || NStr::FindNoCase(s, "unidentified") != NPOS) {
                return;
            }
        }
        if (obj.GetOrg().IsSetOrgname() && obj.GetOrg().GetOrgname().IsSetMod()) {
            ITERATE (COrgName::TMod, it, obj.GetOrg().GetOrgname().GetMod()) {
                if ((*it)->IsSetSubtype() && (*it)->GetSubtype() == COrgMod::eSubtype_other
                        && (*it)->IsSetSubname() && NStr::FindNoCase((*it)->GetSubname(), "amplified with species-specific primers") != NPOS) {
                    return;
                }
            }
        }
    }
    m_Objs["[n] biosource[s] [has] unnecessary environmental qualifier"].Add(*context.FeatOrDescObj());
}


DISCREPANCY_SUMMARIZE(UNNECESSARY_ENVIRONMENTAL)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}

// END_COLON_IN_COUNTRY

DISCREPANCY_CASE(END_COLON_IN_COUNTRY, CBioSource, eOncaller, "Country name end with colon")
{
    if (!obj.IsSetSubtype()) {
        return;
    }
    ITERATE (CBioSource::TSubtype, subtype, obj.GetSubtype()) {
        if ((*subtype)->IsSetSubtype() && (*subtype)->GetSubtype() == CSubSource::eSubtype_country) {
            const string& s = (*subtype)->GetName();
            if (s.length() && s[s.length()-1] == ':') {
                m_Objs["[n] country source[s] end[S] with a colon."].Add(*context.FeatOrDescObj(true));
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(END_COLON_IN_COUNTRY)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


static bool RemoveCountryColon(CBioSource& src)
{
    if (!src.IsSetSubtype()) {
        return false;
    }
    bool fixed = false;
    ITERATE (CBioSource::TSubtype, subtype, src.GetSubtype()) {
        if ((*subtype)->IsSetSubtype() && (*subtype)->GetSubtype() == CSubSource::eSubtype_country) {
            CSubSource& ss = const_cast<CSubSource&>(**subtype);
            string& s = ss.SetName();
            while (s.length() && s[s.length()-1] == ':') {
                s.resize(s.length()-1);
                fixed = true;
            }
        }
    }
    return fixed;
}


DISCREPANCY_AUTOFIX(END_COLON_IN_COUNTRY)
{
    TReportObjectList list = item->GetDetails();
    unsigned int n = AutofixBiosrc(list, scope, RemoveCountryColon);
    return CRef<CAutofixReport>(n ? new CAutofixReport("END_COLON_IN_COUNTRY: [n] country name[s] fixed", n) : 0);
}


// COUNTRY_COLON

DISCREPANCY_CASE(COUNTRY_COLON, CBioSource, eOncaller, "Country description should only have 1 colon")
{
    if (!obj.IsSetSubtype()) {
        return;
    }
    ITERATE (CBioSource::TSubtype, subtype, obj.GetSubtype()) {
        if ((*subtype)->IsSetSubtype() && (*subtype)->GetSubtype() == CSubSource::eSubtype_country) {
            const string& s = (*subtype)->GetName();
            int count = 0;
            for (size_t i = 0; i < s.length(); i++) {
                if (s[i] == ':') {
                    count++;
                    if (count > 1) {
                        m_Objs["[n] country source[s] [has] more than 1 colon."].Add(*context.FeatOrDescObj(true));
                        break;
                    }
                }
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(COUNTRY_COLON)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


static bool ChangeCountryColonToComma(CBioSource& src)
{
    if (!src.IsSetSubtype()) {
        return false;
    }
    bool fixed = false;
    ITERATE (CBioSource::TSubtype, subtype, src.GetSubtype()) {
        if ((*subtype)->IsSetSubtype() && (*subtype)->GetSubtype() == CSubSource::eSubtype_country) {
            CSubSource& ss = const_cast<CSubSource&>(**subtype);
            string& s = ss.SetName();
            int count = 0;
            for (size_t i = 0; i < s.length(); i++) {
                if (s[i] == ':') {
                    count++;
                    if (count > 1) {
                        s[i] = ',';
                        fixed = true;
                    }
                }
            }
        }
    }
    return fixed;
}


DISCREPANCY_AUTOFIX(COUNTRY_COLON)
{
    TReportObjectList list = item->GetDetails();
    unsigned int n = AutofixBiosrc(list, scope, ChangeCountryColonToComma);
    return CRef<CAutofixReport>(n ? new CAutofixReport("COUNTRY_COLON: [n] country name[s] fixed", n) : 0);
}


// HUMAN_HOST

DISCREPANCY_CASE(HUMAN_HOST, CBioSource, eDisc | eOncaller, "\'Human\' in host should be \'Homo sapiens\'")
{
    if (!obj.IsSetOrg() || !obj.GetOrg().CanGetOrgname() || !obj.GetOrg().GetOrgname().CanGetMod()) {
        return;
    }
    ITERATE (COrgName::TMod, it, obj.GetOrg().GetOrgname().GetMod()) {
        if ((*it)->CanGetSubtype() && (*it)->GetSubtype() == COrgMod::eSubtype_nat_host && NStr::FindNoCase((*it)->GetSubname(), "human") != NPOS) {
            m_Objs["[n] organism[s] [has] \'human\' host qualifiers"].Add(*context.FeatOrDescObj(true));
        }
    }
}


DISCREPANCY_SUMMARIZE(HUMAN_HOST)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


static bool FixHumanHost(CBioSource& src)
{
    if (!src.IsSetOrg()) {
        return false;
    }
    bool fixed = false;
    ITERATE (COrgName::TMod, it, src.GetOrg().GetOrgname().GetMod()) {
        if ((*it)->CanGetSubtype() && (*it)->GetSubtype() == COrgMod::eSubtype_nat_host && NStr::FindNoCase((*it)->GetSubname(), "human") != NPOS) {
            COrgMod& om = const_cast<COrgMod&>(**it);
            NStr::ReplaceInPlace(om.SetSubname(), "human", "Homo sapiens");
            fixed = true;
        }
    }
    return fixed;
}


DISCREPANCY_AUTOFIX(HUMAN_HOST)
{
    TReportObjectList list = item->GetDetails();
    unsigned int n = AutofixBiosrc(list, scope, FixHumanHost);
    return CRef<CAutofixReport>(n ? new CAutofixReport("HUMAN_HOST: [n] host qualifier[s] fixed", n) : 0);
}


// CHECK_AUTHORITY

DISCREPANCY_CASE(CHECK_AUTHORITY, CBioSource, eDisc | eOncaller, "Authority and Taxname should match first two words")
{
    if (!obj.IsSetOrg() || !obj.GetOrg().CanGetOrgname() || !obj.GetOrg().GetOrgname().CanGetMod() || !obj.GetOrg().CanGetTaxname() || !obj.GetOrg().GetTaxname().length()) {
        return;
    }
    string tax1, tax2;
    ITERATE (COrgName::TMod, it, obj.GetOrg().GetOrgname().GetMod()) {
        if ((*it)->CanGetSubtype() && (*it)->GetSubtype() == COrgMod::eSubtype_authority) {
            if (tax1.empty()) {
                list<CTempString> tmp;
                NStr::Split(obj.GetOrg().GetTaxname(), " ", tmp, NStr::fSplit_Tokenize);
                list<CTempString>::iterator p = tmp.begin();
                if (p != tmp.end()) {
                    tax1 = *p;
                    p++;
                    if (p != tmp.end()) {
                        tax2 = *p;
                    }
                }
            }
            string aut1, aut2;
            list<CTempString> tmp;
            NStr::Split((*it)->GetSubname(), " ", tmp, NStr::fSplit_Tokenize);
            list<CTempString>::iterator p = tmp.begin();
            if (p != tmp.end()) {
                aut1 = *p;
                p++;
                if (p != tmp.end()) {
                    aut2 = *p;
                }
            }
            if (aut1 != tax1 || aut2 != tax2) {
                m_Objs["[n] biosource[s] [has] taxname/authority conflict"].Add(*context.FeatOrDescObj());
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(CHECK_AUTHORITY)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// TRINOMIAL_SHOULD_HAVE_QUALIFIER

static const pair<int, string> srcqual_keywords[] = {
    { COrgMod::eSubtype_forma_specialis, " f. sp." } ,
    { COrgMod::eSubtype_forma, " f." } ,
    { COrgMod::eSubtype_sub_species, " subsp." } ,
    { COrgMod::eSubtype_variety, " var." } ,
    { COrgMod::eSubtype_pathovar, " pv." }
};

static const size_t srcqual_keywords_sz = sizeof(srcqual_keywords) / sizeof(srcqual_keywords[0]);

static string GetSrcQual(const CBioSource& bs, int qual)
{
    if (bs.GetOrg().CanGetOrgname() && bs.GetOrg().GetOrgname().CanGetMod()) {
        ITERATE (COrgName::TMod, it, bs.GetOrg().GetOrgname().GetMod()) {
            if ((*it)->CanGetSubtype() && (*it)->GetSubtype() == qual) {
                return (*it)->GetSubname();
            }
        }
    }
    return kEmptyStr;
}


DISCREPANCY_CASE(TRINOMIAL_SHOULD_HAVE_QUALIFIER, CBioSource, eDisc | eOncaller | eSmart, "Trinomial sources should have corresponding qualifier")
{
    if (!obj.IsSetOrg() || !obj.GetOrg().CanGetTaxname() || !obj.GetOrg().GetTaxname().length() || NStr::FindNoCase(obj.GetOrg().GetTaxname(), " x ") != NPOS 
            || CDiscrepancyContext::HasLineage(obj, context.GetLineage(), "Viruses")) {
        return;
    }
    const string& taxname = obj.GetOrg().GetTaxname();
    for (size_t i = 0; i < srcqual_keywords_sz; i++) {
        size_t n = NStr::FindNoCase(taxname, srcqual_keywords[i].second);
        if (n != NPOS) {
            for (n+= srcqual_keywords[i].second.length(); n < taxname.length(); n++) {
                if (taxname[n] != ' ') {
                    break;
                }
            }
            if (n < taxname.length()) {
                string q = GetSrcQual(obj, srcqual_keywords[i].first);
                string s = taxname.substr(n, q.length());
                if (!q.length() || NStr::CompareNocase(s, q)) {
                    m_Objs["[n] trinomial source[s] lack[S] corresponding qualifier"].Add(*context.FeatOrDescObj());
                }
                break;
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(TRINOMIAL_SHOULD_HAVE_QUALIFIER)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// AMPLIFIED_PRIMERS_NO_ENVIRONMENTAL_SAMPLE

static const string kAmplifiedPrimers = "[n] biosource[s] [has] \'amplified with species-specific primers\' note but no environmental-sample qualifier.";

DISCREPANCY_CASE(AMPLIFIED_PRIMERS_NO_ENVIRONMENTAL_SAMPLE, CBioSource, eOncaller, "Species-specific primers, no environmental sample")
{
    if (obj.HasSubtype(CSubSource::eSubtype_environmental_sample)) {
        return;
    }
    bool has_primer_note = false;
    if (obj.CanGetSubtype()) {
        ITERATE (CBioSource::TSubtype, it, obj.GetSubtype()) {
            if ((*it)->GetSubtype() == CSubSource::eSubtype_other && NStr::FindNoCase((*it)->GetName(), "amplified with species-specific primers") != NPOS) {
                has_primer_note = true;
                break;
            }
        }
    }
    if (!has_primer_note && obj.IsSetOrg() && obj.GetOrg().CanGetOrgname() && obj.GetOrg().GetOrgname().CanGetMod()) {
        ITERATE (COrgName::TMod, it, obj.GetOrg().GetOrgname().GetMod()) {
            if ((*it)->CanGetSubtype() && (*it)->GetSubtype() == COrgMod::eSubtype_other && (*it)->IsSetSubname() && NStr::FindNoCase((*it)->GetSubname(), "amplified with species-specific primers") != NPOS) {
                has_primer_note = true;
                break;
            }
        }
    }
    if (has_primer_note) {
        m_Objs[kAmplifiedPrimers].Add(*context.FeatOrDescObj(true));
    }
}


DISCREPANCY_SUMMARIZE(AMPLIFIED_PRIMERS_NO_ENVIRONMENTAL_SAMPLE)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


static bool SetEnvSampleFixAmplifiedPrimers(CBioSource& src)
{
    bool change = false;
    if (!src.HasSubtype(CSubSource::eSubtype_environmental_sample)) {
        src.SetSubtype().push_back(CRef<CSubSource>(new CSubSource(CSubSource::eSubtype_environmental_sample, " ")));
        change = true;
    }
    NON_CONST_ITERATE(CBioSource::TSubtype, s, src.SetSubtype()) {
        if ((*s)->GetSubtype() == CSubSource::eSubtype_other && (*s)->IsSetName()) {
            string orig = (*s)->GetName();
            NStr::ReplaceInPlace((*s)->SetName(), "[amplified with species-specific primers", "amplified with species-specific primers");
            NStr::ReplaceInPlace((*s)->SetName(), "amplified with species-specific primers]", "amplified with species-specific primers");
            if (!NStr::Equal(orig, (*s)->GetName())) {
                change = true;
                break;
            }
        }
    }

    return change;
}


DISCREPANCY_AUTOFIX(AMPLIFIED_PRIMERS_NO_ENVIRONMENTAL_SAMPLE)
{
    TReportObjectList list = item->GetDetails();
    unsigned int n = AutofixBiosrc(list, scope, SetEnvSampleFixAmplifiedPrimers);
    return CRef<CAutofixReport>(n ? new CAutofixReport("AMPLIFIED_PRIMERS_NO_ENVIRONMENTAL_SAMPLE: Set environmental_sample, fixed amplified primers note for [n] source[s]", n) : 0);
}


// MISSING_PRIMER

DISCREPANCY_CASE(MISSING_PRIMER, CBioSource, eOncaller, "Missing values in primer set")
{
    static const char* msg = "[n] biosource[s] [has] primer set[s] with missing values";
    if (!obj.CanGetPcr_primers() || !obj.GetPcr_primers().CanGet()) {
        return;
    }
    ITERATE (CPCRReactionSet::Tdata, pr, obj.GetPcr_primers().Get()) {
        if ((*pr)->CanGetForward() != (*pr)->CanGetReverse()) {
            m_Objs[msg].Add(*context.FeatOrDescObj());
            return;
        }
        if (!(*pr)->CanGetForward()) {
            continue;
        }
        const CPCRPrimerSet& fwdset = (*pr)->GetForward();
        const CPCRPrimerSet& revset = (*pr)->GetReverse();
        CPCRPrimerSet::Tdata::const_iterator fwd = fwdset.Get().begin();
        CPCRPrimerSet::Tdata::const_iterator rev = revset.Get().begin();
        while (fwd != fwdset.Get().end() && rev != revset.Get().end()) {
            if (((*fwd)->CanGetName() && !(*fwd)->GetName().Get().empty()) != ((*rev)->CanGetName() && !(*rev)->GetName().Get().empty())
                    || ((*fwd)->CanGetSeq() && !(*fwd)->GetSeq().Get().empty()) != ((*rev)->CanGetSeq() && !(*rev)->GetSeq().Get().empty())) {
                m_Objs[msg].Add(*context.FeatOrDescObj());
                return;
            }
            fwd++;
            rev++;
        }
    }
}


DISCREPANCY_SUMMARIZE(MISSING_PRIMER)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// DUPLICATE_PRIMER_SET

static bool EqualPrimerSets(const CPCRPrimerSet::Tdata& a, const CPCRPrimerSet::Tdata& b)
{
    size_t count_a = 0;
    size_t count_b = 0;
    for (CPCRPrimerSet::Tdata::const_iterator it = a.begin(); it != a.end(); it++) {
        count_a++;
    }
    for (CPCRPrimerSet::Tdata::const_iterator jt = b.begin(); jt != b.end(); jt++) {
        count_b++;
    }
    if (count_a != count_b) {
        return false;
    }
    for (CPCRPrimerSet::Tdata::const_iterator it = a.begin(); it != a.end(); it++) {
        CPCRPrimerSet::Tdata::const_iterator jt = b.begin();
        for (; jt != b.end(); jt++) {
            if ((*it)->CanGetName() == (*jt)->CanGetName() && (*it)->CanGetSeq() == (*jt)->CanGetSeq()
                    && (!(*it)->CanGetName() || (*it)->GetName().Get() == (*jt)->GetName().Get())
                    && (!(*it)->CanGetSeq() || (*it)->GetSeq().Get() == (*jt)->GetSeq().Get())) {
                break;
            }
        }
        if (jt == b.end()) {
            return false;
        }
    }
    return true;
}


static bool inline FindDuplicatePrimers(const CPCRReaction& a, const CPCRReaction& b)
{
    return a.CanGetForward() == b.CanGetForward() && a.CanGetReverse() == b.CanGetReverse()
        && (!a.CanGetForward() || EqualPrimerSets(a.GetForward().Get(), b.GetForward().Get()))
        && (!a.CanGetReverse() || EqualPrimerSets(a.GetReverse().Get(), b.GetReverse().Get()));
}


DISCREPANCY_CASE(DUPLICATE_PRIMER_SET, CBioSource, eOncaller, "Duplicate PCR primer pair")
{
    if (!obj.CanGetPcr_primers() || !obj.GetPcr_primers().CanGet()) {
        return;
    }
    const CPCRReactionSet::Tdata data = obj.GetPcr_primers().Get();
    for (CPCRReactionSet::Tdata::const_iterator it = data.begin(); it != data.end(); it++) {
        CPCRReactionSet::Tdata::const_iterator jt = it;
        for (jt++; jt != data.end(); jt++) {
            if (FindDuplicatePrimers(**it, **jt)) {
                m_Objs["[n] BioSource[s] [has] duplicate primer pairs."].Add(*context.FeatOrDescObj(true));
                return;
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(DUPLICATE_PRIMER_SET)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


static bool FixDuplicatePrimerSet(CBioSource& src)
{
    if (!src.CanGetPcr_primers() || !src.GetPcr_primers().CanGet()) {
        return false;
    }
    bool fixed = false;
    // todo
    return fixed;
}


DISCREPANCY_AUTOFIX(DUPLICATE_PRIMER_SET)
{
    TReportObjectList list = item->GetDetails();
    unsigned int n = AutofixBiosrc(list, scope, FixDuplicatePrimerSet);
    return CRef<CAutofixReport>(n ? new CAutofixReport("DUPLICATE_PRIMER_SET: [n] PCR primer set[s] removed", n) : 0);
}


// METAGENOMIC

DISCREPANCY_CASE(METAGENOMIC, CBioSource, eDisc | eOncaller | eSmart, "Source has metagenomic qualifier")
{
    if (obj.CanGetSubtype()) {
        ITERATE (CBioSource::TSubtype, it, obj.GetSubtype()) {
            if ((*it)->GetSubtype() == CSubSource::eSubtype_metagenomic) {
                m_Objs["[n] biosource[s] [has] metagenomic qualifier"].Add(*context.FeatOrDescObj());
                return;
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(METAGENOMIC)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// METAGENOME_SOURCE

DISCREPANCY_CASE(METAGENOME_SOURCE, CBioSource, eDisc | eOncaller | eSmart, "Source has metagenome_source qualifier")
{
    if (obj.IsSetOrg() && obj.GetOrg().CanGetOrgname() && obj.GetOrg().GetOrgname().CanGetMod() && obj.GetOrg().IsSetTaxname() && !obj.GetOrg().GetTaxname().empty()) {
        ITERATE (COrgName::TMod, it, obj.GetOrg().GetOrgname().GetMod()) {
            if ((*it)->CanGetSubtype() && (*it)->GetSubtype() == COrgMod::eSubtype_metagenome_source) {
                m_Objs["[n] biosource[s] [has] metagenome_source qualifier"].Add(*context.FeatOrDescObj());
                return;
            }
        }
    }
}


DISCREPANCY_SUMMARIZE(METAGENOME_SOURCE)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


// DUP_SRC_QUAL

static string GetOrgModName(const COrgMod& qual)
{
    const COrgMod::TSubtype& subtype = qual.GetSubtype();
    return subtype == COrgMod::eSubtype_other ? "note-orgmod" : subtype == COrgMod::eSubtype_nat_host ? "host" : qual.GetSubtypeName(subtype, COrgMod::eVocabulary_raw);
}


static string GetSubtypeName(const CSubSource& qual)
{
    const CSubSource::TSubtype& subtype = qual.GetSubtype();
    return subtype == CSubSource::eSubtype_other ? "note-subsrc" : qual.GetSubtypeName(subtype, CSubSource::eVocabulary_raw);
}


static const char* kDupSrc = "[n] source[s] [has] two or more qualifiers with the same value";


DISCREPANCY_CASE(DUP_SRC_QUAL, CBioSource, eDisc | eOncaller | eSmart, "Each qualifier on a source should have different value")
{
    map<string, vector<string> > Map;
    string collected_by;
    string identified_by;
    if (obj.CanGetSubtype()) {
        for (auto& it: obj.GetSubtype()) {
            if (it->CanGetName()) {
                const string& s = it->GetName();
                if (it->CanGetSubtype()) {
                    if (it->GetSubtype() == CSubSource::eSubtype_collected_by) {
                        collected_by = s;
                    }
                    else if (it->GetSubtype() == CSubSource::eSubtype_identified_by) {
                        identified_by = s;
                    }
                }
                if (!s.empty()) {
                    Map[s].push_back(GetSubtypeName(*it));
                }
            }
        }
    }
    if (obj.IsSetOrg() && obj.GetOrg().CanGetOrgname() && obj.GetOrg().GetOrgname().CanGetMod()) {
        for (auto& it: obj.GetOrg().GetOrgname().GetMod()) {
            if (it->IsSetSubname()) {
                const string& s = it->GetSubname();
                if (it->CanGetSubtype()) {
                    if (it->GetSubtype() == COrgMod::eSubtype_anamorph || it->GetSubtype() == COrgMod::eSubtype_common ||
                        it->GetSubtype() == COrgMod::eSubtype_old_name || it->GetSubtype() == COrgMod::eSubtype_old_lineage ||
                        it->GetSubtype() == COrgMod::eSubtype_gb_acronym || it->GetSubtype() == COrgMod::eSubtype_gb_anamorph ||
                        it->GetSubtype() == COrgMod::eSubtype_gb_synonym) {
                        continue;
                    }
                }
                if (!s.empty()) {
                    Map[s].push_back(GetOrgModName(*it));
                }
            }
        }
    }
    if (obj.IsSetOrg() && obj.GetOrg().CanGetTaxname()) {
        const string& s = obj.GetOrg().GetTaxname();
        if (!s.empty()) {
            Map[s].push_back("organism");
        }
    }
    if (obj.CanGetPcr_primers()) {
        for (auto& it: obj.GetPcr_primers().Get()) {
            if (it->CanGetForward()) {
                for (auto& pr: it->GetForward().Get()) {
                    if (pr->CanGetName()) {
                        Map[pr->GetName()].push_back("fwd-primer-name");
                    }
                    if (pr->CanGetSeq()) {
                        Map[pr->GetSeq()].push_back("fwd-primer-seq");
                    }
                }
            }
            if (it->CanGetReverse()) {
                for (auto& pr: it->GetReverse().Get()) {
                    if (pr->CanGetName()) {
                        Map[pr->GetName()].push_back("rev-primer-name");
                    }
                    if (pr->CanGetSeq()) {
                        Map[pr->GetSeq()].push_back("rev-primer-seq");
                    }
                }
            }
        }
    }
    bool bad = false;
    for (auto& it: Map) {
        if (it.second.size() > 1) {
            if (it.second.size() == 2 && it.first == collected_by && collected_by == identified_by) {
                continue; // there is no error if collected_by equals to identified_by
            }
            string s = "[n] biosource[s] [has] value\'";
            s += it.first;
            s += "\' for these qualifiers: ";
            for (size_t i = 0; i < it.second.size(); i++) {
                if (i) {
                    s += ", ";
                }
                s += it.second[i];
            }
            m_Objs[kDupSrc][s].Add(*context.FeatOrDescObj());
        }
    }
    if (bad) {
        m_Objs[kDupSrc].Incr();
    }
}


DISCREPANCY_SUMMARIZE(DUP_SRC_QUAL)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


DISCREPANCY_ALIAS(DUP_SRC_QUAL, DUP_SRC_QUAL_DATA)


// UNUSUAL_ITS
const string kUnusualITS = "[n] Bioseq[s] [has] unusual rRNA / ITS";

DISCREPANCY_CASE(UNUSUAL_ITS, CBioSource, eDisc | eOncaller, "Test Bioseqs for unusual rRNA / ITS")
{
    if (!context.HasLineage(obj, "", "Microsporidia")) {
        return;
    }

    CConstRef<CBioseq> bioseq = context.GetCurrentBioseq();
    if (!bioseq || !bioseq->IsSetAnnot()) {
        return;
    }

    const CSeq_annot* annot = nullptr;
    ITERATE(CBioseq::TAnnot, annot_it, bioseq->GetAnnot()) {
        if ((*annot_it)->IsFtable()) {
            annot = *annot_it;
            break;
        }
    }

    bool has_unusual = false;

    if (annot) {
        ITERATE(CSeq_annot::TData::TFtable, feat, annot->GetData().GetFtable()) {
            if ((*feat)->IsSetComment() && (*feat)->IsSetData() && (*feat)->GetData().IsRna()) {
                const CRNA_ref& rna = (*feat)->GetData().GetRna();
                if (rna.IsSetType() && rna.GetType() == CRNA_ref::eType_miscRNA) {
                    if (NStr::StartsWith((*feat)->GetComment(), "contains", NStr::eNocase)) {
                        has_unusual = true;
                        break;
                    }
                }
            }
        }
    }

    if (has_unusual) {
        m_Objs[kUnusualITS].Add(*context.BioseqObj(), false);
    }
}


DISCREPANCY_SUMMARIZE(UNUSUAL_ITS)
{
    m_ReportItems = m_Objs.Export(*this)->GetSubitems();
}


END_SCOPE(NDiscrepancy)
END_NCBI_SCOPE
