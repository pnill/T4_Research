// LocoAnimTable.cpp: implementation of the CLocoAnimTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "LocoAnimTable.h"
#include "AnimTreeNode.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLocoAnimTable::CLocoAnimTable()
{
	m_Name				= "NEW LOCOMOTION TABLE";
	m_Type				= TABLETYPE_LOCOMOTION;
	m_Orientation		= ORIENTATION_POINTFOREWARD;
	m_Flags				= FALSE;
	m_PhaseShift		= 0.0f;
	m_MaxNSR			= 1.0f;
	m_OverlapNSR		= 0.0f;
	m_OverlapArc		= 0.0f;
	m_CenterNSR			= 1.0f;
	m_CenterAnim		= -1; // ANIM_NONE
	m_ReorientAnim		= "NONE" ;
	m_bDoNotExport		= FALSE;
	m_IndexAngleStart	= INDEXANGLESTART_DESTFACEDIR;
	m_IndexAngleEnd		= INDEXANGLEEND_CURMOVEDIR;

	//---	create a default arc
//	CLocoArcData* pArcData = new CLocoArcData;
//	m_ArcDataList.AddHead( pArcData );
}

CLocoAnimTable::~CLocoAnimTable()
{
	EmptyList();
}

void CLocoAnimTable::Serialize(CArchive &ar)
{
	int version = 7;
	POSITION pos;
	CLocoArcData* pArcData;
	int count;
	CString str;
	int ListLen;
	int i;

	if (ar.IsStoring())
	{
		ar << version;

		ar.WriteString( m_Name );
		ar.WriteString( "\xd\xa" );
		ar << m_Orientation;
		ar << m_PhaseShift;
		ar << m_MaxNSR;
		ar << m_OverlapNSR;
		ar << m_CenterNSR;
		ar << m_CenterAnim;

		//---	add new saved infor here   ---
		ar << m_IndexAngleStart;
		ar << m_IndexAngleEnd;

		ar << m_bDoNotExport;

		ar << m_OverlapArc;

		ar.WriteString( m_Callback );
		ar.WriteString( "\xd\xa" );
		ar << m_Type;
		ar << m_Flags;
		ar.WriteString( m_ReorientAnim );
		ar.WriteString( "\xd\xa" );

//		m_AnimationList.Serialize( ar );
		ListLen = m_AnimationList.GetSize();
		ar << ListLen;
		for( i=0; i<ListLen; i++ )
		{
			str = m_AnimationList[i];
			ar.WriteString( str );
			ar.WriteString( "\xd\xa" );
		}

		//---	store the Anim/NSR list
		ar << m_ArcDataList.GetCount();
		pos = m_ArcDataList.GetHeadPosition();
		while( pos )
		{
			pArcData = m_ArcDataList.GetNext( pos );
			pArcData->Serialize( ar );
		}
	}
	else
	{
		ar >> version;

		ar.ReadString( m_Name );
		ar >> m_Orientation;
		ar >> m_PhaseShift;
		ar >> m_MaxNSR;
		ar >> m_OverlapNSR;
		ar >> m_CenterNSR;
		ar >> m_CenterAnim;

		switch( version )
		{
		case 7:
			ar >> m_IndexAngleStart;
			ar >> m_IndexAngleEnd;
		case 6:
			ar >> m_bDoNotExport;
		case 5:
			ar >> m_OverlapArc;
		case 4:
			ar.ReadString( m_Callback );
		case 3:
			ar >> m_Type;
		case 2:
			ar >> m_Flags;
			ar.ReadString( m_ReorientAnim );
			break;
		}

		ar >> ListLen;
		for( i=0; i<ListLen; i++ )
		{
			ar.ReadString( str );
			m_AnimationList.Add( str );
		}

		//---	empty the old list
		EmptyList();

		//---	load the Anim/NSR list
		ar >> count;
		while( count-- )
		{
			pArcData = new CLocoArcData;
			pArcData->Serialize( ar );
			m_ArcDataList.AddTail( pArcData );
		}

		//---	conversion from type 6 and below to type 7 data
		if( version <= 6 )
		{
			if( m_Flags & TABLEFLAG_FACEDIR )
				m_Type = TABLETYPE_TURN;

			if( m_Type == TABLETYPE_LOCOMOTION )
			{
				m_IndexAngleStart	= INDEXANGLESTART_DESTFACEDIR;
				m_IndexAngleEnd		= INDEXANGLEEND_CURMOVEDIR;
			}

			if( m_Type == TABLETYPE_CUT )
			{
				m_IndexAngleStart	= INDEXANGLESTART_CURMOVEDIR;
				m_IndexAngleEnd		= INDEXANGLEEND_DESTMOVEDIR;
			}

			if( m_Type == TABLETYPE_TURN )
			{
				m_IndexAngleStart	= INDEXANGLESTART_CURFACEDIR;
				m_IndexAngleEnd		= INDEXANGLEEND_DESTFACEDIR;
			}

			if( m_Flags & TABLEFLAG_FIXED )
				m_IndexAngleStart	= INDEXANGLESTART_FIXED;
		}
	}
}


void CLocoAnimTable::Export( FILE* pFile, class CAnimTblEdDoc* pDoc, int Phase )
{
	POSITION pos;
	CString Name;

	if (m_bDoNotExport) return;
	if (Phase == 0)	return;	// if this is the defines phase, return
	if (m_ArcDataList.GetCount() == 0) return;

	Name = GetExportName();

	if (Phase == 1)
	{
		//---	export the children first
		pos = m_ArcDataList.GetHeadPosition();
		int i = 0;
		while( pos )
			m_ArcDataList.GetNext( pos )->Export( pFile, pDoc, 1, Name, i++, m_AnimationList, m_CenterNSR, m_OverlapNSR );

		//---	print the header information
		fprintf( pFile, "\n//---   Locomotion Table %s - Arc Data\n", Name );
		fprintf( pFile, "t_SLOCTBL_ArcData	%s_ArcData[%d]=\n{\n", Name, m_ArcDataList.GetCount() );

		//---	add the children to the list
		pos = m_ArcDataList.GetHeadPosition();
		i=0;
		while( pos )
			m_ArcDataList.GetNext( pos )->Export( pFile, pDoc, 2, Name, i++, m_AnimationList, m_CenterNSR, m_OverlapNSR );
		fprintf( pFile, "};\n");

		return;
	}

	CAnimTreeNode* pCenterAnim = (m_CenterAnim != -1 ) ? pDoc->m_AnimTree.FindTreeNodeByName( m_AnimationList[ m_CenterAnim ] ) : NULL;
	CAnimTreeNode* pReorientNode = pDoc->m_AnimTree.FindTreeNodeByName( m_ReorientAnim );

	CString Type;
	switch( m_Type )
	{
	case TABLETYPE_LOCOMOTION:	Type = "LOCTBL_TYPE_LOCOMOTION,";	break;
	case TABLETYPE_CUT:			Type = "LOCTBL_TYPE_CUT,       ";	break;
	case TABLETYPE_TURN:		Type = "LOCTBL_TYPE_TURN,      ";	break;
	case TABLETYPE_UNDEFINED:	Type = "LOCTBL_TYPE_UNDEFINED, ";	break;
	}

	CString Flags = "";
//	if ((m_Type == TABLETYPE_LOCOMOTION) && (m_Flags & TABLEFLAG_FIXED))
//		Flags += "LOCTBL_FLAG_FIXED|";
//	if ((m_Type == TABLETYPE_CUT) && (m_Flags & TABLEFLAG_FACEDIR))
//		Flags += "LOCTBL_FLAG_FACEDIR|";
	if (m_Flags & TABLEFLAG_LIMITNSR)
		Flags += "LOCTBL_FLAG_LIMITNSR|";

	if (Flags.GetLength() == 0)
		Flags = "0,                   ";
	else
	{
		Flags = Flags.Left( Flags.GetLength() - 1 );
		Flags += ",";
	}

	CString IndexMethod;
	switch( m_IndexAngleStart )
	{
	case INDEXANGLESTART_FIXED:
		switch( m_IndexAngleEnd )
		{
		case INDEXANGLEEND_CURFACEDIR:		IndexMethod	= "LOCTBL_INDEX_FIXED_CFACEDIR,   ";	break;
		case INDEXANGLEEND_CURMOVEDIR:		IndexMethod	= "LOCTBL_INDEX_FIXED_CMOVEDIR,   ";	break;
		case INDEXANGLEEND_DESTFACEDIR:		IndexMethod	= "LOCTBL_INDEX_FIXED_DFACEDIR,   ";	break;
		case INDEXANGLEEND_DESTMOVEDIR:		IndexMethod	= "LOCTBL_INDEX_FIXED_DMOVEDIR,   ";	break;
		}
		break;

	case INDEXANGLESTART_CURFACEDIR:
		switch( m_IndexAngleEnd )
		{
		case INDEXANGLEEND_CURFACEDIR:		IndexMethod	= "LOCTBL_INDEX_ZERO,             ";	break;
		case INDEXANGLEEND_CURMOVEDIR:		IndexMethod	= "LOCTBL_INDEX_CFACEDIR_CMOVEDIR,";	break;
		case INDEXANGLEEND_DESTFACEDIR:		IndexMethod	= "LOCTBL_INDEX_CFACEDIR_DFACEDIR,";	break;
		case INDEXANGLEEND_DESTMOVEDIR:		IndexMethod	= "LOCTBL_INDEX_CFACEDIR_DMOVEDIR,";	break;
		}
		break;

	case INDEXANGLESTART_CURMOVEDIR:
		switch( m_IndexAngleEnd )
		{
		case INDEXANGLEEND_CURFACEDIR:		IndexMethod	= "LOCTBL_INDEX_CMOVEDIR_CFACEDIR,";	break;
		case INDEXANGLEEND_CURMOVEDIR:		IndexMethod	= "LOCTBL_INDEX_ZERO,             ";	break;
		case INDEXANGLEEND_DESTFACEDIR:		IndexMethod	= "LOCTBL_INDEX_CMOVEDIR_DFACEDIR,";	break;
		case INDEXANGLEEND_DESTMOVEDIR:		IndexMethod	= "LOCTBL_INDEX_CMOVEDIR_DMOVEDIR,";	break;
		}
		break;

	case INDEXANGLESTART_DESTFACEDIR:
		switch( m_IndexAngleEnd )
		{
		case INDEXANGLEEND_CURFACEDIR:		IndexMethod	= "LOCTBL_INDEX_DFACEDIR_CFACEDIR,";	break;
		case INDEXANGLEEND_CURMOVEDIR:		IndexMethod	= "LOCTBL_INDEX_DFACEDIR_CMOVEDIR,";	break;
		case INDEXANGLEEND_DESTFACEDIR:		IndexMethod	= "LOCTBL_INDEX_ZERO,             ";	break;
		case INDEXANGLEEND_DESTMOVEDIR:		IndexMethod	= "LOCTBL_INDEX_DFACEDIR_DMOVEDIR,";	break;
		}
		break;

	case INDEXANGLESTART_DESTMOVEDIR:
		switch( m_IndexAngleEnd )
		{
		case INDEXANGLEEND_CURFACEDIR:		IndexMethod	= "LOCTBL_INDEX_DMOVEDIR_CFACEDIR,";	break;
		case INDEXANGLEEND_CURMOVEDIR:		IndexMethod	= "LOCTBL_INDEX_DMOVEDIR_CMOVEDIR,";	break;
		case INDEXANGLEEND_DESTFACEDIR:		IndexMethod	= "LOCTBL_INDEX_DMOVEDIR_DFACEDIR,";	break;
		case INDEXANGLEEND_DESTMOVEDIR:		IndexMethod	= "LOCTBL_INDEX_ZERO,             ";	break;
		}
		break;
	}

	//---	print the header information
	fprintf( pFile, "\n\t//---   Locomotion Animation Table '%s'\n", m_Name );
	fprintf( pFile, "\t{ " );
	fprintf( pFile, "%s ", Type );
	fprintf( pFile, "%s ", Flags );
	fprintf( pFile, "%s ", IndexMethod );
	fprintf( pFile, "%2d, ", m_ArcDataList.GetCount() );
	fprintf( pFile, "ANIM_%s, ", (pCenterAnim == NULL) ? "NONE" : pCenterAnim->GetName() );
	fprintf( pFile, "ANIM_%s, ", (pReorientNode == NULL) ? "NONE" : pReorientNode->GetName() );
	fprintf( pFile, "%7.4ff, ", m_PhaseShift-(m_OverlapArc/2.0f) );
	fprintf( pFile, "%7.4ff, ", m_OverlapArc );
	fprintf( pFile, "%7.4ff, ", m_CenterNSR );
	fprintf( pFile, "%7.4ff, ", m_MaxNSR );
	fprintf( pFile, "%s, ",  (m_Callback == "") ? "NULL" : "LOCCB_" + m_Callback );
	fprintf( pFile, "%s_ArcData },", Name );
	fprintf( pFile, "\n" );
}


CLocoAnimTable& CLocoAnimTable::operator =(CLocoAnimTable &Other)
{
	CLocoArcData* pArcData;
	CLocoArcData* pNewArcData;
	POSITION pos;
	CString str;
	int i;

	//---	copy member variables
	m_Name				= Other.m_Name;
	m_Type				= Other.m_Type;
	m_Orientation		= Other.m_Orientation;
	m_PhaseShift		= Other.m_PhaseShift;
	m_Flags				= Other.m_Flags;
	m_MaxNSR			= Other.m_MaxNSR;
	m_OverlapNSR		= Other.m_OverlapNSR;
	m_OverlapArc		= Other.m_OverlapArc;
	m_CenterNSR			= Other.m_CenterNSR;
	m_CenterAnim		= Other.m_CenterAnim;
	m_ReorientAnim		= Other.m_ReorientAnim;
	m_Callback			= Other.m_Callback;
	m_pOldSelf			= Other.m_pOldSelf;
	m_bDoNotExport		= Other.m_bDoNotExport;
	m_IndexAngleStart	= Other.m_IndexAngleStart;
	m_IndexAngleEnd		= Other.m_IndexAngleEnd;
		
	//---	copy the animation list
	m_AnimationList.RemoveAll();
	for( i=0; i<Other.m_AnimationList.GetSize(); i++ )
	{
		str = Other.m_AnimationList[i];
		m_AnimationList.Add( str );
	}

	//---	copy the list
	EmptyList();
	pos = Other.m_ArcDataList.GetHeadPosition();
	while( pos )
	{
		pArcData = Other.m_ArcDataList.GetNext( pos );
		pNewArcData = new CLocoArcData;
		*pNewArcData = *pArcData;
		m_ArcDataList.AddTail( pNewArcData );
	}

	return *this;
}

int CLocoAnimTable::operator ==(CLocoAnimTable &Other)
{
	POSITION pos, pos2;
	CString str;
	int i;

	//---	check member variables
	if (m_Name				!= Other.m_Name)			return FALSE;
	if (m_Type				!= Other.m_Type)			return FALSE;
	if (m_Orientation		!= Other.m_Orientation)		return FALSE;
	if (m_PhaseShift		!= Other.m_PhaseShift)		return FALSE;
	if (m_Flags				!= Other.m_Flags)			return FALSE;
	if (m_MaxNSR			!= Other.m_MaxNSR)			return FALSE;
	if (m_OverlapNSR		!= Other.m_OverlapNSR)		return FALSE;
	if (m_OverlapArc		!= Other.m_OverlapArc)		return FALSE;
	if (m_CenterNSR			!= Other.m_CenterNSR)		return FALSE;
	if (m_CenterAnim		!= Other.m_CenterAnim)		return FALSE;
	if (m_ReorientAnim		!= Other.m_ReorientAnim)	return FALSE;
	if (m_Callback			!= Other.m_Callback)		return FALSE;
	if (m_IndexAngleStart	!= Other.m_IndexAngleStart)	return FALSE;
	if (m_IndexAngleEnd		!= Other.m_IndexAngleEnd)	return FALSE;
		
	//---	check the animation list
	for( i=0; i<Other.m_AnimationList.GetSize(); i++ )
		if (m_AnimationList[i] != Other.m_AnimationList[i]) return FALSE;

	//---	check the list
	pos = m_ArcDataList.GetHeadPosition();
	pos2 = Other.m_ArcDataList.GetHeadPosition();
	while( pos && pos2 )
		if (*m_ArcDataList.GetNext( pos ) != *Other.m_ArcDataList.GetNext( pos2 )) return FALSE;

	if (pos != pos2)
		return FALSE;

	return TRUE;
}

void CLocoAnimTable::EmptyList()
{
	while( m_ArcDataList.GetCount() )
	{
		delete m_ArcDataList.GetHead();
		m_ArcDataList.RemoveHead();
	}
}

CString CLocoAnimTable::GetExportName( void )
{
	CString Name = m_Name;
	char ch;

//	Name.MakeUpper();

	//---	remove any uncompileable characters
	for( int i=0; i<Name.GetLength(); i++ )
	{
		ch = Name[i];
		if (ch == ' ')
		{
			//Name.GetBuffer(Name.GetLength())[i] = '_';
			Name = Name.Left( i ) + Name.Right( Name.GetLength() - i - 1 );
		}
		else if (!(((ch >= 'A') && (ch <= 'Z'))
			|| ((ch >= 'a') && (ch <= 'z'))
			|| ((ch >= '0') && (ch <= '9'))))
		{
			Name = Name.Left( i ) + Name.Right( Name.GetLength() - i - 1 );
		}
	}

	//---	make sure the name doesn't begin with a number
	if ((Name[0] >= '0') && (Name[0] <= '9'))
		Name = CString("_") + Name.Right( Name.GetLength() );

	return CString("LOCTBL_") + Name;
}


CString CLocoAnimTable::GetExportDefine( void )
{
	CString Name = m_Name;
	char ch;

	Name.MakeUpper();

	//---	remove any uncompileable characters
	for( int i=0; i<Name.GetLength(); i++ )
	{
		ch = Name[i];
		if (ch == '_')
		{
			// do nothing
		}
		else if (ch == ' ')
		{
			Name.GetBuffer(Name.GetLength())[i] = '_';
			//Name = Name.Left( i ) + Name.Right( Name.GetLength() - i - 1 );
		}
		else if (!(((ch >= 'A') && (ch <= 'Z'))
			|| ((ch >= 'a') && (ch <= 'z'))
			|| ((ch >= '0') && (ch <= '9'))))
		{
			Name = Name.Left( i ) + Name.Right( Name.GetLength() - i - 1 );
		}
	}

	//---	make sure the name doesn't begin with a number
	if ((Name[0] >= '0') && (Name[0] <= '9'))
		Name = CString("_") + Name.Right( Name.GetLength() );

	return CString("LOCTBL_") + Name;
}

int CLocoAnimTable::RemoveAnim( CString Anim )
{
	int AnimIndex;
	int Count;

	//---	remove the animation from the list
	Count = m_AnimationList.GetSize();
	for( AnimIndex=0; AnimIndex<Count; AnimIndex++ )
		if (Anim == m_AnimationList[AnimIndex])
		{
			m_AnimationList.RemoveAt( AnimIndex );
			break;
		}

	if (AnimIndex == Count)
		return FALSE;

	//---	shift the AnimNSR zone's animation values
	POSITION pos = m_ArcDataList.GetHeadPosition();
	while( pos )
		m_ArcDataList.GetNext( pos )->RemoveAnim( AnimIndex );

	if (m_CenterAnim == AnimIndex) m_CenterAnim = -1;
	else if (m_CenterAnim > AnimIndex) m_CenterAnim -= 1;

	return TRUE;
}

int CLocoAnimTable::ChangeAnim( CString Anim, CString NewAnim )
{
	int AnimIndex;
	int Count;

	//---	remove the animation from the list
	Count = m_AnimationList.GetSize();
	for( AnimIndex=0; AnimIndex<Count; AnimIndex++ )
		if (Anim == m_AnimationList[AnimIndex])
		{
			m_AnimationList.RemoveAt( AnimIndex );
			break;
		}

	if (AnimIndex == Count)
		return FALSE;

	m_AnimationList.InsertAt( AnimIndex, NewAnim );

	return TRUE;
}

void CLocoAnimTable::InsertAnim( int Index, CString Anim )
{
	//---	insert the animation into the list
	if (Index == -1)	m_AnimationList.Add( Anim );
	else				m_AnimationList.InsertAt( Index, Anim );

	if (Index == -1)
		return;

	//---	shift the AnimNSR zone's animation values
	POSITION pos = m_ArcDataList.GetHeadPosition();
	while( pos )
		m_ArcDataList.GetNext( pos )->InsertAnim( Index );
}

void CLocoAnimTable::FillCallbackStringArray( CStringArray& rStringArray )
{
	int i;

	if (m_Callback != "")
	{
		//---	look for the given callback
		for (i=0; i<rStringArray.GetSize(); i++)
			if (m_Callback == rStringArray[i])
				break;

		//---	if it was not found, add it.
		if (i == rStringArray.GetSize())
			rStringArray.Add( m_Callback );
	}
}
