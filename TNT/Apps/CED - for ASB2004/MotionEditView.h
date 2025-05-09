#if !defined(AFX_MOTIONEDITVIEW_H__B008EC42_DF44_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_MOTIONEDITVIEW_H__B008EC42_DF44_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MotionEditView.h : header file
//

#include "CeDView.h"
#include "MotionEditFrame.h"
#include "MouseStatic.h"

/////////////////////////////////////////////////////////////////////////////
// CMotionEditView view

enum {
	MEVS_SELECT,

	MEVS_CAMERA_PAN,
	MEVS_CAMERA_ROTATE,
	MEVS_CAMERA_ZOOM,

	MEVS_SET_FACEDIR,
	MEVS_SET_MOVEDIR,
	MEVS_SET_EVENT,

	MEVS_MOVEPLANEY,

	MEVS_END
} ;

class CMotionEditView : public CCeDView
{
protected:
	CMotionEditView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMotionEditView)

// Attributes
public:
	CCharacter	*m_pCharacter ;
	UINT		m_TimerEvent ;
	int			m_nTimerMessages ;

	bool		m_CameraControlEditsLocked ;
	int			m_CameraLockedBone ;
	bool		m_Playing ;

	bool		m_ViewAllFaceDirs ;
	bool		m_ViewAllMoveDirs ;
	bool		m_ViewMirroredSkeleton ;
	bool		m_ViewCompressedMotion ;
	bool		m_ViewPrimaryMotion ;
	bool		m_ViewMotionBlended ;

	bool		m_TimeAccuratePlay ;
	bool		m_StartTiming ;
	s32			m_LastTime ;
	f32			m_LastFrac ;

	CEvent		*m_pEventUnderMouse ;

// Operations
public:
	void UpdateEventProjectEvents();
	void AttachBoneCurrentEvent( INT iBone );
	CEventList* GetAttachedEvents();
	CSkeleton* GetSkeleton();
	CEvent* EventGetCurrent();

	CMotionEditFrame *GetFrame( ) ;

	virtual void UpdateMenuItems( CMenu *pMenu ) ;

	CMotion *GetCurMotion		( ) ;

	void DrawSkeleton			( CRenderContext *pRC, CCharacter *pCharacter ) ;
	void DrawView				( CRenderContext *pRC ) ;
	void UpdateCursorState		( ) ;
	bool GetPointOnPlane		( vector3d *pPoint, CPoint p, f32 yPlane ) ;
	void SetViewState			( int State ) ;

	void UpdateCharacterList	( ) ;
	void UpdateCameraControls	( ) ;
	void UpdateCameraBoneList	( ) ;
	void UpdateMotionControls	( ) ;
	void UpdateMotionList		( ) ;
	void MotionSetCurrent		( CMotion *pMotion ) ;
	void MotionAdd				( ) ;
	void MotionDelete			( CMotion *pMotion, CMotion *pNewCurrentMotion, int PromptUser = 1) ;
	void MotionModify			( CMotion *pMotion );
	void MotionSetLinkCharacter	( CCharacter *pCharacter ) ;
	void MotionSetLinkMotion	( CMotion *pLinkMotion ) ;

	void UpdateEventList		( ) ;
	void UpdateEventControls	( ) ;
	void EventSetCurrent		( CEvent *pEvent ) ;
	void EventAdd				( ) ;
	void EventDelete			( CEvent *pEvent, CEvent *pNewCurrentEvent ) ;
	void ModifyEventPosition	( f32 dx, f32 dy, f32 dz ) ;
	void ModifyEventRadius		( f32 dr ) ;

	void UpdateSkelPointList	() ;
	void UpdateSkelPointControls( ) ;
	void SkelPointSetCurrent	( CSkelPoint *pSkelPoint ) ;
	void SkelPointAdd			( ) ;
	void SkelPointDelete		( CSkelPoint *pSkelPoint, CSkelPoint *pNewCurrentSkelPoint ) ;
	void ModifySkelPointPosition( f32 dx, f32 dy, f32 dz ) ;
	void ModifySkelPointRadius	( f32 dr ) ;

	void SetCharacter			( CCharacter *pCharacter ) ;
	void SetCameraBone			( int iBone ) ;

	void SetTimeAccuratePlayback( int State ) ;

	CEvent *PickGetEvent		( CPoint p ) ;

	CEventList*	GetProjectEvents();
//	CEvent *PickGetEvent		( CPoint p ) ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMotionEditView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMotionEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	BOOL m_bInitialUpdate;
	//{{AFX_MSG(CMotionEditView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnStateSelect();
	afx_msg void OnStatePan();
	afx_msg void OnStateRotate();
	afx_msg void OnStateZoom();
	afx_msg void OnZoomAll();
	afx_msg void OnStateSelectmove();
	afx_msg void OnStateSelectrotate();
	afx_msg void OnAnimGotoStart();
	afx_msg void OnAnimPreviousFrame();
	afx_msg void OnAnimPlay();
	afx_msg void OnAnimNextFrame();
	afx_msg void OnAnimGotoEnd();
	afx_msg void OnStateMoveplaney();
	afx_msg void OnStateFacedir();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCharacterImportSkel();
	afx_msg void OnCharacterImportMotion();
	afx_msg void OnViewAllFacedirs();
	afx_msg void OnViewAllMovedirs();
	afx_msg void OnStateMovedir();
	afx_msg void OnViewMirroredSkeleton();
	afx_msg void OnViewCompressedMotion();
	afx_msg void OnViewPrimaryMotion();
	afx_msg void OnViewMotionBlended();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnCharacterExport();
	afx_msg void OnUpdateStateFacedir(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStateMovedir(CCmdUI* pCmdUI);
	afx_msg void OnCharacterLoadmotions();
	afx_msg void OnStateEvent();
	afx_msg void OnUpdateStateEvent(CCmdUI* pCmdUI);
	//}}AFX_MSG

	afx_msg void OnClkFrameStart( ) ;
	afx_msg void OnClkFrameEnd( ) ;

	afx_msg void OnFaceDirInterpolate( ) ;
	afx_msg void OnMoveDirInterpolate( ) ;

	afx_msg void OnUpdateMoveDirInterpolate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFaceDirInterpolate(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOTIONEDITVIEW_H__B008EC42_DF44_11D1_AEC0_00A024569FF3__INCLUDED_)
