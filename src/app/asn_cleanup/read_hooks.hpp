#ifndef READ_HOOKS
#define READ_HOOKS

#include <serial/objhook.hpp>
#include <objects/seqset/gb_release_file.hpp>

BEGIN_NCBI_SCOPE

class ISubmitBlockHandler
{
public:
	/// user code for handling a Submit-block goes here.
	/// The return value indicates whethear to continue (true),
	/// or abort (false) the read.
	virtual bool HandleSubmitBlock(CSubmit_block& block) = 0;
	virtual ~ISubmitBlockHandler(void) {};
};

class CReadSubmitBlockHook : public CReadClassMemberHook
{
public:
    CReadSubmitBlockHook(ISubmitBlockHandler& handler, CObjectOStream& out);

    virtual void ReadClassMember(CObjectIStream &in, const CObjectInfoMI &member);

private:
    CObjectOStream& m_out;
	ISubmitBlockHandler& m_handler;
};

class CReadEntryHook : public CReadObjectHook
{
public:
    CReadEntryHook(CGBReleaseFile::ISeqEntryHandler& handler, CObjectOStream& out);

    virtual void ReadObject(CObjectIStream &in, const CObjectInfo& obj);

private:
    CGBReleaseFile::ISeqEntryHandler& m_handler;
    CObjectOStream& m_out;
    bool m_isGenbank;

    void x_SetBioseqsetHook(CObjectIStream &in, bool isSet);

    friend class CReadBioseqsetClassHook;
};

END_NCBI_SCOPE

#endif // READ_HOOKS
