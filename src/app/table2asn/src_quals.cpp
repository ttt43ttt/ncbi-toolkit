/*  $Id: src_quals.cpp 572181 2018-10-10 12:12:32Z ivanov $
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
*   High level reader for source qualifiers 
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_ci.hpp>

#include <util/line_reader.hpp>
#include <objtools/edit/dblink_field.hpp>

#include <objtools/readers/source_mod_parser.hpp>

#include "src_quals.hpp"
#include "struc_cmt_reader.hpp"
#include "table2asn_context.hpp"
#include <objtools/readers/line_error.hpp>
#include <objtools/readers/message_listener.hpp>

#include <common/test_assert.h>  /* This header must go last */

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

namespace
{
    static const CTempString g_dblink = "DBLink";
};

void TSrcQuals::AddQualifiers(CSourceModParser& mod, const vector<CTempString>& values)
{
    // first is always skipped since it's an id 
    for (size_t i = 1; i < values.size() && i < m_cols.size(); i++)
    {
        if (!values[i].empty())
        {
            mod.AddMods(m_cols[i], values[i]);
        }
    }
}

bool TSrcQuals::AddQualifiers(objects::CSourceModParser& mod, const string& id)
{
    TLineMap::iterator it = m_lines_map.find(id);
    if (it != m_lines_map.end())
    {
        vector<CTempString> values;
        NStr::Split(it->second.m_unparsed, "\t", values, 0);

        AddQualifiers(mod, values);
        m_lines_map.erase(it);
        return true;
    }

    return false;
}

bool TSrcQuals::AddQualifiers(CSourceModParser& mod, const CBioseq& id)
{
    if (m_cols.empty())
        return false;

    for (auto id_it: id.GetId() )
    {
        string id = id_it->AsFastaString();
        NStr::ToLower(id);
        if (AddQualifiers(mod, id))
            return true;
    }

    return false;
}

CSourceQualifiersReader::CSourceQualifiersReader(CTable2AsnContext* context) : m_context(context)
{
}

CSourceQualifiersReader::~CSourceQualifiersReader()
{
}

bool CSourceQualifiersReader::ApplyQualifiers(objects::CSourceModParser& mod, objects::CBioseq& bioseq, objects::ILineErrorListener* listener)
{
    CSourceModParser::TMods unused_mods = mod.GetMods(CSourceModParser::fUnusedMods);
    for (auto mod : unused_mods)
    {
        if (!x_ParseAndAddTracks(bioseq, mod.key, mod.value))
        {
            if (listener)
                listener->PutError(*auto_ptr<CLineError>(
                CLineError::Create(ILineError::eProblem_GeneralParsingError, eDiag_Error, "", 0,
                    mod.key)));
            return false;
        }
    }
    return true;
}

bool CSourceQualifiersReader::x_ApplyAllQualifiers(objects::CSourceModParser& mod, objects::CBioseq& bioseq)
{
    // first is always skipped since it's an id 
    CSourceModParser::TModsRange mods[2];
    mods[0] = mod.FindAllMods("note");
    mods[1] = mod.FindAllMods("notes");
    for (size_t i = 0; i < 2; i++)
    {
        for (CSourceModParser::TModsCI it = mods[i].first; it != mods[i].second; it++)
        {
            NStr::ReplaceInPlace((string&)it->value, "<", "[");
            NStr::ReplaceInPlace((string&)it->value, ">", "]");
        }
    }
    mod.ApplyAllMods(bioseq);

    return ApplyQualifiers(mod, bioseq, m_context->m_logger);
}

bool CSourceQualifiersReader::LoadSourceQualifiers(const string& filename, const string& opt_map_filename)
{
    bool loaded = false;
    if (CFile(filename).Exists())
    {
        loaded = true;
        x_LoadSourceQualifiers(m_quals[0], filename, opt_map_filename);
    }

    if (!m_context->m_single_source_qual_file.empty())
    {
        loaded = true;
        x_LoadSourceQualifiers(m_quals[1], m_context->m_single_source_qual_file, opt_map_filename);
    }
    return loaded;
}

void CSourceQualifiersReader::x_LoadSourceQualifiers(TSrcQuals& quals, const string& filename, const string& opt_map_filename)
{
    quals.m_id_col = 0;
    quals.m_filemap.reset(new CMemoryFileMap(filename));

    size_t sz = quals.m_filemap->GetFileSize();
    const char* ptr = (const char*)quals.m_filemap->Map(0, sz);
    const char* end = ptr + sz;
    while (ptr < end)
    {
        // search for next non empty line
        if (*ptr == '\r' || *ptr == '\n')
        {
            ptr++;
            continue;
        }

        const char* start = ptr;

        // search for end of line
        const char* endline = (const char*)memchr(ptr, '\n', end - ptr);
        if (endline == 0) // this is the last line
        {
            endline = ptr;
            ptr = end;
        }
        else
        {
            ptr = endline + 1;
            endline--;
        }

        while (start < endline && *endline == '\r')
            endline--;


        // compose line control structure
        if (start < endline)
        {
            CTempString newline(start, endline - start + 1);
            // parse the header
            if (quals.m_cols.empty())
            {
                NStr::Split(newline, "\t", quals.m_cols, 0);
                if (!opt_map_filename.empty())
                {
                    //LCOV_EXCL_START
                    // optical maps are no longer supported
                    ITERATE(vector<CTempString>, it, quals.m_cols)
                    {
                        if (*it == "id" ||
                            *it == "seqid" ||
                            NStr::CompareNocase(*it, "Filename") == 0 ||
                            NStr::CompareNocase(*it, "File name") == 0)
                        {
                            quals.m_id_col = (it - quals.m_cols.begin());
                            break;
                        }
                    }
                    //LCOV_EXCL_STOP
                }
            }
            // parse regular line
            else
            {
                const char* endid = (const char*)memchr(start, '\t', endline - start);
                if (endid)
                {
                    string id_text(start, endid - start);
#if 0
                    NStr::ReplaceInPlace(id_text, ":", "|");
                    if (id_text.find('|') != string::npos)
                    {
                        if (!NStr::StartsWith(id_text, "gnl|"))
                        {
                            id_text.insert(0, "gnl|");
                        }
                    else {
                        if (!m_context->m_genome_center_id.empty())
                        {
                            id_text = "gnl|" + m_context->m_genome_center_id + "|" + id_text;
                        }
                    }
#endif

                    if (opt_map_filename.empty())
                    {
                        CRef<CSeq_id> id(new CSeq_id(id_text, 
                            m_context->m_allow_accession ? CSeq_id::fParse_AnyRaw | CSeq_id::fParse_ValidLocal : CSeq_id::fParse_AnyLocal));

                        id_text = id->AsFastaString();
                        NStr::ToLower(id_text);
                        TSrcQualParsed& ref = quals.m_lines_map[id_text];
                        ref.m_id = id;
                        ref.m_unparsed = newline;
                    }
                    else
                    {
                        //LCOV_EXCL_START
                        // optical maps are no longer supported
                        TSrcQualParsed& ref = quals.m_lines_map[id_text];
                        ref.m_unparsed = newline;
                        //LCOV_EXCL_STOP
                    }
                }
            }
        }
    }

    if (quals.m_cols.empty())
       NCBI_THROW(CArgException, eConstraint,
       "source modifiers file header line is not valid");
}

void CSourceQualifiersReader::ProcessSourceQualifiers(CSeq_entry& entry, const string& opt_map_filename)
{
    CScope scope(*CObjectManager::GetInstance());

    CSeq_entry_EditHandle h_entry = scope.AddTopLevelSeqEntry(entry).GetEditHandle();

    // apply for all sequences
    for (CBioseq_CI bioseq_it(h_entry); bioseq_it; ++bioseq_it)
    {
        CSourceModParser mod;

        CBioseq* dest = (CBioseq*)bioseq_it->GetEditHandle().GetCompleteBioseq().GetPointerOrNull();

        if (!m_context->m_source_mods.empty())
           mod.ParseTitle(m_context->m_source_mods, CConstRef<CSeq_id>(dest->GetFirstId()));

        bool handled = false;

        if (opt_map_filename.empty())
        {
            handled |= m_quals[1].AddQualifiers(mod, *dest);
            handled |= m_quals[0].AddQualifiers(mod, *dest);
        }
        else
        {
            //LCOV_EXCL_START
            // optical maps are no longer supported
            handled |= m_quals[1].AddQualifiers(mod, opt_map_filename);
            handled |= m_quals[0].AddQualifiers(mod, opt_map_filename);
            //LCOV_EXCL_STOP
        }

        if (!x_ApplyAllQualifiers(mod, *dest))
          NCBI_THROW(CArgException, eConstraint,
             "there are found unrecognised source modifiers");

        if (m_context->m_verbose && !handled)
        {
            m_context->m_logger->PutError(*auto_ptr<CLineError>(
                CLineError::Create(ILineError::eProblem_GeneralParsingError, eDiag_Error, "", 0,
                    "Source qualifiers file doesn't contain qualifiers for sequence id " + dest->GetId().front()->AsFastaString())));
        }
    }
    if (m_context->m_verbose)
    {
        for (auto m : m_quals)
            for (auto line : m.m_lines_map)
            {
                m_context->m_logger->PutError(*auto_ptr<CLineError>(
                    CLineError::Create(ILineError::eProblem_GeneralParsingError, eDiag_Error, "", 0,
                    "File " + m_context->m_current_file + " doesn't contain sequence with id " + line.first)));
            }
    }
}


//LCOV_EXCL_START
// no longer used
void CSourceQualifiersReader::x_AddQualifiers(CSourceModParser& mod, const string& filename)
{
    CRef<ILineReader> reader(ILineReader::New(filename));

    vector<CTempString> cols;

    //size_t filename_id = string::npos;
    while (!reader->AtEOF())
    {
        reader->ReadLine();
        // First line is a collumn definitions
        CTempString current = reader->GetCurrentLine();
        if (current.empty())
            continue;

        if (cols.empty())
        {
            NStr::Split(current, "\t", cols, 0);
#if 0
            if (!opt_map_filename.empty())
            {
                ITERATE(vector<CTempString>, it, cols)
                {
                    if (*it == "id" ||
                        *it == "seqid" ||
                        NStr::CompareNocase(*it, "Filename") == 0 ||
                        NStr::CompareNocase(*it, "File name") == 0)
                    {
                        filename_id = (it - cols.begin());
                        break;
                    }
                }
            }
#endif
            if (cols.empty())
                NCBI_THROW(CArgException, eConstraint,
                "source modifiers file header line is not valid");
            continue;
        }

        if (current.empty())
            continue;

        // Each line except first is a set of values, first collumn is a sequence id
        vector<CTempString> values;
        NStr::Split(current, "\t", values, 0);
#if 0
        string id;

        if (opt_map_filename.empty())
        {
            id = values[0];
        }
        else
        {
            if (filename_id < values.size())
                id = values[filename_id];
        }
#endif

        //x_AddQualifiers(mod, cols, values);
    }
}
//LCOV_EXCL_STOP

bool CSourceQualifiersReader::x_ParseAndAddTracks(CBioseq& container, const string& name, const string& value)
{
    if (name == "ft-url" ||
        name == "ft-map")
        CTable2AsnContext::AddUserTrack(container.SetDescr(), "FileTrack", "Map-FileTrackURL", value);
    else
    if (name == "ft-mod")
        CTable2AsnContext::AddUserTrack(container.SetDescr(), "FileTrack", "BaseModification-FileTrackURL", value);
    else
        return false;

    return true;
}


END_NCBI_SCOPE

