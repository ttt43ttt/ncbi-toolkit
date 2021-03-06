/*  $Id: discrepancy.hpp 575301 2018-11-27 15:24:59Z ivanov $
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
 * Authors:  Sema
 * Created:  01/29/2015
 */

#ifndef _MISC_DISCREPANCY_DISCREPANCY_H_
#define _MISC_DISCREPANCY_DISCREPANCY_H_

#include <serial/iterator.hpp>
#include <corelib/ncbistd.hpp>
#include <serial/serialbase.hpp>
#include <objmgr/scope.hpp>
#include <objects/macro/Suspect_rule.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(NDiscrepancy)

class NCBI_DISCREPANCY_EXPORT CReportObj : public CObject
{
public:
    enum EType {
        eType_feature,
        eType_descriptor,
        eType_sequence,
        eType_seq_set,
        eType_submit_block,
        eType_string
    };
    virtual ~CReportObj(void){}
    virtual const string& GetText(void) const = 0;
    virtual const string& GetFeatureType() const = 0;
    virtual const string& GetProductName() const = 0;
    virtual const string& GetLocation(void) const = 0;
    virtual const string& GetLocusTag() const = 0;
    virtual const string& GetShort(void) const = 0;
    virtual size_t GetFileID(void) const = 0;
    virtual EType GetType(void) const = 0;
    virtual CConstRef<CSerialObject> GetObject(void) const = 0;
    virtual bool CanAutofix(void) const = 0;
};
typedef vector<CRef<CReportObj> > TReportObjectList;


class NCBI_DISCREPANCY_EXPORT CAutofixReport : public CObject
{
public:
    CAutofixReport(const string&s, unsigned int n = 0) : S(s), N(n) {}
    void AddSubitems(const vector<CRef<CAutofixReport>>& v) { copy(v.begin(), v.end(), back_inserter(V)); }
    string GetS(void) { return S; }
    unsigned int GetN(void) { return N; }
    const vector<CRef<CAutofixReport>>& GetSubitems() { return V; }
protected:
    string S;
    unsigned int N;
    vector<CRef<CAutofixReport>> V;
};
typedef string(*TAutofixHook)(void*);


class NCBI_DISCREPANCY_EXPORT CReportItem : public CObject
{
public:
    enum ESeverity {
        eSeverity_info    = 0,
        eSeverity_warning = 1,
        eSeverity_error   = 2
    };
    virtual ~CReportItem(void){}
    virtual string GetTitle(void) const = 0;
    virtual string GetStr(void) const = 0;
    virtual string GetMsg(void) const = 0;
    virtual string GetXml(void) const = 0;
    virtual string GetUnit(void) const = 0;
    virtual size_t GetCount(void) const = 0;
    virtual TReportObjectList GetDetails(void) const = 0;
    virtual vector<CRef<CReportItem> > GetSubitems(void) const = 0;
    virtual bool CanAutofix(void) const = 0;
    virtual ESeverity GetSeverity(void) const = 0;
    virtual bool IsFatal(void) const = 0;
    virtual bool IsInfo(void) const = 0;
    virtual bool IsExtended(void) const = 0;
    virtual bool IsSummary(void) const = 0;
    virtual bool IsReal(void) const = 0;
    virtual CRef<CAutofixReport> Autofix(objects::CScope&) const = 0;
    static CRef<CReportItem> CreateReportItem(const string& test, const string& msg, bool autofix = false);
    virtual void PushReportObj(CReportObj& obj) = 0;
};
typedef vector<CRef<CReportItem> > TReportItemList;


class NCBI_DISCREPANCY_EXPORT CDiscrepancyCase : public CObject
{
public:
    virtual ~CDiscrepancyCase(void){}
    virtual string GetName(void) const = 0;
    virtual string GetType(void) const = 0;
    virtual TReportItemList GetReport(void) const = 0;
    virtual TReportObjectList GetObjects(void) const = 0;
};
typedef map<string, CRef<CDiscrepancyCase> > TDiscrepancyCaseMap;

class NCBI_DISCREPANCY_EXPORT CDiscrepancySet : public CObject
{
public:
    CDiscrepancySet(void) : m_SesameStreetCutoff(0.75), m_Eucariote(false), m_Gui(false), m_KeepRef(false), m_UserData(0) {}
    virtual ~CDiscrepancySet(void){}

    template<typename Container>
    bool AddTests(const Container& cont)
    {
        bool success = true;
        for_each(cont.begin(), cont.end(), [this, &success](const string& test_name) { success &= this->AddTest(test_name); });
        return success;
    }

    virtual bool AddTest(const string& name) = 0;
    virtual bool SetAutofixHook(const string& name, TAutofixHook func) = 0;
    virtual void Parse(const CSerialObject& root) = 0;
    virtual void TestString(const string& str) = 0;
    virtual void Summarize(void) = 0;
    virtual void AutofixAll(void) = 0;
    virtual const TDiscrepancyCaseMap& GetTests(void) = 0;
    virtual void OutputText(CNcbiOstream& out, bool fatal = false, bool summary = false, bool ext = false, char group = 0) = 0;
    virtual void OutputXML(CNcbiOstream& out, bool ext = false) = 0;

    bool IsGui(void) const { return m_Gui;}
    const string& GetFile(void) const { return m_Files[m_File];}
    const string& GetLineage(void) const { return m_Lineage; }
    float GetSesameStreetCutoff(void) const { return m_SesameStreetCutoff; }
    bool GetKeepRef(void) const { return m_KeepRef; }
    void* GetUserData(void) const { return m_UserData; }
    void SetFile(const string& s){ m_Files.push_back(s); m_File = m_Files.size() - 1; }
    void SetLineage(const string& s){ m_Lineage = s; }
    void SetEucariote(bool b){ m_Eucariote = b; }
    void SetSesameStreetCutoff(float f){ m_SesameStreetCutoff = f; }
    virtual void SetSuspectRules(const string& name, bool read = true) = 0;
    void SetGui(bool b){ m_Gui = b; }
    void SetKeepRef(bool b){ m_KeepRef = b; }
    void SetUserData(void* p){ m_UserData = p; }
    static CRef<CDiscrepancySet> New(objects::CScope& scope);
    static string Format(const string& str, unsigned int count);
    static const char** GetTestSuiteKClark();

protected:
    size_t m_File;
    vector<string> m_Files;
    string m_Lineage;
    float m_SesameStreetCutoff;
    bool m_Eucariote;
    bool m_Gui;
    bool m_KeepRef;     // set true to allow autofix
    void* m_UserData;
};


class NCBI_DISCREPANCY_EXPORT CDiscrepancyGroup : public CObject
{
public:
    CDiscrepancyGroup(const string& name = "", const string& test = "") : m_Name(name), m_Test(test) {}
    void Add(CRef<CDiscrepancyGroup> child) { m_List.push_back(child); }
    TReportItemList Collect(TDiscrepancyCaseMap& tests, bool all = true) const;
    const CDiscrepancyGroup& operator[](size_t n) const { return *m_List[n]; }

protected:
    string m_Name;
    string m_Test;
    vector<CRef<CDiscrepancyGroup> > m_List;
};


enum EGroup {
    eNone = 0,
    eDisc = 1,
    eOncaller = 2,
    eSubmitter = 4,
    eSmart = 8,
    eBig = 16,
    eAutofix = 64
};
typedef unsigned short TGroup;


struct CAutofixHookRegularArguments // (*TAutofixHook)(void*) can accept any other input if needed
{
    void* m_User;
    string m_Title;
    string m_Message;
    vector<string> m_List;
};


NCBI_DISCREPANCY_EXPORT string GetDiscrepancyCaseName(const string&);
NCBI_DISCREPANCY_EXPORT string GetDiscrepancyDescr(const string&);
NCBI_DISCREPANCY_EXPORT TGroup GetDiscrepancyGroup(const string&);
NCBI_DISCREPANCY_EXPORT vector<string> GetDiscrepancyNames(TGroup group = 0);
NCBI_DISCREPANCY_EXPORT vector<string> GetDiscrepancyAliases(const string&);
NCBI_DISCREPANCY_EXPORT bool IsShortrRNA(const objects::CSeq_feat& f, objects::CScope* scope);

typedef std::function < CRef<objects::CSeq_feat>() > GetFeatureFunc;
NCBI_DISCREPANCY_EXPORT string FixProductName(const objects::CSuspect_rule* rule, objects::CScope& scope, string& prot_name, GetFeatureFunc get_mrna, GetFeatureFunc get_cds);


END_SCOPE(NDiscrepancy)
END_NCBI_SCOPE

#endif  // _MISC_DISCREPANCY_DISCREPANCY_H_
