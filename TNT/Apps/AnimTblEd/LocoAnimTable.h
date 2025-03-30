// LocoAnimTable.h: interface for the CLocoAnimTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCOANIMTABLE_H__E132CA60_DBED_11D2_8D9C_00A0CC23CE22__INCLUDED_)
#define AFX_LOCOANIMTABLE_H__E132CA60_DBED_11D2_8D9C_00A0CC23CE22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



//////////////////////////////////////////////////////////////////////
//						I N C L U D E S
//////////////////////////////////////////////////////////////////////

#include <afxtempl.h>
#include "LocoArcData.h"

//////////////////////////////////////////////////////////////////////
//				D E F I N E S   /   E N U M E R A T I O N S
//////////////////////////////////////////////////////////////////////

enum
{
	TABLETYPE_LOCOMOTION,
	TABLETYPE_CUT,
	TABLETYPE_TURN,
	TABLETYPE_UNDEFINED
};

enum
{
	INDEXANGLESTART_FIXED,
	INDEXANGLESTART_CURFACEDIR,
	INDEXANGLESTART_CURMOVEDIR,
	INDEXANGLESTART_DESTFACEDIR,
	INDEXANGLESTART_DESTMOVEDIR,
};

enum
{
	INDEXANGLEEND_CURFACEDIR,
	INDEXANGLEEND_CURMOVEDIR,
	INDEXANGLEEND_DESTFACEDIR,
	INDEXANGLEEND_DESTMOVEDIR,
};

enum
{
	ORIENTATION_POINTFOREWARD,
	ORIENTATION_SIDEFOREWARD,
	ORIENTATION_PHASESHIFT
};

#define	TABLEFLAG_FIXED		(1<<0)
#define	TABLEFLAG_FACEDIR	(1<<1)
#define	TABLEFLAG_LIMITNSR	(1<<2)


//////////////////////////////////////////////////////////////////////
//						T Y P E D E F S
//////////////////////////////////////////////////////////////////////

typedef CTypedPtrList<CObList, CLocoArcData*>	CLocoArcDataList;

//////////////////////////////////////////////////////////////////////
//				T H E   C L A S S
//////////////////////////////////////////////////////////////////////

class CLocoAnimTable : public CLATObject  
{
public:
	CLocoAnimTable();
	virtual ~CLocoAnimTable();

	void FillCallbackStringArray( CStringArray& rStringArray );
	int ChangeAnim( CString Anim, CString NewAnim );
	int RemoveAnim( CString Anim );
	void InsertAnim( int Index, CString Anim );
	int GetType(){ return LATOBJECT_TYPE_TABLE; }
	CString GetExportName( void );
	CString GetExportDefine( void );
	CLocoAnimTable& operator =(CLocoAnimTable &Other);
	int operator ==(CLocoAnimTable &Other);
	int operator !=(CLocoAnimTable &Other){*this == Other ? FALSE : TRUE;}
	void EmptyList(void);
	void Serialize( CArchive& ar );
	void Export( FILE* pFile, class CAnimTblEdDoc* pDoc, int Phase );

	CString				m_Name;				// name of the table
	int					m_Type;				// the type of the table 0 - locomotion, 1 - run cut, 2 - stand cut
	int					m_Flags;			// table is fixed to the ground.  Is not reletive to the player
	int					m_Orientation;		// 0 - Point foreward, 1 - Side foreward, 2 - Phase shift as described in m_PhaseShift memeber variable
	float				m_PhaseShift;		// phase shift of the LocoAnimTable
	float				m_MaxNSR;			// maximum NSR which works for this locomotion table
	float				m_OverlapNSR;		// sets the overlap for the zones
	float				m_OverlapArc;		// sets the overlap for the arcs
	float				m_CenterNSR;		// center NSR for the case where the person is not moving
	int					m_CenterAnim;		// center Animation as index to anim table
	CString				m_Callback;			// callback function (it will be empty if there is none)
	CString				m_ReorientAnim;		// reorientation animation as string
	CStringArray		m_AnimationList;	// list of animations in the table			
	CLocoArcDataList	m_ArcDataList;		// arcs
	int					m_bDoNotExport;		// T/F should this table be exported
	int					m_IndexAngleStart;	// which value to use for the arc start to index the table
	int					m_IndexAngleEnd;	// which value to use for the arc end to index the table

	CLocoAnimTable*		m_pOldSelf;			// used for re-linking after editing locomotion tables.
};

#endif // !defined(AFX_LOCOANIMTABLE_H__E132CA60_DBED_11D2_8D9C_00A0CC23CE22__INCLUDED_)
