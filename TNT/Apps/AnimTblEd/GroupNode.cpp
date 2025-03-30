// GroupNode.cpp: implementation of the CGroupNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"
#include "AnimTblEdView.h"
#include <afxtempl.h>
#include "GroupNode.h"
#include "AnimID1.h"
#include "IDList.h"

//#include "GroupNodeEditDlg.h"
#include "AnimationEditDlg.h"

#include "AnimTblEdGlobals.h"
#include "AnimTblEdDefs.h"
#include "StateClass.h"
#include "State.h"

#include "QuestionDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern int CurrentExportCount;
extern int CurrentExportType;
extern int LastTokenType;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGroupNode::CGroupNode()
{
	m_nChildren = 0;
	m_Method = 0;
	m_Flags = 0;
	m_States = 0;
	m_Priority = 0;
	m_pCutTable = NULL;
	m_SelectCallback = "";
	m_Callback = "";
}

//////////////////////////////////////////////////////////////////////
CGroupNode::CGroupNode(class CAnimTblEdDoc *pDoc, class CAnimTreeNode *pParent, int type, const char *lpszName):CAnimTreeNode( pDoc, pParent )
{
	//---	Set the type into the base structure.
	CAnimTreeNode::m_Type = type;
	m_nChildren = 0;

	if (lpszName)
		m_Name.Format( "%s", lpszName );
	else
		m_Name.Format( "%s", "GROUP" );

	m_Method = 0;
	m_Flags = 0;
	m_States = 0;
	m_Priority = 0;

	m_pCutTable = NULL;
	m_SelectCallback = "";
	m_Callback = "";
}

//////////////////////////////////////////////////////////////////////
CGroupNode::~CGroupNode()
{

}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetMouseMenuIndex()
{
	return 0;
}

//////////////////////////////////////////////////////////////////////
bool CGroupNode::Edit()
{
	//---	For now we won't allow editing of the ENUM or the UNUSED groups.
	if (m_Type == ANIMTREENODE_TYPE_GROUP)
	{
		CAnimationEditDlg	GroupEditDlg( this );

//		CGroupNodeEditDlg	GroupEditDlg;
//		GroupEditDlg.m_pGroupNode = this;

		if (GroupEditDlg.DoModal() == IDOK)
		{
			//---	force a recount of the dependancies
			GetNumDependancies( TRUE );

			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetImage()
{
	switch (m_Type)
	{
	case ANIMTREENODE_TYPE_GROUP :
		return TREEIMAGE_GROUP;
	case ANIMTREENODE_TYPE_ENUM :
		return TREEIMAGE_ENUM;
	case ANIMTREENODE_TYPE_UNUSED :
		return TREEIMAGE_UNUSED;
	case ANIMTREENODE_TYPE_PROJECT :
		return TREEIMAGE_PROJECT;
	default :
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetSelectedImage()
{
	switch (m_Type)
	{
	case ANIMTREENODE_TYPE_GROUP :
		return TREEIMAGE_GROUP_SELECT;
	case ANIMTREENODE_TYPE_ENUM :
		return TREEIMAGE_ENUM_SELECT;
	case ANIMTREENODE_TYPE_UNUSED :
		return TREEIMAGE_UNUSED_SELECT;
	case ANIMTREENODE_TYPE_PROJECT :
		return TREEIMAGE_PROJECT_SELECT;
	default :
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::Remove(CIDList &IDList)
{
	CAnimTreeNode	*pAnimTreeNode;

	while (!m_ChildList.IsEmpty())
	{
		pAnimTreeNode = m_ChildList.RemoveHead();

		pAnimTreeNode->Remove(IDList);

		delete pAnimTreeNode;
	}

	m_nChildren = 0;
}

//////////////////////////////////////////////////////////////////////
bool CGroupNode::HasChildren()
{
	return (!m_ChildList.IsEmpty());
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::Serialize(CArchive &ar)
{
	short int		version;
	int				nChildren;
	CAnimTreeNode	*pAnimTreeNode;
	CString			CutTable;
	int				i;

	//---	Serialize the group class.
	if (ar.IsStoring())
	{
		//---	Usual version stuff.
		version = 8;
		ar.Write( &version, 2);

		//---********  ADD NEW SERIALIZATIONS HERE ***********

		//---	callback function for all animations under group
		ar.WriteString( m_Callback );
		ar.WriteString( "\xd\xa" );

		//---	selection callback function used to chose a single animation from a group of animations
		ar.WriteString( m_SelectCallback );
		ar.WriteString( "\xd\xa" );

		//---	save the cut table name so that it can be re-linked at load
		if (m_pCutTable != NULL)
			ar.WriteString( m_pCutTable->m_Name );
		ar.WriteString( "\xd\xa" );

		//---	Serialize the base class.
		CAnimTreeNode::Serialize( ar );

		//---	Group Comment.
		ar.WriteString( m_Comment );
		ar.WriteString( "\xd\xa" );

		//---	Group specific stuff.
		ar.WriteString( m_Name );
		ar.WriteString( "\xd\xa" );

		ar << m_Method;
		ar << m_Flags;
		ar << m_Priority;
		ar << m_States;
		
		//---	Children.
		ar << m_nChildren;
		pAnimTreeNode = CGroupNode::GetFirstChild();
		while (pAnimTreeNode)
		{
			pAnimTreeNode->WriteTreeItem( ar );
			pAnimTreeNode = CGroupNode::GetNextChild();
		}
	}
	else
	{
		//---	get the document
		m_pDoc = (CAnimTblEdDoc*)ar.m_pDocument;

		ar.Read( &version, 2 );
		switch (version)
		{
		case 1:
			ar.ReadString( m_Name );
			break;

		case 3:
			ar.ReadString( m_Comment );
		case 2:
			ar.ReadString( m_Name );

			ar >> m_Flags;
			ar >> m_Priority;
			ar >> m_States;

			if		(m_Flags & 0x1)		m_Method = ANIMMETHOD_NETDISPLACEMENT;
//			else if (m_Flags & 0x10)	m_Method = ANIMMETHOD_NODISPLACEMENT;
			else if (m_Flags & 0x2)		m_Method = ANIMMETHOD_FINALMOVEDIR;
			else if (m_Flags & 0x4)		m_Method = ANIMMETHOD_FINALFACEDIR;
			else if (m_Flags & 0x8)		m_Method = ANIMMETHOD_FRAMEFACING;

			break;

		////////////////////////////////////////////////////////////////////
		//	ADD NEW VERSION CODE HERE
		////////////////////////////////////////////////////////////////////


		case 8:
		case 7:
		case 6:
			//---	callback function for all animations under group
			ar.ReadString( m_Callback );

			//---	selection callback function used to chose a single animation from a group of animations
			ar.ReadString( m_SelectCallback );

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

		case 5:
			//---	Serialize the base class.
			CAnimTreeNode::Serialize( ar );
		case 4:
			ar.ReadString( m_Comment );
			ar.ReadString( m_Name );

			ar >> m_Method;
			ar >> m_Flags;
			ar >> m_Priority;
			ar >> m_States;

			//---	special case code to remove the ANIMMETHOD_NODISPLACEMENT
			//		and add the ANIMFLAG_NODISPLACEMENT
			if( version <= 6 )
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
			if( version <= 7 )
			{
				#define	NUMDEFAULTFLAGSBEFOREUSEDESTSPEED		7
				#define	DEFAULTFLAGSMASKBEFOREUSEDESTSPEED	((1<<NUMDEFAULTFLAGSBEFOREUSEDESTSPEED)-1)
				m_Flags = (m_Flags & DEFAULTFLAGSMASKBEFOREUSEDESTSPEED) | ((m_Flags & ~DEFAULTFLAGSMASKBEFOREUSEDESTSPEED) << 1);
				m_Flags &= ~(1<<ANIMFLAG_USEDESTSPEED);
			}

			break;
		}

		ar >> nChildren;
		for (; nChildren > 0; nChildren--)
			AddChild( CAnimTreeNode::ReadTreeItem( ar, this ) );
	}
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::SetFirstChild()
{
	m_ChildPos = m_ChildList.GetHeadPosition();
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetAddGroupOptionEnabled()
{
	switch (m_Type)
	{
	case ANIMTREENODE_TYPE_PROJECT:
	case ANIMTREENODE_TYPE_UNUSED:
		return MF_GRAYED;

	case ANIMTREENODE_TYPE_ENUM:
	case ANIMTREENODE_TYPE_GROUP:
		return MF_ENABLED;

	default:
		ASSERT( 0 );
		break;
	}
	return MF_DISABLED;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetAddAnimOptionEnabled()
{
	switch (m_Type)
	{
	case ANIMTREENODE_TYPE_PROJECT:
		return MF_GRAYED;

	case ANIMTREENODE_TYPE_UNUSED:
	case ANIMTREENODE_TYPE_ENUM:
	case ANIMTREENODE_TYPE_GROUP:
		return MF_ENABLED;

	default:
		ASSERT( 0 );
		break;
	}
	return MF_DISABLED;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetEditOptionEnabled()
{
	switch (m_Type)
	{
	case ANIMTREENODE_TYPE_PROJECT:
	case ANIMTREENODE_TYPE_UNUSED:
	case ANIMTREENODE_TYPE_ENUM:
		return MF_GRAYED;

	case ANIMTREENODE_TYPE_GROUP:
		return MF_ENABLED;

	default:
		ASSERT( 0 );
		break;
	}
	return MF_DISABLED;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetDeleteOptionEnabled()
{
	switch (m_Type)
	{
	case ANIMTREENODE_TYPE_PROJECT:
	case ANIMTREENODE_TYPE_UNUSED:
	case ANIMTREENODE_TYPE_ENUM:
		return MF_GRAYED;

	case ANIMTREENODE_TYPE_GROUP:
		return MF_ENABLED;

	default:
		ASSERT( 0 );
		break;
	}
	return MF_DISABLED;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetScanSourceCodeOptionEnabled()
{
	switch (m_Type)
	{
	case ANIMTREENODE_TYPE_PROJECT:
	case ANIMTREENODE_TYPE_UNUSED:
	case ANIMTREENODE_TYPE_ENUM:
		return MF_GRAYED;

	case ANIMTREENODE_TYPE_GROUP:
		return MF_ENABLED;

	default:
		ASSERT( 0 );
		break;
	}
	return MF_DISABLED;
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::Export(FILE *pFile)
{
	CString str;
	bool Ignore;
	bool IgnoreOnly;

	//---	Export file ANIM_ list.
	if (CAnimTreeNode::GetCurrentExportType() == EXPORTTYPE_ANIMLIST)
	{
		switch (m_Type)
		{
		case ANIMTREENODE_TYPE_ENUM:
//			fprintf( pFile, "enum\n{\n" );
//			ExportChildren();
//			fprintf( pFile, "}\n" );
//			break;
			CAnimTreeNode::ClearCount();
			CAnimTreeNode::ClearExportTransIndex();
			CAnimTreeNode::SetLastTokenType( TOKEN_NONE );
			ExportChildren( pFile );
			break;

		case ANIMTREENODE_TYPE_GROUP:
			if ((CAnimTreeNode::GetLastTokenType() != TOKEN_GROUPSTART) && (CAnimTreeNode::GetLastTokenType() != TOKEN_GROUPEND))
				CAnimTreeNode::IncCount();

			Ignore = GetIgnore();
			IgnoreOnly = Ignore && (!m_pParent || !m_pParent->GetIgnore());
			if( IgnoreOnly )
				fprintf( pFile, "/*\n");

			fprintf( pFile, "\n#define\tSTART_%s_ANIMS\t%d\n", GetName(), CAnimTreeNode::GetCount() );
			if( !Ignore ) CAnimTreeNode::SetLastTokenType( TOKEN_GROUPSTART );

			ExportChildren( pFile );

			if ((CAnimTreeNode::GetLastTokenType() != TOKEN_GROUPEND) && (CAnimTreeNode::GetLastTokenType() != TOKEN_GROUPSTART))
				CAnimTreeNode::IncCount();

			fprintf( pFile, "#define\tEND_%s_ANIMS\t%d\n", GetName(), CAnimTreeNode::GetCount() );
			if( !Ignore ) CAnimTreeNode::SetLastTokenType( TOKEN_GROUPEND );

			if( IgnoreOnly  )
				fprintf( pFile, "*/\n");

			break;


		case ANIMTREENODE_TYPE_PROJECT:
			fprintf( pFile, "#define\tANIM_NONE\t0\n" );
			ExportChildren( pFile );

			//---	Save the number of anims to be exported later.
			GLB_NumAnims = CAnimTreeNode::GetCount();
			break;

		case ANIMTREENODE_TYPE_UNUSED:
		default:
			break;
		}
	}

	//---	Export the ANIMGROUP_ defines.
	else if (CAnimTreeNode::GetCurrentExportType() == EXPORTTYPE_ANIMLISTGROUP)
	{
		switch (m_Type)
		{
		case ANIMTREENODE_TYPE_GROUP:

			if( GetIgnore() )	fprintf( pFile, "//" );
			fprintf( pFile, "#define\tANIMGROUP_%s\t%d\n", GetName(), CAnimTreeNode::GetCount() );
//			fprintf( pFile, "#define\tANIMGROUP_%s\t((START_%s_ANIMS<<16)+NUM_%s_ANIMS)\n", GetName(), GetName(), GetName() );

			CAnimTreeNode::IncCount();

			ExportChildren( pFile );
			break;


		case ANIMTREENODE_TYPE_PROJECT:
		case ANIMTREENODE_TYPE_ENUM:
			ExportChildren( pFile );
			break;

		case ANIMTREENODE_TYPE_UNUSED:
		default:
			break;
		}
	}
	
	//---	Export file NUM_ defines.
	else if (CAnimTreeNode::GetCurrentExportType() == EXPORTTYPE_NUMANIMS)
	{
		switch (m_Type)
		{
		case ANIMTREENODE_TYPE_GROUP:
			if( GetIgnore() )	fprintf( pFile, "//" );
//			fprintf( pFile, "#define\tNUM_%s_ANIMS\t(END_%s_ANIMS - START_%s_ANIMS)\n", GetName(), GetName(), GetName());
			fprintf( pFile, "#define\tNUM_%s_ANIMS\t%d\n", GetName(), this->GetNumAnims() );
			ExportChildren( pFile );
			break;

		case ANIMTREENODE_TYPE_PROJECT:
			fprintf( pFile, "#define\tNUM_ALL_ANIMS\t%d\n\n", GLB_NumAnims );

		case ANIMTREENODE_TYPE_ENUM:
			ExportChildren( pFile );
			break;

		case ANIMTREENODE_TYPE_UNUSED:
		default:
			break;
		}
	}

	//---	Export Macros.
	else if (CAnimTreeNode::GetCurrentExportType() == EXPORTTYPE_MACROS)
	{
		switch (m_Type)
		{
		case ANIMTREENODE_TYPE_GROUP:
			ExportChildren( pFile );
			if( GetIgnore() )	fprintf( pFile, "//" );
			fprintf( pFile, "#define\tIS_%s_ANIM(n)\t(((n) >= START_%s_ANIMS) && ((n) < END_%s_ANIMS))\n", GetName(), GetName(), GetName());
			break;

		case ANIMTREENODE_TYPE_PROJECT:
			fprintf( pFile, "#define\tANM_MIR_BIT\t0x8000\n", CurrentExportCount );
			fprintf( pFile, "#define\tANM_REV_BIT\t0x4000\n", CurrentExportCount );
			fprintf( pFile, "\n" );
			fprintf( pFile, "#define\tSET_BLENDING(n)\t((n)<<12)\n", CurrentExportCount );
			fprintf( pFile, "#define\tGET_BLENDING(n)\t((n)>>12)\n", CurrentExportCount );
			fprintf( pFile, "#define\tGET_TRANSANIM(n)\t((n)&0xfff)\n", CurrentExportCount );
			fprintf( pFile, "\n" );
		case ANIMTREENODE_TYPE_ENUM:
			ExportChildren( pFile );
			break;

		case ANIMTREENODE_TYPE_UNUSED:
		default:
			break;
		}
	}
#if 0
	//---	Export file AnimGroup_ data structures.
	else if (CAnimTreeNode::GetCurrentExportType() == EXPORTTYPE_ANIMOBJECTS)
	{
		switch (m_Type)
		{
		case ANIMTREENODE_TYPE_GROUP:
			fprintf( pFile, "//---  %s Animation Data Structures.\n", GetName() );

			//---	Export the comment.
			if (!m_Comment.IsEmpty())
			{
				CString	CommentCopy;
				CString	CommentLine;

				CommentCopy.Format( "%s", m_Comment );
				while (!CommentCopy.IsEmpty())
				{
					CommentLine.Format( "%s", CommentCopy.Left( LENGTH_OF_COMMENT ) );
					CommentCopy.Delete( 0, LENGTH_OF_COMMENT );

					fprintf( pFile, "// %s\n", CommentLine );
				}
			}

			ExportChildren( pFile );
			fprintf( pFile, "\n" );
			break;

		case ANIMTREENODE_TYPE_PROJECT:
			fprintf( pFile, "s16 AnimGroup_None[] = {0};\n\n" );
		case ANIMTREENODE_TYPE_ENUM:
			ExportChildren( pFile );
			break;

		default:
			break;
		}
	}
#endif
	//---	Export animation transition table.
	else if (CAnimTreeNode::GetCurrentExportType() == EXPORTTYPE_DEPENDANCIES)
	{
		switch (m_Type)
		{
		case ANIMTREENODE_TYPE_ENUM:
			fprintf( pFile, "const s16    AnimTransTable[] =\n{\n" );
			fprintf( pFile, "\t-1,\n" );
			ExportChildren( pFile );
			fprintf( pFile, "};\n\n" );
			break;

		case ANIMTREENODE_TYPE_GROUP:
			IgnoreOnly = GetIgnore() && (!m_pParent || !m_pParent->GetIgnore());
			if( IgnoreOnly )
				fprintf( pFile, "/* \n" );

			fprintf( pFile, "\n\t// %s Animations.\n", this->GetName() );
			ExportChildren( pFile );

			if( IgnoreOnly )
				fprintf( pFile, "*/\n" );
			fprintf( pFile, "\n" );
			break;
/*	the animation is exported inside of CAnimNode::Export
		case ANIMTREENODE_TYPE_ANIM:
			fprintf( pFile, "\n\t// %s Dependancies.\n", this->GetName() );
			ExportChildren( pFile );
			fprintf( pFile, "\n" );
			break;
*/
		case ANIMTREENODE_TYPE_PROJECT:
			ExportChildren( pFile );
			break;

		default:
			break;
		}
	}

	//---	Export animation table.
	else if (CAnimTreeNode::GetCurrentExportType() == EXPORTTYPE_OBJECTLIST)
	{
		switch (m_Type)
		{
		case ANIMTREENODE_TYPE_ENUM:
			fprintf( pFile, "const t_SAnimTable    AnimTable[NUM_ALL_ANIMS] =\n{\n" );
			fprintf( pFile, "\t{{{ANIM_NONE, ANIM_NONE}}, 0x0, 0.0f, 0, ANIM_NONE, 0},\n");
			CAnimTreeNode::IncCount();// count the ANIM_NONE anim
			ExportChildren( pFile );
			fprintf( pFile, "};\n\n" );
			break;

		case ANIMTREENODE_TYPE_GROUP:
			fprintf( pFile, "\n\t// %s Animations.\n", this->GetName() );
			ExportChildren( pFile );
			fprintf( pFile, "\n" );
			break;

		case ANIMTREENODE_TYPE_PROJECT:
			ExportChildren( pFile );
			break;

		default:
			break;
		}
	}

	//---	Export animation table.
	else if (CAnimTreeNode::GetCurrentExportType() == EXPORTTYPE_OBJECTLISTGROUP)
	{
		switch (m_Type)
		{
		case ANIMTREENODE_TYPE_ENUM:
			//---	start the group info table export
			fprintf( pFile, "t_SAnimGroupTable	AnimGroupTable[%d] =\n", this->GetGroupCount() );
			fprintf( pFile, "{\n" );
			ExportChildren( pFile );
			fprintf( pFile, "};\n\n" );
			break;

		case ANIMTREENODE_TYPE_GROUP:
			if( GetIgnore() )	fprintf( pFile, "//" );
			str = GetSelectCallback();
			if (str == "")	str = "NULL";
			else			str = "GROUPCB_" + str;
			fprintf( pFile, "\t{START_%s_ANIMS, NUM_%s_ANIMS, %s},\n", this->GetName(), this->GetName(), str );
			ExportChildren( pFile );
			break;

		case ANIMTREENODE_TYPE_PROJECT:
			ExportChildren( pFile );
			break;

		default:
			break;
		}
	}

	//---	Export .C file AnimTrans_ dependancy structures.
	else
	{
		switch (m_Type)
		{
		case ANIMTREENODE_TYPE_ENUM:
		case ANIMTREENODE_TYPE_PROJECT:
		case ANIMTREENODE_TYPE_GROUP:
			ExportChildren( pFile );
			break;

		default:
			break;
		}
	}
}

//DEL void CGroupNode::ExportChildren(FILE *pFile)
//DEL {
//DEL 	CAnimTreeNode *pAnimTreeNode;
//DEL 
//DEL 	pAnimTreeNode = GetFirstChild();
//DEL 
//DEL 	while (pAnimTreeNode)
//DEL 	{
//DEL 		pAnimTreeNode->Export( pFile );
//DEL 		pAnimTreeNode = GetNextChild();
//DEL 	}
//DEL }


//////////////////////////////////////////////////////////////////////
int CGroupNode::BS_GetFlags()
{
	return m_Flags;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::BS_GetStates()
{
	return m_States;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::BS_GetPriority()
{
	return m_Priority;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::BS_GetMethod()
{
	return m_Method;
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::BS_SetFlags(int Value)
{
	static int AlreadyComparingDifferences = FALSE;

	//---	return if the values are identical
	if (m_Flags == Value)
		return;

	//---	look for changes which should be sent down to the chhildren
	if (!AlreadyComparingDifferences && this->HasChildren())
	{
		AlreadyComparingDifferences = TRUE;

		int Changes = m_Flags ^ Value;
		int bit;

		for (bit=0; bit<GetDocument()->m_FlagList.GetNumFlags(); bit++)
		{
			//---	determine if this flag changed
			if (Changes & 1)
			{
				//---	was the flag added or removed
				if (m_Flags & (1<<bit))
				{
					//---	the flag was removed so ask the user if he wants the children to have the flag automatically set
					CString Question = "The flag '" + GetDocument()->m_FlagList.GetFlag(bit)->GetName() + "' has been removed.  Should it be applied to all child nodes?";
					CQuestionDlg Dialog(Question);
					if (Dialog.DoModal() == IDOK)
						SetChildrenFlag(bit, TRUE);
				}
				else
				{
					//---	the flag was added so remove it from the children
					SetChildrenFlag(bit, FALSE);
				}
			}

			//---	increment to the next bit
			Changes >>= 1;
		}

		AlreadyComparingDifferences = FALSE;
	}

	m_Flags = Value;
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::BS_SetStates(int Value)
{
	CStateClass *pClass;
	int i;

	if (m_States == Value)
		return;

	for (i=0; i<GetDocument()->m_StateClassList.GetCount(); i++)
	{
		pClass = GetDocument()->m_StateClassList.GetStateClass(i);

		if (pClass->GetSetting(m_States) != pClass->GetSetting(Value))
			SetChildrenDefaultState(i, pClass->GetSetting(m_States), pClass->GetSetting(Value));
	}

	m_States = Value;
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::BS_SetPriority(int Value)
{
	if (m_Priority == Value)
		return;

	SetChildrenDefaultPriority(m_Priority, Value);

	m_Priority = Value;
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::BS_SetMethod(int Value)
{
	if (m_Method == Value)
		return;

	SetChildrenDefaultMethod(m_Method, Value);

	m_Method = Value;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::BS_GetGroupFlags()
{
	if (GetParent() == NULL)
		return 0;

	return GetParent()->BS_GetGroupFlags() | GetParent()->BS_GetFlags();
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::BS_GetGroupStates()
{
	if (GetParent() == NULL)
		return 0;

	return GetParent()->BS_GetStates();
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::BS_GetGroupPriority()
{
	if (GetParent() == NULL)
		return 0;

	return GetParent()->BS_GetPriority();
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::BS_GetGroupMethod()
{
	if (GetParent() == NULL)
		return 0;

	return GetParent()->BS_GetMethod();
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::SetChildrenDefaultState(int StateClass, int CurValue, int NewValue)
{
	CStateClass *pClass;
	CAnimTreeNode *pChild;
	int i;

	pClass = GetDocument()->m_StateClassList.GetStateClass(StateClass);

	//---	if the current setting of the state is not the default, return
	if (pClass->GetSetting(m_States) != CurValue)
		return;

	//---	change the setting to the new default
	pClass->SetSetting(m_States, NewValue);

	//---	change the children also
	for (i=0; i<m_ChildList.GetCount(); i++)
	{
		pChild = m_ChildList.GetAt(m_ChildList.FindIndex(i));

		if (pChild->m_Type == ANIMTREENODE_TYPE_GROUP)
			((CGroupNode*) pChild)->SetChildrenDefaultState(StateClass, CurValue, NewValue);
		else if (((CAnimNode*) pChild)->BS_GetState(StateClass) == CurValue)
			((CAnimNode*) pChild)->BS_SetState(StateClass, NewValue);
	}
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::SetChildrenDefaultPriority(int CurValue, int NewValue)
{
	CAnimTreeNode *pChild;
	int i;

	//---	if the current setting of the state is not the default, return
	if (m_Priority != CurValue)
		return;

	//---	change the setting to the new default
	m_Priority = NewValue;

	//---	change the children also
	for (i=0; i<m_ChildList.GetCount(); i++)
	{
		pChild = m_ChildList.GetAt(m_ChildList.FindIndex(i));

		if (pChild->m_Type == ANIMTREENODE_TYPE_GROUP)
			((CGroupNode*) pChild)->SetChildrenDefaultPriority(CurValue, NewValue);
		else if (((CAnimNode*) pChild)->BS_GetPriority() == CurValue)
			((CAnimNode*) pChild)->BS_SetPriority(NewValue);
	}
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::SetChildrenDefaultMethod(int CurValue, int NewValue)
{
	CAnimTreeNode *pChild;
	int i;

	//---	if the current setting of the state is not the default, return
	if (m_Method != CurValue)
		return;

	//---	change the setting to the new default
	m_Method = NewValue;

	//---	change the children also
	for (i=0; i<m_ChildList.GetCount(); i++)
	{
		pChild = m_ChildList.GetAt(m_ChildList.FindIndex(i));

		if (pChild->m_Type == ANIMTREENODE_TYPE_GROUP)
			((CGroupNode*) pChild)->SetChildrenDefaultMethod(CurValue, NewValue);
		else if (((CAnimNode*) pChild)->BS_GetMethod() == CurValue)
			((CAnimNode*) pChild)->BS_SetMethod(NewValue);
	}
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::SetChildrenFlag(int Flag, int Value)
{
	CAnimTreeNode *pChild;
	int i;

	//---	change the children also
	for (i=0; i<m_ChildList.GetCount(); i++)
	{
		pChild = m_ChildList.GetAt(m_ChildList.FindIndex(i));

		if (pChild->m_Type == ANIMTREENODE_TYPE_GROUP)
		{
			if (Value)
				((CGroupNode*) pChild)->BS_SetFlags(((CGroupNode*) pChild)->BS_GetFlags() | (1<<Flag));
			else
				((CGroupNode*) pChild)->BS_SetFlags(((CGroupNode*) pChild)->BS_GetFlags() & ~(1<<Flag));

			((CGroupNode*) pChild)->SetChildrenFlag(Flag, Value);
		}
		else if (pChild->m_Type == ANIMTREENODE_TYPE_ANIM)
		{
			if (Value)
				((CAnimNode*) pChild)->BS_SetFlags(((CAnimNode*) pChild)->BS_GetFlags() | (1<<Flag));
			else
				((CAnimNode*) pChild)->BS_SetFlags(((CAnimNode*) pChild)->BS_GetFlags() & ~(1<<Flag));
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::ShiftStateClasses(int Operation, int StateClass, int State, int Bit, int Num)
{
	int LowMask = (1 << Bit) - 1;
	int HighMask = (1 << (Bit + Num)) - 1;
	int CurState;

	switch (Operation)
	{
	case -2://---	REMOVE SOME BITS
			m_States = (m_States & LowMask) | ((m_States & ~HighMask) >> Num);
		break;

	case -1://---	REMOVE A STATE
			CurState = (m_States & (HighMask & ~LowMask)) >> Bit;
			if ((CurState >= State) && (CurState != 0))
			{
				CurState--;
				m_States = (m_States & ~(HighMask & ~LowMask)) | (CurState << Bit);
			}
		break;

	case 1://---	ADD A STATE
			CurState = (m_States & (HighMask & ~LowMask)) >> Bit;
			if (CurState >= State)
			{
				CurState++;
				m_States = (m_States & ~(HighMask & ~LowMask)) | ((CurState << Bit) & (HighMask & ~LowMask));
			}
		break;

	case 2://---	ADD SOME BITS
			m_States = (m_States & LowMask) | ((m_States & ~LowMask) << Num);
		break;

	default:ASSERT(0);
	}
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::ShiftPriorities(int Operation, int Priority)
{
	if (m_Priority >= Priority)
	{
		if (Operation > 0)
			m_Priority++;
		else if (m_Priority > 0)
			m_Priority--;
	}
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::ShiftMethods(int Operation, int Method)
{
	if (m_Method >= Method)
	{
		if (Operation > 0)
			m_Method++;
		else if (m_Method > 0)
			m_Method--;
	}
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::ShiftFlags(int Operation, int Flag)
{
	int Mask = ((1<<Flag) - 1);

	if (Operation > 0)
		m_Flags = (m_Flags & Mask) | (((m_Flags & ~Mask) << 1) & ~Mask);
	else
		m_Flags = (m_Flags & Mask) | (((m_Flags & ~Mask) >> 1) & ~Mask);
}


//////////////////////////////////////////////////////////////////////
int CGroupNode::GetGroupCount()
{
	int				localCount = 0;
	CAnimTreeNode	*pAnimTreeNode = GetFirstChild();

	while (pAnimTreeNode)
	{
		if (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_GROUP)
			localCount += 1;												// for this group.

		if ((pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_GROUP) 
			|| (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_ENUM)
			|| (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_PROJECT))
			localCount += ((CGroupNode*)pAnimTreeNode)->GetGroupCount();	// for the group's children.

		pAnimTreeNode = GetNextChild();
	}

	return localCount;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetGroupExportCount()
{
	int				localCount = 0;
	CAnimTreeNode	*pAnimTreeNode = GetFirstChild();

	while (pAnimTreeNode)
	{
		if ((pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_GROUP) && !pAnimTreeNode->GetIgnore())
			localCount += 1;												// for this group.

		if ((pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_GROUP) 
			|| (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_ENUM)
			|| (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_PROJECT))
			localCount += ((CGroupNode*)pAnimTreeNode)->GetGroupCount();	// for the group's children.

		pAnimTreeNode = GetNextChild();
	}

	return localCount;
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::Copy(CAnimTreeNode &AnimTreeNode)
{
	CAnimTreeNode	*pAnimTreeNode;
	CAnimTreeNode	*pNewAnimTreeNode;
	CGroupNode		*pGroupNode = (CGroupNode*)&AnimTreeNode;

	//---	Copy the base node.
	this->CopyBaseNode( AnimTreeNode );

	//---	Copy the Group fields.
	m_Comment.Format( "%s", pGroupNode->m_Comment );
	m_Name		= pGroupNode->m_Name;
	m_Method	= pGroupNode->m_Method;
	m_Flags		= pGroupNode->m_Flags;
	m_Priority	= pGroupNode->m_Priority;
	m_States	= pGroupNode->m_States;

	SetCutTable( pGroupNode->GetCutTable() );
	SetCallback( pGroupNode->GetCallback() );
	SetSelectCallback( pGroupNode->GetSelectCallback() );

	//---	Copy the children.
	pAnimTreeNode = AnimTreeNode.GetFirstChild();
	while (pAnimTreeNode)
	{
		//---	Make a copy of the AnimTreeNode.
		switch( pAnimTreeNode->m_Type )
		{
		case ANIMTREENODE_TYPE_PROJECT:
		case ANIMTREENODE_TYPE_ENUM:
		case ANIMTREENODE_TYPE_UNUSED:
		case ANIMTREENODE_TYPE_GROUP:
				pNewAnimTreeNode = (CAnimTreeNode*)new CGroupNode( AnimTreeNode.m_pDoc, this );
			break;

		case ANIMTREENODE_TYPE_ANIM:
				pNewAnimTreeNode = (CAnimTreeNode*)new CAnimNode( AnimTreeNode.m_pDoc, this );
			break;

		case ANIMTREENODE_TYPE_UNDEF:
		case ANIMTREENODE_TYPE_TRANS:
		default:
				ASSERT(0 && "Bad type.");
			break;
		}
		pNewAnimTreeNode->Copy( *pAnimTreeNode );
		this->AddChild( pNewAnimTreeNode );

		//---	Step to the next AnimTreeNode from our source.
		pAnimTreeNode = pAnimTreeNode->GetNextChild();
	}
}


//DEL void CGroupNode::InsertBefore(CAnimTreeNode *pNewChild, CAnimTreeNode *pNodeAfter)
//DEL {
//DEL 	POSITION		ListPos;
//DEL 	CAnimTreeNode	*pChild;
//DEL 	CAnimTreeNode	*pNextChild;
//DEL 
//DEL 	ListPos = m_ChildList.GetHeadPosition();
//DEL 	pChild = m_ChildList.GetAt( ListPos );
//DEL 
//DEL 	if (pChild == pNodeAfter)
//DEL 	{
//DEL 		//---	Insert at beginning.
//DEL 		m_ChildList.AddHead( pNewChild );
//DEL 		return;
//DEL 	}
//DEL 
//DEL 	while (ListPos)
//DEL 	{
//DEL 		m_ChildList.GetNext( ListPos );
//DEL 
//DEL 		pNextChild = m_ChildList.GetAt( ListPos );
//DEL 
//DEL 		if (pNextChild == pNodeAfter)
//DEL 		{
//DEL 			m_ChildList.InsertBefore( ListPos, pNewChild );
//DEL 			return;
//DEL 		}
//DEL 	}
//DEL }


//////////////////////////////////////////////////////////////////////
void CGroupNode::SwitchCutTablePointer( void )
{
	int i;
	CLocoAnimTable* pTable;

	//---	if this animation has no cut table, exit now
	if (m_pCutTable != NULL)
	{
		//---	look for this cut table in the newly modified tables
		for( i=0; i<m_pDoc->m_LocomotionTables.GetCount(); i++ )
		{
			pTable = m_pDoc->m_LocomotionTables.GetTable( i );

			//---	if this is the table which used to be pointed to by this animation, relink it now.
			if (pTable->m_pOldSelf == m_pCutTable)
			{
				m_pCutTable = pTable;
				break;
			}
		}

		//---	if it was not found, set the cut table to NULL
		if (i==m_pDoc->m_LocomotionTables.GetCount())
			m_pCutTable = NULL;
	}

	PushChildStep();

	CAnimTreeNode* pChild = GetFirstChild();
	while (pChild)
	{
		pChild->SwitchCutTablePointer();
		pChild = GetNextChild();
	}

	PopChildStep();
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::CanHaveChildren()
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
CAnimTreeNode * CGroupNode::GetPrevChild(CAnimTreeNode *pNodeAfter)
{
	//---	If a node was specified then find it in the list and return the following child.
	if (pNodeAfter)
		m_ChildPos = m_ChildList.Find( pNodeAfter );

	//---	Otherwise assume the m_ChildPos points to the node we want the following child from.
	if (m_ChildPos)
	{
		m_ChildList.GetPrev( m_ChildPos );

		if (m_ChildPos)
			return m_ChildList.GetAt( m_ChildPos );;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetNumAnims()
{
	POSITION				ListPos;
	int						nAnims = 0;
	static CAnimTreeNode	*pAnimTreeNode;

	ListPos = m_ChildList.GetHeadPosition();
	while (ListPos)
	{
		pAnimTreeNode = m_ChildList.GetNext( ListPos );
		nAnims += pAnimTreeNode->GetNumAnims();
	}

	return nAnims;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetNumExportedAnims()
{
	POSITION				ListPos;
	int						nAnims = 0;
	static CAnimTreeNode	*pAnimTreeNode;

	//---	if this guy is ignored, nothing below him
	//		is exported.
	if( GetIgnore() )
		return 0;

	ListPos = m_ChildList.GetHeadPosition();
	while (ListPos)
	{
		pAnimTreeNode = m_ChildList.GetNext( ListPos );
		nAnims += pAnimTreeNode->GetNumExportedAnims();
	}

	return nAnims;
}

//////////////////////////////////////////////////////////////////////
CAnimTreeNode * CGroupNode::Clone()
{
	CGroupNode	*pGroupNode = new CGroupNode( m_pDoc, m_pParent );

	pGroupNode->Copy( *this );
	
	return pGroupNode;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetTotalNumDependancies( bool Recount, bool CountDisabledOnes )
{
	POSITION				ListPos;
	int						Count = 0;
	static CAnimTreeNode	*pAnimTreeNode;

	ListPos = m_ChildList.GetHeadPosition();
	while (ListPos)
	{
		pAnimTreeNode = m_ChildList.GetNext( ListPos );
		Count += pAnimTreeNode->GetTotalNumDependancies( Recount, CountDisabledOnes );
	}

	return Count;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetNumAnimsWithDependancies( bool Recount, bool CountDisabledOnes )
{
	POSITION				ListPos;
	int						nAnims = 0;
	static CAnimTreeNode	*pAnimTreeNode;

	ListPos = m_ChildList.GetHeadPosition();
	while (ListPos)
	{
		pAnimTreeNode = m_ChildList.GetNext( ListPos );
		nAnims += pAnimTreeNode->GetNumAnimsWithDependancies( Recount, CountDisabledOnes );
	}

	return nAnims;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetTotalNumDependanciesWithLink( bool Recount, bool CountDisabledOnes )
{
	POSITION				ListPos;
	int						Count = 0;
	static CAnimTreeNode	*pAnimTreeNode;

	ListPos = m_ChildList.GetHeadPosition();
	while (ListPos)
	{
		pAnimTreeNode = m_ChildList.GetNext( ListPos );
		Count += pAnimTreeNode->GetTotalNumDependanciesWithLink( Recount, CountDisabledOnes );
	}

	return Count;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetNumAnimsWithDependanciesWithLink( bool Recount, bool CountDisabledOnes )
{
	POSITION				ListPos;
	int						nAnims = 0;
	static CAnimTreeNode	*pAnimTreeNode;

	ListPos = m_ChildList.GetHeadPosition();
	while (ListPos)
	{
		pAnimTreeNode = m_ChildList.GetNext( ListPos );
		nAnims += pAnimTreeNode->GetNumAnimsWithDependanciesWithLink( Recount, CountDisabledOnes );
	}

	return nAnims;
}

//////////////////////////////////////////////////////////////////////
int CGroupNode::GetNumAnimsWithExtras()
{
	POSITION				ListPos;
	int						nAnims = 0;
	static CAnimTreeNode	*pAnimTreeNode;

	ListPos = m_ChildList.GetHeadPosition();
	while (ListPos)
	{
		pAnimTreeNode = m_ChildList.GetNext( ListPos );
		nAnims += pAnimTreeNode->GetNumAnimsWithExtras();
	}

	return nAnims;
}

//////////////////////////////////////////////////////////////////////
void CGroupNode::AddToListBox(CListBox *pListBox, int nFlags)
{
	static CString	ListName;
	CAnimTreeNode	*pAnimTreeNode;

	//---	Add this name to the list.
	if ((this->m_Type == ANIMTREENODE_TYPE_GROUP) && (nFlags & LISTBOX_FLAG_INCLUDEGROUPS))
	{
		ListName.Format( "[%s]", m_Name );

		//---	Add this animation to the list box.
		pListBox->AddString( ListName );
	}

	//---	Loop through children allowing them to add themselves to the list box.
	pAnimTreeNode = (CAnimTreeNode*)this->GetFirstChild();
	while (pAnimTreeNode)
	{
		pAnimTreeNode->AddToListBox( pListBox, nFlags );
		pAnimTreeNode = (CAnimTreeNode*)this->GetNextChild();
	}
}

//////////////////////////////////////////////////////////////////////
CString& CGroupNode::GetVPString(int *pDisplayOrder)
{
	int Option, i;
	CString str;
	int count, setting;
//	int setting2;
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
			break;

		case VP_OPTION_METHOD:
			str.Format( "%d", m_Method );
			m_VPString += "Method:" + str + m_pDoc->m_Methods.GetMethod( m_Method );
			break;

		case VP_OPTION_FLAGS:
			if (!m_pDoc)
				break;

			pFlagList = &m_pDoc->m_FlagList;
			count = pFlagList->GetNumFlags();
			
			if (count == 0)
				break;

			m_VPString += "Flags:";

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
			break;

		case VP_OPTION_STATE:
			if (!m_pDoc)
				break;

			pStateClassList = &m_pDoc->m_StateClassList;
			count = pStateClassList->GetCount();

			if (count == 0)
				break;

			m_VPString += "States:";
				
			for( i=0; i<count; i++ )
			{
				pStateClass = pStateClassList->GetStateClass( i );
				setting = pStateClass->GetSetting( m_States );
//				setting2 = pStateClass->GetSetting( m_EndStates );
				m_VPString += pStateClass->GetState( setting )->GetName();
//				if (setting != setting2)
//					m_VPString += "->" + pStateClass->GetState( setting2 )->GetName();
				if (i != (count-1))
					m_VPString += "|";
			}

			break;

		case VP_OPTION_PRIORITY:
			if (!m_pDoc)
				break;

			str.Format( "%d", m_Priority );
			m_VPString += "Priority:" + str + m_pDoc->m_Priorities.GetPriority( m_Priority );
			break;

		case VP_OPTION_COMMENT:
			m_VPString += "'" + m_Comment + "'";
			break;
		}
		m_VPString += " ";
	}

	m_VPString.TrimRight();

	return m_VPString;
}

//////////////////////////////////////////////////////////////////////////////
//
//	FindAnimTreeNodeByName
//
//		Gets an aniamtion tree node given the name of the node.
//
//		NOTE: it uses recursion to taverse the given node's children
//
CAnimTreeNode* CGroupNode::FindTreeNodeByName(CString &Name)
{
	CAnimTreeNode* pNode;

	//---	do the regular check
	if (CAnimTreeNode::FindTreeNodeByName( Name ) != NULL)
		return this;

	//---	check the children
	POSITION pos = m_ChildList.GetHeadPosition();
	while( pos != NULL )
	{
		pNode = m_ChildList.GetNext( pos )->FindTreeNodeByName( Name );
		if ( pNode != NULL ) return pNode;
	}

	//---	nothing found, return
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
int CGroupNode::GetState()
{
	int StateFlags=0;
	CAnimTreeNode	*pAnimTreeNode;
	
	PushChildStep();
	pAnimTreeNode = GetFirstChild();

	while (pAnimTreeNode)
	{
		StateFlags |= pAnimTreeNode->GetState();
		pAnimTreeNode = GetNextChild();
	}

	PopChildStep();

	return (StateFlags & STATE_ERROR_BIT) ? STATE_ERROR_BIT : 0;
}

////////////////////////////////////////////////////////////////////////////
void CGroupNode::SetCallback( CString Callback )
{
	CString OldCallback = m_Callback;

	if (m_Callback == Callback)
		return;

	SetChildrenDefaultCallback( OldCallback, Callback );
	m_Callback = Callback;
}

////////////////////////////////////////////////////////////////////////////
void CGroupNode::SetChildrenDefaultCallback( CString& Callback, CString& NewCallback )
{
	CAnimTreeNode *pChild;
	int i;

	//---	if the current setting is not the default, return
	if (m_Callback != Callback)
		return;

	//---	change the setting to the new default
	m_Callback = NewCallback;

	//---	change the children also
	for (i=0; i<m_ChildList.GetCount(); i++)
	{
		pChild = m_ChildList.GetAt( m_ChildList.FindIndex(i) );

		if ((pChild->m_Type == ANIMTREENODE_TYPE_ANIM) && ((CAnimNode*) pChild)->GetCallback() == Callback)
			((CAnimNode*) pChild)->SetCallback( NewCallback );
		else if (pChild->m_Type == ANIMTREENODE_TYPE_GROUP)
			((CGroupNode*) pChild)->SetChildrenDefaultCallback( Callback, NewCallback );
	}
}

////////////////////////////////////////////////////////////////////////////
void CGroupNode::SetSelectCallback( CString Callback )
{
	CString OldCallback = m_SelectCallback;

	if (m_SelectCallback == Callback)
		return;

// TURNED OFF SetChildrenDefaultSelectCallback because it is not the kind of thing which should be propogated down.
//	I also removed the <def> from being displayed in front of the callbacks in the dialog
//	SetChildrenDefaultSelectCallback( OldCallback, Callback );

	m_SelectCallback = Callback;
}

////////////////////////////////////////////////////////////////////////////
void CGroupNode::SetChildrenDefaultSelectCallback( CString& Callback, CString& NewCallback )
{
	CAnimTreeNode *pChild;
	int i;

	//---	if the current setting is not the default, return
	if (m_SelectCallback != Callback)
		return;

	//---	change the setting to the new default
	m_SelectCallback = NewCallback;

	//---	change the children also
	for (i=0; i<m_ChildList.GetCount(); i++)
	{
		pChild = m_ChildList.GetAt( m_ChildList.FindIndex(i) );

		if (pChild->m_Type == ANIMTREENODE_TYPE_GROUP)
			((CGroupNode*) pChild)->SetChildrenDefaultSelectCallback( Callback, NewCallback );
	}
}

////////////////////////////////////////////////////////////////////////////
void CGroupNode::SetCutTable( class CLocoAnimTable* pCutTable )
{
	if (m_pCutTable == pCutTable)
		return;

	SetChildrenDefaultCutTable( m_pCutTable, pCutTable );

	m_pCutTable = pCutTable;
}

////////////////////////////////////////////////////////////////////////////
void CGroupNode::SetChildrenDefaultCutTable( class CLocoAnimTable* pCutTable, CLocoAnimTable* pNewCutTable )
{
	CAnimTreeNode *pChild;
	int i;

	//---	if the current setting is not the default, return
	if (m_pCutTable != pCutTable)
		return;

	//---	change the setting to the new default
	m_pCutTable = pNewCutTable;

	//---	change the children also
	for (i=0; i<m_ChildList.GetCount(); i++)
	{
		pChild = m_ChildList.GetAt( m_ChildList.FindIndex(i) );

		if (pChild->m_Type == ANIMTREENODE_TYPE_GROUP)
			((CGroupNode*) pChild)->SetChildrenDefaultCutTable( pCutTable, pNewCutTable );
		else if (((CAnimNode*) pChild)->GetCutTable() == pCutTable)
			((CAnimNode*) pChild)->SetCutTable( pNewCutTable );
	}
}

////////////////////////////////////////////////////////////////////////////
void CGroupNode::FillCallbackStringArray( CStringArray& rStringArray )
{
	CAnimTreeNode* pAnimTreeNode;
	int i;

	if (m_Type == ANIMTREENODE_TYPE_TRANS)
		return;

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

	PushChildStep();
	pAnimTreeNode = GetFirstChild();
	while( pAnimTreeNode )
	{
		if (pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_ANIM)
			((CAnimNode*) pAnimTreeNode)->FillCallbackStringArray( rStringArray );
		else if ((pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_GROUP) ||
				(pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_PROJECT) ||
				(pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_ENUM))
			((CGroupNode*) pAnimTreeNode)->FillCallbackStringArray( rStringArray );

		pAnimTreeNode = GetNextChild();
	}
	PopChildStep();
}

////////////////////////////////////////////////////////////////////////////
void CGroupNode::FillSelectCallbackStringArray( CStringArray& rStringArray )
{
	CAnimTreeNode* pAnimTreeNode;
	int i;

	if (m_Type == ANIMTREENODE_TYPE_TRANS)
		return;

	if (m_SelectCallback != "")
	{
		//---	look for the given callback
		for (i=0; i<rStringArray.GetSize(); i++)
			if (m_SelectCallback == rStringArray[i])
				break;

		//---	if it was not found, add it.
		if (i == rStringArray.GetSize())
			i = rStringArray.Add( m_SelectCallback );
	}

	PushChildStep();
	pAnimTreeNode = GetFirstChild();
	while( pAnimTreeNode )
	{
		if ((pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_GROUP) ||
			(pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_PROJECT) ||
			(pAnimTreeNode->m_Type == ANIMTREENODE_TYPE_ENUM))
			((CGroupNode*) pAnimTreeNode)->FillSelectCallbackStringArray( rStringArray );

		pAnimTreeNode = GetNextChild();
	}
	PopChildStep();
}

////////////////////////////////////////////////////////////////////////////
int	CGroupNode::GetNumDependancies( bool Recount, bool CountDisabledOnes, bool CountInheritedOnes )
{
	int Count = GetNumPrerequisites( Recount, CountDisabledOnes, CountInheritedOnes );
	Count += GetNumTransitions( Recount, CountDisabledOnes, CountInheritedOnes );

	return Count;
}

////////////////////////////////////////////////////////////////////////////
int CGroupNode::GetNumParentDependancies( bool Recount, bool CountDisabledOnes )
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
CAnimTrans* CGroupNode::GetFirstDependancy( void )
{
	CAnimTreeNode* pNode=GetFirstChild();

	while( pNode && ( pNode->m_Type != ANIMTREENODE_TYPE_TRANS ))
		pNode = GetNextChild();

	return (CAnimTrans*)pNode;
}


////////////////////////////////////////////////////////////////////////////
CAnimTrans* CGroupNode::GetNextDependancy( void )
{
	CAnimTreeNode* pNode=GetNextChild();

	while( pNode && ( pNode->m_Type != ANIMTREENODE_TYPE_TRANS ))
		pNode = GetNextChild();

	return (CAnimTrans*)pNode;
}


////////////////////////////////////////////////////////////////////////////
void CGroupNode::ClearDependancies( void )
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

		pNode = GetFirstDependancy();
	}
}

////////////////////////////////////////////////////////////////////////////
void CGroupNode::GetDependancies( CAnimTransList& rAnimTransList )
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
void CGroupNode::SetDependancies( CAnimTransList& rAnimTransList )
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
int	CGroupNode::GetNumPrerequisites( bool Recount, bool CountDisabledOnes, bool CountInheritedOnes )
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

		pDep = (CAnimTrans*) GetNextDependancy();
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
int CGroupNode::GetNumParentPrerequisites( bool Recount, bool CountDisabledOnes )
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
CAnimTrans* CGroupNode::GetFirstPrerequisite( void )
{
	CAnimTrans* pNode=GetFirstDependancy();

	while( pNode && !((CAnimTrans*)pNode)->IsPrerequisite() )
		pNode = GetNextDependancy();

	return pNode;
}

////////////////////////////////////////////////////////////////////////////
CAnimTrans* CGroupNode::GetNextPrerequisite( void )
{
	CAnimTrans* pNode=GetNextDependancy();

	while( pNode && !((CAnimTrans*)pNode)->IsPrerequisite() )
		pNode = GetNextDependancy();

	return pNode;
}

////////////////////////////////////////////////////////////////////////////
void CGroupNode::GetPrerequisites( CAnimTransList& rAnimTransList, CString TagString )
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
void CGroupNode::GetParentPrerequisites( CAnimTransList& rAnimTransList, CString TagString )
{
	CGroupNode* pParent = GetParent();

	while( pParent )
	{
		pParent->GetPrerequisites( rAnimTransList, TagString );
		pParent = pParent->GetParent();
	}

}

////////////////////////////////////////////////////////////////////////////
int	CGroupNode::GetNumTransitions( bool Recount, bool CountDisabledOnes, bool CountInheritedOnes )
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
int CGroupNode::GetNumParentTransitions( bool Recount, bool CountDisabledOnes )
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
CAnimTrans* CGroupNode::GetFirstTransition( void )
{
	CAnimTrans* pNode=GetFirstDependancy();

	while( pNode && !((CAnimTrans*)pNode)->IsTransition() )
		pNode = GetNextDependancy();

	return pNode;
}

////////////////////////////////////////////////////////////////////////////
CAnimTrans* CGroupNode::GetNextTransition( void )
{
	CAnimTrans* pNode=GetNextDependancy();

	while( pNode && !((CAnimTrans*)pNode)->IsTransition() )
		pNode = GetNextDependancy();

	return pNode;
}

////////////////////////////////////////////////////////////////////////////
void CGroupNode::GetTransitions( CAnimTransList& rAnimTransList, CString TagString )
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

////////////////////////////////////////////////////////////////////////////
void CGroupNode::GetParentTransitions( CAnimTransList& rAnimTransList, CString TagString )
{
	CGroupNode* pParent = GetParent();

	while( pParent )
	{
		pParent->GetTransitions( rAnimTransList, TagString );
		pParent = pParent->GetParent();
	}
}

int CGroupNode::CountAllDependancies( void )
{
	int Count;
	CAnimTreeNode* pNode;

	//---	count this guy's dependancies
	Count = GetNumDependancies( TRUE, FALSE, FALSE );

	//---	count the dependancies of any child which is a group or anim
	POSITION pos = m_ChildList.GetHeadPosition();
	while( pos )
	{
		pNode = m_ChildList.GetNext( pos );
		if( pNode->m_Type == ANIMTREENODE_TYPE_ANIM )
			Count += ((CAnimNode*)pNode)->CountAllDependancies();
		else if( pNode->m_Type == ANIMTREENODE_TYPE_GROUP )
			Count += ((CGroupNode*)pNode)->CountAllDependancies();
		else if( pNode->m_Type == ANIMTREENODE_TYPE_ENUM )
			Count += ((CGroupNode*)pNode)->CountAllDependancies();
	}

	return Count;
}

