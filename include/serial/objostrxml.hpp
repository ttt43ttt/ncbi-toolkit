#ifndef OBJOSTRXML__HPP
#define OBJOSTRXML__HPP

/*  $Id: objostrxml.hpp 546693 2017-09-20 17:04:38Z gouriano $
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
* Author: Eugene Vasilchenko
*
* File Description:
*   Encode data object using XML format
*/

#include <corelib/ncbistd.hpp>
#include <serial/objostr.hpp>
#include <deque>


/** @addtogroup ObjStreamSupport
 *
 * @{
 */


BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
///
/// CObjectOStreamXml --
///
/// Encode serial data object using XML format
class NCBI_XSERIAL_EXPORT CObjectOStreamXml : public CObjectOStream
{
public:

    /// Constructor.
    ///
    /// @param out
    ///   Output stream    
    /// @param deleteOut
    ///   when TRUE, the output stream will be deleted automatically
    ///   when the writer is deleted
    /// @deprecated
    ///   Use one with EOwnership enum instead
    NCBI_DEPRECATED_CTOR(CObjectOStreamXml(CNcbiOstream& out, bool deleteOut));

    /// Constructor.
    ///
    /// @param out
    ///   Output stream    
    /// @param deleteOut
    ///   When eTakeOwnership, the output stream will be deleted automatically
    ///   when the writer is deleted
    CObjectOStreamXml(CNcbiOstream& out, EOwnership deleteOut);

    /// Destructor.
    virtual ~CObjectOStreamXml(void);

    /// Get current stream position as string.
    /// Useful for diagnostic and information messages.
    ///
    /// @return
    ///   string
    virtual string GetPosition(void) const override;

    /// Set XML character encoding
    ///
    /// @return
    ///   Encoding
    void SetEncoding(EEncoding enc);

    /// Get XML character encoding
    ///
    /// @return
    ///   Encoding
    EEncoding GetEncoding(void) const;

    /// Set default encoding of 'string' objects
    /// If XML data encoding is different, string will be converted to
    /// this encoding
    ///
    /// @param enc
    ///   Encoding
    void SetDefaultStringEncoding(EEncoding enc);

    /// Get default encoding of 'string' objects
    ///
    /// @return
    ///   Encoding
    EEncoding GetDefaultStringEncoding(void) const;

    /// Set up scope prefixes handling.
    /// Historically, using scope prefixes is the default for C++ data objects
    /// generated by ASN.1 specification.
    /// For objects generated by DTD, the default is NOT using prefixes.
    ///    
    /// @param set
    ///   When TRUE, the stream omits 'scope prefixes'.
    void SetEnforcedStdXml(bool set = true);

    /// Get scope prefixes handling parameter.
    ///
    /// @return
    ///   TRUE (omit scope prefixes) or FALSE
    bool GetEnforcedStdXml(void)     {return m_StdXml ? false : m_EnforcedStdXml;}

    /// Make generated XML document reference XML schema
    ///
    /// @param use_schema
    ///   When TRUE, the generated document will reference Schema
    void SetReferenceSchema(bool use_schema = true);

    /// Get Schema referencing parameter
    ///
    /// @return
    ///   TRUE or FALSE
    bool GetReferenceSchema(void) const;

    /// Make generated XML document reference DTD
    ///
    /// @param use_dtd
    ///   When TRUE, the generated document will reference DTD
    void SetReferenceDTD(bool use_dtd = true);

    /// Get DTD referencing parameter
    ///
    /// @return
    ///   TRUE or FALSE
    bool GetReferenceDTD(void) const;

    /// Put Schema location information into generated XML document
    ///
    /// @param use_loc
    ///   When TRUE, the generated document will have schemaLocation attribute
    void SetUseSchemaLocation(bool use_loc = true);

    /// Get Schema location output parameter
    ///
    /// @return
    ///   TRUE or FALSE
    bool GetUseSchemaLocation(void) const;
    
    /// Set default value of namespace name of generated DTD documents
    ///
    /// @param schema_ns
    ///   namespace name
    void   SetDefaultSchemaNamespace(const string& schema_ns);

    /// Get default value of namespace name of generated DTD documents
    ///
    /// @return
    ///   namespace name
    string GetDefaultSchemaNamespace(void);

    /// Set DTD or schema file prefix.
    /// Reference to DTD or schema in XML document has the form
    /// [DTDFilePrefix][DTDFileName].[dtd|xsd]
    /// If "DTDFilePrefix" has never been set for this stream, then
    /// the global "DefaultDTDFilePrefix" will be used.
    /// If it has been set to any value (including empty string), then
    /// that value will be used.
    ///
    /// @param prefix
    ///   File prefix    
    void   SetDTDFilePrefix(const string& prefix);

    /// Get DTD or schema file prefix.
    ///
    /// @return
    ///   File prefix
    string GetDTDFilePrefix(void) const;

    /// Set default (global) DTD file prefix.
    ///
    /// @param prefix
    ///   File prefix    
    static void   SetDefaultDTDFilePrefix(const string& prefix);

    /// Get default (global) DTD file prefix.
    ///
    /// @param prefix
    ///   File prefix    
    static string GetDefaultDTDFilePrefix(void);

    /// Set DTD or schema file name.
    /// Reference to DTD or schema in XML document has the form
    /// [DTDFilePrefix][DTDFileName].[dtd|xsd]
    /// If "DTDFileName" is not set or set to empty string for this stream,
    /// then module name (in ASN.1 sense) will be used as the file name.
    ///
    /// @param filename
    ///   File name
    void   SetDTDFileName(const string& filename);

    /// Get DTD or schema file name.
    ///
    /// @return
    ///   File name
    string GetDTDFileName(void) const;

    /// Enable DTD public identifier.
    /// If disabled (it is ENABLED by default), only system identifier
    /// will be written in the output XML stream
    void EnableDTDPublicId(void);

    /// Disable DTD public identifier.
    /// If disabled (it is ENABLED by default), only system identifier
    /// will be written in the output XML stream
    void DisableDTDPublicId(void);

    /// Set DTD public identifier.
    /// If set to a non-empty string, the stream will write this into the
    /// output XML file. Otherwise the "default" public id
    /// will be generated
    ///
    /// @param publicId
    ///   Public ID
    void SetDTDPublicId(const string& publicId);

    /// Get DTD public identifier.
    ///
    /// @return
    ///   Public ID
    string GetDTDPublicId(void) const;

    /// formatting of values of type 'real' ('double')
    enum ERealValueFormat {
        eRealFixedFormat,      ///< use 'f' formatting type
        eRealScientificFormat  ///< use 'g' formatting type
    };

    /// Get formatting of values of type real
    ///
    /// @return
    ///   Formatting flag
    ERealValueFormat GetRealValueFormat(void) const;
    
    /// Set formatting of values of type real
    /// The method is provided for convenience only.
    ///
    /// @param fmt
    ///   Formatting flag
    void SetRealValueFormat(ERealValueFormat fmt);
    
    /// Set output formatting flags
    ///
    /// @param flags
    ///   Formatting flag
    virtual void SetFormattingFlags(TSerial_Format_Flags flags) override;

    virtual void WriteFileHeader(TTypeInfo type) override;
    virtual void EndOfWrite(void) override;

protected:
    virtual void WriteBool(bool data) override;
    virtual void WriteChar(char data) override;
    virtual void WriteInt4(Int4 data) override;
    virtual void WriteUint4(Uint4 data) override;
    virtual void WriteInt8(Int8 data) override;
    virtual void WriteUint8(Uint8 data) override;
    virtual void WriteFloat(float data) override;
    virtual void WriteDouble(double data) override;
    void WriteDouble2(double data, unsigned digits);
    virtual void WriteCString(const char* str) override;
    virtual void WriteString(const string& s,
                             EStringType type = eStringTypeVisible) override;
    virtual void WriteStringStore(const string& s) override;
    virtual void CopyString(CObjectIStream& in,
                            EStringType type = eStringTypeVisible) override;
    virtual void CopyStringStore(CObjectIStream& in) override;

    virtual void WriteNullPointer(void) override;
    virtual void WriteObjectReference(TObjectIndex index) override;
    virtual void WriteOtherBegin(TTypeInfo typeInfo) override;
    virtual void WriteOtherEnd(TTypeInfo typeInfo) override;
    virtual void WriteOther(TConstObjectPtr object, TTypeInfo typeInfo) override;

    virtual void WriteNull(void) override;
    virtual void WriteAnyContentObject(const CAnyContentObject& obj) override;
    virtual void CopyAnyContentObject(CObjectIStream& in) override;

    virtual void WriteBitString(const CBitString& obj) override;
    virtual void CopyBitString(CObjectIStream& in) override;

    void WriteEscapedChar(char c);
    void WriteEncodedChar(const char*& src,
                          EStringType type = eStringTypeVisible);

    virtual void WriteEnum(const CEnumeratedTypeValues& values,
                           TEnumValueType value) override;
    virtual void CopyEnum(const CEnumeratedTypeValues& values,
                          CObjectIStream& in) override;
    void WriteEnum(const CEnumeratedTypeValues& values,
                   TEnumValueType value, const string& valueName);

#ifdef VIRTUAL_MID_LEVEL_IO
    virtual void WriteNamedType(TTypeInfo namedTypeInfo,
                                TTypeInfo typeInfo, TConstObjectPtr object) override;
    virtual void CopyNamedType(TTypeInfo namedTypeInfo,
                               TTypeInfo typeInfo,
                               CObjectStreamCopier& copier) override;

    virtual void WriteContainer(const CContainerTypeInfo* containerType,
                                TConstObjectPtr containerPtr) override;

    virtual void WriteClass(const CClassTypeInfo* objectType,
                            TConstObjectPtr objectPtr) override;
    virtual void WriteClassMember(const CMemberId& memberId,
                                  TTypeInfo memberType,
                                  TConstObjectPtr memberPtr) override;
    virtual bool WriteClassMember(const CMemberId& memberId,
                                  const CDelayBuffer& buffer) override;

    virtual void WriteClassMemberSpecialCase(
        const CMemberId& memberId, TTypeInfo memberType,
        TConstObjectPtr memberPtr, ESpecialCaseWrite how) override;
#endif
    void WriteContainerContents(const CContainerTypeInfo* containerType,
                                TConstObjectPtr containerPtr);
    void WriteChoiceContents(const CChoiceTypeInfo* choiceType,
                             TConstObjectPtr choicePtr);
    // low level I/O
    virtual void BeginNamedType(TTypeInfo namedTypeInfo) override;
    virtual void EndNamedType(void) override;

    virtual void BeginContainer(const CContainerTypeInfo* containerType) override;
    virtual void EndContainer(void) override;
    virtual void BeginContainerElement(TTypeInfo elementType) override;
    virtual void EndContainerElement(void) override;
    void BeginArrayElement(TTypeInfo elementType);
    void EndArrayElement(void);

    void CheckStdXml(TTypeInfo classType);

    virtual void BeginClass(const CClassTypeInfo* classInfo) override;
    virtual void EndClass(void) override;
    virtual void BeginClassMember(const CMemberId& id) override;
    void BeginClassMember(TTypeInfo memberType,
                          const CMemberId& id);
    virtual void EndClassMember(void) override;

    virtual void BeginChoice(const CChoiceTypeInfo* choiceType) override;
    virtual void EndChoice(void) override;
    virtual void BeginChoiceVariant(const CChoiceTypeInfo* choiceType,
                                    const CMemberId& id) override;
    virtual void EndChoiceVariant(void) override;

    virtual void WriteBytes(const ByteBlock& block,
                            const char* bytes, size_t length) override;
    virtual void WriteChars(const CharBlock& block,
                            const char* chars, size_t length) override;

    // Write current separator to the stream
    virtual void WriteSeparator(void) override;

#if defined(NCBI_SERIAL_IO_TRACE)
    void TraceTag(const string& name);
#endif

private:
    void WriteBase64Bytes(const char* bytes, size_t length);
    void WriteBytes(const char* bytes, size_t length);
    void WriteString(const char* str, size_t length);

    void OpenTagStart(void);
    void OpenTagEnd(void);
    void OpenTagEndBack(void);
    void SelfCloseTagEnd(void);
    void CloseTagStart(void);
    void CloseTagEnd(void);

    void WriteTag(const string& name);
    void OpenTag(const string& name);
    void EolIfEmptyTag(void);
    void CloseTag(const string& name);
    void OpenStackTag(size_t level);
    void CloseStackTag(size_t level);
    void OpenTag(TTypeInfo type);
    void CloseTag(TTypeInfo type);
    void OpenTagIfNamed(TTypeInfo type);
    void CloseTagIfNamed(TTypeInfo type);
    void PrintTagName(size_t level);
    bool WillHaveName(TTypeInfo elementType);
    string GetModuleName(TTypeInfo type);
    bool x_IsStdXml(void) {return m_StdXml || m_EnforcedStdXml;}

    void x_WriteClassNamespace(TTypeInfo type);
    bool x_ProcessTypeNamespace(TTypeInfo type);
    void x_EndTypeNamespace(void);
    bool x_BeginNamespace(const string& ns_name, const string& ns_prefix);
    void x_EndNamespace(const string& ns_name);
    bool x_SpecialCaseWrite(void);
    char x_VerifyChar(char);

    enum ETagAction {
        eTagOpen,
        eTagClose,
        eTagSelfClosed,
        eAttlistTag
    };
    ETagAction m_LastTagAction;
    bool m_EndTag;

    // DTD file name and prefix
    bool   m_UseDefaultDTDFilePrefix;
    string m_DTDFilePrefix;
    string m_DTDFileName;
    bool   m_UsePublicId;
    string m_PublicId;
    static string sm_DefaultDTDFilePrefix;
    bool m_Attlist;
    bool m_StdXml;
    bool m_EnforcedStdXml;
    ERealValueFormat m_RealFmt;
    EEncoding m_Encoding;
    EEncoding m_StringEncoding;
    bool m_UseXmlDecl;
    bool m_UseSchemaRef;
    bool m_UseSchemaLoc;
    bool m_UseDTDRef;
    string m_DefaultSchemaNamespace;
    string m_CurrNsPrefix;
    map<string,string> m_NsNameToPrefix;
    map<string,string> m_NsPrefixToName;
    deque<string> m_NsPrefixes;
    bool m_SkipIndent;
    bool m_SkipNextTag;
};


/* @} */


#include <serial/impl/objostrxml.inl>

END_NCBI_SCOPE

#endif
