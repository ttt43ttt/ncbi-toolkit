/*  $Id: gff2_data.hpp 575510 2018-11-29 19:38:03Z ivanov $
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
 * Author: Frank Ludwig
 *
 * File Description:
 *   GFF3 transient data structures
 *
 */

#ifndef OBJTOOLS_READERS___GFF2DATA__HPP
#define OBJTOOLS_READERS___GFF2DATA__HPP

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE // namespace ncbi::objects::

//  ----------------------------------------------------------------------------
class CGff2Record
//  ----------------------------------------------------------------------------
{
public:
    typedef CCdregion::EFrame TFrame;
    typedef map<string, string> TAttributes;
    typedef TAttributes::iterator TAttrIt;
    typedef TAttributes::const_iterator TAttrCit;
    using SeqIdResolver = CRef<CSeq_id> (*)(const string&, unsigned int, bool);


public:
    CGff2Record();
    CGff2Record(
            const CGff2Record& rhs):
        m_strId(rhs.m_strId),
        m_uSeqStart(rhs.m_uSeqStart),
        m_uSeqStop(rhs.m_uSeqStop),
        m_strSource(rhs.m_strSource),
        m_strType(rhs.m_strType),
        m_pdScore(nullptr),
        m_peStrand(nullptr),
        m_pePhase(nullptr)
    {
        if (rhs.m_pdScore) {
            m_pdScore = new double(rhs.Score());
        }
        if (rhs.m_peStrand) {
            m_peStrand = new ENa_strand(rhs.Strand());
        }
        if (rhs.m_pePhase) {
            m_pePhase = new TFrame(rhs.Phase());
        }
        m_Attributes.insert(rhs.m_Attributes.begin(), rhs.m_Attributes.end());
    };

    virtual ~CGff2Record();

    static unsigned int NextId();
    static void ResetId();

    //
    //  Input/output:
    //
    virtual bool AssignFromGff(
        const string& );

    //
    // Accessors:
    //        
    const string& Id() const { 
        return m_strId; 
    };
    size_t SeqStart() const { 
        return m_uSeqStart; 
    };
    size_t SeqStop() const { 
        return m_uSeqStop; 
    };
    const string& Source() const {
        return m_strSource; 
    };
    const string& Type() const {
        return m_strType; 
    };
    double Score() const { 
        return IsSetScore() ? *m_pdScore : 0.0; 
    };
    ENa_strand Strand() const { 
        return IsSetStrand() ? *m_peStrand : eNa_strand_unknown; 
    };
    TFrame Phase() const {
        return IsSetPhase() ? *m_pePhase : CCdregion::eFrame_not_set; 
    };

    bool IsSetScore() const { 
        return m_pdScore != 0; 
    };
    bool IsSetStrand() const { 
        return m_peStrand != 0; 
    };
    bool IsSetPhase() const { 
        return m_pePhase != 0; 
    };
    bool IsAlignmentRecord() const {
        if (NStr::StartsWith(Type(), "match") ||
            NStr::EndsWith(Type(), "_match")) {
            return true;
        }
        return false;
    };
    CRef<CSeq_id> GetSeqId(
        int,
        SeqIdResolver = nullptr ) const;
    CRef<CSeq_loc> GetSeqLoc(
        int,
        SeqIdResolver seqidresolve = nullptr) const;

    const TAttributes& Attributes() const { 
        return m_Attributes; 
    };

    bool GetAttribute(
        const string&,
        string& ) const;

    bool GetAttribute(
        const string&,
        list<string>& ) const;

    virtual bool InitializeFeature(
        int,
        CRef<CSeq_feat>,
        SeqIdResolver = nullptr ) const; 

    virtual bool UpdateFeature(
        int,
        CRef<CSeq_feat>,
        SeqIdResolver = nullptr ) const;

    static void TokenizeGFF(
        vector<CTempStringEx>& columns, 
        const CTempStringEx& line);

protected:
    virtual bool xAssignAttributesFromGff(
        const string&,
        const string& );

	bool xSplitGffAttributes(
		const string&,
		vector< string >& ) const;

    virtual bool xMigrateId(
        CRef<CSeq_feat> ) const;

    virtual bool xMigrateStartStopStrand(
        CRef<CSeq_feat> ) const;

    virtual bool xMigrateType(
        CRef<CSeq_feat> ) const;

    virtual bool xMigrateScore(
        CRef<CSeq_feat> ) const;

    virtual bool xMigratePhase(
        CRef<CSeq_feat> ) const;

    virtual bool xMigrateAttributes(
        int,
        CRef<CSeq_feat> ) const;

    virtual bool xInitFeatureLocation(
        int,
        CRef<CSeq_feat>,
        SeqIdResolver = nullptr ) const;

    virtual bool xInitFeatureData(
        int,
        CRef<CSeq_feat>) const;

    virtual bool xUpdateFeatureData(
        int,
        CRef<CSeq_feat>,
        SeqIdResolver = nullptr ) const;

    virtual bool xMigrateAttributesSubSource(
        int,
        CRef<CSeq_feat>,
        TAttributes& ) const;

    virtual bool xMigrateAttributesOrgName(
        int,
        CRef<CSeq_feat>,
        TAttributes& ) const;

    //utility helpers:
    //
    static string xNormalizedAttributeKey(
        const CTempString&);

    static string xNormalizedAttributeValue(
        const CTempString&);

    static bool xMigrateAttributeDefault(
        TAttributes&,
        const string&,
        CRef<CSeq_feat>,
        const string&,
        int);

    static bool xMigrateAttributeSingle(
        TAttributes&,
        const string&,
        CRef<CSeq_feat>,
        const string&,
        int);

    //
    // Data:
    //
    string m_strId;
    size_t m_uSeqStart;
    size_t m_uSeqStop;
    string m_strSource;
    string m_strType;
    double* m_pdScore;
    ENa_strand* m_peStrand;
    TFrame* m_pePhase;
    string m_strAttributes;    
    TAttributes m_Attributes;
    static unsigned int m_nextId;
};

END_objects_SCOPE
END_NCBI_SCOPE

#endif // OBJTOOLS_READERS___GFF2DATA__HPP
