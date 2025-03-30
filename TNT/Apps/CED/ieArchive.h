#ifndef _ieArchive_h_
#define	_ieArchive_h_
// ieArchive.h : header file
//

#include <afxtempl.h>

#include "x_types.hpp"
#include "x_math.hpp"

/////////////////////////////////////////////////////////////////////////////
// CieArchive Tags & Types

//---	System Tags begin at 0xC000, below that are user
enum {
	IE_TAG_START = 0xC000,
	IE_TAG_END,
	IE_TAG_ARCHIVE_TYPE,			// CString Type of Archive
	IE_TAG_ARCHIVE_NAME,			// CString Name of Archive
	IE_TAG_ARCHIVE_VERSION,			// u32 Archive Version v(u16)000.(u16)00
	IE_TAG_ARCHIVE_DATE,			// CString Date of Archive creation
} ;

//---	System Types begin at 0x8000, below that are user
//		User 0x0000 - 0x3FFF are fixed len
//		User 0x4000 - 0x4000 are variable len
enum {
	//---	0x8000 - 0xBFFF simple types
	IE_TYPE_FIXEDLEN = 0x8000,
	IE_TYPE_u8,
	IE_TYPE_u16,
	IE_TYPE_u32,
	IE_TYPE_s8,
	IE_TYPE_s16,
	IE_TYPE_s32,
	IE_TYPE_f32,
	IE_TYPE_f64,
	IE_TYPE_bool,
	IE_TYPE_END,						// Group/Struct End
	IE_TYPE_vector3,
	IE_TYPE_matrix4,

	//---	0xC000 - 0xFFFF Variable types, followed by u32 Length
	IE_TYPE_VARIABLELEN = 0xC000,
	IE_TYPE_START,						// Group/Struct Start
	IE_TYPE_string,
} ;

#define IS_TYPE_FIXEDLEN(a) ((a & 0x4000) == 0)
#define IS_TYPE_VARIABLELEN(a) ((a & 0x4000) != 0)

/////////////////////////////////////////////////////////////////////////////
// CieArchive

class CieArchive : public CObject
{
	DECLARE_DYNCREATE(CieArchive)

// Attributes
public:
	CFile			*m_pFile ;							// Pointer to File
	bool			m_IsStoring ;						// TRUE = storing
	CDocument		*m_pDocument ;						// Document Pointer

	u32				m_wTagPosition ;					// Position of Last Tag Written
	u16				m_wTag ;							// Tag written
	u16				m_wType ;							// Type written

	u16				m_rTag ;							// Tag read from file
	u16				m_rType ;							// Type read from file
	u32				m_rLength ;							// Length read from file

	CList<u32,u32>	m_StructStack ;						// Stack of structure positions

// Constructors
public:
	CieArchive() ;
	CieArchive(CFile *pFile, bool IsStoring, CDocument *pDoc) ; // Construct with File attached
	~CieArchive() ;

// Operations
public:
//	CieArchive &operator=( CieArchive & ) ;				// Right side is the argument.

	bool WriteTag(u16 Type, u16 Tag) ;					// Write Tag to Archive
	bool PatchTagLength(u32 PatchPosition) ;

	bool Writeu8 (u8  Value) ;
	bool Writeu16(u16 Value) ;
	bool Writeu32(u32 Value) ;
	bool Writes8 (s8  Value) ;
	bool Writes16(s16 Value) ;
	bool Writes32(s32 Value) ;
	bool Writef32(f32 Value) ;
	bool Writef64(f64 Value) ;
	bool Writevector3(vector3  Value) ;
	bool Writematrix4(matrix4 Value) ;
	bool Writebool(bool Value) ;
	bool WriteString(CString &String) ;

	bool WriteTaggedu8 (u16 Tag, u8  Value) ;
	bool WriteTaggedu16(u16 Tag, u16 Value) ;
	bool WriteTaggedu32(u16 Tag, u32 Value) ;
	bool WriteTaggeds8 (u16 Tag, s8  Value) ;
	bool WriteTaggeds16(u16 Tag, s16 Value) ;
	bool WriteTaggeds32(u16 Tag, s32 Value) ;
	bool WriteTaggedf32(u16 Tag, f32 Value) ;
	bool WriteTaggedf64(u16 Tag, f64 Value) ;
	bool WriteTaggedvector3(u16 Tag, vector3  Value) ;
	bool WriteTaggedmatrix4(u16 Tag, matrix4 Value) ;
	bool WriteTaggedbool(u16 Tag, bool Value) ;
	bool WriteTaggedString(u16 Tag, CString &String) ;

	bool ReadTag() ;									// Read Tag from Archive

	bool Readu8 (u8  &Value) ;
	bool Readu16(u16 &Value) ;
	bool Readu32(u32 &Value) ;
	bool Reads8 (s8  &Value) ;
	bool Reads16(s16 &Value) ;
	bool Reads32(s32 &Value) ;
	bool Readf32(f32 &Value) ;
	bool Readf64(f64 &Value) ;
	bool Readvector3(vector3  &Value) ;
	bool Readmatrix4(matrix4 &Value) ;
	bool Readbool(bool &Value) ;
	bool ReadString(CString &String) ;

	bool ReadSkip() ;

	u32	GetTypeLength(u16 Type) ;

	bool IsStoring() ;

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_ieArchive_h_
