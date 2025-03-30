#if !defined(AFX_TRANSITIONVIEWER_H__EEFCD6EE_2567_44EA_AB56_032EC558A0A4__INCLUDED_)
#define AFX_TRANSITIONVIEWER_H__EEFCD6EE_2567_44EA_AB56_032EC558A0A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TransitionViewer.h : header file
//

#include <afxtempl.h>
#include "x_types.hpp"
#include "x_math.hpp"

class CMotionEditView;
class CMotion;

#define	MAX_NUM_TRANSITIONS		16

//---	
class CAnimTransData : public CObject
{
public:
	CMotion*	pMotion;
	s32			BlendFrames;
	s32			StartFrame;
	s32			EndFrame;
	matrix4		Orient;
};

typedef CTypedPtrList<CObList, CAnimTransData*>  CAnimTransList;

/////////////////////////////////////////////////////////////////////////////
// CTransitionViewer dialog

class CTransitionViewer : public CDialog
{
// Construction
public:
	CTransitionViewer(CWnd* pParent = NULL);	// standard constructor
	virtual ~CTransitionViewer();				// standard destructor

	//---	member variables
	CMotionEditView*	m_pView;
	CAnimTransList		m_AnimTransList;
	xbool				m_bSettingSelected;
	xbool				m_bCycle;				// if TRUE, the system will blend from the last motion to the first
	s32					m_CurrentPlayIndex;
	CImageList			m_ImageList;
	s32					m_TotalFrames;


	s32		AddTransition		( CMotion* pMotion );
	s32		FindTransition		( CMotion* pMotion );
	void	RemoveTransition	( s32 Index );

	void	UpdateTransList		( xbool bResetSelected=FALSE );
	void	SetSelected			( s32* Selected, s32 count );

	void	OrderUp				( s32 i );
	void	OrderDown			( s32 i );

	void	SetBlendFrames		( s32 i, s32 v );
	void	SetStartFrame		( s32 i, s32 v );
	void	SetEndFrame			( s32 i, s32 v );

	void	GetCurrentPlayMotion( matrix4& rBaseMat, CMotion*& rNewMotion, f32& rBlendFrames, f32& rStartFrame, f32& rEndFrame, CMotion*& rLastMotion, f32& rLastFrame );
	void	GetNextPlayMotion	( matrix4& rBaseMat, CMotion*& rNewMotion, f32& rBlendFrames, f32& rStartFrame, f32& rEndFrame, CMotion*& rLastMotion, f32& rLastFrame );
	void	GetScrollPlayMotion	( s32 Frame, s32& rFrame, matrix4& rBaseMat, CMotion*& rNewMotion, f32& rBlendFrames, f32& rStartFrame, f32& rEndFrame, CMotion*& rLastMotion, f32& rLastFrame );

	void	GetScrollProperties	( s32& ScrollBase, s32& ScrollRange );

// Dialog Data
	//{{AFX_DATA(CTransitionViewer)
	enum { IDD = IDD_TRANSITIONVIEWER };
	CButton	m_EndFrameToEnd;
	CSpinButtonCtrl	m_EndFrameSpin;
	CEdit	m_EndFrame;
	CEdit	m_StartFrame;
	CSpinButtonCtrl	m_StartFrameSpin;
	CSpinButtonCtrl	m_BlendFramesSpin;
	CButton	m_DOWN;
	CButton	m_UP;
	CListCtrl	m_TransitionList;
	CEdit	m_BlendFrames;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTransitionViewer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTransitionViewer)
	afx_msg void OnOrderup();
	afx_msg void OnOrderdown();
	virtual void OnOK();
	afx_msg void OnDblclkTransitionlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickTransitionlist(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnBegindragTransitionlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedTransitionlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownTransitionlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditTransitionlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditTransitionlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeBlendframes();
	afx_msg void OnChangeEndframe();
	afx_msg void OnChangeStartframe();
	afx_msg void OnEndframetoend();
	afx_msg void OnPlaypause();
	afx_msg void OnReset();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnCycle();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSITIONVIEWER_H__EEFCD6EE_2567_44EA_AB56_032EC558A0A4__INCLUDED_)
