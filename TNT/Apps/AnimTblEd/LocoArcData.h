// LocoArcData.h: interface for the CLocoArcData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCOARCDATA_H__B6D2C4C9_2CC6_11D4_9397_0050DA2C723D__INCLUDED_)
#define AFX_LOCOARCDATA_H__B6D2C4C9_2CC6_11D4_9397_0050DA2C723D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
//						I N C L U D E S
//////////////////////////////////////////////////////////////////////

#include <afxtempl.h>
#include "LocoAnimNSR.h"

//////////////////////////////////////////////////////////////////////
//						T Y P E D E F S
//////////////////////////////////////////////////////////////////////

typedef CTypedPtrList<CObList, CLocoAnimNSR*>	CLocoAnimNSRList;


//////////////////////////////////////////////////////////////////////
//						T H E   C L A S S
//////////////////////////////////////////////////////////////////////

class CLocoArcData : public CLATObject  
{
public:
	CLocoArcData();
	virtual ~CLocoArcData();

	void RemoveAnim( int Index );
	void InsertAnim( int Index );
	int GetType(){ return LATOBJECT_TYPE_ARC; }
	CString GetExportName( CString& rParentName, int Index ){CString n; n.Format("%s_Arc_%d", rParentName, Index ); return n; }
	void EmptyList(void);
	CLocoArcData& operator =(CLocoArcData &Other);
	int operator ==(CLocoArcData &Other);
	int operator !=(CLocoArcData &Other){return *this == Other ? FALSE : TRUE;}
	void Serialize( CArchive& ar );
	void Export( FILE* pFile, class CAnimTblEdDoc* pDoc, int Phase, CString& rParentName, int Index, CStringArray& rAnimationList, float CenterNSR, float OverlapNSR );

	float				m_Angle;			// angle in radiams
	CLocoAnimNSRList	m_AnimNSRList;		// pointer to the animation/nsr groups for this arc
};

#endif // !defined(AFX_LOCOARCDATA_H__B6D2C4C9_2CC6_11D4_9397_0050DA2C723D__INCLUDED_)
