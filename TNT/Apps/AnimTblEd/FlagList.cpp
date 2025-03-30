// FlagList.cpp: implementation of the CFlagList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTreeNode.h"
#include "AnimNode.h"
#include "GroupNode.h"
#include "FlagList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


char* DefaultFlags[ NUM_DEFAULT_FLAGS ] =
{
	"CYCLING",
	"RATEANALOG",
	"NOMOVEDIR",
	"RATERAMPED",
	"READVELOCITY",
	"LOCKFACEDIR",
	"NODISPLACEMENT",
	"USE_DEST_SPEED"
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFlagList::CFlagList()
{
	int i;

	for( i=0; i<NUM_DEFAULT_FLAGS; i++ )
		AddFlag( CString( DefaultFlags[ i ] ) );
}

CFlagList::~CFlagList()
{
	MakeEmpty();
}

void CFlagList::AddFlag(CString &Name, int Index)
{
	int i;

	//---	verify that a flag of the same name doesn't already exist
	for(i=0; i<GetNumFlags(); i++)
	{
		if (Name == GetFlag( i )->GetName())
			return;
	}

	CFlag *pNewFlag = new CFlag(this, Name);

	if (Index >= 0)
	{
		if (Index < NUM_DEFAULT_FLAGS)
			Index = NUM_DEFAULT_FLAGS;

		POSITION pos = m_FlagList.FindIndex(Index);
		if (pos)
		{
			m_FlagList.InsertBefore(pos, pNewFlag);
			return;
		}
	}

	m_FlagList.AddTail(pNewFlag);
}

void CFlagList::RemoveAll( void )
{
	CFlag* pFlag;
	POSITION pos;

	pos = m_FlagList.GetHeadPosition();
	while( pos )
	{
		pFlag = m_FlagList.GetAt(pos);
		m_FlagList.RemoveAt(pos);
		delete pFlag;

		pos = m_FlagList.GetHeadPosition();
	}
}

int CFlagList::RemoveFlag(int Index)
{
	POSITION pos;
	CFlag *pFlag;

	//--	default flags cannot be removed
	if (Index < NUM_DEFAULT_FLAGS)
		return FALSE;

	pos = m_FlagList.FindIndex(Index);

	if (!pos)
		return FALSE;

	pFlag = m_FlagList.GetAt(pos);
	delete pFlag;
	m_FlagList.RemoveAt(pos);

	return TRUE;
}

int CFlagList::RemoveFlag(CString &Name)
{
	CFlag *pFlag;
	POSITION pos;
	int i;

	for (i=0; i<GetTotalFlagBits(); i++)
	{
		pos = m_FlagList.FindIndex(i);
		if (pos)
		{
			pFlag = m_FlagList.GetAt(pos);
			if (pFlag->GetName() == Name)
			{
				//--	default flags cannot be removed
				if (i < NUM_DEFAULT_FLAGS)
					return FALSE;

				m_FlagList.RemoveAt(pos);
				delete pFlag;
				return TRUE;
			}
		}
	}

	return FALSE;
}

int CFlagList::RemoveFlag(CFlag *pFlag)
{
	CFlag *pFoundFlag;
	POSITION pos;
	int i;

	for (i=0; i<GetTotalFlagBits(); i++)
	{
		pos = m_FlagList.FindIndex(i);
		if (pos)
		{
			pFoundFlag = m_FlagList.GetAt(pos);
			if (pFoundFlag == pFlag)
			{
				//--	default flags cannot be removed
				if (i < NUM_DEFAULT_FLAGS)
					return FALSE;

				m_FlagList.RemoveAt(pos);
				delete pFoundFlag;
				return TRUE;
			}
		}
	}

	return FALSE;
}

int CFlagList::GetTotalFlagBits()
{
	return GetNumFlags();
}

CFlag* CFlagList::GetFlag(int Index)
{
	POSITION pos;

	pos = m_FlagList.FindIndex(Index);

	if (!pos)
		return NULL;

	return m_FlagList.GetAt(pos);
}

void CFlagList::MakeEmpty()
{
	int i=0;

	for (i=GetTotalFlagBits()-1; i>=0; i--)
		RemoveFlag(i);
}

CFlagList& CFlagList::operator =(CFlagList &Other)
{
	int i;
	CFlag *pFlag;
	CFlag *pNewFlag;

	RemoveAll();

	for (i=0/*NUM_DEFAULT_FLAGS*/; i<Other.GetTotalFlagBits(); i++)
	{
		pFlag = Other.GetFlag(i);
		if (pFlag)
		{
			pNewFlag = new CFlag(this);
			*pNewFlag = *pFlag;
			m_FlagList.AddTail(pNewFlag);
		}
	}

	return *this;
}

int CFlagList::GetNumFlags()
{
	return m_FlagList.GetCount();
}

CFlag* CFlagList::FindFlag(CString &Name, CFlag *pSkipFlag)
{
	CFlag *pFoundFlag;
	POSITION pos;
	int i;

	for (i=0; i<GetNumFlags(); i++)
	{
		pos = m_FlagList.FindIndex(i);
		if (pos)
		{
			pFoundFlag = m_FlagList.GetAt(pos);
			if ((pFoundFlag != pSkipFlag) && (pFoundFlag->GetName() == Name))
				return pFoundFlag;
		}
	}

	return NULL;
}

int CFlagList::GetFlagBit(CFlag *pFlag)
{
	POSITION pos;
	int i;

	for (i=0; i<GetNumFlags(); i++)
	{
		pos = m_FlagList.FindIndex(i);
		if (pos)
		{
			if (m_FlagList.GetAt(pos) == pFlag)
				return i;
		}
	}

	return -1;
}

int CFlagList::GetMask(int StartBit)
{
	int bits = GetTotalFlagBits();
	int mask;

	if (bits == 0)
		return 0;

	mask = 1;
	do
	{
		mask *= 2;
	}
	while (--bits);

	return (mask - 1) << StartBit;
}

void CFlagList::Serialize(CArchive &ar)
{
	int version;
	int i;
	int count;

	if (ar.IsStoring())
	{
		version = 3;
		ar << version;

		//---	store out the flag list
		count = GetNumFlags();
		ar << count;
		for (i=0; i<count; i++)
			GetFlag(i)->Serialize(ar);
	}
	else
	{
		CFlag Flag( this );

		ar >> version;

		//---	remove the default list
		RemoveAll();

		//---	load the flag list
		ar >> count;
		for (i=0; i<count; i++)
		{
			if( (version==1) && ( i==ANIMFLAG_NODISPLACEMENT ) )
				AddFlag( CString(DefaultFlags[ANIMFLAG_NODISPLACEMENT]) );

			if( (version==2) && ( i==ANIMFLAG_USEDESTSPEED ) )
				AddFlag( CString(DefaultFlags[ANIMFLAG_USEDESTSPEED]) );

			Flag.Serialize( ar );
			AddFlag( Flag.GetName() );
		}
	}
}

void CFlagList::ShiftFlags(CAnimTreeNode *pNode, int Operation, int Flag)
{
	CAnimTreeNode *pChild;

	if (!pNode)
		return;

	if (pNode->m_Type == ANIMTREENODE_TYPE_GROUP)
		((CGroupNode*) pNode)->ShiftFlags(Operation, Flag);
	else if (pNode->m_Type == ANIMTREENODE_TYPE_ANIM)
		((CAnimNode*) pNode)->ShiftFlags(Operation, Flag);

	if (!pNode->HasChildren())
		return;

	pChild = pNode->GetFirstChild();
	while (pChild)
	{
		ShiftFlags(pChild, Operation, Flag);
		pChild = pNode->GetNextChild();
	}
}

void CFlagList::ExportDefines(FILE *pFile, int Shift)
{
	CFlag *pFlag;
	int i;

	fprintf(pFile, "\n//---   Animation flag defines\n");
	fprintf(pFile, "#define\tANIMBF_FLAGSMASK\t\t0x%x\n", ((1 << GetTotalFlagBits()) - 1) << Shift);
	fprintf(pFile, "#define\tANIMBF_FLAGSSHIFT\t\t%d\n", Shift);

	fprintf(pFile, "\n//---   Animation flags\n");
	fprintf(pFile, "//      Remove any default defines which have been created\n");
	for (i=0; i<NUM_DEFAULT_FLAGS; i++)
	{
		pFlag = GetFlag(i);
		fprintf( pFile, "#ifdef\tANIMFLAG_%s\n", pFlag->GetName() );
		fprintf( pFile, "#undef\tANIMFLAG_%s\n", pFlag->GetName() );
		fprintf( pFile, "#endif\n" );
	}

	fprintf(pFile, "\n//---   Animation flag defines\n");
	for (i=0; i<this->GetNumFlags(); i++)
	{
		pFlag = GetFlag(i);
		fprintf(pFile, "#define\tANIMFLAG_%s\t\t0x%x\n", pFlag->GetName(), 1 << (pFlag->GetBit() + 0/*Shift*/));
	}
}

void CFlagList::ExportMacros(FILE *pFile, int Shift)
{
//	CFlag *pFlag;
//	int i;

	fprintf(pFile, "\n//---   Animation flag macros\n");
	fprintf(pFile, "#define\tANIMBF_GETFLAGS(v)\t(((v)&ANIMBF_FLAGMASK)) // >> ANIMBF_FLAGSHIFT)\n");
//	fprintf(pFile, "#define\tANIM_GETFLAGS(a)\tANIMBF_GETFLAGS(AnimTable[a].BitField)\n\n");
	fprintf( pFile, "#define\t_FLAGS_(v)\t((v) << ANIMBF_FLAGSSHIFT)\n" );
/*
	for (i=0; i<this->GetNumFlags(); i++)
	{
		pFlag = GetFlag(i);
		fprintf(pFile, "#define\tANIMBF_FLAGSET_%s(v)\t((v)&ANIMBF_FLAG_%s)\n", pFlag->GetName(), pFlag->GetName());
		fprintf(pFile, "#define\tANIM_FLAGSET_%s(a)\tANIMBF_FLAGSET_%s(AnimTable[a].BitField)\n", pFlag->GetName(), pFlag->GetName());
	}*/
}
