#if !defined(AFX_LOCOMOTIONTABLESDLG_H__758206A1_2BCE_11D4_9397_0050DA2C723D__INCLUDED_)
#define AFX_LOCOMOTIONTABLESDLG_H__758206A1_2BCE_11D4_9397_0050DA2C723D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LocomotionTablesDlg.h : header file
//

#include "LocomotionTables.h"

class CSelect : public CObject
{
public:
	virtual void Inc( float Steps )=0;
	virtual void Set( float Value )=0;
	virtual float Get( void )=0;
	virtual void SetDisplay( CDialog* pDialog )=0;

	CLATObject*	m_pSelected;

	CSelect(void){};
	~CSelect(void){};
};

class CSelectValue : public CSelect
{
public:
	CSelectValue( CLATObject* pSelected, float* pMin, float* pMax, float ValueDisplayConversion );
	~CSelectValue(){};
	void Inc( float Steps );
	void Set( float Value );
	float Get( void );
	void SetDisplay( CDialog* pDialog );

	float*	m_pMin;
	float*	m_pMax;
	float	m_ValueDisplayConversion;
};

class CSelectAnim : public CSelect
{
public:
	CSelectAnim( CLATObject* pSelected, int Min, int Max );
	~CSelectAnim(void){};
	void Inc( float Steps );
	void Set( float Value );
	float Get( void );
	void SetDisplay( CDialog* pDialog );

	int*	m_pAnim;
	int		m_Min;
	int		m_Max;
};


//////////////////////////////////////////////////////////////////////
//						T Y P E D E F S
//////////////////////////////////////////////////////////////////////

typedef CTypedPtrList<CObList, CSelect*>	CSelectedItems;


/////////////////////////////////////////////////////////////////////////////
// CLocomotionTablesDlg dialog

class CLocomotionTablesDlg : public CDialog
{
// Construction
public:
	//---	Draw Functions
	void DrawAnimBrushKey( CPaintDC& dc );
	void DrawArcLine( CPaintDC& dc, float MinRadius, float MaxRadius,
									float SinA1, float CosA1,
									float SinA2, float CosA2,
									float FillSin, float FillCos,
									bool LineSelected );
	void DrawArc( CPaintDC& dc, CLocoArcData* pLastArc, CLocoArcData* pArc, int Arc );
	bool GetArcRadius( float NSR, float MaxNSR, int& rArcRadius );
	void SetBrushForAnim( CPaintDC& dc, int Anim, bool Selected );

	//---	Dailog Setting functions
	void SetUsedAnimations( void );
	void SetOrientation( void );
	void EnableDisplay( int EnableBool );
	void SetDisplayToTable( int Table);
	bool DisplayRadian( void ) {return m_Radian.GetCheck()?TRUE:FALSE;}
	void ScrollUsedAnims( void );
	void SetVScroll( UINT nPos, CScrollBar* pScrollBar );
	void OnOK( void );
	void SetIndexAngles( void );

	//---	Selection functions
	void ClearAllSelectedItems( void );
	void AddSelectedItem( CSelect* pItem );
	void IncSelectedValue( float Value );
	void SetSelectedValue( float Value );
	void IncSelected( int Steps );
	void SetSelected( int OriginArc, int Arc, int Zone, bool Line, bool Center );
	void SetSelectedButtonDisplay( bool Display );
	void SetSelectedArcZoneDisplay( int Arc, int Zone );
	void SetSelectedValueDisplay( bool Display, float Value, bool Radian );
	void SetSelectedAnimationDisplay( bool Display, int AnimationIndex );
	void SetSelectedCallback( void );

	//---	Mouse functions (other than message handlers which are lower in the file)
	bool MouseButtonPressed( int WhichButton, bool DoubleClick, bool Shift, bool Ctrl, CPoint Point );

	//---	tool functions
	void CopyArc( int Arc );
	void CopyZone( int Arc, int Zone );
	void DeleteArc( int Arc );
	void DeleteZone( int Arc, int Zone );

	//---	redraw functions
	void RedrawAnimKey(void);
	void RedrawTable(void);

	//---	constructor destructor
	CLocomotionTablesDlg(CAnimTblEdDoc* pDoc, CWnd* pParent = NULL);   // standard constructor
	~CLocomotionTablesDlg(void);

// Dialog Data
	//{{AFX_DATA(CLocomotionTablesDlg)
	enum { IDD = IDD_LOCOMOTIONTALES };
	CComboBox	m_TableTypeListBoxCtrl;
	CButton	m_Radian;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocomotionTablesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

public:
	CLocomotionTables m_LocomotionTables;

protected:
	//---	locomotion table objects
	CLocoAnimTable*	m_pCurLocoTable;
	CLocoArcData*	m_pCurArc;
	CLocoAnimNSR*	m_pCurZone;

	//---	bounding box
	CRect			m_TableRect;

	//---	selected info
	int				m_SelectedArc;
	int				m_SelectedZone;
	bool			m_LineSelected;
	bool			m_CenterSelected;
	CSelectedItems	m_SelectedItems;

	//---	mouse control data
	CPoint			m_MouseLastPos;
	int				m_MouseSensitivity;
	int				m_CursorRadiusOffset;

	//---	drawing objects
	CBrush			m_BlackBrush;
	CBrush			m_BackgroundBrush;
	CBrush			m_ShadeBrush;
	CBrush			m_SelectBrush;
	CBrush*			m_pAnimBrushes;
	CPoint			m_ArcCenter;
	int				m_ArcRadius;
	CPen			m_BasicPen;
	CPen			m_SelectPen;
	CPen			m_BrushKeyPen;
	CPen			m_DottedLinePen;

	//---	constant variables needed for selection bounds
	float			m_Zero;
	float			m_2Pi;
	float			m_One;

	CRect			m_UsedAnimsList_NoScroll;
	CRect			m_UsedAnimsList_Scroll;
	CRect			m_UsedAnimsList_ScrollBar;
	CRect			m_AnimBrushKeyRect;

	bool			m_DrawTableOnly;
	bool			m_DrawAnimKeyOnly;

	class CAnimTblEdDoc* m_pDoc;

	// Generated message map functions
	//{{AFX_MSG(CLocomotionTablesDlg)
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnCancelMode();
	afx_msg void OnSelchangeLocomotiontables();
	afx_msg void OnSelcancelLocomotiontables();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnPhase();
	afx_msg void OnSideforeward();
	afx_msg void OnPointforeward();
	afx_msg void OnCopy();
	afx_msg void OnDeltaposPhaseshiftspin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusLocomotiontables();
	afx_msg void OnKillfocusLocomotiontables();
	afx_msg void OnKillfocusPhaseshift();
	afx_msg void OnKillfocusName();
	afx_msg void OnChangeName();
	afx_msg void OnKillfocusMaxnsr();
	afx_msg void OnDeltaposMaxnsrspin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnAddanim();
	afx_msg void OnRemoveanim();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSelchangeUsedanims();
	afx_msg void OnPlus();
	afx_msg void OnMinus();
	afx_msg void OnEqual();
	afx_msg void OnChangeValue();
	afx_msg void OnDeltaposValuespin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRadian();
	afx_msg void OnChangeOverlap();
	afx_msg void OnKillfocusOverlap();
	afx_msg void OnDeltaposOverlapspin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusValue();
	afx_msg void OnChangeanim();
	afx_msg void OnReltoactor();
	afx_msg void OnReorientanimchange();
	afx_msg void OnDblclkUsedanims();
	afx_msg void OnSelchangeTabletype();
	afx_msg void OnFixedonfield();
	afx_msg void OnFacedir();
	afx_msg void OnKillfocusCallback();
	afx_msg void OnChangeOverlaparc();
	afx_msg void OnKillfocusOverlaparc();
	afx_msg void OnDeltaposOverlaparcspin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLimitednsr();
	afx_msg void OnAnimScansourcecode();
	afx_msg void OnGroupScansourcecode();
	afx_msg void OnDonotexport();
	afx_msg void OnSelchangeStartangle();
	afx_msg void OnSelchangeEndangle();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCOMOTIONTABLESDLG_H__758206A1_2BCE_11D4_9397_0050DA2C723D__INCLUDED_)
