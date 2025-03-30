// AnimTrans.cpp: implementation of the CAnimTrans class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTrans.h"
#include "AnimTransEditDlg.h"
#include "AnimTblEdDefs.h"

extern int CurrentExportType;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAnimTrans::CAnimTrans(class CAnimTblEdDoc *pDoc, class CAnimTreeNode *pParent):CAnimTreeNode( pDoc, pParent )
{
	CAnimTreeNode::m_Type = ANIMTREENODE_TYPE_TRANS;

	m_nBlendFrames = 0;
	m_Disable = FALSE;
}

CAnimTrans::~CAnimTrans()
{

}

void CAnimTrans::SetFromAnim(const char *lpszAnim)
{
	m_FromAnim.Format( "%s", lpszAnim );
}

void CAnimTrans::SetToAnim(const char *lpszAnim)
{
	m_ToAnim.Format( "%s", lpszAnim );
}

void CAnimTrans::SetPlayAnim(const char *lpszAnim)
{
	m_PlayAnim.Format( "%s", lpszAnim );
}

void CAnimTrans::GetNLDescription(CString &Description)
{
	this->GetNLDescription( Description, this->GetFromAnim(), this->GetToAnim(), this->GetPlayAnim() );
}

void CAnimTrans::GetNLDescription(CString &Description, const char *lpszFromAnim, const char *lpszToAnim, const char *lpszPlayAnim)
{
	CString BuildString;

	if (strcmp( lpszToAnim, "<this>" ) == 0)
	{
		//---	Prerequisite
		if (strcmp( lpszFromAnim, "<any>" ) == 0)
		{
			BuildString.Format( "Blend from any animation to this animation." );
		}
		else
		{
			if (strlen( lpszFromAnim ) == 0)
				BuildString.Format( "Must be in _____ to play this animation." );
			else
				BuildString.Format( "Must be in %s to play this animation.", lpszFromAnim );
		}
	}
	else
	{
		//---	Transition
		if ((strcmp( lpszPlayAnim, "<any>" ) == 0) && (strlen( lpszToAnim ) != 0))
		{
			if (strcmp( lpszToAnim, "<any>" ) == 0)
				BuildString.Format( "Blend to any animation from this animation." );
			else
				BuildString.Format( "Blend to %s animation from this animation.", lpszToAnim );
		}
		else
		{
			if (strlen( lpszToAnim ) == 0)
			{
				if (strlen( lpszPlayAnim ) == 0)
					BuildString.Format( "When going to _____, first play _____." );
				else
					BuildString.Format( "When going to _____, first play %s.", lpszPlayAnim );
			}
			else
			{
				if (strlen( lpszPlayAnim ) == 0)
					BuildString.Format( "When going to %s, first play _____.", lpszToAnim );
				else
					BuildString.Format( "When going to %s, first play %s.", lpszToAnim, lpszPlayAnim );
			}
		}
	}

	//---	If this transition is disabled then mark it at the beginning.
	if (m_Disable)
	{
		Description.Format( "<<<DISABLED>>> %s", BuildString );
		BuildString.Format( "%s", Description );
	}

	//---	Tack the blendingonto the end.
	if (this->GetBlendFrames() == 1)
		Description.Format( "%s  Blend 1 frame.", BuildString );
	else if (this->GetBlendFrames())
		Description.Format( "%s  Blend %d frames.", BuildString, this->GetBlendFrames() );
	else
		Description.Format( "%s", BuildString );
}

bool CAnimTrans::IsPrerequisite()
{
	if (m_ToAnim == "<this>")
		return TRUE;

	return FALSE;
}

bool CAnimTrans::IsTransition()
{
	return !this->IsPrerequisite();
}


void CAnimTrans::Serialize(CArchive &ar)
{
	short int version;

	if (ar.IsStoring())
	{
		//---	Usual version stuff.
		version = 2;
		ar.Write( &version, 2 );

		//---	Add new save items here  ****************

		//---	Disable.
		ar << m_Disable;

		//---	AnimTrans specific stuff.
		ar.WriteString( m_FromAnim );
		ar.WriteString( "\xd\xa" );

		ar.WriteString( m_ToAnim );
		ar.WriteString( "\xd\xa" );

		ar.WriteString( m_PlayAnim );
		ar.WriteString( "\xd\xa" );

		ar << m_nBlendFrames;
	}
	else
	{
		ar.Read( &version, 2 );

		switch (version)
		{
		case 2:
			ar >> m_Disable;
		case 1:
			ar.ReadString( m_FromAnim );

			ar.ReadString( m_ToAnim );

			ar.ReadString( m_PlayAnim );

			ar >> m_nBlendFrames;
			break;
		}
	}
}

void CAnimTrans::Export(FILE *pFile, const char *lpszAnimName)
{
	CString Anim1;
	CString	Anim2;
	CString Type;
	CString	IDName1;
	CString	IDName2;
	CString DefStr;

	if (this->GetTag() == "<def> ")
		DefStr = "- Inherited ";

	if (this->IsPrerequisite())
	{
		Anim1.Format( "%s", lpszAnimName );

		Anim2.Format( "%s", this->GetFromAnim() );

		Type.Format( "// Prerequisite %s", DefStr );
	}
	else
	{
		Anim1.Format( "%s", this->GetToAnim() );

		Anim2.Format( "%s", this->GetPlayAnim() );

		//---	When we are just adding blending <any> in the "Play" anim should be set to the "To" anim.
		if (Anim2.Compare( "<any>" ) == 0)
			Anim2.Format( "%s", Anim1 );

		Type.Format( "// Transition %s", DefStr );
	}

	if (Anim1.Compare( "<any>" ) == 0)
		Anim1.Format( "NONE" );

	if (Anim2.Compare( "<any>" ) == 0)
		Anim2.Format( "NONE" );

	//---	Create the identifire
	if (Anim1.GetAt( 0 ) == '[')
	{
		Anim1.Remove( '[' );
		Anim1.Remove( ']' );
		IDName1.Format( "ANIMGROUP_%s", Anim1 );
	}
	else
		IDName1.Format( "ANIM_%s", Anim1 );

	if (Anim2.GetAt( 0 ) == '[')
	{
		Anim2.Remove( '[' );
		Anim2.Remove( ']' );
		IDName2.Format( "ANIMGROUP_%s", Anim2 );
	}
	else
		IDName2.Format( "ANIM_%s", Anim2 );


	//---	If we are disabled then we will export commented structures.
	if (m_Disable)
		fprintf( pFile, "//" );

	//---	Export the formated identifiers.
	if (this->GetBlendFrames())
		fprintf( pFile, "\t%s, SET_BLENDING(%d)|%s, %s", IDName1, this->GetBlendFrames(), IDName2, Type );
	else
		fprintf( pFile, "\t%s, %s, %s", IDName1, IDName2, Type );

	//---	If we are disabled then we will export commented structures.
	if (m_Disable)
		fprintf( pFile, "   <<<DISABLED>>>" );

	fprintf( pFile, "\n" );
}

void CAnimTrans::Export(FILE *pFile)
{
// These items do not export themselves anymore.  They are exported at the animation level only (so that the inherited dependancies can be added)
/*
	if (CurrentExportType == EXPORTTYPE_DEPENDANCIES)
	{
		this->Export( pFile, this->GetParent()->GetName() );	
	}
*/
}

bool CAnimTrans::Edit(const char *lpszAnimName)
{
	CAnimTransEditDlg	AnimTransEditDlg;

	AnimTransEditDlg.m_pAnimTrans = this;
	AnimTransEditDlg.m_pDoc = CAnimTreeNode::m_pDoc;
	AnimTransEditDlg.m_AnimName.Format( "%s", lpszAnimName );

	if (AnimTransEditDlg.DoModal() == IDOK)
		return TRUE;
	
	return FALSE;
}

bool CAnimTrans::Edit()
{
	CString ParentName;

	ParentName.Format( "%s", this->GetParent()->GetName() );

	return this->Edit( ParentName );
}

int CAnimTrans::GetDisabled()
{
	return m_Disable;
}

int CAnimTrans::SetDisabled(int Set)
{
	int	cur;

	if( Set == m_Disable )
		return m_Disable;

	cur = m_Disable;

	m_Disable = Set;

	SetIgnore( m_Disable ? TRUE : FALSE );
//	ModifyViews();

	return cur;
}

void CAnimTrans::Copy(CAnimTrans &AnimTrans)
{
	m_FromAnim = AnimTrans.m_FromAnim;
	m_PlayAnim = AnimTrans.m_PlayAnim;
	m_ToAnim = AnimTrans.m_ToAnim;

	SetTag( AnimTrans.GetTag() );

	m_Disable = AnimTrans.m_Disable;
	m_nBlendFrames = AnimTrans.m_nBlendFrames;
}

bool CAnimTrans::HasChildren()
{
	return FALSE;
}

const CString& CAnimTrans::GetName()
{
	this->GetNLDescription( m_Name );

	return m_Name;
}

int CAnimTrans::GetEditOptionEnabled()
{
	return MF_ENABLED;
}

int CAnimTrans::GetDeleteOptionEnabled()
{
	return MF_ENABLED;
}

int CAnimTrans::GetMouseMenuIndex()
{
	return 2;
}


CAnimTrans& CAnimTrans::operator =(CAnimTrans &Other)
{
	Copy( Other );

	return *this;
}

int CAnimTrans::GetImage(void)
{
	if (IsTransition())
		return 16;
	else
		return 18;
}

int CAnimTrans::GetSelectedImage(void)
{
	if (IsTransition())
		return 17;
	else
		return 19;
}
