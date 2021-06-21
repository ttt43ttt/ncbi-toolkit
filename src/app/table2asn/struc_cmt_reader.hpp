#ifndef STRUCT_CMT_READER_HPP
#define STRUCT_CMT_READER_HPP

#include <corelib/ncbistl.hpp>

#include <objtools/readers/struct_cmt_reader.hpp>

BEGIN_NCBI_SCOPE

// forward declarations
namespace objects
{
    class CUser_object;
    class CSeq_descr;
    class CBioseq;
    class CObject_id;
    class CSeq_entry;
    class ILineErrorListener;
};

class CSerialObject;
class ILineReader;

/*
  Usage examples
  ProcessCommentsFileByRows tries to apply structure comments to all sequences
  in the container.
  ProcessCommentsFileByCols tries to apply structure comments to those
  sequences which id is found in first collumn of each row in the file.

  CRef<CSeq_entry> entry;

  CStructuredCommentsReader reader;

  ILineReader reader1(ILineReader::New(filename);
  reader.ProcessCommentsFileByRows(reader1, *entry);

  ILineReader reader2(ILineReader::New(filename);
  reader.ProcessCommentsFileByRows(reader2, *entry);
*/

class CTable2AsnStructuredCommentsReader : public CStructuredCommentsReader
{
public:
   // If you need messages and error to be logged
   // supply an optional ILineErrorListener instance
    CTable2AsnStructuredCommentsReader(objects::ILineErrorListener* logger);
    ~CTable2AsnStructuredCommentsReader();
   // Read input tab delimited file and apply Structured Comments to the container
   // First row of the file is a list of Field to apply
   // First collumn of the file is an ID of the object (sequence) to apply
   void ProcessCommentsFileByCols(ILineReader& reader, objects::CSeq_entry& container);
   // Read input tab delimited file and apply Structured Comments to all objects
   // (sequences) in the container
   // First collumn of the file is the name of the field
   // Second collumn of the file is the value of the field
   void ProcessCommentsFileByRows(ILineReader& reader, objects::CSeq_entry& container);
   static
   void AddStructuredComments(objects::CSeq_descr& descr, const CStructComment& comments);
   bool IsVertical(ILineReader& reader);

private:
    void _AddStructuredComments(objects::CSeq_entry& entry, const CStructComment& comments);

   objects::ILineErrorListener* m_logger;
};

END_NCBI_SCOPE

#endif
