// LocoAnimNSR.h: interface for the CLocoAnimNSR class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCOANIMNSR_H__B6D2C4CA_2CC6_11D4_9397_0050DA2C723D__INCLUDED_)
#define AFX_LOCOANIMNSR_H__B6D2C4CA_2CC6_11D4_9397_0050DA2C723D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum
{
	LATOBJECT_TYPE_TABLE,
	LATOBJECT_TYPE_ARC,
	LATOBJECT_TYPE_ANIMNSR,
};

class CLATObject : public CObject
{
public:
	virtual int	GetType(void)=0;
};

class CLocoAnimNSR : public CLATObject  
{
public:
	CLocoAnimNSR();
	virtual ~CLocoAnimNSR();

	void RemoveAnim( int Index );
	void InsertAnim( int Index );
	int GetType(){ return LATOBJECT_TYPE_ANIMNSR; }
	CString GetExportName( CString& rParentName, int Index ){CString n; n.Format("%s_AnimNSR_%d", rParentName, Index ); return n; }
	CLocoAnimNSR& operator =(CLocoAnimNSR &Other);
	int operator ==(CLocoAnimNSR &Other);
	int operator !=(CLocoAnimNSR &Other){ return *this == Other ? FALSE : TRUE;}
	void Serialize( CArchive& ar );
	void Export( FILE* pFile, class CAnimTblEdDoc* pDoc, CString& rParentName, int Index, CStringArray& rAnimationList, float& rLastNSR, float OverlapNSR );

	float	m_NSR;				// maximum motion count for the animation associated with this anim/mc group
	int		m_Anim;				// Animation for this range of NSR (this is an index into the animation list for this table)
};


#endif // !defined(AFX_LOCOANIMNSR_H__B6D2C4CA_2CC6_11D4_9397_0050DA2C723D__INCLUDED_)
