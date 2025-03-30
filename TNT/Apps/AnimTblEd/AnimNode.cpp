// AnimNode.cpp: implementation of the CAnimNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"
#include "AnimTblEdView.h"
#include "AnimNode.h"
#include "GroupNode.h"
#include "AnimID1.h"
#include "EquivNode.h"
#include "IDList.h"
#include "AnimTransList.h"

#include "AnimTblEdGlobals.h"
#include "AnimTblEdDefs.h"
#include "AnimationEditDlg.h"

extern int CurrentExportCount;
extern int CurrentExportType;
extern int LastTokenType;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAnimNode::CAnimNode(class CAnimTblEdDoc *pDoc, class CAnimTreeNode *pParent, const char *lpszAnimID, const char *lpszAnimName):CAnimTreeNode( pDoc, pParent )
{
	CString	Name;

	//---	Set the output enum identifier.
	if (lpszAnimName)
		SetName( lpszAnimName );
	else
		SetName( "ANIMATION" );

	//---	Set the type into the base structure.
	CAnimTreeNode::m_Type = ANIMTREENODE_TYPE_ANIM;

	m_nChildren			= 0;
	m_Method			= 0;
	m_Flags				= 0;
	m_States			= 0;
	m_EndStates			= 0;
	m_Priority			= 0;
	m_FrameRate			= "1.0";
	m_pCutTable			= NULL;

	m_LinkAnim			= "";
	m_bLinkMethod		= FALSE;
	m_bLinkPriority		= FALSE;
	m_bLinkFrameRate	= FALSE;
	m_bLinkFlags		= FALSE;
	m_bLinkStates		= FALSE;
	m_bLinkTransitions	= FALSE;
	m_LinkEnterFrame	= 0;

	if ((pParent != NULL) && (pParent->m_Type == ANIMTREENODE_TYPE_GROUP))
	{
		m_States = ((CGroupNode*) pParent)->BS_GetStates();
		m_Priority = ((CGroupNode*) pParent)->BS_GetPriority();
	}
}

CAnimNode::~CAnimNode()
{
}

int CAnimNode::GetMouseMenuIndex()
{
	return 1;
}

int CAnimNode::GetImage()
{
	char ch = m_FrameRate[0];
	if( m_FrameRate.SpanIncluding( "0123456789.-" ) == m_FrameRate )
	{
		if (GetNumDependancies( TRUE, TRUE ))
			return TREEIMAGE_ANIM_WITHDEP;
		else
			return TREEIMAGE_ANIM;
	}
	else
	{
		if (GetNumDependancies( TRUE, TRUE ))
			return TREEIMAGE_ANIM_LINKED_WITHDEP;
		else
			return TREEIMAGE_ANIM_LINKED;
	}
}

int CAnimNode::GetSelectedImage()
{
	if (GetNumDependancies( TRUE, TRUE ))
		return TREEIMAGE_ANIM_SELECT_WITHDEP;
	else
		return TREEIMAGE_ANIM_SELECT;
}

int CAnimNode::GetStateImage()
{
	int Image = 0;

	if ((m_RLIDs.GetLeftID() == "") || (m_RLIDs.GetRightID() == ""))
		Image |= 1;

	if (GetIgnore())
		Image |= 2;

	if (GetSearchMatched())
		Image |= 4;

	return Image;
}

//DEL bool CAnimNode::AddID(const char *lpszAnimID)
//DEL {
//DEL 	//---	If the id is already there then don't add it again.
//DEL 	if (m_IDList.FindID( lpszAnimID ))
//DEL 		return FALSE;
//DEL 		
//DEL 	m_IDList.Add( lpszAnimID );
//DEL 	return TRUE;
//DEL };


bool CAnimNode::Edit(void)
{
	CAnimationEditDlg	AnimEditDlg( this );

	if (m_Type == ANIMTREENODE_TYPE_ANIM)
	{
		if (AnimEditDlg.DoModal() == IDOK)
		{
			//---	force a recount of the dependancies
			GetNumDependancies( TRUE );
			return TRUE;	
		}
	}

	return FALSE;
}

bool CAnimNode::HasChildren()
{
	return (bool)!m_ChildList.IsEmpty();
}

//DEL void CAnimNode::SetDefaultName()
//DEL {
//DEL 	CString	DefaultName;
//DEL 
//DEL 	DefaultName.Format( "%s", GetFirstAnim() );
//DEL 	DefaultName.Replace( "ID_", "" );
//DEL 
//DEL 	SetName( DefaultName );
//DEL }

/*****************************************************************************************\
*
*	Up through version 6 the m_RLIDs "CEquivNode" structure was the child nodes.
*	With version 7 the transitions were changed to be the children and the m_AnimTransList field
*	was removed.
*
\*****************************************************************************************/
void CAnimNode::Serialize(CArchive &ar)
{
	short int		version;
	int				nChildren;
	int				nTransitions;
	CAnimTreeNode	*pAnimTreeNode;
	CAnimTrans		*pAnimTrans;
	u32				Flags;
	CString			CutTable;
	u32				Unused;
	s32				i;

	if (ar.IsStoring())
	{
		//---	Write Version
		version = 15;
		ar.Write( &version, 2 );

		//---********  ADD NEW SERIALIZATIONS HERE ***********

		//---	link anim data
		ar.WriteString( m_LinkAnim );
		ar.WriteString( "\xd\xa" );
		ar << m_bLinkMethod;
		ar << m_bLinkPriority;
		ar << m_bLinkFrameRate;
		ar << m_bLinkFlags;
		ar << m_bLinkStates;
		ar << m_bLinkTransitions;
		ar << m_LinkEnterFrame;

		//---	save out the callback
		ar.WriteString( m_Callback );
		ar.WriteString( "\xd\xa" );

		//---	save the cut table name so that it can be re-linked at load
		if (m_pCutTable != NULL)
			ar.WriteString( m_pCutTable->m_Name );
		ar.WriteString( "\xd\xa" );

		//---	serialize the base class
		CAnimTreeNode::Serialize(ar);

		ar << m_FrameRate;

		//---	Write the comment.
		ar.WriteString( m_Comment );
		ar.WriteString( "\xd\xa" );

		//---	Write Name
		ar.WriteString( m_AnimName );
		ar.WriteString( "\xd\xa" );

		ar << m_Method;
		ar << m_Flags;
		ar << m_States;
		ar << m_EndStates;
		ar << m_Priority;

		//---	Right/Left Source ID.
		m_RLIDs.Serialize( ar );

		//---	Write Children
		ar << m_nChildren;
		pAnimTreeNode = GetFirstChild();
		while (pAnimTreeNode)
		{
			pAnimTreeNode->WriteTreeItem( ar );
			pAnimTreeNode = GetNextChild();
		}
	}
	else
	{
		//---	get the document
		m_pDoc = (CAnimTblEdDoc*)ar.m_pDocument;

		//---	Read Version
		ar.Read( &version, 2 );

		switch (version)
		{
		case 1:
			//---	Read Name
			ar.ReadString( m_AnimName );

			//---	nTransition
			ar >> nTransitions;
			for (; nTransitions > 0; nTransitions--)
			{
				pAnimTrans = new CAnimTrans( this->m_pDoc, this );
				pAnimTrans->Serialize( ar );
				this->AddChild( pAnimTrans );
//				m_AnimTransList.Add( pAnimTrans );
			}
			break;

		case 2:
			//---	Read Name
			ar.ReadString( m_AnimName );

			//---	nTransition
			ar >> nTransitions;
			for (; nTransitions > 0; nTransitions--)
			{
				pAnimTrans = new CAnimTrans( this->m_pDoc, this );
				pAnimTrans->Serialize( ar );
				this->AddChild( pAnimTrans );
//				m_AnimTransList.Add( pAnimTrans );
			}

			ar >> m_Flags;

			break;

		case 3:
			//---	Read Name
			ar.ReadString( m_AnimName );

			//---	nTransition
			ar >> nTransitions;
			for (; nTransitions > 0; nTransitions--)
			{
				pAnimTrans = new CAnimTrans( this->m_pDoc, this );
				pAnimTrans->Serialize( ar );
				this->AddChild( pAnimTrans );
//				m_AnimTransList.Add( pAnimTrans );
			}

			ar >> m_Flags;
			ar >> m_States;
			ar >> m_Priority;

			break;

		case 6:
			ar >> m_FrameRate;
		case 5:
			ar.ReadString( m_Comment );
		case 4:
			//---	Read Name
			ar.ReadString( m_AnimName );

			//---	nTransition
			ar >> nTransitions;
			for (; nTransitions > 0; nTransitions--)
			{
				pAnimTrans = new CAnimTrans( this->m_pDoc, this );
				pAnimTrans->Serialize( ar );
				this->AddChild( pAnimTrans );
//				m_AnimTransList.Add( pAnimTrans );
			}

			ar >> m_Flags;
			ar >> m_States;
			ar >> m_EndStates;
			ar >> m_Priority;

			Flags = m_Flags | BS_GetGroupFlags();
			if		(Flags & 0x1)	m_Method = 1; // ANIMFLAG_NETDISPLACEMENT
			else if (Flags & 0x10)	m_Method = 2; // ANIMFLAG_NODISPLACEMENT
			else if (Flags & 0x2)	m_Method = 3; // ANIMFLAG_FINALMOVEDIR
			else if (Flags & 0x4)	m_Method = 4; // ANIMFLAG_FINALFACEDIR
			else if (Flags & 0x8)	m_Method = 5; // ANIMFLAG_FRAMEFACING

			break;

		case 7:
			ar >> m_FrameRate;
			ar.ReadString( m_Comment );
			//---	Read Name
			ar.ReadString( m_AnimName );

			ar >> m_Flags;
			ar >> m_States;
			ar >> m_EndStates;
			ar >> m_Priority;
			
			Flags = m_Flags | BS_GetGroupFlags();
			if		(Flags & 0x1)	m_Method = 1; // ANIMFLAG_NETDISPLACEMENT
			else if (Flags & 0x10)	m_Method = 2; // ANIMFLAG_NODISPLACEMENT
			else if (Flags & 0x2)	m_Method = 3; // ANIMFLAG_FINALMOVEDIR
			else if (Flags & 0x4)	m_Method = 4; // ANIMFLAG_FINALFACEDIR
			else if (Flags & 0x8)	m_Method = 5; // ANIMFLAG_FRAMEFACING

			//---	Right/Left Source ID.
			m_RLIDs.Serialize( ar );
			break;

		case 9:
			CAnimTreeNode::Serialize(ar);
		case 8:
			ar >> m_FrameRate;
			ar.ReadString( m_Comment );
			//---	Read Name
			ar.ReadString( m_AnimName );

			ar >> m_Method;
			ar >> m_Flags;
			ar >> m_States;
			ar >> m_EndStates;
			ar >> m_Priority;

			//---	Right/Left Source ID.
			m_RLIDs.Serialize( ar );
			break;

		////////////////////////////////////////////////////////////////////
		//	ADD NEW VERSION CODE HERE
		////////////////////////////////////////////////////////////////////


		case 15:
		case 14:
			//---	link anim data
			ar.ReadString( m_LinkAnim );
			ar >> m_bLinkMethod;
			ar >> m_bLinkPriority;
			ar >> m_bLinkFrameRate;
			ar >> m_bLinkFlags;
			ar >> m_bLinkStates;
			ar >> m_bLinkTransitions;
			ar >> m_LinkEnterFrame;

		case 13:
		case 12:
			//---	save out the callback
			ar.ReadString( m_Callback );

		case 11:
			//---	load the cut talbe's name and then search for it in the list
			//		NOTE: LocomotionTables must have been loaded before this step
			ar.ReadString( CutTable );
			m_pCutTable = NULL;
			if (CutTable != "")
			{
				for( i=0; i<m_pDoc->m_LocomotionTables.GetCount(); i++ )
				{
					if (m_pDoc->m_LocomotionTables.GetTable( i )->m_Name == CutTable)
					{
						m_pCutTable = m_pDoc->m_LocomotionTables.GetTable( i );
						break;
					}
				}
			}

		case 10:
			CAnimTreeNode::Serialize(ar);
			ar >> m_FrameRate;
			ar.ReadString( m_Comment );
			//---	Read Name
			ar.ReadString( m_AnimName );

			ar >> m_Method;
			ar >> m_Flags;
			ar >> m_States;
			ar >> m_EndStates;
			ar >> m_Priority;

			if (version == 10)
			{
				ar >> Unused;
				ar >> Unused;
			}

			//---	special case code to remove the ANIMMETHOD_NODISPLACEMENT
			//		and add the ANIMFLAG_NODISPLACEMENT
			if( version <= 12 )
			{
				#define	NUMDEFAULTFLAGSATTHETIME	6
				#define	DEFAULTFLAGSMASKATTHETIME	((1<<NUMDEFAULTFLAGSATTHETIME)-1)
				m_Flags = (m_Flags & DEFAULTFLAGSMASKATTHETIME) | ((m_Flags & ~DEFAULTFLAGSMASKATTHETIME) << 1);

				if( m_Method == 2 )
				{
					m_Method = ANIMMETHOD_FRAMEFACING;
					m_Flags |= 1<<ANIMFLAG_NODISPLACEMENT;
				}
				else if( m_Method > 2 )
				{
					m_Method -= 1;
					m_Flags &= ~(1<<ANIMFLAG_NODISPLACEMENT);
				}
			}

			//---	special case code add the ANIMFLAG_USEDESTSPEED flag.
			if( version <= 14 )
			{
				#define	NUMDEFAULTFLAGSBEFOREUSEDESTSPEED		7
				#define	DEFAULTFLAGSMASKBEFOREUSEDESTSPEED	((1<<NUMDEFAULTFLAGSBEFOREUSEDESTSPEED)-1)
				m_Flags = (m_Flags & DEFAULTFLAGSMASKBEFOREUSEDESTSPEED) | ((m_Flags & ~DEFAULTFLAGSMASKBEFOREUSEDESTSPEED) << 1);
				m_Flags &= ~(1<<ANIMFLAG_USEDESTSPEED);
			}

			//---	Right/Left Source ID.
			m_RLIDs.Serialize( ar );
			break;
		}

		//---	Read Children -	As we add the children to the animation m_nChildren will be incremented to the correct value.
		ar >> nChildren;
		for (; nChildren > 0; nChildren--)
		{
			pAnimTreeNode = CAnimTreeNode::ReadTreeItem( ar, this );
			if (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_EQUIV)
			{
				//---	We have gotten rid of equivalencies in the tree so instead write this data into the animation.
				m_RLIDs.Copy( *pAnimTreeNode );
			}
			else
				AddChild( pAnimTreeNode );
		}

		GetNumDependancies( TRUE, TRUE );

		if( version <= 13 )
		{
			if ( m_FrameRate.IsEmpty() || (m_FrameRate.FindOneOf(".") != -1))
			{
				m_LinkAnim = "";
			}
			else
			{
				m_bLinkFrameRate = TRUE;
				m_LinkAnim = m_FrameRate;
				m_FrameRate = "1.0f";
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::Remove(CIDList &IDList)
{
	CAnimTrans	*pAnimTrans;

	while (!m_ChildList.IsEmpty())
	{
		pAnimTrans = (CAnimTrans*)m_ChildList.RemoveTail(); // These should all be of type CAnimTrans

		if (pAnimTrans->m_Type == ANIMTREENODE_TYPE_TRANS)
			pAnimTrans->Remove( IDList );

		delete pAnimTrans;
	}

//	m_AnimTransList.Delete();
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetAddGroupOptionEnabled()
{
	return MF_GRAYED;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetAddAnimOptionEnabled()
{
	return MF_GRAYED;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetAddTransOptionEnabled()
{
	return MF_ENABLED;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetEditOptionEnabled()
{
	return MF_ENABLED;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetDeleteOptionEnabled()
{
	return MF_ENABLED;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetScanSourceCodeOptionEnabled()
{
	return MF_ENABLED;
}
int glbvar;
////////////////////////////////////////////////////////////////////////////
void CAnimNode::Export(FILE *pFile)
{
	bool Ignore;
	bool IgnoreOnly;

	static s32 DepCount=9999999;
	static s32 AnimCount=9999999;

	ASSERT(m_RLIDs.GetLeftID() != "");
	ASSERT(m_RLIDs.GetRightID() != "");

	//---	Export .H file ANIM_ list.
	if (CurrentExportType == EXPORTTYPE_ANIMLIST)
	{
		if ((CAnimTreeNode::GetLastTokenType() != TOKEN_GROUPSTART) && (CAnimTreeNode::GetLastTokenType() != TOKEN_GROUPEND))
			CAnimTreeNode::IncCount();

		//---	Export the define.
		Ignore = GetIgnore();
		if( Ignore ) fprintf( pFile, "//" );
		fprintf( pFile, "#define\tANIM_%s\t%d\n", GetName(), CAnimTreeNode::GetCount() );

		if( !Ignore ) CAnimTreeNode::SetLastTokenType( TOKEN_ANIM );

		DepCount = 1;
		AnimCount = 1;
	}

	//---	Export .C file AnimTrans_ dependancy structures.
	else if (CurrentExportType == EXPORTTYPE_DEPENDANCIES)
	{
		if (!GetNumDependanciesWithLink())
			return;

		Ignore = GetIgnore();
		IgnoreOnly = ( Ignore && (!m_pParent || !m_pParent->GetIgnore())  );
		if( IgnoreOnly )	fprintf( pFile, "/* " );
		fprintf( pFile, "\t// ANIM_%s Dependancies  (Index: %d)\n", this->GetName(), DepCount );
		
		CAnimTransList Dependancies;
		GetDependanciesWithLink( Dependancies );
		CAnimTrans* pDep = Dependancies.GetFirst();
		while (pDep)
		{
			pDep->Export( pFile, this->GetName() );
			if( !pDep->GetDisabled() && !Ignore )
				DepCount+=2;

			pDep = Dependancies.GetNext();
		}

		fprintf( pFile, "\t-1, // End ANIM_%s Dependancies\n", this->GetName() );
		if( IgnoreOnly )	fprintf( pFile, "*/" );
		fprintf( pFile, "\n" );
		if( Ignore )	DepCount+=1;
	}
	
	//---	Export .C file animation table.
	else if (CurrentExportType == EXPORTTYPE_OBJECTLIST)
	{
		CString str;
		CString	str2;
		bool Ignore = GetIgnore();
		CString Index;

		if( Ignore )
		{
			fprintf( pFile, "//" );
			Index = "*****";
		}
		else
			Index.Format( "%4d", AnimCount++ );

		fprintf( pFile, "\t/* (%5s) ANIM_%s */{", Index, GetName() );
		m_RLIDs.Export( pFile );

		CString LinkAnim = "ANIM_NONE";
		int LinkData = 0;
		if( m_LinkAnim.GetLength() )
		{
			//---	if the linked animation the linked animation is a special run-time animation, do not export any linked animation
			if( m_LinkAnim[0] != '<' )
			{
				CAnimNode* pLink = m_pDoc->FindAnimByName( m_LinkAnim );
				if( pLink )
				{
					LinkAnim = "ANIM_" + pLink->GetName();
					LinkData = m_LinkEnterFrame;
				}
			}
		}

		s32 NumDeps = GetNumDependanciesWithLink( FALSE );
		if (NumDeps)
		{
			fprintf( pFile, "%s/*BF*/, %s/*FRate*/, %d/*Trans Table*/, %d/*CB*/, %s/*CutTable*/, %s/*Link Anim*/, %d/*Link Data*/},\n",
				this->BS_GetExportValue( str ),
				CAnimNode::GetFrameRateExportString( str2 ),
				CAnimTreeNode::GetCurrentExportTransIndex(),
				GetCallbackExport(),
				GetCutTableExport(),
				LinkAnim, LinkData);

			//---	Icrement the dependancy counter so the next anim to export will have accurate values.
			if( !Ignore ) CAnimTreeNode::IncExportTransIndex( NumDeps*2 + 1 );
		}
		else
		{
			fprintf( pFile, "%s/*BF*/, %s/*FRate*/, 0/*No Trans*/, %d/*CB*/, %s/*CutTable*/, %s/*Link Anim*/, %d/*Link Data*/},\n",
				this->BS_GetExportValue( str ),
				CAnimNode::GetFrameRateExportString( str2 ),
				/* no dependancies */
				GetCallbackExport(),
				GetCutTableExport(),
				LinkAnim, LinkData);
		}
	}

	//---	Export the Locomotion animation rate array which will get initialized at run-time
	else if (CurrentExportType == EXPORTTYPE_LOCOMOTIONANIMARRAY)
	{
		if (RateAnalogControlled())
		{
			if( GetIgnore() )	fprintf( pFile, "//" );
			fprintf( pFile, "\tANALOG_ANIM_RATE,\n" );
		}
	}
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::BS_GetMethod()
{
	return m_Method;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::BS_GetFlags()
{
	return m_Flags;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::BS_GetStates()
{
	return m_States;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::BS_GetEndStates()
{
	return m_EndStates;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::BS_GetPriority()
{
	return m_Priority;
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::BS_SetMethod(int Value)
{
	m_Method = Value;
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::BS_SetFlags(int Value)
{
	m_Flags = Value;
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::BS_SetStates(int Value)
{
	int i;
	CStateClass *pClass;

	for (i=0; i<GetDocument()->m_StateClassList.GetCount(); i++)
	{
		pClass = GetDocument()->m_StateClassList.GetStateClass(i);

		if (pClass->GetSetting(m_States) == pClass->GetSetting(m_EndStates))
			pClass->SetSetting(m_EndStates, pClass->GetSetting(Value));
	}

	m_States = Value;
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::BS_SetEndStates(int Value)
{
	m_EndStates = Value;
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::BS_SetPriority(int Value)
{
	m_Priority = Value;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::BS_GetGroupMethod()
{
	if (GetParent() == NULL)
		return 0;

	return GetParent()->BS_GetMethod();
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::BS_GetGroupFlags()
{
	if (GetParent() == NULL)
		return 0;

	return GetParent()->BS_GetGroupFlags() | GetParent()->BS_GetFlags();
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::BS_GetGroupStates()
{
	if (GetParent() == NULL)
		return 0;

	return GetParent()->BS_GetStates();
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::BS_GetGroupPriority()
{
	if (GetParent() == NULL)
		return 0;

	return GetParent()->BS_GetPriority();
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::BS_GetState(int StateClass)
{
	CStateClass *pClass = GetDocument()->m_StateClassList.GetStateClass(StateClass);

	if (!pClass)
		return -1;

	return pClass->GetSetting(m_States);
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::BS_SetState(int StateClass, int Value)
{
	CStateClass *pClass = GetDocument()->m_StateClassList.GetStateClass(StateClass);

	if (!pClass)
		return;

	if (pClass->GetSetting(m_States) == pClass->GetSetting(m_EndStates))
		pClass->SetSetting(m_EndStates, Value);

	pClass->SetSetting(m_States, Value);
}

////////////////////////////////////////////////////////////////////////////
CString CAnimNode::GetParentCallback()
{
	if (GetParent() == NULL)
		return CString("");

	return GetParent()->GetCallback();
}

////////////////////////////////////////////////////////////////////////////
class CLocoAnimTable* CAnimNode::GetParentCutTable()
{
	if (!m_pParent)
		return NULL;

	return ((CGroupNode*) m_pParent)->GetCutTable();
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::ShiftStateClasses(int Operation, int StateClass, int State, int Bit, int Num)
{
	int LowMask = (1 << Bit) - 1;
	int HighMask = (1 << (Bit + Num)) - 1;
	int CurState;

	switch (Operation)
	{
	case -2://---	REMOVE SOME BITS
			m_States = (m_States & LowMask) | ((m_States & ~HighMask) >> Num);
			m_EndStates = (m_EndStates & LowMask) | ((m_EndStates & ~HighMask) >> Num);
		break;

	case -1://---	REMOVE A STATE
			CurState = (m_States & (HighMask & ~LowMask)) >> Bit;
			if ((CurState >= State) && (CurState > 0))
			{
				CurState--;
				m_States = (m_States & ~(HighMask & ~LowMask)) | (CurState << Bit);
			}

			CurState = (m_EndStates & (HighMask & ~LowMask)) >> Bit;
			if ((CurState >= State) && (CurState > 0))
			{
				CurState--;
				m_EndStates = (m_EndStates & ~(HighMask & ~LowMask)) | (CurState << Bit);
			}
		break;

	case 1://---	ADD A STATE
			CurState = (m_States & (HighMask & ~LowMask)) >> Bit;
			if (CurState >= State)
			{
				CurState++;
				m_States = (m_States & ~(HighMask & ~LowMask)) | ((CurState << Bit) & (HighMask & ~LowMask));
			}

			CurState = (m_EndStates & (HighMask & ~LowMask)) >> Bit;
			if (CurState >= State)
			{
				CurState++;
				m_EndStates = (m_EndStates & ~(HighMask & ~LowMask)) | ((CurState << Bit) & (HighMask & ~LowMask));
			}
		break;

	case 2://---	ADD SOME BITS
			m_States = (m_States & LowMask) | ((m_States & ~LowMask) << Num);
			m_EndStates = (m_EndStates & LowMask) | ((m_EndStates & ~LowMask) << Num);
		break;

	default:ASSERT(0);
	}
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::ShiftMethods(int Operation, int Method)
{
	if (m_Method >= Method)
	{
		if (Operation > 0)
			m_Method++;
		else if (m_Method > 0)
			m_Method--;
	}
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::ShiftFlags(int Operation, int Flag)
{
	int Mask = ((1<<Flag) - 1);

	if (Operation > 0)
		m_Flags = (m_Flags & Mask) | (((m_Flags & ~Mask) << 1) & ~Mask);
	else
		m_Flags = (m_Flags & Mask) | (((m_Flags & ~Mask) >> 1) & ~Mask);
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::ShiftPriorities(int Operation, int Priority)
{
	if (m_Priority >= Priority)
	{
		if (Operation > 0)
			m_Priority++;
		else if (m_Priority > 0)
			m_Priority--;
	}
}

////////////////////////////////////////////////////////////////////////////
CString& CAnimNode::BS_GetExportValue(CString &String)
{
	CAnimNode* pLink = NULL;
	int Priority;
	int Flags;
	int States;
	int Method;

	if( m_LinkAnim.GetLength() )
		pLink = m_pDoc->FindAnimByName( m_LinkAnim );

	if( pLink )
	{
		Priority	= m_bLinkPriority	? pLink->BS_GetPriority()							: m_Priority;
		Flags		= m_bLinkFlags		? pLink->BS_GetFlags()|pLink->BS_GetGroupFlags()	: m_Flags|BS_GetGroupFlags();
		States		= m_bLinkStates		? pLink->BS_GetStates()								: m_States;
		Method		= m_bLinkMethod		? pLink->BS_GetMethod()								: m_Method;
	}
	else
	{
		Priority	= m_Priority;
		Flags		= m_Flags | BS_GetGroupFlags();
		States		= m_States;
		Method		= m_Method;
	}

#if 0
	int MethodBits = GetDocument()->m_Methods.GetTotalMethodBits();
	int StateBits = GetDocument()->m_StateClassList.GetTotalStateBits();
	int FlagBits = GetDocument()->m_FlagList.GetTotalFlagBits();
	int PriorityBits = GetDocument()->m_Priorities.GetTotalPriorityBits();

	int export = (((Priority & ((1 << PriorityBits) - 1)) << (StateBits + FlagBits))
		| ((Flags & ((1 << FlagBits) - 1)) << StateBits)
		| (States & ((1 << StateBits) - 1)));

	String.Format("0x%x", export);
#else

	String.Format("_METHOD_(%02d)|_PRIORITY_(%02d)|_FLAGS_(0x%02x)|_STATE_(%02d)", Method, Priority, Flags, States);

#endif

	return String;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::IsTransition()
{
	return m_States != m_EndStates;
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::Copy(CAnimTreeNode &AnimTreeNode)
{
	CAnimTrans	*pAnimTrans;
	CAnimTrans	*pNewAnimTrans;
	CAnimNode	*pAnimNode = (CAnimNode*)&AnimTreeNode;

	//---	Copy the base node.
	this->CopyBaseNode( AnimTreeNode );

	//---	Copy the Anim fields.
	m_AnimName.Format( "%s", pAnimNode->m_AnimName );
	m_Comment.Format( "%s", pAnimNode->m_Comment );
	m_EndStates	= pAnimNode->m_EndStates;
	m_Method	= pAnimNode->m_Method;
	m_Flags		= pAnimNode->m_Flags;
	m_Priority	= pAnimNode->m_Priority;
	m_States	= pAnimNode->m_States;
	m_FrameRate	= pAnimNode->m_FrameRate;
	m_RLIDs.Copy( pAnimNode->m_RLIDs );

	m_LinkAnim			= pAnimNode->m_LinkAnim;
	m_bLinkMethod		= pAnimNode->m_bLinkMethod;
	m_bLinkPriority		= pAnimNode->m_bLinkPriority;
	m_bLinkFrameRate	= pAnimNode->m_bLinkFrameRate;
	m_bLinkFlags		= pAnimNode->m_bLinkFlags;
	m_bLinkFlags		= pAnimNode->m_bLinkStates;
	m_bLinkTransitions	= pAnimNode->m_bLinkTransitions;
	m_LinkEnterFrame	= pAnimNode->m_LinkEnterFrame;

	SetCutTable( pAnimNode->GetCutTable() );
	
	//---	Copy the children.
	pAnimTrans = (CAnimTrans*)pAnimNode->GetFirstChild();
	while (pAnimTrans)
	{
		//---	Make a copy of the EquivNode.
		pNewAnimTrans = new CAnimTrans( pAnimTrans->m_pDoc, this );
		pNewAnimTrans->Copy( *pAnimTrans );
		this->AddChild( pNewAnimTrans );

		//---	Step to the next EquivNode from our source.
		pAnimTrans = (CAnimTrans*)pAnimNode->GetNextChild();
	}
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::CanHaveChildren()
{
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetNumAnims()
{
	return 1;	// It is its only anim, but if it has no Equivs then it is commented out.
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetNumExportedAnims()
{
	if( GetIgnore() )
		return 0;

	return 1;	// It is its only anim, but if it has no Equivs then it is commented out.
}

////////////////////////////////////////////////////////////////////////////
CAnimTreeNode * CAnimNode::Clone()
{
	CAnimNode	*pNewAnim = new CAnimNode( m_pDoc, m_pParent );

	pNewAnim->Copy( *this );

	return pNewAnim;
}

////////////////////////////////////////////////////////////////////////////
BOOL CAnimNode::HasExtraInfo()
{

	if ((m_FrameRate != "1.0") || RateAnalogControlled())
		return TRUE;
	else
		return FALSE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CAnimNode::RateAnalogControlled()
{
	CFlagList *pFlags = &m_pDoc->m_FlagList;
	int FlagBit = 1<<pFlags->GetFlagBit(pFlags->FindFlag(CString("RATEANALOG")));

	return ((m_Flags | BS_GetGroupFlags()) & FlagBit) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetNumAnimsWithDependancies( bool Recount, bool CountDisabledOnes )
{
	return (GetNumDependancies( Recount, CountDisabledOnes ) != 0) ? 1 : 0;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetTotalNumDependancies( bool Recount, bool CountDisabledOnes )
{
	return GetNumDependancies( Recount, CountDisabledOnes );
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetNumAnimsWithDependanciesWithLink( bool Recount, bool CountDisabledOnes )
{
	return (GetNumDependanciesWithLink( Recount, CountDisabledOnes ) != 0) ? 1 : 0;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetTotalNumDependanciesWithLink( bool Recount, bool CountDisabledOnes )
{
	return GetNumDependanciesWithLink( Recount, CountDisabledOnes );
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetNumAnimsWithExtras()
{
	return this->HasExtraInfo() ? 1 : 0;
}

////////////////////////////////////////////////////////////////////////////
CString CAnimNode::GetFrameRateExportString(CString Buffer)
{
	if (RateAnalogControlled())
		Buffer.Format( "%d /* Loco Rate Index */", m_Count );
	else if ((m_LinkAnim.GetLength() != 0) && m_bLinkFrameRate)
	{
		if (m_LinkAnim == "<previous anim>")
			Buffer.Format( "PREVIOUS_ANIM_RATE" );
		else if (m_LinkAnim == "<destination anim>")
			Buffer.Format( "DESTINATION_ANIM_RATE" );
		else
			Buffer.Format( "LINK_RATE_TO_ANIM(%s)", "ANIM_" + this->m_LinkAnim );
	}
	else
		Buffer.Format( "SET_ANIM_RATE(%s)", this->m_FrameRate );

	return Buffer;
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::AddToListBox(CListBox *pListBox, int nFlags)
{
	if (nFlags & LISTBOX_FLAG_INCLUDEANIMS)
	{
		//---	Add this animation to the list box.
		pListBox->AddString( m_AnimName );
	}
}

////////////////////////////////////////////////////////////////////////////
CString& CAnimNode::GetVPString(int *pDisplayOrder)
{
	int Option, i;
	CString str;
	int count, setting, setting2;
	CStateClass* pStateClass;
	CStateClassList* pStateClassList;
	CFlagList*	pFlagList;
	CFlag*		pFlag;
	m_VPString = "";

	while( (Option = *pDisplayOrder++) != VP_OPTION_NONE )
	{
		switch ( Option )
		{
		case VP_OPTION_NAME:
			m_VPString += GetName();
			m_VPString += "  ";
			break;

		case VP_OPTION_LRID:
			m_VPString += "Left/Right [ ";
			if (m_RLIDs.GetLeftReversed())
				m_VPString += "(r)";
			if (m_RLIDs.GetLeftMirrored())
				m_VPString += "~";
			m_VPString += m_RLIDs.GetLeftID();

			m_VPString += " / ";
			if (m_RLIDs.GetRightReversed())
				m_VPString += "(r)";
			if (m_RLIDs.GetRightMirrored())
				m_VPString += "~";
			m_VPString += m_RLIDs.GetRightID();
			m_VPString += " ]  ";
			break;

		case VP_OPTION_METHOD:
			if (!m_pDoc)
				break;

			str.Format( "%d", m_Method );
			m_VPString += "Method:: " + str + " (" + m_pDoc->m_Methods.GetMethod( m_Method ) + ")  ";
			break;

		case VP_OPTION_FLAGS:
			if (!m_pDoc)
				break;

			pFlagList = &m_pDoc->m_FlagList;
			count = pFlagList->GetNumFlags();
			
			if (count == 0)
				break;

			m_VPString += "Flags:: (";

			setting = 0;
			for( i=0; i<count; i++ )
			{
				pFlag = pFlagList->GetFlag( i );
				if (m_Flags & pFlag->GetMask())
				{
					if (setting)
						m_VPString += "|";
					m_VPString += pFlag->GetName();
					setting = 1;
				}
			}

			m_VPString += ")  ";
			break;

		case VP_OPTION_STATE:
			if (!m_pDoc)
				break;

			pStateClassList = &m_pDoc->m_StateClassList;
			count = pStateClassList->GetCount();

			if (count == 0)
				break;

			m_VPString += "States:: ";
				
			for( i=0; i<count; i++ )
			{
				pStateClass = pStateClassList->GetStateClass( i );
				setting = pStateClass->GetSetting( m_States );
				setting2 = pStateClass->GetSetting( m_EndStates );
				m_VPString += pStateClass->GetState( setting )->GetName();
				if (setting != setting2)
					m_VPString += "->" + pStateClass->GetState( setting2 )->GetName();
				if (i != (count-1))
					m_VPString += "|";
			}

			m_VPString += "  ";
			break;

		case VP_OPTION_PRIORITY:
			if (!m_pDoc)
				break;

			str.Format( "%d", m_Priority );
			m_VPString += "Priority:: " + str + m_pDoc->m_Priorities.GetPriority( m_Priority ) + "  ";
			break;

		case VP_OPTION_COMMENT:
			m_VPString += "'" + m_Comment + "'";
			break;
		}
	}

	m_VPString.TrimRight();

	return m_VPString;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetState()
{
	if ((m_RLIDs.GetLeftID() == "") || (m_RLIDs.GetRightID() == ""))
		return STATE_ERROR_BIT;

	return 0;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::IsExported()
{
	if (GetState() & STATE_ERROR_BIT)
		return FALSE;
		
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
void CAnimNode::FillCallbackStringArray( CStringArray& rStringArray )
{
	s32 i;

	if (m_Callback != "")
	{
		//---	look for the given callback
		for (i=0; i<rStringArray.GetSize(); i++)
			if (m_Callback == rStringArray[i])
				break;

		//---	if it was not found, add it.
		if (i == rStringArray.GetSize())
			i = rStringArray.Add( m_Callback );
	}
}

////////////////////////////////////////////////////////////////////////////
int	CAnimNode::GetNumDependanciesWithLink( bool Recount, bool CountDisabledOnes, bool CountInheritedOnes )
{
	int Count = GetNumDependancies( Recount, CountDisabledOnes, CountInheritedOnes );

	if( m_LinkAnim.GetLength() && m_bLinkTransitions )
	{
		CAnimNode* pLink = m_pDoc->FindAnimByName( m_LinkAnim );
		if( pLink )
			Count += pLink->GetNumTransitions( Recount, CountDisabledOnes, CountInheritedOnes );
	}

	return Count;
}

////////////////////////////////////////////////////////////////////////////
int	CAnimNode::GetNumDependancies( bool Recount, bool CountDisabledOnes, bool CountInheritedOnes )
{
	int Count = GetNumPrerequisites( Recount, CountDisabledOnes, CountInheritedOnes );
	Count += GetNumTransitions( Recount, CountDisabledOnes, CountInheritedOnes );

	return Count;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetNumParentDependancies( bool Recount, bool CountDisabledOnes )
{
	CGroupNode* pParent = GetParent();
	int Count=0;

// //	while( pParent )
//	{
//		Count += pParent->GetNumDependancies( Recount, CountDisabledOnes );
//		pParent = pParent->GetParent();
//	}
//
//	return Count;
	if( pParent )
		Count += pParent->GetNumDependancies( Recount, CountDisabledOnes );

	return Count;
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::SwitchCutTablePointer( void )
{
	int i;
	CLocoAnimTable* pTable;

	//---	if this animation has no cut table, exit now
	if (m_pCutTable == NULL)
		return;

	//---	look for this cut table in the newly modified tables
	for( i=0; i<m_pDoc->m_LocomotionTables.GetCount(); i++ )
	{
		pTable = m_pDoc->m_LocomotionTables.GetTable( i );

		//---	if this is the table which used to be pointed to by this animation, relink it now.
		if (pTable->m_pOldSelf == m_pCutTable)
		{
			m_pCutTable = pTable;
			return;
		}
	}

	m_pCutTable = NULL;
}

////////////////////////////////////////////////////////////////////////////
CAnimTrans* CAnimNode::GetFirstDependancy( void )
{
	CAnimTreeNode* pNode=GetFirstChild();

	while( pNode && ( pNode->m_Type != ANIMTREENODE_TYPE_TRANS ))
		pNode = GetNextChild();

	return (CAnimTrans*)pNode;
}

////////////////////////////////////////////////////////////////////////////
CAnimTrans* CAnimNode::GetNextDependancy( void )
{
	CAnimTreeNode* pNode=GetNextChild();

	while( pNode && ( pNode->m_Type != ANIMTREENODE_TYPE_TRANS ))
		pNode = GetNextChild();

	return (CAnimTrans*)pNode;
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::ClearDependancies( void )
{
	CAnimTreeNode* pNode = GetFirstDependancy();

	while( pNode )
	{
		RemoveChild( pNode );

		//---	Add the new group and setup the CUpdateView structure.
		m_pDoc->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_DELETEITEM, pNode, this );
		m_pDoc->UpdateAllViews( NULL );
		m_pDoc->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_UPDATEITEM, this );
		m_pDoc->UpdateAllViews( NULL );

		//---	it is safter to keep gettting the first one
		pNode = GetFirstDependancy();
	}
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::GetDependanciesWithLink( CAnimTransList& rAnimTransList )
{
	//---	get the animation transitions
	GetDependancies( rAnimTransList );

	//---	get any linked animation transitions
	if( m_LinkAnim.GetLength() && m_bLinkTransitions )
	{
		CAnimNode* pLink = m_pDoc->FindAnimByName( m_LinkAnim );
		if( pLink )
		{
			pLink->GetTransitions( rAnimTransList, "          " );
			pLink->GetParentTransitions( rAnimTransList, "<def> " );
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::GetDependancies( CAnimTransList& rAnimTransList )
{
	//---	get the rest of the prerequisites which are associated with this item
	GetPrerequisites( rAnimTransList, "          " );

	//---	get the parent prerequisites
	GetParentPrerequisites( rAnimTransList, "<def> " );

	//---	get the rest of the transitions which are associated with this item
	GetTransitions( rAnimTransList, "          " );

	//---	get the parents transitions
	GetParentTransitions( rAnimTransList, "<def> " );
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::SetDependancies( CAnimTransList& rAnimTransList )
{
	CAnimTrans* pNewNode;
	CAnimTrans* pTrans = rAnimTransList.GetFirst();
	CAnimTrans*	pLastNode = NULL;

	PushChildStep();
	ClearDependancies();
	CAnimTreeNode* pInsert = GetFirstChild();
	PopChildStep();

	while( pTrans )
	{
		//---	if the dependancy is tagged, do not add it to this list (it is from the parent)
		if (pTrans->GetTag() != "<def> ")
		{
			//---	make a copy of the anim trans node from the dependancy list
			pNewNode = new CAnimTrans( m_pDoc, this );
			*pNewNode = *pTrans;

			//---	add the node to the tree
			AddChild( pNewNode, NULL );

			//---	Add the new group and setup the CUpdateView structure.
			m_pDoc->m_Update.SetUpdateAction( UPDATEVIEW_ACTION_ADDITEM, pNewNode, this, pLastNode );
			m_pDoc->UpdateAllViews( NULL );

			pLastNode = pNewNode;
		}

		pTrans = rAnimTransList.GetNext();
	}
}

////////////////////////////////////////////////////////////////////////////
int	CAnimNode::GetNumPrerequisites( bool Recount, bool CountDisabledOnes, bool CountInheritedOnes )
{
	if( !CountDisabledOnes && !Recount )
	{
		int NumDeps = m_NumPrerequisites;
		if( CountInheritedOnes )
			NumDeps += GetNumParentPrerequisites( Recount, CountDisabledOnes );
		return NumDeps;
	}

	PushChildStep();

	int Count=0, DisCount=0;
	CAnimTrans* pDep=(CAnimTrans*)GetFirstDependancy();

	while( pDep )
	{
		if (pDep->IsPrerequisite())
		{
			if (pDep->GetDisabled())
				DisCount++;
			else
				Count++;
		}

		pDep = (CAnimTrans*)GetNextDependancy();
	}

	PopChildStep();

	//---	if we are to count inherited ones, do so now
	if( CountInheritedOnes )
		Count += GetNumParentPrerequisites( Recount, FALSE );

	m_NumPrerequisites = Count;

	if( CountDisabledOnes )
		return m_NumPrerequisites + GetNumParentPrerequisites( Recount, TRUE );
	else
		return m_NumPrerequisites;
}

////////////////////////////////////////////////////////////////////////////
int	CAnimNode::GetNumParentPrerequisites( bool Recount, bool CountDisabledOnes )
{
	CGroupNode* pParent = GetParent();
	int count=0;
//
//	while( pParent )
//	{
//		count += pParent->GetNumPrerequisites( Recount, CountDisabledOnes );
//		pParent = pParent->GetParent();
//	}
//
//	return count;
	if( pParent )
		count = pParent->GetNumPrerequisites( Recount, CountDisabledOnes );

	return count;
}

////////////////////////////////////////////////////////////////////////////
CAnimTrans* CAnimNode::GetFirstPrerequisite( void )
{
	CAnimTrans* pNode=GetFirstDependancy();

	while( pNode && !((CAnimTrans*)pNode)->IsPrerequisite() )
		pNode = GetNextDependancy();

	return pNode;
}

////////////////////////////////////////////////////////////////////////////
CAnimTrans* CAnimNode::GetNextPrerequisite( void )
{
	CAnimTrans* pNode=GetNextDependancy();

	while( pNode && !((CAnimTrans*)pNode)->IsPrerequisite() )
		pNode = GetNextDependancy();

	return pNode;
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::GetPrerequisites( CAnimTransList& rAnimTransList, CString TagString )
{
	PushChildStep();

	CAnimTrans* pNewNode;
	CAnimTrans* pNode = (CAnimTrans*)GetFirstPrerequisite();

	while( pNode )
	{
		pNewNode = new CAnimTrans( m_pDoc, this );
		*pNewNode = *pNode;
		pNewNode->SetTag( TagString );
		rAnimTransList.Add( pNewNode );
		pNode = (CAnimTrans*)GetNextPrerequisite();
	}

	PopChildStep();
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::GetParentPrerequisites( CAnimTransList& rAnimTransList, CString TagString )
{
	CGroupNode* pParent = GetParent();

	while( pParent )
	{
		pParent->GetPrerequisites( rAnimTransList, TagString );
		pParent = pParent->GetParent();
	}
}

////////////////////////////////////////////////////////////////////////////
int	CAnimNode::GetNumTransitions( bool Recount, bool CountDisabledOnes, bool CountInheritedOnes )
{
	if( !CountDisabledOnes && !Recount )
	{
		int NumDeps = m_NumTransitions;
		if( CountInheritedOnes )
			NumDeps += GetNumParentTransitions( Recount, CountDisabledOnes );
		return NumDeps;
	}

	PushChildStep();

	int Count=0, DisCount=0;
	CAnimTrans* pDep=(CAnimTrans*)GetFirstDependancy();

	while( pDep )
	{
		if (pDep->IsTransition())
		{
			if (pDep->GetDisabled())
				DisCount++;
			else
				Count++;
		}

		pDep = (CAnimTrans*)GetNextDependancy();
	}

	PopChildStep();

	//---	if we are to count inherited ones, do so now
	if( CountInheritedOnes )
		Count += GetNumParentTransitions( Recount, FALSE );

	m_NumTransitions = Count;

	if( CountDisabledOnes )
		return m_NumTransitions + GetNumParentTransitions( Recount, TRUE );
	else
		return m_NumTransitions;
}

////////////////////////////////////////////////////////////////////////////
int CAnimNode::GetNumParentTransitions( bool Recount, bool CountDisabledOnes )
{
	CGroupNode* pParent = GetParent();
	int count=0;
//
//	while( pParent )
//	{
//		count += pParent->GetNumPrerequisites( Recount, CountDisabledOnes );
//		pParent = pParent->GetParent();
//	}
//
//	return count;
	if( pParent )
		count = pParent->GetNumTransitions( Recount, CountDisabledOnes );

	return count;
}

////////////////////////////////////////////////////////////////////////////
CAnimTrans* CAnimNode::GetFirstTransition( void )
{
	CAnimTrans* pNode=GetFirstDependancy();

	while( pNode && !((CAnimTrans*)pNode)->IsTransition() )
		pNode = GetNextDependancy();

	return pNode;
}

////////////////////////////////////////////////////////////////////////////
CAnimTrans* CAnimNode::GetNextTransition( void )
{
	CAnimTrans* pNode=GetNextDependancy();

	while( pNode && !((CAnimTrans*)pNode)->IsTransition() )
		pNode = GetNextDependancy();

	return pNode;
}

////////////////////////////////////////////////////////////////////////////
void CAnimNode::GetTransitions( CAnimTransList& rAnimTransList, CString TagString )
{
	PushChildStep();

	CAnimTrans* pNewNode;
	CAnimTrans* pNode = (CAnimTrans*)GetFirstTransition();

	while( pNode )
	{
		pNewNode = new CAnimTrans( m_pDoc, this );
		*pNewNode = *pNode;
		pNewNode->SetTag( TagString );
		rAnimTransList.Add( pNewNode );
		pNode = (CAnimTrans*)GetNextTransition();
	}

	PopChildStep();
}

void CAnimNode::GetParentTransitions( CAnimTransList& rAnimTransList, CString TagString )
{
	CGroupNode* pParent = GetParent();

	while( pParent )
	{
		pParent->GetTransitions( rAnimTransList, TagString );
		pParent = pParent->GetParent();
	}
}

int CAnimNode::GetCallbackExport( void )
{
	int i;

	if (m_Callback == "")
		return 0;

	//===	ASSUMES THAT THE CALLBACKS LIST HAS BEEN FILLED IN THE DOCUMENT SINCE THE
	//		CALLBACK FUNCTION PROTOTYPES WOULD HAVE ALREADY BEEN EXPORTED

	//---	search for this callback in the list of callbacks
	for( i=0; i<m_pDoc->m_CallbackStrings.GetSize(); i++ )
		if (m_Callback == m_pDoc->m_CallbackStrings[i])
			break;

	ASSERT( i<m_pDoc->m_CallbackStrings.GetSize() );
	if (i >= m_pDoc->m_CallbackStrings.GetSize())
		return 0;

	return i+1;	// we must add 1 because the first element of the array is a NULL pointer for the "return 0" cases
}

CString CAnimNode::GetCutTableExport( void )
{
#if MAX_NUM_CUTTABLES==255
	if (m_pCutTable == NULL) return "0xff";
#else
	if (m_pCutTable == NULL) return "0xffff";
#endif

	return m_pCutTable->GetExportDefine();
}


int CAnimNode::CountAllDependancies( void )
{
	return GetNumDependancies( TRUE, FALSE, FALSE );
}

void CAnimNode::SwapEquivalency( void )
{
	m_RLIDs.Swap();
	ModifyViews();
	m_pDoc->SetModifiedFlag();
}
