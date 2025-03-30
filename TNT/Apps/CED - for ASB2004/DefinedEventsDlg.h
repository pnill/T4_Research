#if !defined(AFX_DEFINEDEVENTSDLG_H__AA440F61_D029_11D3_88A5_00105A29F84C__INCLUDED_)
#define AFX_DEFINEDEVENTSDLG_H__AA440F61_D029_11D3_88A5_00105A29F84C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DefinedEventsDlg.h : header file
//

#include "MouseStatic.h"
#include "CeDView.h"
#include "DefinedEventListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CDefinedEventsDlg dialog

class CDefinedEventsDlg : public CDialog
{
// Construction
public:
	CDefinedEventsDlg(CWnd* pParent = NULL);   // standard constructor
	void SetDocument( CCeDDoc* pView );

// Dialog Data
	//{{AFX_DATA(CDefinedEventsDlg)
	enum { IDD = IDD_PROJECT_EVENTS };
	CStatic	m_StaticEditRadius;
	CButton	m_Attached;
	CButton	m_AttachedForcedRelativeToJoint;
	CStatic	m_StaticAttachedRotZ;
	CStatic	m_StaticAttachedRotY;
	CStatic	m_StaticAttachedRotX;
	CStatic	m_StaticAttachedDistance;
	CEdit	m_AttachedForcePosZ;
	CEdit	m_AttachedForcePosY;
	CEdit	m_AttachedForcePosX;
	CButton	m_AttachedForcePosZBut;
	CButton	m_AttachedForcePosYBut;
	CButton	m_AttachedForcePosXBut;
	CEdit	m_AttachedRotZ;
	CEdit	m_AttachedRotY;
	CEdit	m_AttachedRotX;
	CEdit	m_AttachedDistance;
	CButton	m_ButtonEventDelete;
	CButton	m_ButtonExportList;
	CButton	m_ButtonImportList;
	CButton	m_ButtonInsertEvent;
	CComboBox	m_ComboAttachedBone;
	CListBox	m_EventListCtrl;
	CEdit	m_EditEventName;
	CEdit	m_EditRadius;
	CBitmapButton	m_ButtonOK;
	CBitmapButton	m_ButtonCancel;
	//}}AFX_DATA
//	CDefinedEventListCtrl	m_EventListCtrl;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDefinedEventsDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateUI( void );
	BOOL OnToolTipNotify( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );
	BOOL ImportAttachedEventList(CString FileName);
	void AttachedImportLoadFileDialog();
	void ProjectImportLoadFileDialog();
	BOOL ExportAttachedEventList(CString FileName);
	void AttachedExportSaveFileDialog();
	void ProjectExportSaveFileDialog();
	void OnokProjectEventList();
	void OnokAttachedEventList();
	void ProjectEventDelete();
	void AttachedEventDelete();
	void OnButtonProjectInsertEvent();
	void OnButtonAttachedInsertEvent();
	CSkeleton* m_pSkeleton;
	void InitBoneCombo();
	void InitUI();
	void UpdateEventListCtrl();
	void InitActiveEventListCombo();
	void SetInitialActiveEventListSelection();
	BOOL m_bAttachedEventListAvailable;
	BOOL m_bAttachedEventListSelected;
	BOOL ExportProjectEventList( CString FileName );
	BOOL ImportProjectEventList( CString FileName );
	void UpdateProjectEventList();
	void UpdateAttachedEventListCtrl();
	void UpdateAttachedEventList();
	void InitEventListCtrl();
	void InitProjectEventListCtrl();
	void InitAttachedEventListCtrl();
	void UpdateProjectEventListCtrl();
	void ReInitDialog();
	void InitInsertEventUI();
	void InitAttachedUI();
	void InitProjectUI();
	void InitBoneUI( bool Active );
	void UpdateBoneUI( void );
	void SearchForAndChangeModifiedEvents( CEventList* pEventList, CCharacter* pCharacter );

	CToolTipCtrl m_ToolTipCtrl;
	float m_Radius;
	CMouseStatic m_StaticEventR;
	CCeDDoc* m_pDoc;					// Pointer to Document
	CEventList	m_ProjectEventList;		// Local copy of project event list used for editing.
										// The even data is read from the document on dialog initialization
										// and written back to the document after editing.
	CEventList	m_AttachedEventList;	// Local copy of character specific event list used for editing.
										// Events in this list are attached to bones of the character.
										// The even data is read from the document on dialog initialization
										// and written back to the document after editing.

	// Generated message map functions
	//{{AFX_MSG(CDefinedEventsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnMyok();
	afx_msg void OnKillfocusEditProjectEventDefaultRadius();
	afx_msg void OnButtonInsertEvent();
	afx_msg void OnButtonEventDelete();
	afx_msg void OnButtonProjectEventImportList();
	afx_msg void OnButtonProjectEventExportList();
	afx_msg void OnSelchangeComboSelectActiveEventList();
	afx_msg void OnChangeDefinedEventsDlgName();
	afx_msg void OnChangeDefinedEventsDlgRotx();
	afx_msg void OnChangeDefinedEventsDlgRoty();
	afx_msg void OnChangeDefinedEventsDlgRotz();
	afx_msg void OnChangeDefinedEventsDlgDistance();
	afx_msg void OnChangeDefinedEventsDlgForceposx();
	afx_msg void OnChangeDefinedEventsDlgForceposy();
	afx_msg void OnChangeDefinedEventsDlgForceposz();
	afx_msg void OnDefinedEventsDlgButtonForceposx();
	afx_msg void OnDefinedEventsDlgButtonForceposy();
	afx_msg void OnDefinedEventsDlgButtonForceposz();
	afx_msg void OnDefinedEventsDlgButtonRelativetojoint();
	afx_msg void OnButtonAttached();
	afx_msg void OnSelchangeComboSelectAttachedBone();
	afx_msg void OnSelchangeLinstProjectEvents();
	//}}AFX_MSG
	afx_msg LONG OnMouseStaticMove(UINT wParam, LONG lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFINEDEVENTSDLG_H__AA440F61_D029_11D3_88A5_00105A29F84C__INCLUDED_)
