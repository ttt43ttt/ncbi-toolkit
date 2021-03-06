/*  $Id: test_bam_loader.cpp 558961 2018-03-05 16:59:33Z vasilche $
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
* Author:  Eugene Vasilchenko
*
* File Description:
*   Unit tests for SRA data loader
*
* ===========================================================================
*/
#define NCBI_TEST_APPLICATION
#include <ncbi_pch.hpp>
#include <sra/data_loaders/bam/bamloader.hpp>
#include <sra/readers/ncbi_traces_path.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/graph_ci.hpp>
#include <objtools/readers/idmapper.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <objects/seqalign/seqalign__.hpp>
#include <corelib/ncbi_system.hpp>
#include <thread>

#include <corelib/test_boost.hpp>
#include <common/test_data_path.h>
#include <common/test_assert.h>  /* This header must go last */


USING_NCBI_SCOPE;
USING_SCOPE(objects);

#define PILEUP_NAME_SUFFIX " pileup graphs"

CRef<CObjectManager> sx_GetOM(void)
{
    SetDiagPostLevel(eDiag_Info);
    CRef<CObjectManager> om = CObjectManager::GetInstance();
    CObjectManager::TRegisteredNames names;
    om->GetRegisteredNames(names);
    ITERATE ( CObjectManager::TRegisteredNames, it, names ) {
        om->RevokeDataLoader(*it);
    }
    return om;
}

void sx_CheckNames(CScope& scope, const CSeq_loc& loc,
                   const string& name, bool more_names = false)
{
    SAnnotSelector sel;
    sel.SetSearchUnresolved();
    sel.SetCollectNames();
    CAnnotTypes_CI it(CSeq_annot::C_Data::e_not_set, scope, loc, &sel);
    CAnnotTypes_CI::TAnnotNames names = it.GetAnnotNames();
    ITERATE ( CAnnotTypes_CI::TAnnotNames, i, names ) {
        if ( i->IsNamed() ) {
            NcbiCout << "Named annot: " << i->GetName()
                     << NcbiEndl;
        }
        else {
            NcbiCout << "Unnamed annot"
                     << NcbiEndl;
        }
    }
    //NcbiCout << "Checking for name: " << name << NcbiEndl;
    if ( !more_names ) {
        BOOST_REQUIRE_EQUAL(names.size(), 1u);
        BOOST_CHECK_EQUAL(names.begin()->GetName(), name);
    }
    else {
        BOOST_CHECK(names.find(CAnnotName(name)) != names.end());
    }
}

string sx_GetPath(const string& dir, const string& path = "traces02:traces04")
{
    vector<string> reps;
    NStr::Split(path, ":", reps);
    ITERATE ( vector<string>, it, reps ) {
        string path = CFile::MakePath(CFile::MakePath(NCBI_GetTestDataPath(), *it), dir);
        if ( CDirEntry(path).Exists() ) {
            return path;
        }
    }
    return dir;
}

void sx_ReportBamLoaderName(const string& name)
{
    NcbiCout << "BAM loader: " << name << NcbiEndl;
    CDataLoader* loader = CObjectManager::GetInstance()->FindDataLoader(name);
    CBAMDataLoader* bam_loader = dynamic_cast<CBAMDataLoader*>(loader);
    BOOST_CHECK(bam_loader);
    CBAMDataLoader::TAnnotNames names = bam_loader->GetPossibleAnnotNames();
    ITERATE ( CBAMDataLoader::TAnnotNames, it, names ) {
        NcbiCout << "  annot name: " << it->GetName() << NcbiEndl;
    }
}

void sx_CheckSeq(CScope& scope,
                 const CSeq_id_Handle& main_idh,
                 const CSeq_id& id)
{
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);
    if ( idh == main_idh ) {
        return;
    }
    //ERR_POST("Checking: "<<idh);
    const CBioseq_Handle::TId& ids = scope.GetIds(idh);
    BOOST_CHECK_EQUAL(ids.size(), 1u);
    BOOST_CHECK_EQUAL(ids[0], idh);
    CBioseq_Handle bh = scope.GetBioseqHandle(idh);
    BOOST_CHECK(bh);
}

BOOST_AUTO_TEST_CASE(FetchSeq1)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(false);

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name, id;
    TSeqPos from, to, align_count;

    {
        params.m_DirPath =
            sx_GetPath("/1kg_pilot_data/ftp/pilot_data/data/NA10851/alignment");
        //             "/1kg_pilot_data/data/NA19371/alignment");
        bam_name = "NA10851.SLX.maq.SRP000031.2009_08.bam";
        //bam_name = "NA19371.454.MOSAIK.SRP000033.2009_11.bam";
        id = "NT_113960";
        //id = "NC_000001.9";
        from = 0;
        to = 100000;
        align_count = 397; // 6455 with zero quality;
    }
    params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName(bam_name));

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    CScope scope(*om);
    scope.AddDefaults();

    CRef<CSeq_id> seqid(new CSeq_id(id));
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
    CRef<CSeq_loc> loc(new CSeq_loc);
    loc->SetInt().SetId(*seqid);
    loc->SetInt().SetFrom(from);
    loc->SetInt().SetTo(to);
    sx_CheckNames(scope, *loc, CDirEntry(bam_name).GetBase());
    SAnnotSelector sel(CSeq_annot::C_Data::e_Align);
    sel.SetSearchUnresolved();
    CAlign_CI it(scope, *loc, sel);
    if ( it ) {
        cout << "Align count: "<<it.GetSize()<<endl;
        if ( it.GetAnnot().IsNamed() ) {
            cout << "Annot name: " << it.GetAnnot().GetName()<<endl;
        }
    }
    BOOST_CHECK_EQUAL(align_count, it.GetSize());

    for ( ; it; ++it ) {
        const CSeq_align& align = *it;
        ITERATE( CDense_seg::TIds, j, align.GetSegs().GetDenseg().GetIds() ) {
            sx_CheckSeq(scope, idh, **j);
        }
    }
}

BOOST_AUTO_TEST_CASE(FetchSeq2)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(false);

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name, id;
    TSeqPos from, to, align_count;

    {
        params.m_DirPath = sx_GetPath("/srz0/SRZ/000000/SRZ000200/provisional", "traces01");
        bam_name = "GSM409307_UCSD.H3K4me1.chr1.bam";
        CNcbiIfstream mapfile("mapfile");
        BOOST_REQUIRE(mapfile);
        params.m_IdMapper.reset(new CIdMapperConfig(mapfile, "", false));
        id = "89161185";
        from = 0;
        to = 100000;
        align_count = 792;
    }
    params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName(bam_name));

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    CScope scope(*om);
    scope.AddDefaults();

    CRef<CSeq_id> seqid(new CSeq_id(id));
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
    CRef<CSeq_loc> loc(new CSeq_loc);
    loc->SetInt().SetId(*seqid);
    loc->SetInt().SetFrom(from);
    loc->SetInt().SetTo(to);
    sx_CheckNames(scope, *loc, CDirEntry(bam_name).GetBase());
    SAnnotSelector sel(CSeq_annot::C_Data::e_Align);
    sel.SetSearchUnresolved();
    CAlign_CI it(scope, *loc, sel);
    if ( it ) {
        cout << "Align count: "<<it.GetSize()<<endl;
        if ( it.GetAnnot().IsNamed() ) {
            cout << "Annot name: " << it.GetAnnot().GetName()<<endl;
        }
    }
    BOOST_CHECK_EQUAL(align_count, it.GetSize());

    for ( ; it; ++it ) {
        const CSeq_align& align = *it;
        ITERATE( CDense_seg::TIds, j, align.GetSegs().GetDenseg().GetIds() ) {
            sx_CheckSeq(scope, idh, **j);
        }
    }
}

BOOST_AUTO_TEST_CASE(FetchSeq1_WithGenBank)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(false);

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name, id;
    TSeqPos from, to, align_count;

    {
        params.m_DirPath =
            sx_GetPath("/1kg_pilot_data/ftp/pilot_data/data/NA10851/alignment");
        //             "/1kg_pilot_data/data/NA19371/alignment");
        bam_name = "NA10851.SLX.maq.SRP000031.2009_08.bam";
        //params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName("NA19371.454.MOSAIK.SRP000033.2009_11.bam"));
        id = "NT_113960";
        //id = "NC_000001.9";
        from = 0;
        to = 100000;
        align_count = 397; // 6455 with zero quality;
    }
    params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName(bam_name));

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    string gbloader_name =
        CGBDataLoader::RegisterInObjectManager(*om).GetLoader()->GetName();
    CScope scope(*om);
    scope.AddDefaults();

    CRef<CSeq_id> seqid(new CSeq_id(id));
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
    CRef<CSeq_loc> loc(new CSeq_loc);
    loc->SetInt().SetId(*seqid);
    loc->SetInt().SetFrom(from);
    loc->SetInt().SetTo(to);
    sx_CheckNames(scope, *loc, CDirEntry(bam_name).GetBase(), true);
    SAnnotSelector sel(CSeq_annot::C_Data::e_Align);
    CAlign_CI it(scope, *loc, sel);
    if ( it ) {
        cout << "Align count: "<<it.GetSize()<<endl;
        if ( it.GetAnnot().IsNamed() ) {
            cout << "Annot name: " << it.GetAnnot().GetName()<<endl;
        }
    }
    BOOST_CHECK_EQUAL(align_count, it.GetSize());

    for ( ; it; ++it ) {
        const CSeq_align& align = *it;
        ITERATE( CDense_seg::TIds, j, align.GetSegs().GetDenseg().GetIds() ) {
            sx_CheckSeq(scope, idh, **j);
        }
    }
}

BOOST_AUTO_TEST_CASE(FetchSeqSRZ1)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(false);

    if ( !CDirEntry("/home/vasilche/data/bam").Exists() ) {
        return;
    }

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name, id, annot_name;
    TSeqPos from, to, align_count;
    CSeq_id_Handle main_idh;
    {
        params.m_DirPath = "/home/vasilche/data/bam/tests/SRZ000200";
        annot_name = "GSM409307_UCSD.H3K4me1.sorted";
        id = "89161185";
        main_idh = CSeq_id_Handle::GetHandle("NC_000001.9");
        from = 1100000;
        to   = 1200000;
        align_count = 283;
    }

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    string gbloader_name =
        CGBDataLoader::RegisterInObjectManager(*om).GetLoader()->GetName();
    CScope scope(*om);
    scope.AddDefaults();

    CRef<CSeq_id> seqid(new CSeq_id(id));
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
    if ( !main_idh ) {
        main_idh = idh;
    }
    CRef<CSeq_loc> loc(new CSeq_loc);
    loc->SetInt().SetId(*seqid);
    loc->SetInt().SetFrom(from);
    loc->SetInt().SetTo(to);
    if ( annot_name.empty() ) {
        annot_name = CDirEntry(bam_name).GetBase();
    }
    sx_CheckNames(scope, *loc, annot_name, true);

    SAnnotSelector sel;
    sel.SetSearchUnresolved();

    CGraph_CI git(scope, *loc, sel);
    BOOST_CHECK_EQUAL(git.GetSize(), 1u);

    CAlign_CI it(scope, *loc, sel);
    if ( it ) {
        cout << "Align count: "<<it.GetSize()<<endl;
        if ( it.GetAnnot().IsNamed() ) {
            cout << "Annot name: " << it.GetAnnot().GetName()<<endl;
        }
    }
    BOOST_CHECK_EQUAL(align_count, it.GetSize());

    for ( ; it; ++it ) {
        const CSeq_align& align = *it;
        ITERATE( CDense_seg::TIds, j, align.GetSegs().GetDenseg().GetIds() ) {
            sx_CheckSeq(scope, main_idh, **j);
        }
    }
}

BOOST_AUTO_TEST_CASE(FetchSeqSRZ2)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(false);

    if ( !CDirEntry("/home/vasilche/data/bam").Exists() ) {
        return;
    }

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name, id, annot_name;
    TSeqPos from, to, align_count;

    {
        params.m_DirPath = "/home/vasilche/data/bam/tests/SRZ000200";
        annot_name = "GSM409307_UCSD.H3K4me1.sorted";
        id = "NC_000001.9";
        from = 1100000;
        to   = 1200000;
        align_count = 283;
    }

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    CScope scope(*om);
    scope.AddDefaults();

    CRef<CSeq_id> seqid(new CSeq_id(id));
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
    CRef<CSeq_loc> loc(new CSeq_loc);
    loc->SetInt().SetId(*seqid);
    loc->SetInt().SetFrom(from);
    loc->SetInt().SetTo(to);
    if ( annot_name.empty() ) {
        annot_name = CDirEntry(bam_name).GetBase();
    }
    sx_CheckNames(scope, *loc, annot_name);
    SAnnotSelector sel;
    sel.SetSearchUnresolved();

    CGraph_CI git(scope, *loc, sel);
    BOOST_CHECK_EQUAL(git.GetSize(), 1u);

    CAlign_CI it(scope, *loc, sel);
    if ( it ) {
        cout << "Align count: "<<it.GetSize()<<endl;
        if ( it.GetAnnot().IsNamed() ) {
            cout << "Annot name: " << it.GetAnnot().GetName()<<endl;
        }
    }
    BOOST_CHECK_EQUAL(align_count, it.GetSize());

    for ( ; it; ++it ) {
        const CSeq_align& align = *it;
        ITERATE( CDense_seg::TIds, j, align.GetSegs().GetDenseg().GetIds() ) {
            sx_CheckSeq(scope, idh, **j);
        }
    }
}

BOOST_AUTO_TEST_CASE(FetchSeqSRZ3)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(false);

    CRef<CObjectManager> om = sx_GetOM();

    string srz_acc;
    string bam_name, id, annot_name;
    TSeqPos from, to, align_count;

    {
        srz_acc = "SRZ000200";
        annot_name = "GSM409307_UCSD.H3K4me1.sorted";
        id = "NC_000001.9";
        from = 1100000;
        to   = 1200000;
        align_count = 283;
    }

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, srz_acc,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    CScope scope(*om);
    scope.AddDataLoader(loader_name);

    CRef<CSeq_id> seqid(new CSeq_id(id));
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
    CRef<CSeq_loc> loc(new CSeq_loc);
    loc->SetInt().SetId(*seqid);
    loc->SetInt().SetFrom(from);
    loc->SetInt().SetTo(to);
    if ( annot_name.empty() ) {
        annot_name = CDirEntry(bam_name).GetBase();
    }
    sx_CheckNames(scope, *loc, annot_name);
    SAnnotSelector sel;
    sel.SetSearchUnresolved();

    CGraph_CI git(scope, *loc, sel);
    BOOST_CHECK_EQUAL(git.GetSize(), 1u);

    CAlign_CI it(scope, *loc, sel);
    if ( it ) {
        cout << "Align count: "<<it.GetSize()<<endl;
        if ( it.GetAnnot().IsNamed() ) {
            cout << "Annot name: " << it.GetAnnot().GetName()<<endl;
        }
    }
    BOOST_CHECK_EQUAL(align_count, it.GetSize());

    for ( ; it; ++it ) {
        const CSeq_align& align = *it;
        ITERATE( CDense_seg::TIds, j, align.GetSegs().GetDenseg().GetIds() ) {
            sx_CheckSeq(scope, idh, **j);
        }
    }
}

BOOST_AUTO_TEST_CASE(FetchSeq4)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(false);

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name, id, annot_name;
    TSeqPos from, to, align_count;

    CSeq_id_Handle main_idh;
    {
        bam_name = "HG00096.chrom20.ILLUMINA.bwa.GBR.low_coverage.20111114.bam";
        params.m_DirPath =
            sx_GetPath("/1000genomes/ftp/data/HG00096/alignment");
        if ( !CDirEntry(params.m_DirPath+"/"+bam_name).Exists() ) {
            params.m_DirPath = 
                sx_GetPath("/1000genomes2/ftp/data/HG00096/alignment");
        }
        if ( !CDirEntry(params.m_DirPath+"/"+bam_name).Exists() ) {
            params.m_DirPath = 
                sx_GetPath("/1000genomes2/ftp/phase1/data/HG00096/alignment");
        }
        if ( !CDirEntry(params.m_DirPath+"/"+bam_name).Exists() ) {
            params.m_DirPath = 
                sx_GetPath("/1000genomes4/ftp/data/HG00096/alignment");
        }
        CNcbiIfstream mapfile("mapfile");
        BOOST_CHECK(mapfile);
        AutoPtr<CIdMapperConfig> mapper
            (new CIdMapperConfig(mapfile, "", false));
        mapper->AddMapping(CSeq_id_Handle::GetHandle(20),
                           main_idh = CSeq_id_Handle::GetHandle(51511747));
        params.m_IdMapper.reset(mapper.release());
        id = "NC_000020.9";
        from = 0;
        to = 100000;
        align_count = 2046; // 6455 with zero quality;
    }
    params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName(bam_name));

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    string gbloader_name =
        CGBDataLoader::RegisterInObjectManager(*om).GetLoader()->GetName();
    CScope scope(*om);
    scope.AddDefaults();
    scope.AddDataLoader(loader_name);

    CRef<CSeq_id> seqid(new CSeq_id(id));
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
    if ( !main_idh ) {
        main_idh = idh;
    }
    CRef<CSeq_loc> loc(new CSeq_loc);
    loc->SetInt().SetId(*seqid);
    loc->SetInt().SetFrom(from);
    loc->SetInt().SetTo(to);
    if ( annot_name.empty() ) {
        annot_name = CDirEntry(bam_name).GetBase();
    }
    sx_CheckNames(scope, *loc, annot_name, true);
    SAnnotSelector sel;
    sel.SetSearchUnresolved();

    CGraph_CI git(scope, *loc, sel);
    BOOST_CHECK(git.GetSize() <= 1u);

    CAlign_CI it(scope, *loc, sel);
    if ( it ) {
        cout << "Align count: "<<it.GetSize()<<endl;
        if ( it.GetAnnot().IsNamed() ) {
            cout << "Annot name: " << it.GetAnnot().GetName()<<endl;
        }
    }
    BOOST_CHECK_EQUAL(align_count, it.GetSize());

    for ( ; it; ++it ) {
        const CSeq_align& align = *it;
        ITERATE( CDense_seg::TIds, j, align.GetSegs().GetDenseg().GetIds() ) {
            sx_CheckSeq(scope, main_idh, **j);
        }
    }
}


BOOST_AUTO_TEST_CASE(FetchSeq5)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(true);

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name, id, annot_name, pileup_name;
    TSeqPos from, to, align_count;

    CSeq_id_Handle main_idh;
    {
        bam_name = "HG00096.chrom20.ILLUMINA.bwa.GBR.low_coverage.20111114.bam";
        params.m_DirPath =
            sx_GetPath("/1000genomes/ftp/data/HG00096/alignment");
        if ( !CDirEntry(params.m_DirPath+"/"+bam_name).Exists() ) {
            params.m_DirPath = 
                sx_GetPath("/1000genomes2/ftp/data/HG00096/alignment");
        }
        if ( !CDirEntry(params.m_DirPath+"/"+bam_name).Exists() ) {
            params.m_DirPath = 
                sx_GetPath("/1000genomes2/ftp/phase1/data/HG00096/alignment");
        }
        if ( !CDirEntry(params.m_DirPath+"/"+bam_name).Exists() ) {
            params.m_DirPath = 
                sx_GetPath("/1000genomes4/ftp/data/HG00096/alignment");
        }
        CNcbiIfstream mapfile("mapfile");
        BOOST_CHECK(mapfile);
        AutoPtr<CIdMapperConfig> mapper
            (new CIdMapperConfig(mapfile, "", false));
        mapper->AddMapping(CSeq_id_Handle::GetHandle(20),
                           main_idh = CSeq_id_Handle::GetHandle(51511747));
        params.m_IdMapper.reset(mapper.release());
        id = "NC_000020.9";
        from = 0;
        to = 100000;
        align_count = 2046; // 6455 with zero quality;
    }
    params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName(bam_name));

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    string gbloader_name =
        CGBDataLoader::RegisterInObjectManager(*om).GetLoader()->GetName();
    CRef<CScope> scope_ref(new CScope(*om));
    CScope& scope = *scope_ref;
    scope.AddDefaults();
    scope.AddDataLoader(loader_name);

    CRef<CSeq_id> seqid(new CSeq_id(id));
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
    if ( !main_idh ) {
        main_idh = idh;
    }

    CRef<CSeq_loc> loc(new CSeq_loc);
    loc->SetInt().SetId(*seqid);
    loc->SetInt().SetFrom(from);
    loc->SetInt().SetTo(to);
    if ( annot_name.empty() ) {
        annot_name = CDirEntry(bam_name).GetBase();
    }
    pileup_name = annot_name+PILEUP_NAME_SUFFIX;
    sx_CheckNames(scope, *loc, annot_name, true);
    SAnnotSelector sel;
    sel.SetSearchUnresolved();

    CGraph_CI git(scope, *loc, sel);
    BOOST_CHECK(git.GetSize() % 5 <= 1u); // no 'match' graph in this file
    BOOST_CHECK(git.GetSize());

    CAlign_CI it(scope, *loc, sel);
    if ( it ) {
        cout << "Align count: "<<it.GetSize()<<endl;
        if ( it.GetAnnot().IsNamed() ) {
            cout << "Annot name: " << it.GetAnnot().GetName()<<endl;
        }
    }
    BOOST_CHECK_EQUAL(align_count, it.GetSize());

    for ( ; it; ++it ) {
        const CSeq_align& align = *it;
        ITERATE( CDense_seg::TIds, j, align.GetSegs().GetDenseg().GetIds() ) {
            sx_CheckSeq(scope, main_idh, **j);
        }
    }

    if ( 1 ) {
        sel.ResetAnnotsNames();
        sel.AddNamedAnnots(pileup_name);
        CGraph_CI git(scope, *loc, sel);
        BOOST_CHECK(git.GetSize() > 0);
        BOOST_CHECK(git.GetSize() % 5 == 0); // no 'match' graph in this file
        if ( 0 ) {
            for ( ; git; ++git ) {
                NcbiCout << MSerial_AsnText << git->GetOriginalGraph();
            }
        }
    }
}


BOOST_AUTO_TEST_CASE(MemoryTest)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(true);

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name, id, annot_name, pileup_name;

    CSeq_id_Handle main_idh;
    {
        bam_name = "HG00096.chrom20.ILLUMINA.bwa.GBR.low_coverage.20111114.bam";
        params.m_DirPath =
            sx_GetPath("/1000genomes/ftp/data/HG00096/alignment");
        if ( !CDirEntry(params.m_DirPath+"/"+bam_name).Exists() ) {
            params.m_DirPath = 
                sx_GetPath("/1000genomes2/ftp/data/HG00096/alignment");
        }
        if ( !CDirEntry(params.m_DirPath+"/"+bam_name).Exists() ) {
            params.m_DirPath = 
                sx_GetPath("/1000genomes2/ftp/phase1/data/HG00096/alignment");
        }
        if ( !CDirEntry(params.m_DirPath+"/"+bam_name).Exists() ) {
            params.m_DirPath = 
                sx_GetPath("/1000genomes4/ftp/data/HG00096/alignment");
        }
        CNcbiIfstream mapfile("mapfile");
        BOOST_CHECK(mapfile);
        AutoPtr<CIdMapperConfig> mapper
            (new CIdMapperConfig(mapfile, "", false));
        mapper->AddMapping(CSeq_id_Handle::GetHandle(20),
                           main_idh = CSeq_id_Handle::GetHandle(51511747));
        params.m_IdMapper.reset(mapper.release());
        id = "NC_000020.9";
    }
    params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName(bam_name));

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    string gbloader_name =
        CGBDataLoader::RegisterInObjectManager(*om).GetLoader()->GetName();
    
    CRef<CScope> scope_ref(new CScope(*om));
    CScope& scope = *scope_ref;
    scope.AddDefaults();
    scope.AddDataLoader(loader_name);

    CRef<CSeq_id> seqid(new CSeq_id(id));
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
    if ( !main_idh ) {
        main_idh = idh;
    }

#ifdef _DEBUG
    const TSeqPos kRangeStep = 100000;
#else
    const TSeqPos kRangeStep = 1000000;
#endif
    const TSeqPos kRangeStart = kRangeStep;
    const TSeqPos kRangeCount = 10;
    Uint8 first_used_memory = 0;
    for ( TSeqPos i = 0; i < kRangeCount; ++i ) {
        TSeqPos from = kRangeStart+i*kRangeStep;
        TSeqPos to = from+kRangeStep;
        CRef<CSeq_loc> loc(new CSeq_loc);
        loc->SetInt().SetId(*seqid);
        loc->SetInt().SetFrom(from);
        loc->SetInt().SetTo(to);
        if ( annot_name.empty() ) {
            annot_name = CDirEntry(bam_name).GetBase();
        }
        pileup_name = annot_name+PILEUP_NAME_SUFFIX;
        SAnnotSelector sel;
        sel.SetSearchUnresolved();
        
        cout << "Scan range "<<from<<"-"<<to<<":"<<endl;
        CTSE_Handle tseh;

        CGraph_CI git(scope, *loc, sel);
        if ( git ) {
            cout << "Loaded "<<git.GetSize()<<" graphs"<<endl;
            tseh = git.GetAnnot().GetTSE_Handle();
        }
        
        CAlign_CI ait(scope, *loc, sel);
        if ( ait ) {
            cout << "Loaded "<<ait.GetSize()<<" alignments"<<endl;
            tseh = ait.GetAnnot().GetTSE_Handle();
            for ( int i = 0; ait && i < 2; ++i, ++ait ) {
                auto& id = ait->GetSeq_id(1);
                LOG_POST("Short read: " << id.AsFastaString());
                auto bsh = scope.GetBioseqHandle(id);
                BOOST_CHECK(bsh);
            }
        }
        
        if ( tseh ) {
            Uint8 used_memory = tseh.GetUsedMemory();
            if ( !first_used_memory ) {
                first_used_memory = used_memory;
            }
            cout << "BAM TSE memory: " << used_memory << endl;
            BOOST_CHECK(used_memory <= 1.5*first_used_memory);
            scope.RemoveFromHistory(tseh);
            BOOST_CHECK(tseh);
            if (i % 2) {
                git = CGraph_CI();
                ait = CAlign_CI();
                scope.RemoveFromHistory(tseh);
                BOOST_CHECK(!tseh);
            }
            else {
                scope.RemoveFromHistory(tseh, CScope::eRemoveIfLocked);
                BOOST_CHECK(!tseh);
            }
        }
    }
}


typedef tuple<string, CRange<TSeqPos>, bool, size_t, size_t> TQuery;
vector<TQuery> s_GetQueries1()
{
    vector<TQuery> queries;
    queries.push_back(make_tuple("NT_113960", CRange<TSeqPos>(0, 100000), true, 4, 5));
    queries.push_back(make_tuple("NT_113960", CRange<TSeqPos>(0, 100000), false, 397, 0));
    queries.push_back(make_tuple("lcl|1", CRange<TSeqPos>(100000, 200000), true, 8, 9));
    queries.push_back(make_tuple("lcl|1", CRange<TSeqPos>(200000, 300000), true, 12, 9));
    queries.push_back(make_tuple("lcl|1", CRange<TSeqPos>(0, 400000), false, 1424, 0));
    queries.push_back(make_tuple("lcl|2", CRange<TSeqPos>(1000000, 2000000), true, 120, 161));
    queries.push_back(make_tuple("lcl|2", CRange<TSeqPos>(2000000, 3000000), true, 120, 155));
    queries.push_back(make_tuple("lcl|2", CRange<TSeqPos>(2700000, 2800000), false, 15816, 0));
    queries.push_back(make_tuple("lcl|2", CRange<TSeqPos>(11000000, 12000000), true, 120, 161));
    queries.push_back(make_tuple("lcl|2", CRange<TSeqPos>(12000000, 13000000), true, 120, 155));
    queries.push_back(make_tuple("lcl|2", CRange<TSeqPos>(11500000, 11600000), false, 15841, 0));
    queries.push_back(make_tuple("lcl|3", CRange<TSeqPos>(21000000, 22000000), true, 134, 149));
    queries.push_back(make_tuple("lcl|3", CRange<TSeqPos>(22000000, 23000000), true, 134, 155));
    queries.push_back(make_tuple("lcl|3", CRange<TSeqPos>(21500000, 21600000), false, 14054, 0));
    queries.push_back(make_tuple("lcl|4", CRange<TSeqPos>(31000000, 32000000), true, 139, 121));
    queries.push_back(make_tuple("lcl|4", CRange<TSeqPos>(42000000, 43000000), true, 145, 156));
    queries.push_back(make_tuple("lcl|4", CRange<TSeqPos>(21500000, 21600000), false, 14936, 0));
    return queries;
}

vector<TQuery> s_GetQueries2()
{
    vector<TQuery> queries;
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(0, 100000), true, 6, 7));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(0, 100000), false, 3, 0));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(100000, 200000), true, 6, 7));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(200000, 300000), true, 6, 7));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(0, 400000), false, 53, 0));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(1000000, 2000000), true, 18, 25));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(2000000, 3000000), true, 12, 19));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(2700000, 2800000), false, 0, 0));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(11000000, 12000000), true, 18, 25));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(12000000, 13000000), true, 12, 13));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(11500000, 11600000), false, 4, 0));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(21000000, 22000000), true, 18, 13));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(22000000, 23000000), true, 17, 13));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(21500000, 21600000), false, 575, 0));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(31000000, 32000000), true, 18, 19));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(42000000, 43000000), true, 12, 25));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(21500000, 21600000), false, 575, 0));
    return queries;
}

vector<TQuery> s_GetQueries2full()
{
    vector<TQuery> queries;
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(0, 248956422), false, 300552, 0));
    queries.push_back(make_tuple("NC_000001.11", CRange<TSeqPos>(0, 248956422), true, 2736, 1639));
    for ( int i = 0; i < 3; ++i ) {
        queries.push_back(queries[0]);
        queries.push_back(queries[1]);
    }
    return queries;
}

BOOST_AUTO_TEST_CASE(FetchSeqST1)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(true);

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name;

    {
        params.m_DirPath =
            sx_GetPath("/1kg_pilot_data/ftp/pilot_data/data/NA10851/alignment");
        bam_name = "NA10851.SLX.maq.SRP000031.2009_08.bam";
    }
    params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName(bam_name));

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    CScope scope(*om);
    scope.AddDefaults();

    vector<TQuery> queries = s_GetQueries1();
    
    const size_t NQ = queries.size();
    
    for ( size_t i = 0; i < NQ; ++i ) {
        const TQuery& query = queries[i];
        SAnnotSelector sel;
        sel.SetSearchUnresolved();
            
        CRef<CSeq_id> seqid(new CSeq_id(get<0>(query)));
        CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
        CRef<CSeq_loc> loc(new CSeq_loc);
        loc->SetInt().SetId(*seqid);
        loc->SetInt().SetFrom(get<1>(query).GetFrom());
        loc->SetInt().SetTo(get<1>(query).GetTo());
        size_t count = 0;
        if ( get<2>(query) ) {
            for ( CGraph_CI it(scope, *loc, sel); it; ++it ) {
                ++count;
            }
        }
        else {
            for ( CAlign_CI it(scope, *loc, sel); it; ++it ) {
                ++count;
            }
        }
        if ( !get<4>(query) || count != get<4>(query) ) {
            BOOST_CHECK_EQUAL(count, get<3>(query));
        }
    }
}


BOOST_AUTO_TEST_CASE(FetchSeqST2)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(true);

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name = sx_GetPath("hs108_sra.fil_sort.chr1.bam", "bam");
    params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName(bam_name));

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    CScope scope(*om);
    scope.AddDefaults();
    
    SAnnotSelector sel;
    sel.SetSearchUnresolved();
    sel.AddNamedAnnots("hs108_sra.fil_sort.chr1");
    sel.AddNamedAnnots("hs108_sra.fil_sort.chr1 pileup graphs");
    
    for ( int pass = 0; pass < 2; ++pass ) {
        vector<TQuery> queries;
        size_t NQ;
        if ( pass == 0 ) {
            queries = s_GetQueries2();
            NQ = queries.size();
        }
        else {
            queries = s_GetQueries2full();
            NQ = min<size_t>(0, queries.size());
        }
    
        for ( size_t i = 0; i < NQ; ++i ) {
            const TQuery& query = queries[i];
            CRef<CSeq_id> seqid(new CSeq_id(get<0>(query)));
            CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
            CRef<CSeq_loc> loc(new CSeq_loc);
            loc->SetInt().SetId(*seqid);
            loc->SetInt().SetFrom(get<1>(query).GetFrom());
            loc->SetInt().SetTo(get<1>(query).GetTo());
            size_t count = 0;
            if ( get<2>(query) ) {
                for ( CGraph_CI it(scope, *loc, sel); it; ++it ) {
                    ++count;
                }
            }
            else {
                for ( CAlign_CI it(scope, *loc, sel); it; ++it ) {
                    ++count;
                }
            }
            if ( !get<4>(query) || count != get<4>(query) ) {
                BOOST_CHECK_EQUAL(count, get<3>(query));
            }
        }
    }
}


#ifdef NCBI_THREADS
BOOST_AUTO_TEST_CASE(FetchSeqMT1)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(true);

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name;

    {
        params.m_DirPath =
            sx_GetPath("/1kg_pilot_data/ftp/pilot_data/data/NA10851/alignment");
        bam_name = "NA10851.SLX.maq.SRP000031.2009_08.bam";
    }
    params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName(bam_name));

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    CScope scope(*om);
    scope.AddDefaults();

    vector<TQuery> queries = s_GetQueries1();
    
    const size_t NQ = queries.size();
    
    vector<thread> tt(NQ);
    for ( size_t i = 0; i < NQ; ++i ) {
        tt[i] =
            thread([&]
                   (const TQuery& query)
                   {
                       SAnnotSelector sel;
                       sel.SetSearchUnresolved();

                       CRef<CSeq_id> seqid(new CSeq_id(get<0>(query)));
                       CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
                       CRef<CSeq_loc> loc(new CSeq_loc);
                       loc->SetInt().SetId(*seqid);
                       loc->SetInt().SetFrom(get<1>(query).GetFrom());
                       loc->SetInt().SetTo(get<1>(query).GetTo());
                       size_t count = 0;
                       if ( get<2>(query) ) {
                           for ( CGraph_CI it(scope, *loc, sel); it; ++it ) {
                               ++count;
                           }
                       }
                       else {
                           for ( CAlign_CI it(scope, *loc, sel); it; ++it ) {
                               ++count;
                           }
                       }
                       if ( !get<4>(query) || count != get<4>(query) ) {
                           BOOST_CHECK_EQUAL(count, get<3>(query));
                       }
                   }, queries[i]);
    }
    for ( size_t i = 0; i < NQ; ++i ) {
        tt[i].join();
    }
}


BOOST_AUTO_TEST_CASE(FetchSeqMT2)
{
    CBAMDataLoader::SetPileupGraphsParamDefault(true);

    CRef<CObjectManager> om = sx_GetOM();

    CBAMDataLoader::SLoaderParams params;
    string bam_name = sx_GetPath("hs108_sra.fil_sort.chr1.bam", "bam");
    params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName(bam_name));

    string loader_name =
        CBAMDataLoader::RegisterInObjectManager(*om, params,
                                                CObjectManager::eDefault)
        .GetLoader()->GetName();
    sx_ReportBamLoaderName(loader_name);
    CScope scope(*om);
    scope.AddDefaults();

    SAnnotSelector sel;
    sel.SetSearchUnresolved();
    sel.AddNamedAnnots("hs108_sra.fil_sort.chr1");
    sel.AddNamedAnnots("hs108_sra.fil_sort.chr1 pileup graphs");

    for ( int pass = 0; pass < 2; ++pass ) {
        vector<TQuery> queries;
        size_t NQ;
        if ( pass == 0 ) {
            queries = s_GetQueries2();
        }
        else {
            queries = s_GetQueries2full();
            queries.clear();
        }
        NQ = queries.size();
    
        vector<thread> tt(NQ);
        for ( size_t i = 0; i < NQ; ++i ) {
            tt[i] =
                thread([&]
                       (const TQuery& query)
                       {
                           CRef<CSeq_id> seqid(new CSeq_id(get<0>(query)));
                           CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*seqid);
                           CRef<CSeq_loc> loc(new CSeq_loc);
                           loc->SetInt().SetId(*seqid);
                           loc->SetInt().SetFrom(get<1>(query).GetFrom());
                           loc->SetInt().SetTo(get<1>(query).GetTo());
                           size_t count = 0;
                           if ( get<2>(query) ) {
                               for ( CGraph_CI it(scope, *loc, sel); it; ++it ) {
                                   ++count;
                               }
                           }
                           else {
                               for ( CAlign_CI it(scope, *loc, sel); it; ++it ) {
                                   ++count;
                               }
                           }
                           if ( !get<4>(query) || count != get<4>(query) ) {
                               BOOST_CHECK_EQUAL(count, get<3>(query));
                           }
                       }, queries[i]);
        }
        for ( size_t i = 0; i < NQ; ++i ) {
            tt[i].join();
        }
    }
}
#endif
