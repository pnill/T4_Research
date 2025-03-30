// LocomotionTables.h: interface for the CLocomotionTables class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCOMOTIONTABLES_H__B6D2C4C6_2CC6_11D4_9397_0050DA2C723D__INCLUDED_)
#define AFX_LOCOMOTIONTABLES_H__B6D2C4C6_2CC6_11D4_9397_0050DA2C723D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
//						I N C L U D E S
//////////////////////////////////////////////////////////////////////

#include <afxtempl.h>
#include "LocoAnimTable.h"

//////////////////////////////////////////////////////////////////////
//						S T R U C T S
//////////////////////////////////////////////////////////////////////
/*
typedef struct
{
	s32					AnimID;			// exported animation ID
	f32					MinNSR;			// Minimum NSR for this Anim
	f32					MaxNSR;			// Maximum NSR for this Anim
} t_SLOCTBL_AnimNSR;

typedef struct
{
	f32					Angle;			// angle out of 4096 which contains this arc
	s32					NumAnimNSRs;	// number of animation/motioncount groups in this arc
	t_SLOCTBL_AnimNSR	*pAnimNSR;		// pointer to the animation/motioncount groups for this arc
} t_SLOCTBL_ArcData;

typedef struct
{
	f32					PhaseShift;		// rotates the table around the actor
	s16					NumArcs;		// number of arcs in this locomotion animation table
	s16					CenterAnim;		// index of the center animation
	f32					CenterNSR;		// NSR for center position (motion count less than this is directionless)
										// The corresponding animation index is assumed to be zero.
	f32					MaxNSR;			// Maximum NSR allowed with this table
	t_SLOCTBL_ArcData	*pArcData;		// pointer to the data for each arc in the locomotion table
//	s16					*pAnimSet;		// pointer to the data for the animation sets for this locomotion table
} t_SLOCTBL_Table;

typedef struct
{
	u32					NumAnimTables;	// number of locomotion animation tables exported for this animation set
	t_SLOCTBL_Table		*pAnimTable;	// pointer to the data for the locomotion animation tables
} t_SLOCTBL_AnimTables;
*/

//////////////////////////////////////////////////////////////////////
//						T Y P E D E F S
//////////////////////////////////////////////////////////////////////

typedef CTypedPtrList<CObList, CLocoAnimTable*>		CLocoTableList;


//////////////////////////////////////////////////////////////////////
//					T H E   C L A S S
//////////////////////////////////////////////////////////////////////

class CLocomotionTables : CObject
{
public:
	CLocomotionTables();
	virtual ~CLocomotionTables();

	void RecordPointers( void );
	void FillCallbackStringArray( CStringArray& rStringArray );
	CLocomotionTables& operator =(CLocomotionTables &Other);
	void AddTable( CLocoAnimTable* pTable );
	void Delete( CString Name );
	void Delete( int Index );
	int GetCount() { return m_TableList.GetCount(); };
	CLocoAnimTable* GetTable( int Index );
	CLocoAnimTable* GetTable( CString Name );
	void Serialize( CArchive& ar );
	void Export( FILE* pFile, class CAnimTblEdDoc* pDoc, bool DefinesOnly );
	void EmptyList(void);

protected:
	CLocoTableList	m_TableList;
};

#endif // !defined(AFX_LOCOMOTIONTABLES_H__B6D2C4C6_2CC6_11D4_9397_0050DA2C723D__INCLUDED_)
