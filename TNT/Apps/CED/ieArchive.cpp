// ieArchive.cpp : implementation file
//

#include "stdafx.h"

#include "ieArchive.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CieArchive Implementation

IMPLEMENT_DYNCREATE(CieArchive, CObject)

/////////////////////////////////////////////////////////////////////////////
// CieArchive Constructors

CieArchive::CieArchive()
{
	ASSERT(0) ;
}

CieArchive::CieArchive(CFile *pFile, bool IsStoring, CDocument *pDoc)
{
	m_pFile = pFile ;
	m_IsStoring = IsStoring ;
	m_pDocument = pDoc ;
}

CieArchive::~CieArchive()
{
	ASSERT (m_StructStack.IsEmpty()) ;
}

bool CieArchive::IsStoring()
{
	return m_IsStoring ;
}

/////////////////////////////////////////////////////////////////////////////
// CieArchive Implementation

bool CieArchive::WriteTag(u16 Type, u16 Tag)
{
	bool	Success = TRUE ;			// Success Code

	//---	Setup Values for Tag written
	m_wTag = Tag ;
	m_wType = Type ;

	//---	Write the Tag
	try
	{
		m_pFile->Write(&Tag,sizeof(u16)) ;
		m_pFile->Write(&Type,sizeof(u16)) ;
		m_wTagPosition = (u32)m_pFile->GetPosition() ;

		//---	If the Tag is a struct start then push onto stack
		if (Type == IE_TYPE_START)
			m_StructStack.AddTail(m_wTagPosition) ;

		//---	If a Variable length tag then leave 4 bytes for length
		if (IS_TYPE_VARIABLELEN(Type))
			m_pFile->Seek(sizeof(u32), CFile::current) ;

		//---	If the Tag is a struct end then pop stack and write length
		if (Type == IE_TYPE_END)
		{
			if (!m_StructStack.IsEmpty())
			{
				Success &= PatchTagLength(m_StructStack.RemoveHead()) ;
			}
		}
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::PatchTagLength(u32 PatchPosition)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		u32 OldPosition = (u32)m_pFile->GetPosition() ;
		m_pFile->Seek(PatchPosition,CFile::begin) ;
		u32	Length = OldPosition-(PatchPosition+4) ;
		m_pFile->Write(&Length,sizeof(u32)) ;
		m_pFile->Seek(OldPosition,CFile::begin) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Writeu8(u8 Value)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		m_pFile->Write(&Value,sizeof(u8)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Writeu16(u16 Value)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		m_pFile->Write(&Value,sizeof(u16)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Writeu32(u32 Value)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		m_pFile->Write(&Value,sizeof(u32)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Writes8(s8 Value)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		m_pFile->Write(&Value,sizeof(s8)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Writes16(s16 Value)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		m_pFile->Write(&Value,sizeof(s16)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Writes32(s32 Value)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		m_pFile->Write(&Value,sizeof(s32)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Writef32(f32 Value)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		m_pFile->Write(&Value,sizeof(f32)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Writef64(f64 Value)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		m_pFile->Write(&Value,sizeof(f64)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Writevector3 (vector3  Value)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		m_pFile->Write(&Value,sizeof(vector3 )) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Writematrix4(matrix4 Value)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		m_pFile->Write(&Value,sizeof(matrix4)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Writebool(bool Value)
{
	bool	Success = TRUE ;			// Success Code

	try
	{
		m_pFile->Write(&Value,sizeof(bool)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteString(CString &String)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_wType == IE_TYPE_string) ;
	Success &= (m_wType == IE_TYPE_string) ;

	try
	{
		u32 Length = String.GetLength() ;
		m_pFile->Write((LPCTSTR)String,Length) ;
		PatchTagLength(m_wTagPosition) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggedu8(u16 Tag, u8 Value)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_u8, Tag) ;
	Success &= Writeu8 (Value) ;

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggedu16(u16 Tag, u16 Value)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_u16, Tag) ;
	Success &= Writeu16 (Value) ;

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggedu32(u16 Tag, u32 Value)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_u32, Tag) ;
	Success &= Writeu32 (Value) ;

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggeds8(u16 Tag, s8 Value)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_s8, Tag) ;
	Success &= Writes8 (Value) ;

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggeds16(u16 Tag, s16 Value)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_s16, Tag) ;
	Success &= Writes16 (Value) ;

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggeds32(u16 Tag, s32 Value)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_s32, Tag) ;
	Success &= Writes32 (Value) ;

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggedf32(u16 Tag, f32 Value)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_f32, Tag) ;
	Success &= Writef32 (Value) ;

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggedf64(u16 Tag, f64 Value)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_f64, Tag) ;
	Success &= Writef64 (Value) ;

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggedvector3 (u16 Tag, vector3  Value)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_vector3 , Tag) ;
	Success &= Writevector3  (Value) ;

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggedmatrix4(u16 Tag, matrix4 Value)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_matrix4, Tag) ;
	Success &= Writematrix4 (Value) ;

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggedbool(u16 Tag, bool Value)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_bool, Tag) ;
	Success &= Writebool (Value) ;

	//---	Return Success Code
	return Success ;
}

bool CieArchive::WriteTaggedString(u16 Tag, CString &String)
{
	bool	Success = TRUE ;			// Success Code

	Success &= WriteTag (IE_TYPE_string, Tag) ;
	Success &= WriteString (String) ;

	//---	Return Success Code
	return Success ;
}


bool CieArchive::ReadTag()
{
	bool	Success = TRUE ;			// Success Code

	//---	Read Tag
	Success &= (m_pFile->Read(&m_rTag,sizeof(u16)) == sizeof(u16)) ;
	Success &= (m_pFile->Read(&m_rType,sizeof(u16)) == sizeof(u16)) ;
	if (IS_TYPE_FIXEDLEN(m_rType))
	{
		//---	Set Length
		m_rLength = GetTypeLength(m_rType) ;
	}
	else
	{
		//---	Read Length from File
		Success &= (m_pFile->Read(&m_rLength,sizeof(u32)) == sizeof(u32)) ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Readu8(u8 &Value)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_u8) ;
	Success &= (m_rType == IE_TYPE_u8) ;

	try
	{
		m_pFile->Read(&Value,sizeof(u8)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Readu16(u16 &Value)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_u16) ;
	Success &= (m_rType == IE_TYPE_u16) ;

	try
	{
		m_pFile->Read(&Value,sizeof(u16)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Readu32(u32 &Value)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_u32) ;
	Success &= (m_rType == IE_TYPE_u32) ;

	try
	{
		m_pFile->Read(&Value,sizeof(u32)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Reads8(s8 &Value)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_s8) ;
	Success &= (m_rType == IE_TYPE_s8) ;

	try
	{
		m_pFile->Read(&Value,sizeof(s8)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Reads16(s16 &Value)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_s16) ;
	Success &= (m_rType == IE_TYPE_s16) ;

	try
	{
		m_pFile->Read(&Value,sizeof(s16)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Reads32(s32 &Value)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_s32) ;
	Success &= (m_rType == IE_TYPE_s32) ;

	try
	{
		m_pFile->Read(&Value,sizeof(s32)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Readf32(f32 &Value)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_f32) ;
	Success &= (m_rType == IE_TYPE_f32) ;

	try
	{
		m_pFile->Read(&Value,sizeof(f32)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Readf64(f64 &Value)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_f64) ;
	Success &= (m_rType == IE_TYPE_f64) ;

	try
	{
		m_pFile->Read(&Value,sizeof(f64)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Readvector3 (vector3  &Value)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_vector3 ) ;
	Success &= (m_rType == IE_TYPE_vector3 ) ;

	try
	{
		m_pFile->Read(&Value,sizeof(vector3 )) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Readmatrix4(matrix4 &Value)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_matrix4) ;
	Success &= (m_rType == IE_TYPE_matrix4) ;

	try
	{
		m_pFile->Read(&Value,sizeof(matrix4)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::Readbool(bool &Value)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_bool) ;
	Success &= (m_rType == IE_TYPE_bool) ;

	try
	{
		m_pFile->Read(&Value,sizeof(bool)) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

bool CieArchive::ReadString(CString &String)
{
	bool	Success = TRUE ;			// Success Code

	//---	Assert Type
	ASSERT (m_rType == IE_TYPE_string) ;
	Success &= (m_rType == IE_TYPE_string) ;

	try
	{
		LPTSTR Buffer = String.GetBuffer(m_rLength) ;
		m_pFile->Read(Buffer,m_rLength) ;
		String.ReleaseBuffer(m_rLength) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

//	Skip Current Item
bool CieArchive::ReadSkip()
{
	bool	Success = TRUE ;			// Success Code
	u32		Length ;					// Length to Skip

	//---	Seek beyond current Item
	try
	{
		Length = GetTypeLength(m_rType) ;
		m_pFile->Seek(Length,CFile::current) ;
	}
	catch(...)
	{
		Success = FALSE ;
	}

	//---	Return Success Code
	return Success ;
}

u32 CieArchive::GetTypeLength(u16 Type)
{
	u32		Length = 0 ;

	switch (Type)
	{
	case IE_TYPE_u8:
	case IE_TYPE_s8:
		Length = 1 ;
		break ;
	case IE_TYPE_u16:
	case IE_TYPE_s16:
		Length = 2 ;
		break ;
	case IE_TYPE_u32:
	case IE_TYPE_s32:
		Length = 4 ;
		break ;
	case IE_TYPE_f32:
		Length = 4 ;
		break ;
	case IE_TYPE_f64:
		Length = 8 ;
		break ;
	default:
		if (IS_TYPE_VARIABLELEN(Type))
		{
			Length = m_rLength ;
		}
		break ;
	}

	//---	Return the Length
	return Length ;
}

/////////////////////////////////////////////////////////////////////////////
// CieArchive Overridden operators
/*
CieArchive &CieArchive::operator=( CieArchive &ptArchive )
{
	Copy (*this, ptArchive) ;

    return *this;  // Assignment operator returns left side.
}
*/
/////////////////////////////////////////////////////////////////////////////
// CieArchive diagnostics

#ifdef _DEBUG
void CieArchive::AssertValid() const
{
	CObject::AssertValid();
}

void CieArchive::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG
