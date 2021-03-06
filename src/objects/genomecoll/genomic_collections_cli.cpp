/* $Id: genomic_collections_cli.cpp 547236 2017-09-27 16:32:40Z shchekot $
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
 * Author:  Vinay Kumar
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using the following specifications:
 *   'gencoll_client.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>

// generated includes
#include <objects/genomecoll/genomic_collections_cli.hpp>
#include <objects/genomecoll/GCClient_AssemblyInfo.hpp>
#include <objects/genomecoll/GCClient_AssemblySequenceI.hpp>
#include <objects/genomecoll/GCClient_AssembliesForSequ.hpp>
#include <objects/genomecoll/GCClient_Error.hpp>
#include <objects/genomecoll/GC_Assembly.hpp>
#include <objects/genomecoll/GCClient_ValidateChrTypeLo.hpp>
#include <objects/genomecoll/GCClient_EquivalentAssembl.hpp>
#include <objects/genomecoll/GCClient_GetEquivalentAsse.hpp>
#include <objects/genomecoll/GCClient_GetAssemblyBlobRe.hpp>
#include <objects/genomecoll/cached_assembly.hpp>
#include <sstream>

// generated classes

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE


CGCServiceException::CGCServiceException(const CDiagCompileInfo& diag, const objects::CGCClient_Error& srv_error)
    : CGCServiceException(diag, nullptr, 
                          CGCServiceException::EErrCode(srv_error.CanGetError_id() ? srv_error.GetError_id() : CException::eInvalid), 
                          srv_error.GetDescription())
{}

const char* CGCServiceException::GetErrCodeString(void) const
{
    return CGCClient_Error::ENUM_METHOD_NAME(EError_id)()->FindName((int)GetErrCode(), true).c_str();
}

static const STimeout kTimeout = {600, 0};

CGenomicCollectionsService::CGenomicCollectionsService()
{
    SetTimeout(&kTimeout);
    SetRetryLimit(20);

    // it's a backward-compatibility fix for old versions of server (no much harm to leave it - only little data overhead is expected)
    // always send request and get response in ASN text format so that server can properly parse request
    // For binary ASN request: client\server versions of ASN request must be exactly the same (compiled using one ASN definition - strong typing)
    // For text ASN request: client\server versions of ASN request can be different (use different ASN definitions - duck typing)
    SetFormat(eSerial_AsnText);
    // SetFormat() - sets both Request and Response encoding, so we put "fo=text" as well (though not needed now it may be usefull in the future for the client back-compatibility)
    SetArgs("fi=text&fo=text");
}

template<typename TReq>
void LogRequest(const TReq& req)
{
#ifdef _DEBUG
    ostringstream ostrstrm;
    ostrstrm << "Making request -" << MSerial_AsnText << req;
    LOG_POST(Info << ostrstrm.str());
#endif
}

static void ValidateAsmAccession(const string& acc)
{
    if(acc.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.") != string::npos)
        NCBI_THROW(CException, eUnknown, "Invalid accession format: " + acc);
}

CRef<CGC_Assembly> CGenomicCollectionsService::GetAssembly(const string& acc_, const string& mode)
{
    string acc = NStr::TruncateSpaces(acc_);
    ValidateAsmAccession(acc);

    CGCClient_GetAssemblyBlobRequest req;
    CGCClientResponse reply;

    req.SetAccession(acc);
    req.SetMode(mode);

    LogRequest(req);

    try {
        return CCachedAssembly(AskGet_assembly_blob(req, &reply)).Assembly();
    } catch (CException& ex) {
        if (reply.IsSrvr_error())
            throw CGCServiceException(DIAG_COMPILE_INFO, reply.GetSrvr_error());
        throw;
    }
}

CRef<CGC_Assembly> CGenomicCollectionsService::GetAssembly(int releaseId, const string& mode)
{
    CGCClient_GetAssemblyBlobRequest req;
    CGCClientResponse reply;

    req.SetRelease_id(releaseId);
    req.SetMode(mode);

    LogRequest(req);

    try {
        return CCachedAssembly(AskGet_assembly_blob(req, &reply)).Assembly();
    } catch (CException& ex) {
        if (reply.IsSrvr_error())
            throw CGCServiceException(DIAG_COMPILE_INFO, reply.GetSrvr_error());
        throw;
    }
}

string CGenomicCollectionsService::ValidateChrType(const string& chrType, const string& chrLoc)
{
    CGCClient_ValidateChrTypeLocRequest req;
    CGCClientResponse reply;
    
    req.SetType(chrType);
    req.SetLocation(chrLoc);

    LogRequest(req);

    try {
        return AskGet_chrtype_valid(req, &reply);
    } catch (CException& ex) {
        if (reply.IsSrvr_error())
            throw CGCServiceException(DIAG_COMPILE_INFO, reply.GetSrvr_error());
        throw;
    }
}

CRef<CGCClient_AssemblyInfo> CGenomicCollectionsService::FindOneAssemblyBySequences(const string& sequence_acc, int filter, CGCClient_GetAssemblyBySequenceRequest::ESort sort)
{
    CRef<CGCClient_AssemblySequenceInfo> asmseq_info(FindOneAssemblyBySequences(list<string>(1, sequence_acc), filter, sort));

    return asmseq_info ? CRef<CGCClient_AssemblyInfo>(&asmseq_info->SetAssembly()) : CRef<CGCClient_AssemblyInfo>();
}

CRef<CGCClient_AssemblySequenceInfo> CGenomicCollectionsService::FindOneAssemblyBySequences(const list<string>& sequence_acc, int filter, CGCClient_GetAssemblyBySequenceRequest::ESort sort)
{
    CRef<CGCClient_AssembliesForSequences> assm(FindAssembliesBySequences(sequence_acc, filter, sort, true));

    return assm->CanGetAssemblies() && !assm->GetAssemblies().empty() ?
           CRef<CGCClient_AssemblySequenceInfo>(assm->SetAssemblies().front()) :
           CRef<CGCClient_AssemblySequenceInfo>();
}

CRef<CGCClient_AssembliesForSequences> CGenomicCollectionsService::FindAssembliesBySequences(const string& sequence_acc, int filter, CGCClient_GetAssemblyBySequenceRequest::ESort sort)
{
    return FindAssembliesBySequences(list<string>(1, sequence_acc), filter, sort);
}

CRef<CGCClient_AssembliesForSequences> CGenomicCollectionsService::FindAssembliesBySequences(const list<string>& sequence_acc, int filter, CGCClient_GetAssemblyBySequenceRequest::ESort sort)
{
    return FindAssembliesBySequences(sequence_acc, filter, sort, false);
}
CRef<CGCClient_AssembliesForSequences> CGenomicCollectionsService::FindAssembliesBySequences(const list<string>& sequence_acc, int filter, CGCClient_GetAssemblyBySequenceRequest::ESort sort, bool top_only)

{
    CGCClient_GetAssemblyBySequenceRequest req;
    CGCClientResponse reply;

    for(auto acc : sequence_acc)
        if(acc.length() > 30) {
            NCBI_THROW(CException, eUnknown, "Accession is longer than 30 characters: " + acc);
        }

    req.SetSequence_acc().assign(sequence_acc.begin(), sequence_acc.end());
    req.SetFilter(filter);
    req.SetSort(sort);
    req.SetTop_assembly_only(top_only ? 1 : 0);

    LogRequest(req);

    try {
        return AskGet_assembly_by_sequence(req, &reply);
    } catch (const CException& ex) {
        if (reply.IsSrvr_error())
            throw CGCServiceException(DIAG_COMPILE_INFO, reply.GetSrvr_error());
        throw;
    }
}


CRef<CGCClient_EquivalentAssemblies> CGenomicCollectionsService::GetEquivalentAssemblies(const string& acc, int equivalency)
{
    CGCClient_GetEquivalentAssembliesRequest req;
    CGCClientResponse reply;

    req.SetAccession(acc);
    req.SetEquivalency(equivalency);

    LogRequest(req);

    try {
        return AskGet_equivalent_assemblies(req, &reply);
    } catch (const CException& ex) {
        if (reply.IsSrvr_error())
            throw CGCServiceException(DIAG_COMPILE_INFO, reply.GetSrvr_error());
        throw;
    }
}

END_objects_SCOPE
END_NCBI_SCOPE
