// Methods.cpp: implementation of the CMethods class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "Method.h"
#include "AnimTreeNode.h"
#include "AnimNode.h"
#include "GroupNode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

char* DefaultMethods[NUM_DEFAULT_METHODS] =
{
	"NORMAL",
	"NETDISPLACEMENT",
	"FINALMOVEDIR",
	"FINALFACEDIR",
	"FRAMEFACING"
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMethods::CMethods()
{
	int i;

	for(i=0; i<NUM_DEFAULT_METHODS; i++)
		AddMethod( CString( DefaultMethods[i] ) );
}

CMethods::~CMethods()
{
	MakeEmpty();
}

void CMethods::AddMethod(CString &Name, int Index)
{
	int i;

	//---	verify that a method of the same name doesn't already exist
	for(i=0; i<GetCount(); i++)
	{
		if (Name == GetMethod( i ))
			return;
	}

	if (Index >= 0)
	{
		//---	cannot change the order of the default methods
		if (Index < NUM_DEFAULT_METHODS)
			Index = NUM_DEFAULT_METHODS;

		POSITION pos = m_Methods.FindIndex(Index);
		if (pos)
		{
			m_Methods.InsertBefore(pos, Name);
			return;
		}
	}

	m_Methods.AddTail(Name);
}

int CMethods::GetCount()
{
	return m_Methods.GetCount();
}

int CMethods::GetTotalMethodBits()
{
	return CountNumBits(GetCount());
}

void CMethods::MakeEmpty()
{
	while (m_Methods.GetHeadPosition())
		m_Methods.RemoveHead();
}

CMethods& CMethods::operator =(CMethods &Other)
{
	CString Method;
	int i;

	MakeEmpty();
	for (i=0/*NUM_DEFAULT_METHODS*/; i<Other.GetCount(); i++)
		AddMethod(Other.GetMethod(i));

	return *this;
}

void CMethods::RemoveAll( void )
{
	MakeEmpty();
}

void CMethods::RemoveMethod(int Index)
{
	if (Index < NUM_DEFAULT_METHODS)
		return;

	POSITION Pos = m_Methods.FindIndex(Index);
	m_Methods.RemoveAt(Pos);
}

int CMethods::FindMethod(CString &Name)
{
	POSITION Pos;
	CString Method;
	int i;

	Pos = m_Methods.GetHeadPosition();
	i = 0;
	while (Pos)
	{
		Method = m_Methods.GetNext(Pos);
		if (Method == Name)
			return i;
		i++;
	}

	return -1;
}

CString& CMethods::GetMethod(int Index)
{
	POSITION Pos = m_Methods.FindIndex(Index);

	ASSERT(Pos);

	return m_Methods.GetAt(Pos);
}

int CMethods::CountNumBits(int Value)
{
	int s=Value;
	int n=0;

	if (s == 0)
		return 0;

	s--;
	do
	{
		s >>= 1;
		n++;
	}
	while (s > 0);

	return n;
}

int CMethods::GetMask(int StartBit)
{
	int bits = GetTotalMethodBits();
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

void CMethods::Serialize(CArchive &ar)
{
	int version;
	int i, count;
	CString Name;

	if (ar.IsStoring())
	{
		version = 2;
		ar << version;

		//---	write out each of the Methods
		count = GetCount();
		ar << count;
		for (i=0; i<count; i++)
		{
			ar.WriteString(GetMethod(i));
			ar.WriteString("\xd\xa");
		}
	}
	else
	{
		ar >> version;

		//---	remove the default list
		RemoveAll();

		//---	laod in each of the Methods
		ar >> count;
		for (i=0; i<count; i++)
		{
			ar.ReadString(Name);

			if( version==1 )
			{
				if( i!=5 )
					AddMethod( CString(DefaultMethods[i]) );
				continue;
			}

			AddMethod(Name);
		}
	}
}

void CMethods::ShiftMethods(CAnimTreeNode *pNode, int Operation, int Method)
{
	CAnimTreeNode *pChild;

	if (!pNode)
		return;

	if (pNode->m_Type == ANIMTREENODE_TYPE_GROUP)
		((CGroupNode*) pNode)->ShiftMethods(Operation, Method);
	else if (pNode->m_Type == ANIMTREENODE_TYPE_ANIM)
		((CAnimNode*) pNode)->ShiftMethods(Operation, Method);

	if (!pNode->HasChildren())
		return;

	pChild = pNode->GetFirstChild();
	while (pChild)
	{
		ShiftMethods(pChild, Operation, Method);
		pChild = pNode->GetNextChild();
	}
}

void CMethods::ExportDefines(FILE *pFile, int Shift)
{
	int i;

	fprintf(pFile, "\n//---   Animation Method defines\n");
	fprintf(pFile, "#define\tANIMBF_METHODMASK\t\t0x%x\n", ((1 << GetTotalMethodBits()) - 1) << Shift);
	fprintf(pFile, "#define\tANIMBF_METHODSHIFT\t\t%d\n", Shift);

	fprintf(pFile, "\n//---   Animation Methods\n");
	fprintf(pFile, "//      Remove any default defines which have been created\n");
	for (i=0; i<NUM_DEFAULT_METHODS; i++)
	{
		fprintf( pFile, "#ifdef\tANIMMETHOD_%s\n", GetMethod(i) );
		fprintf( pFile, "#undef\tANIMMETHOD_%s\n", GetMethod(i) );
		fprintf( pFile, "#endif\n" );
	}

	fprintf(pFile, "\n//---   Animation Method defines\n");
	for (i=0; i<this->GetCount(); i++)
		fprintf(pFile, "#define\tANIMMETHOD_%s\t\t%d\n", GetMethod(i), i );
}

void CMethods::ExportMacros(FILE *pFile, int Shift)
{
	fprintf(pFile, "\n//---   Animation Method macros\n");
	fprintf(pFile, "#define\tANIMBF_GETMETHOD(v)\t\t(((v) & ANIMBF_METHODMASK) >> ANIMBF_METHODSHIFT)\n");
	fprintf(pFile, "#define\tANIM_GETMETHOD(a)\t\tANIMBF_GETMETHOD(AnimTable[a].BitField)\n");
	fprintf(pFile, "#define\t_METHOD_(v)\t((v) << ANIMBF_METHODSHIFT)\n" );
}
