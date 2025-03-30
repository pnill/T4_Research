#if !defined(AFX_ANIMATIONEDITDLG_H__6C7DE8A2_7A57_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_ANIMATIONEDITDLG_H__6C7DE8A2_7A57_11D2_9248_00105A29F8F3__INCLUDED_

#include "NodeSearchMap.h"
#include "IDList.h"	// Added by ClassView
#include "AnimTblEdDoc.h"	// Added by ClassView
#include "DataType.h"	// Added by ClassView
//#include "AnimNode.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnimationEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditDlg dialog

class CAnimationEditDlg : public CDialog
{
public:
	// Construction
	CAnimationEditDlg(CAnimNode* pAnim, CWnd* pParent = NULL);   // standard constructor
	CAnimationEditDlg(CGroupNode* pGroup, CWnd* pParent = NULL);   // standard constructor

	static CString GetLastName() {return s_LastName;}
	static CString GetLastLeftID() {return s_LastRightID;}
	static CString GetLastRightID() {return s_LastLeftID;}
	static void SetLastName( CString Name ) { s_LastName = Name;}
	static void SetLastLeftID( CString RightID ) { s_LastRightID = RightID;}
	static void SetLastRightID( CString LeftID ) { s_LastLeftID = LeftID;}

	// Search Criteria
	BOOL			m_bSearchMode;	// if this dialog is to set search criteria set this boolean to TRUE
	CNodeSearchMap	m_NodeSearchMap;

protected:
	void InitDialogForSearch();
	void InitDialogForAnimNode( void );
	void InitDialogForGroupNode( void );
	void OnOKForSearchMode(void);
	void OnOKForAnimNode(void);
	void OnOKForGroupNode(void);
	void InitAnimIDs(bool SearchMode = false);
	void InitCallbackDialog(bool SearchMode = false);
	void InitSelectCallbackDialog(bool SearchMode = false);
	void InitCutTableDialog(bool SearchMode = false);
	void InitFrameRateDialog(bool SearchMode = false);
	void InitLinkAnimDialog(bool SearchMode = false);
	void InitTransitionDialog(bool SearchMode = false);
	void InitStatesDialog(bool SearchMode = false);
	void InitFlagsDialog(bool SearchMode = false);
	void UpdateFrameRateDialog(void);
	void UpdateLinkAnimDialog(void);
	void UpdatePriorityDialog(bool SearchMode = false);
	void UpdateMethodDialog(bool SearchMode = false);
	void UpdateTransitionDialog(void);
	void UpdateStatesDialog(void);
	void UpdateFlagsDialog(void);
	void FillDependancyList();

// Dialog Data
	//{{AFX_DATA(CAnimationEditDlg)
	enum { IDD = IDD_ANIMATIONEDIT };
	CListBox	m_DependancyList;
	CString	m_AnimName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimationEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnimationEditDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDblclkListDependancies();
	afx_msg void OnButtonNewdependancy();
	afx_msg void OnButtonEditdependancy();
	afx_msg void OnButtonDeletedependancy();
	afx_msg void OnButtonDefaultprereq();
	afx_msg void OnSelchangeListDependancies();
	afx_msg void OnSelcancelListDependancies();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFlag0();
	afx_msg void OnFlag1();
	afx_msg void OnFlag2();
	afx_msg void OnFlag3();
	afx_msg void OnFlag4();
	afx_msg void OnFlag5();
	afx_msg void OnFlag6();
	afx_msg void OnFlag7();
	afx_msg void OnSelchangeStatecombo0();
	afx_msg void OnSelchangeStatecombo1();
	afx_msg void OnSelchangeStatecombo2();
	afx_msg void OnSelchangeStatecombo3();
	afx_msg void OnTransitionanim();
	afx_msg void OnSelchangeTranscombo0();
	afx_msg void OnSelchangeTranscombo1();
	afx_msg void OnSelchangeTranscombo2();
	afx_msg void OnSelchangeTranscombo3();
	afx_msg void OnLinktoanim();
	afx_msg void OnKillfocusFramerate();
	afx_msg void OnSelchangeLinkratelist();
	afx_msg void OnButtonLeftchangeid();
	afx_msg void OnButtonRightchangeid();
	afx_msg void OnButtonSwap();
	afx_msg void OnKillfocusCallback();
	afx_msg void OnSetfocusCallback();
	afx_msg void OnSelchangeCallback();
	afx_msg void OnSelchangeSelectcallback();
	afx_msg void OnSetfocusSelectcallback();
	afx_msg void OnKillfocusSelectcallback();
	afx_msg void OnChangeboth();
	afx_msg void OnChangeLinkenterframe();
	afx_msg void OnLinkmethod();
	afx_msg void OnLinkpriority();
	afx_msg void OnLinkrate();
	afx_msg void OnLinktransitions();
	afx_msg void OnFlag8();
	afx_msg void OnFlag9();
	afx_msg void OnFlag10();
	afx_msg void OnFlag11();
	afx_msg void OnLinkflags();
	afx_msg void OnLinkstates();
	afx_msg void OnKillfocusLinkenterframe();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void OnTrans(int Index);
	void OnState(int Index);
	void OnFlag(int Index);

	CAnimNode*		m_pAnimNode;	// if this dialog is to edit an animation, set this pointer
	CGroupNode*		m_pGroupNode;	// if this dialog is to edit a group, set this pointer
	CAnimTblEdDoc*	m_pDoc;

	CString			m_Name;

	CString			m_Callback;
	CString			m_ParentCallback;
	CString			m_SelectCallback;
	CString			m_ParentSelectCallback;
	CLocoAnimTable*	m_pCutTable;
	CLocoAnimTable*	m_pParentCutTable;
	CString			m_FrameRate;
	CIDList*		m_pAnimList;
	CIDList*		m_pIDList;
	CAnimTransList	m_AnimTransList;

	//---	animation link data
	BOOL			m_bLinkCheck;
	BOOL			m_bLinkMethod;
	BOOL			m_bLinkPriority;
	BOOL			m_bLinkFrameRate;
	BOOL			m_bLinkFlags;
	BOOL			m_bLinkStates;
	BOOL			m_bLinkTransitions;
	int				m_LinkEnterFrame;
	CString			m_LinkAnim;
	CAnimNode*		m_pLinkAnim;

	int				m_StatesScrollIndex;
	int				m_FlagsScrollIndex;
	int				m_Method;
	int				m_Flags;
	int				m_Priority;
	int				m_States;
	int				m_GroupMethod;
	int				m_GroupPriority;
	int				m_GroupStates;
	int				m_GroupFlags;
	int				m_EndStates;

	static CString	s_LastName;
	static CString	s_LastLeftID;
	static CString	s_LastRightID;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMATIONEDITDLG_H__6C7DE8A2_7A57_11D2_9248_00105A29F8F3__INCLUDED_)
