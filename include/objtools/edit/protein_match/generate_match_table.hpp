/*  $Id: generate_match_table.hpp 552246 2017-11-30 18:18:23Z foleyjp $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Justin Foley
 */

#ifndef _GENERATE_MATCH_TABLE_HPP_
#define _GENERATE_MATCH_TABLE_HPP_

#include <corelib/ncbistd.hpp>
#include <objects/seq/Annotdesc.hpp>


BEGIN_NCBI_SCOPE

class CObjectIStream;

BEGIN_SCOPE(objects)

class CSeq_annot;
class CSeq_feat;
class CSeq_align;
class CUser_object;
class CSeq_table;
class CScope;
struct SOverwriteIdInfo;
class CMatchIdInfo;
class CUpdateProtIdSelect_Base;

class CMatchTabulate {

public:
    CMatchTabulate(CRef<CScope> db_scope);
    virtual ~CMatchTabulate(void);

    void ReportWildDependents(const string& nuc_accession,
           const list<string>& wild_dependents);

    void OverwriteEntry(const SOverwriteIdInfo& match_info);

    void GenerateMatchTable(
        const list<CMatchIdInfo>& match_id_info,
        CObjectIStream& align_istr,
        CObjectIStream& annot_istr);

    void WriteTable(CNcbiOstream& out) const;;

    static void WriteTable(const CSeq_table& table,
        CNcbiOstream& out);

    TSeqPos GetNum_rows(void) const;

    struct SProtMatchInfo {
        string nuc_accession;
        string prot_accession;
        string local_id;
        bool same;
    };

    class CProtMatchInfo {

    public:
        CProtMatchInfo() : m_IsSetMatchType(false) {}


        bool IsSetUpdateProductId(void) const {
            return m_UpdateProductId.NotNull();
        }

        CRef<CSeq_id> GetUpdateProductId(void) const {
            return m_UpdateProductId;
        }

        void SetUpdateProductId(CRef<CSeq_id> product_id) {
            m_UpdateProductId = product_id;
        }

        void SetUpdateProductId(const CSeq_id& product_id) {
            if (m_UpdateProductId.Empty()) {
                m_UpdateProductId = Ref(new CSeq_id());
            }
            m_UpdateProductId->Assign(product_id);
        }

        bool IsSetAccession(void) const {
            return !NStr::IsBlank(m_Accession);
        }

        void SetAccession(const string& accession) {
            m_Accession = accession;
        }

        const string& GetAccession(void) const {
            return m_Accession;
        } 

        bool IsSetOtherId(void) const {
            return !NStr::IsBlank(m_OtherId);
        }

        void SetOtherId(const string& other_id) {
            m_OtherId = other_id;
        }

        const string& GetOtherId(void) const {
            return m_OtherId;
        }

        bool IsSetMatchType(void) const {
            return m_IsSetMatchType;
        }

        void SetExactMatch(bool exact) {
            m_IsExactMatch = exact;
            m_IsSetMatchType = true;
        }

        bool IsExactMatch(void) const {
            if (!m_IsSetMatchType) {
                // throw an exception - the match type is unknown
            }
            return m_IsExactMatch;
        }

    private:
        string m_Accession;
        string m_OtherId;
        bool m_IsExactMatch;
        bool m_IsSetMatchType;
        CRef<CSeq_id> m_UpdateProductId;
    }; // CProtMatchInfo

    using TProtMatchMap = map<string, list<CProtMatchInfo>>;

private:

    bool x_GetMatch(const CSeq_annot& annot,
        string& nuc_accession,
        CProtMatchInfo& match_info);

    void x_ProcessAnnots(
        CObjectIStream& annot_istr,
        TProtMatchMap& match_map);

    void x_InitMatchTable(void);

    void x_AppendNucleotide(
        const string& accession,
        const string& status);

    void x_AppendNucleotide(
        const string& accession,
        const list<string>& replaced_accessions);

    void x_AppendMatchedProtein(
        const string& nuc_accession,
        const SProtMatchInfo& prot_match_info);

    void x_AppendMatchedProtein(
        const string& nuc_accession,
        const CProtMatchInfo& prot_match_info);

    void x_AppendMatchedProtein(
        const CMatchIdInfo& id_info,
        const CProtMatchInfo& prot_match_info);

    void x_AppendUnchangedProtein(
        const string& nuc_accession,
        const string& prot_accession);

    void x_AppendUnchangedProtein(
        const string& nuc_accession,
        const string& prot_accession,
        const list<string>& replaced_prot_accessions);

    void x_AppendProtein(
        const string& nuc_accession,
        const string& prot_accession,
        const string& status);

    void x_AppendNewProtein(
        const string& nuc_accession,
        const string& local_id);

    void x_AppendDeadProtein(
        const string& nuc_accession,
        const string& prot_accession);

    void x_ProcessAlignments(CObjectIStream& istr,
        map<string, bool>& nucseq_changed);

    bool x_IsPerfectAlignment(const CSeq_align& align) const;

    void x_ReportDeadDBEntry(const CMatchIdInfo& id_info);

    bool x_IsCdsComparison(const CSeq_annot& annot) const;
    bool x_IsGoodGloballyReciprocalBest(const CSeq_annot& annot) const;
    bool x_IsGoodGloballyReciprocalBest(const CUser_object& user_obj) const;
    CAnnotdesc::TName x_GetComparisonClass(const CSeq_annot& annot) const;
    const CSeq_feat& x_GetQuery(const CSeq_annot& compare_annot) const;
    const CSeq_feat& x_GetSubject(const CSeq_annot& compare_annot) const;

    bool x_FetchAccession(const CSeq_align& align, string& accession);

    bool x_FetchLocalId(const CSeq_align& align, string& local_id);

    void x_AddColumn(const string& colName);
    void x_AppendColumnValue(const string& colName,
        const string& colVal);
    bool x_HasCdsQuery(const CSeq_annot& annot) const;
    bool x_HasCdsSubject(const CSeq_annot& annot) const;
    bool x_IsComparison(const CSeq_annot& annot) const;
    string x_GetSubjectNucleotideAccession(const CSeq_annot& compare_annot);

    string x_GetAccession(const CSeq_feat& seq_feat) const;
    string x_GetAccessionVersion(const CSeq_feat& seq_feat) const;
    string x_GetAccessionVersion(const CUser_object& user_obj) const;
    string x_GetGeneralOrLocalID(const CSeq_feat& seq_feat) const;
    CRef<CSeq_id> x_GetProductId(const CSeq_feat& seq_feat) const;

    CRef<CSeq_table> mMatchTable;
    map<string, size_t> mColnameToIndex;
    bool mMatchTableInitialized;

    CRef<CScope> m_DBScope;
    unique_ptr<CUpdateProtIdSelect_Base> m_pUpdateProtIdSelect;
};



class CUpdateProtIdSelect_Base {
public:
    virtual ~CUpdateProtIdSelect_Base(void) = default;
    virtual CRef<CSeq_id> GetBestId(const list<CRef<CSeq_id>>& prot_id) = 0;
};


class CUpdateProtIdSelect : public CUpdateProtIdSelect_Base  {
public:
    CRef<CSeq_id> GetBestId(const list<CRef<CSeq_id>>& prot_id) override;
};


void g_ReadSeqTable(CNcbiIstream& in, CSeq_table& table);

END_SCOPE(objects)
END_NCBI_SCOPE

#endif
