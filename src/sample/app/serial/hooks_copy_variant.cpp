#include <ncbi_pch.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <serial/objcopy.hpp>
#include <serial/objectio.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/serial.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(ncbi::objects);

class CDemoHook : public CCopyChoiceVariantHook
{
public:
    virtual void CopyChoiceVariant(CObjectStreamCopier& copier,
                                   const CObjectTypeInfoCV& passed_info)
    {
        cout << copier.In().GetStackPath() << endl;
#if 1
        DefaultCopy(copier, passed_info);

#else
#if 1
// or skip variant in input stream:
        copier.In().SkipObject(passed_info.GetVariantType());
#endif
#if 0
// or read CSeq_feat objects one by one
        for ( CIStreamContainerIterator i(copier.In(), passed_info.GetVariantType()); i; ++i ) {
            CSeq_feat feat;
            i >> feat;
            cout << MSerial_AsnText << feat << endl;
        }
#endif
#if 0
// or read CSeq_feat objects one by one
        for ( CIStreamContainerIterator i(copier.In(), passed_info.GetVariantType()); i; ++i ) {
            CObjectInfo oi(CSeq_feat::GetTypeInfo());
            i.ReadElement(oi);
            cout << MSerial_AsnText << oi << endl;
        }
#endif
#if 0
// or read CSeq_feat objects one by one and write them into output stream
        COStreamContainer o(copier.Out(), passed_info.GetVariantType());
        for ( CIStreamContainerIterator i(copier.In(), passed_info.GetVariantType()); i; ++i ) {
            CSeq_feat feat;
            i >> feat;
            o << feat;
            cout << MSerial_AsnText << feat << endl;
        }
#endif
#if 0
// or read the whole SET OF Seq-feat at once
        CSeq_annot::TData::TFtable ft;
        CObjectInfo oi(&ft, passed_info.GetVariantType().GetTypeInfo());
        // or, like this:
        //  CObjectInfo oi(passed_info.GetVariantType());
        copier.In().ReadObject(oi);
        // write them one by one
        for( const auto& e: ft) {
            cout << MSerial_AsnText << *e << endl;
        }
        copier.Out().WriteObject(oi);
#endif
#if 0
// or read the whole SET OF Seq-feat at once
        CObjectInfo oi(passed_info.GetVariantType());
        copier.In().ReadObject(oi);
        copier.Out().WriteObject(oi);
#endif

// get information about the variant
        // typeinfo of the parent class (Seq-annot)
        CObjectTypeInfo oti = passed_info.GetChoiceType();
        // typeinfo of the variant  (SET OF Seq-feat)
        CObjectTypeInfo omti = passed_info.GetVariantType();
        // index of the variant in parent class  (1)
        TMemberIndex mi = passed_info.GetVariantIndex();
        // information about the variant, including its name (ftable)
        const CVariantInfo* minfo = passed_info.GetVariantInfo();
#endif
    }
};

int main(int argc, char** argv)
{
    unique_ptr<CObjectIStream> in(CObjectIStream::Open(eSerial_AsnText, "seq-entry-sample.asn"));
    unique_ptr<CObjectOStream> out(CObjectOStream::Open(eSerial_AsnText, "seq-entry-sample_output.asn"));
    CObjectStreamCopier copier(*in, *out);

    (*CObjectTypeInfo(CType<CSeq_annot>()).FindMember("data"))
        .GetPointedType()
        .FindVariant("ftable")
        .SetLocalCopyHook(copier, new CDemoHook);

    copier.Copy(CType<CSeq_entry>());

    return 0;
}
