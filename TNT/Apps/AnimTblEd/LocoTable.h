// LocoTable.h: interface for the CLocoTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCOTABLE_H__B6D2C4C8_2CC6_11D4_9397_0050DA2C723D__INCLUDED_)
#define AFX_LOCOTABLE_H__B6D2C4C8_2CC6_11D4_9397_0050DA2C723D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//////////////////////////////////////////////////////////////////////
//						I N C L U D E S
//////////////////////////////////////////////////////////////////////

#include <afxtempl.h>
#include "LocoArcData.h"

//////////////////////////////////////////////////////////////////////
//						T Y P E D E F S
//////////////////////////////////////////////////////////////////////

typedef CTypedPtrList<CObList, CLocoArcData*>	CLocoArcDataList;


//////////////////////////////////////////////////////////////////////
//						T H E   C L A S S
//////////////////////////////////////////////////////////////////////

class CLocoAnimTable : public CObject  
{
public:
	CLocoAnimTable();
	virtual ~CLocoAnimTable();

	void Serialize( CArchive& ar );

	CString				m_Anim;				// Animation for this range of NSR
	float				m_CenterNSR;		// NSR (normalized speed ratio) for center position (motion count less than this is directionless)
	float				m_MaxNSR;			// Maximum motion count allowed with this table
	float				m_PhaseShift;		// Used to rotate the arcs around the actor
	CLocoArcDataList	m_ArcDataList;		// pointer to the data for each arc in the locomotion table
};

#endif // !defined(AFX_LOCOTABLE_H__B6D2C4C8_2CC6_11D4_9397_0050DA2C723D__INCLUDED_)
