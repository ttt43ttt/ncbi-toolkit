 #ifndef OBJTOOLS_READERS_SEQDB__SEQDBATLAS_HPP
#define OBJTOOLS_READERS_SEQDB__SEQDBATLAS_HPP

/*  $Id: seqdbatlas.hpp 577755 2019-01-08 18:08:53Z ivanov $
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
 * Author:  Kevin Bealer
 *
 */

/// @file seqdbatlas.hpp
/// The SeqDB memory management layer.
///
/// Defines classes:
///     CSeqDBLockHold
///     CSeqDBMemReg
///     CSeqDB_AtlasRegionHolder
///     CSeqDBAtlas
///     CSeqDBAtlasHolder
///
/// Implemented for: UNIX, MS-Windows


#include <corelib/ncbimtx.hpp>
#include <corelib/ncbiatomic.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbi_system.hpp>

#include <objtools/blast/seqdb_reader/impl/seqdbgeneral.hpp>

#include <vector>
#include <map>
#include <set>

BEGIN_NCBI_SCOPE

#ifdef SEQDB_TRACE_LOGFILE

#define SEQDB_LOGCLASS_DEFAULT 1
#define SEQDB_LOGCLASS_MMAP    2
#define SEQDB_LOGCLASS_MFILE   4
#define SEQDB_LOGCLASS_GET     8

extern ofstream * seqdb_logfile;
extern int        seqdb_logclass;

void seqdb_log(const char * s);
void seqdb_log(const char * s1, const string & s2);

void seqdb_log(int cl, const char * s);
void seqdb_log(int cl, const char * s1, const string & s2);
void seqdb_log(int cl, const char * s1, int s2);
#endif // SEQDB_TRACE_LOGFILE

#ifdef _DEBUG

#include <iostream>

// These defines implement a system for testing memory corruption,
// especially overwritten pointers and clobbered objects.  They are
// only enabled in debug mode.

// If this is required for release mode as well, be sure to replace
// _ASSERT() with something useful for release mode.

/// Define memory marker for class (4+ bytes of uppercase ascii).

// Note: this is now strictly 4 bytes and little-endian-centric (on
// big endian architectures the bytes will be in the other order than
// the string would imply.  This should not matter to the code.

#define INT4IFY_STRING(a)    \
    (((a[3] & 0xFF) << 24) | \
     ((a[2] & 0xFF) << 16) | \
     ((a[1] & 0xFF) <<  8) | \
     ((a[0] & 0xFF)))

#define CLASS_MARKER_FIELD(a) \
    static int    x_GetClassMark()  { return INT4IFY_STRING(a);  } \
    static string x_GetMarkString() { return string((a a), 4); } \
    int m_ClassMark;

/// Marker initializer for constructor
#define INIT_CLASS_MARK() m_ClassMark = x_GetClassMark()

/// Assertion to verify the marker
#define CHECK_MARKER() \
   if (m_ClassMark != x_GetClassMark()) { \
       cout << "Marker=" << m_ClassMark << endl; \
       cout << "GetMrk=" << x_GetClassMark() << endl; \
       cout << "\n!! Broken  [" << x_GetMarkString() << "] mark detected.\n" \
            << "!! Mark is [" << hex << m_ClassMark << "], should be [" \
            << hex << x_GetClassMark() << "]." << endl; \
       _ASSERT(m_ClassMark == x_GetClassMark()); \
   }

/// Make the marker of this class invalid.
#define BREAK_MARKER() m_ClassMark |= 0x20202020;

#else

/// Define memory marker for class  (release mode code elision)
#define CLASS_MARKER_FIELD(a)

/// Initializer for constructor (release mode code elision)
#define INIT_CLASS_MARK()

/// Assertion to verify the marker (release mode code elision)
#define CHECK_MARKER()

/// Make the marker of this class invalid (release mode code elision)
#define BREAK_MARKER()

#endif

/// CSeqDBAtlas class - a collection of memory maps.
class CSeqDBAtlas; // WorkShop needs this forward declaration.
class CSeqDBFileMemMap;

/// Return path with delimiters changed to platform preferred kind.
///
/// The path is modified and returned.  The 'Make' interface is more
/// convenient for cases where the input path and output path are
/// different objects.  Delimiter conversion should be called by SeqDB
/// at least once on any path received from the user, or via
/// filesystem sources such as alias files.
///
/// @param dbs This is the input path.
/// @return The modified path is returned.
string SeqDB_MakeOSPath(const string & dbs);


/// CSeqDBLockHold
///
/// This class is used to keep track of whether this thread holds the
/// atlas lock.  The atlas code will skip subsequent Lock() operations
/// during the same transaction if the lock is already held.  This
/// allows code that needs locking to get the lock without worrying
/// about whether the calling function has already done so.  The
/// destructor of this object will call Unlock() on the atlas if this
/// thread has it locked.

class NCBI_XOBJREAD_EXPORT CSeqDBLockHold {
public:
    /// Constructor
    ///
    /// This object constructs to an unlocked state, which means that
    /// the thread owning the stack frame where this object is stored
    /// does not hold the atlas lock.  This object keeps a reference
    /// to the atlas object so that it can release the lock on
    /// destruction, making it easier to write exception safe code.
    /// @param atlas
    ///   A reference to the atlas object.
    CSeqDBLockHold(class CSeqDBAtlas & atlas)
        : m_Atlas(atlas),
          m_Locked(false)
    {
        INIT_CLASS_MARK();
    }

    
    /// Destructor
    ///
    /// The class will unlock the atlas's lock on destruction (if it
    /// is the owner of that lock).
    ~CSeqDBLockHold();

private:
    CLASS_MARKER_FIELD("LHLD")

    /// Private method to prevent copy construction.
    CSeqDBLockHold(CSeqDBLockHold & oth);
    CSeqDBLockHold& operator=(CSeqDBLockHold & oth);

    /// Only the atlas code is permitted to modify this object - it
    /// does so simply by editing the m_Locked member as needed.
    friend class CSeqDBAtlas;

    /// This reference allows unlock on exit.
    class CSeqDBAtlas & m_Atlas;

    /// If this is true, this thread owns the atlas lock.
    bool m_Locked;
};


/// CSeqDBMemReg
///
/// This class is used to keep track of bytes allocated externally to
/// the atlas, but included under its memory bound.

class CSeqDBMemReg {
public:
    /// Constructor
    ///
    /// This object constructs to an empty state, which means that the
    /// atlas does not consider this object to "own" any bytes.
    ///
    /// @param atlas
    ///   A reference to the atlas object.
    CSeqDBMemReg(class CSeqDBAtlas & atlas)
        : m_Atlas(atlas),
          m_Bytes(0)
    {
    }

    /// Destructor
    ///
    /// The class will unlock the atlas's lock on destruction (if it
    /// is the owner of that lock).
    inline ~CSeqDBMemReg();

private:
    /// Private method to prevent copy construction.
    CSeqDBMemReg(CSeqDBMemReg & oth);

    /// Only the atlas code is permitted to modify this object - it
    /// does so simply by editing the m_Locked member as needed.
    friend class CSeqDBAtlas;

    /// This reference allows unlock on exit.
    class CSeqDBAtlas & m_Atlas;

    /// This object "owns" this many bytes of the atlas memory bound.
    size_t m_Bytes;
};




/// Hold a memory region refcount, return to atlas when destroyed.
///
/// This object `owns' a reference to a region of atlas owned memory,
/// releasing that reference when destructed.  This can be used to
/// return a hold on a region of memory to the user.  Care should be
/// taken when managing these objects.  In particular there should
/// never be a case where a "live" object of this type could be
/// destroyed in a thread that already holds the atlas lock.  A simple
/// technique is to keep an extra CRef<> to this object until the
/// thread releasees the lock.

class CSeqDB_AtlasRegionHolder : public CObject {
public:
    /// Constructor.
    CSeqDB_AtlasRegionHolder(CSeqDBAtlas & atlas, const char * ptr);

    /// Destructor.
    ~CSeqDB_AtlasRegionHolder();

private:
    /// Reference to the memory management layer.
    CSeqDBAtlas & m_Atlas;

    /// Pointer to this object.
    const char  * m_Ptr;
};


/// CSeqDBAtlas class
///
/// This object manages a collection of (memory) maps.  It mmaps or
/// reads data from files on demand, to allow a set of files to be
/// accessed efficiently by SeqDB.  The total size of the files used
/// by a multivolume database may exceed the usable address space of
/// the system by several times.  SeqDB also registers certain large,
/// dynamically allocated (via new[]) memory blocks with this object,
/// in an effort to limit the total memory usage.  This class also
/// contains the primary mutex used to sequentialize access to the
/// various SeqDB critical regions, some of which are outside of this
/// class.  ["Atlas: n. 1. A book or bound collection of maps..."; The
/// American Heritage Dictionary of the English Language, 4th Edition.
/// Copyright (c) 2000 by Houghton Mifflin Company.]

class NCBI_XOBJREAD_EXPORT CSeqDBAtlas {
public:
    /// The type used for file offsets.
    //typedef CRegionMap::TIndx TIndx;
    typedef CNcbiStreamoff TIndx;

    /// Constructor
    ///
    /// Initializes the atlas object.
    ///
    /// @param use_mmap
    ///   If false, use read(); if true, use mmap() or similar.
    CSeqDBAtlas(bool use_mmap);

    /// The destructor unmaps and frees all associated memory.
    ~CSeqDBAtlas();

     
    enum {e_MaxFileDescritors = 950};
    
    /// Check if file exists.
    ///
    /// If the file exists, this method will return true, otherwise
    /// false.  Also, if the file exists, the first slice of it will
    /// be mapped.  This method exists for efficiency reasons; it is
    /// cheaper to ask the atlas whether it already knows about a file
    /// than to search the filesystem for the file, particularly in
    /// the case of network file systems.  If a slice of the file has
    /// already been mapped, this will return true without consulting
    /// the file system, so this method will not detect files that
    /// have been deleted since the mapping occurred.
    ///
    /// @param fname
    ///   The filename of the file to get.
    /// @param locked
    ///   The lock hold object for this thread.
    /// @return
    ///   True if the file exists.
    bool DoesFileExist(const string & fname, CSeqDBLockHold & locked);

    /// Check if file exists.
    ///
    /// This is like the previous but accepts CSeqDB_Path.
    ///
    /// @param fname
    ///   The filename of the file to get.
    /// @param locked
    ///   The lock hold object for this thread.
    /// @return
    ///   True if the file exists.
    bool DoesFileExist(const CSeqDB_Path & fname, CSeqDBLockHold & locked)
    {
        return DoesFileExist(fname.GetPathS(), locked);
    }

    bool DoesFileExist(const string & fname)
    {
        TIndx length(0);
        return GetFileSizeL(fname, length);
    }

    bool DoesFileExist(const CSeqDB_Path & fname)
    {
        TIndx length(0);
        return GetFileSizeL(fname.GetPathS(), length);
    }
  
    /// Get size of a file.
    ///
    /// Check whether a file exists and get the file's size.
    ///
    /// @param fname
    ///   The filename of the file to get the size of.
    /// @param length
    ///   The length of the file is returned here.
    /// @param locked
    ///   The lock hold object for this thread.
    /// @return
    ///   true if the file exists.
    bool GetFileSize(const string   & fname,
                     TIndx          & length,
                     CSeqDBLockHold & locked);

    /// Get size of a file.
    ///
    /// Check whether a file exists and get the file's size.  This
    /// version assumes the atlas lock is held.
    ///
    /// @param fname
    ///   The filename of the file to get the size of.
    /// @param length
    ///   The length of the file is returned here.
    /// @return
    ///   true if the file exists.
    bool GetFileSizeL(const string & fname, TIndx & length);

    /// Free allocated memory.
    ///
    /// This method releases the memory aquired 
    /// by the Alloc() method.  With
    /// data known to have originated in Alloc(), it is faster to call
    /// the Free() method.  This method assumes the lock is held.
    ///
    /// @param datap
    ///   Pointer to the data to release or deallocate.
    void RetRegion(const char * datap)
    {
        x_RetRegion(datap);
    }

    

    /// Allocate memory that atlas will keep track of.
    ///
    /// This method allocates memory for the calling code's use.
    /// There are three reasons to do this.  First, the allocated
    /// memory is guaranteed to be deleted when the atlas destructor
    /// runs, so using this method ties the lifetime of the allocated
    /// memory to that of the atlas, which may prevent memory leaks.
    /// Secondly, allocating memory in this way brings the allocated
    /// memory under the total memory bound the atlas imposes on
    /// itself.  The amount of memory assumed to be available for
    /// slice allocation will be reduced by the size of these
    /// allocations during garbage collection.  Thirdly, the memory
    /// allocated this way can be freed by the RetRegion(char*)
    /// method, so the RetSequence code in the volume layers (and
    /// thereabouts) does not need to can return allocated memory to
    /// the user as "sequence data", and does not have to track
    /// whether the data was allocated or mapped.
    ///
    /// @param length
    ///   Amount of memory to allocate in bytes.
    /// @param locked
    ///   The lock hold object for this thread.
    /// @param clear
    ///   Specify true to zero out memory contents.
    /// @return
    ///   A pointer to the allocation region of memory.
    char * Alloc(size_t length, CSeqDBLockHold & locked, bool clear = true);

    /// Return allocated memory.
    ///
    /// This method returns memory acquired from the Alloc() method.
    /// Dynamically allocated memory from other sources should not be
    /// freed with this method, and memory allocated via Alloc()
    /// should not be freed by any means other than this method    
    ///
    /// @param freeme
    ///   A pointer to memory allocated via Alloc().
    /// @param locked
    ///   The lock hold object for this thread.
    void Free(const char * freeme, CSeqDBLockHold & locked);

    /// Register externally allocated memory.
    ///
    /// This method tells the atlas code that memory was allocated
    /// external to the atlas code, and should be included under the
    /// memory bound enforced by the atlas.  These areas of memory
    /// will not be managed by the atlas, but may influence the atlas
    /// by causing database volume files or auxiliary files to be
    /// unmapped earlier or more often.  This method may trigger atlas
    /// garbage collection.  RegisterFree() should be called when the
    /// memory is freed.
    ///
    /// @param memreg
    ///   Memory registration tracking object.
    /// @param bytes
    ///   Amount of memory externally allocated.
    /// @param locked
    ///   The lock hold object for this thread.
    void RegisterExternal(CSeqDBMemReg   & memreg,
                          size_t           bytes,
                          CSeqDBLockHold & locked);

    /// Unregister externally allocated memory.
    ///
    /// This method tells the atlas that external memory registered
    /// with RegisterExternal() has been freed.  The atlas lock is
    /// assumed to be held.
    ///
    /// @param memreg
    ///   Memory registration tracking object.
    void UnregisterExternal(CSeqDBMemReg & memreg);

    /// Lock the atlas.
    ///
    /// If the lock hold object passed to
    /// this method is already in a "locked" state, this call is a
    /// noop.  Otherwise, the lock hold object is put in a locked
    /// state and the lock is acquired.
    ///
    /// @param locked
    ///   This object tracks whether this thread owns the mutex.
    void Lock(CSeqDBLockHold & locked)
    {
        if (! locked.m_Locked) {
            m_Lock.Lock();
            locked.m_Locked = true;
        }
    }

    /// Unlock the atlas.
    ///
    /// If the lock hold object passed to
    /// this method is already in an "unlocked" state, this call is a
    /// noop.  Otherwise, the lock hold object is put in an unlocked
    /// state and the lock is released.
    ///
    /// @param locked
    ///   This object tracks whether this thread owns the mutex.
    void Unlock(CSeqDBLockHold & locked)
    {
        if (locked.m_Locked) {
            locked.m_Locked = false;
            m_Lock.Unlock();
        }
    }

        
    /// Get the current slice size.
    ///
    /// This returns the current slice size used for mmap() style
    /// memory allocations.
    ///
    /// @return
    ///   Atlas will try to map this much data at a time.
    
    Uint8 GetSliceSize()
    {   
        Uint8 max_slice = e_MaxSlice64;
        Uint8 sliceSize = min(max_slice,m_MaxFileSize);
        return sliceSize;
    }
    
    /// Return the current number of bytes allocated.
    ///
    /// This returns the number of bytes currently allocated by the
    /// atlas code.  It does not include overhead or meta-data such as
    /// the CRegionMap objects or the atlas object itself.
    ///
    /// @return
    ///   The amount of memory allocated in bytes.
    TIndx GetCurrentAllocationTotal()
    {
        return m_CurAlloc;
    }
    
    /// Get BlastDB search path.
    const string GetSearchPath() const
    {
        return m_SearchPath;
    }

    /// Generate search path
    static const string GenerateSearchPath() {
        string splitter;
        string path;
#if defined(NCBI_OS_UNIX)
        splitter = ":";
#else
        splitter = ";";
#endif
        // Local directory first;
        path  = CDirEntry::NormalizePath(CDir::GetCwd(),eFollowLinks);
        path += splitter;
        // Then, BLASTDB;
        CNcbiEnvironment env;
        path += CDirEntry::NormalizePath(env.Get("BLASTDB"),eFollowLinks);
        path += splitter;
        // Finally, the config file.
        CNcbiApplication* app = CNcbiApplication::Instance();
        if (app) {
            const CNcbiRegistry& registry = app->GetConfig();
            if (registry.HasEntry("BLAST", "BLASTDB")) {
                path += CDirEntry::NormalizePath(registry.Get("BLAST", "BLASTDB"),eFollowLinks);
                path += splitter;
            }
        } 
        return path;
    }

    map<string, unique_ptr<CMemoryFile> >& GetFilesMemMap(void)
    {
        return m_FileMemMap;
    }

    enum EFilesCount{
        eFileCounterNoChange,
        eFileCounterIncrement,
        eFileCounterDecrement
    };

    int ChangeOpenedFilseCount(EFilesCount fc) 
    {
        switch(fc) {
        case eFileCounterIncrement:
            m_OpenedFilesCount++;            
            break;
        
        case eFileCounterDecrement:        
            m_OpenedFilesCount--;            
            break;
           
        default:
            break;
        }
        m_MaxOpenedFilesCount = max(m_MaxOpenedFilesCount,m_OpenedFilesCount);
        return m_OpenedFilesCount;
    }    

    int GetOpenedFilseCount(void) { return m_OpenedFilesCount;}
    
private:
    /// Private method to prevent copy construction.
    CSeqDBAtlas(const CSeqDBAtlas &);

    /// Iterator type for m_Pool member.
    typedef map<const char *, size_t>::iterator TPoolIter;

    /// Try to find the region and free it.
    ///
    /// This method looks for the region in the memory pool (m_Pool),
    /// which means it must have been allocated with Alloc().
    ///
    /// @param freeme
    ///   The pointer to free.
    /// @return
    ///   true, if the memory block was found and freed.
    bool x_Free(const char * freeme);

    
    /// Releases or deletes a memory region.
    ///
    /// It searches forallocated block that owns the specified address,
    /// and releases it, if found. 
    ///
    /// @param datap
    ///   Pointer to the area to delete or release a hold on.
    void x_RetRegion(const char * datap);
   
    /// Protects most of the critical regions of the SeqDB library.
    CMutex m_Lock;
    
    /// Bytes of "data" currently known to SeqDBAtlas.  This does not
    /// include metadata
    TIndx m_CurAlloc;

    /// Maps from pointers to dynamically allocated blocks to the byte
    /// size of the allocation.
    map<const char *, size_t> m_Pool;

    /// The most recently assigned FID.
    int m_LastFID;

    /// Lookup table of fids by filename.
    map<string, int> m_FileIDs;

    enum {e_MaxSlice64 = 1 << 30};
    
    /// Cache of file existence and length.
    map< string, pair<bool, TIndx> > m_FileSize;

    /// Maxium file size.
    Uint8 m_MaxFileSize;

    /// BlastDB search path.
    const string m_SearchPath;

    bool m_Alloc;//m_pool was used for mrmory allocation
    map<string, unique_ptr<CMemoryFile> > m_FileMemMap;    
    int m_OpenedFilesCount;
    int m_MaxOpenedFilesCount;
};


inline CSeqDBMemReg::~CSeqDBMemReg()
{
    m_Atlas.UnregisterExternal(*this);
}


/// Guard object for the SeqDBAtlas singleton.
///
/// The CSeqDBAtlas object is a singleton - only one exists at any
/// given time, and only if a CSeqDB object exists.  This object
/// implements that policy.  When no CSeqDBAtlas object exists, the
/// first CSeqDB object to be created will decide whether memory
/// mapping is enabled.  One of these objects exists in every
/// CSeqDBImpl and CSeqDBColumn object, and in the frames of a few
/// static functions.

class CSeqDBAtlasHolder {
public:
    /// Constructor.
    /// @param use_mmap If true, memory mapping will be used.
    /// @param flusher The garbage collection callback.
    /// @param locked The lock hold object for this thread (or NULL).
    CSeqDBAtlasHolder(bool             use_mmap,
                      //CSeqDBFlushCB  * flusher,
                      CSeqDBLockHold * lockedp);

    /// Destructor.
    ~CSeqDBAtlasHolder();

    /// Get the CSeqDBAtlas object.
    CSeqDBAtlas & Get();

private:

    /// Lock protecting this object's fields
    DECLARE_CLASS_STATIC_FAST_MUTEX(m_Lock);

    /// Count of users of the CSeqDBAtlas object.
    static int m_Count;

    /// The CSeqDBAtlas object itself.
    static CSeqDBAtlas * m_Atlas;
};


class CSeqDBFileMemMap {
public:

    typedef CNcbiStreamoff TIndx;
    /// Constructor
    ///
    /// Initializes a memory map object.
    ///
    /// @param filename
    ///   file to memory map    
    CSeqDBFileMemMap(class CSeqDBAtlas & atlas, const string filename)
        : m_Atlas(atlas),
          m_DataPtr (NULL),
          m_MappedFile( NULL),
          m_Mapped(false)
    {
        Init(filename);
    }

    CSeqDBFileMemMap(class CSeqDBAtlas & atlas)
        : m_Atlas(atlas),
          m_DataPtr (NULL),
          m_MappedFile( NULL),
          m_Mapped(false)
    {
        
    }

    /// Destructor    
    ~CSeqDBFileMemMap()
    {
        Clear();
    }

    /// Initializes a memory map object.
    ///
    /// @param filename
    ///   file to memory map    
    void Init(const string filename) {                
        if(!m_MappedFile || m_Filename != filename)
        {
            m_Filename = filename;
            Init();
        }
    }

    //m_Filename is set
    void Init(void) {            

            
        map<string, unique_ptr<CMemoryFile> >& fileMemMap =
            m_Atlas.GetFilesMemMap();
            if(IsIndexFile() && fileMemMap.count(m_Filename) > 0) {        
                m_MappedFile = fileMemMap[m_Filename].get();                
                x_LogMessage(eMapExists);       
            }
            else {
                try {                                      
                    if(IsIndexFile()) {
                        CSeqDBLockHold locked(m_Atlas);
                        m_Atlas.Lock(locked);                                                                        
                        if(fileMemMap.count(m_Filename) == 0) {      
                            m_MappedFile = new CMemoryFile(m_Filename);
                            fileMemMap.insert(map<string, unique_ptr<CMemoryFile> >::value_type(m_Filename,unique_ptr<CMemoryFile>(m_MappedFile)));                                                          
                            m_Atlas.ChangeOpenedFilseCount(CSeqDBAtlas::eFileCounterIncrement);
                            x_LogMessage(eMapNewLocked);
                        }
                        else {                                     
                            m_MappedFile = fileMemMap[m_Filename].get();                            
                            x_LogMessage(eMapExistsLocked);
                        }                        
                    }
                    else {
                        m_MappedFile = new CMemoryFile(m_Filename);		                       
                        m_Atlas.ChangeOpenedFilseCount(CSeqDBAtlas::eFileCounterIncrement);                        
                        x_LogMessage(eMapNew);
                    }                     
                    m_Mapped = true;                        
                }
                catch(...) {                     
                     x_LogMessage(eMapError);                     
                     NCBI_THROW(CSeqDBException,
                                eFileErr,
                                "Cannot memory map " + m_Filename + ". Number of files opened: "
                                + NStr::IntToString(m_Atlas.GetOpenedFilseCount()));                    
                }
            }            
            
            m_DataPtr = (char *) (m_MappedFile->GetPtr());            
    }

    
    /// Clears the memory mapobject.
    ///    
    void Clear()
    {
        
        if(m_MappedFile && m_Mapped && !IsIndexFile()) { 
                m_MappedFile->Unmap();		                                                        
                m_Atlas.ChangeOpenedFilseCount(CSeqDBAtlas::eFileCounterDecrement);                        
                x_LogMessage(eUnmap);
                delete m_MappedFile;
                m_MappedFile = NULL;
                m_Mapped = false;
        }        
    }

    bool IsMapped(){return m_Mapped;}

    /// Get a pointer to the specified offset.
    ///
    /// Given an offset (which is assumed to be available here), this
    /// method returns a pointer to the data at that offset.
    ///
    /// @param offset
    ///   The required offset relative to the start of the file.
    /// @return
    ///   A pointer to the data at the requested location.       
    const char *GetFileDataPtr(const string   & fname,TIndx offset)                    
    {
        if(!m_MappedFile || m_Filename != fname) {
            Init(fname);                        
        }

        return(const char *)(m_DataPtr + offset);    
    }

    const char *GetFileDataPtr(TIndx offset)                    
    {
        return(const char *)(m_DataPtr + offset);    
    }

    bool IsIndexFile() {
        bool isIndex = (NStr::Find(m_Filename,".pin") != NPOS ||  NStr::Find(m_Filename,".nin") != NPOS) ? true : false;      
        return isIndex;
    }

private:
    enum ELogMessage {
        eNone,
        eMapNew,
        eMapNewLocked,
        eMapExists,
        eMapExistsLocked,
        eUnmap,
        eMapError
    };

    void x_LogMessage(ELogMessage em,CSeqDBAtlas::EFilesCount fc = CSeqDBAtlas::eFileCounterNoChange)
    {
        //Disable logging
        if(em != eMapError) return; 
        int openedFilesCount;
        if(fc != CSeqDBAtlas::eFileCounterNoChange)
        {
            CSeqDBLockHold locked(m_Atlas);
            m_Atlas.Lock(locked);          
            openedFilesCount = m_Atlas.ChangeOpenedFilseCount(fc);
        }
        else {
            openedFilesCount = m_Atlas.GetOpenedFilseCount();        
        }
        int threadID = CThread::GetSelf();
                
        string logMessage;
        switch (em) {
            case eMapNew: 
                logMessage = "Map    new   :";
                break;

            case eMapNewLocked: 
                logMessage = "Map    new(l):";
                break;

            case eMapExists:
                logMessage = "Map exists   :";                
                break;

            case eMapExistsLocked:
                logMessage = "Map exists(l):";               
                break;

            case eUnmap:
                logMessage = "Unmap        :";
                break;

            case eMapError:
                logMessage = "Error memory mapping:";                
                break;

            case eNone:
                break;

            default:
                break;
        }
        if(!logMessage.empty()) {
            cerr << logMessage << m_Filename  << " openedFilesCount=" << openedFilesCount << " threadID=" << threadID << endl;
        }
    }
    CSeqDBAtlas & m_Atlas;    
     /// Points to the beginning of the data area.
    const char * m_DataPtr;

    string m_Filename;

    CMemoryFile *m_MappedFile;

    bool m_Mapped;
};



END_NCBI_SCOPE

#endif // OBJTOOLS_READERS_SEQDB__SEQDBATLAS_HPP

