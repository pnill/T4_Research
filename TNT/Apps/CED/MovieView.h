#if !defined(AFX_MOVIEVIEW_H__A86D09A2_FDD3_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_MOVIEVIEW_H__A86D09A2_FDD3_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MovieView.h : header file
//

#include "CeDView.h"
#include "qraw.h"


class CMovie ;
class CMovieFrame ;
class CActor ;
class CDlgLayer ;
class CDlgActorList	;
class CDlgKey ;
class CDlgShotList ;
class CKey ;
class CMovieBar ;
class CDlgMoviePreview ;

/////////////////////////////////////////////////////////////////////////////
// CMovieView view

enum
{
	MOVS_SELECT,

	MOVS_CAMERA_MOVE,
	MOVS_CAMERA_ROTATE,
	MOVS_CAMERA_ZOOM,

	MOVS_DRAG_KEY,
	MOVS_DRAG_KEYVECTOR,

	MOVS_END
} ;

#define	MVF_VIEWCHARACTERS		0x01
#define	MVF_VIEWSCENERY			0x02
#define	MVF_VIEWCAMERAS			0x04
#define MVF_VIEWCAMERATARGETS	0x08
#define	MVF_VIEWLIGHTS			0x10

class CMovieView : public CCeDView
{
protected:
	CMovieView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMovieView)

// Attributes
public:
	CMovie			    *m_pMovie ;

	CMovieBar		    *m_pMovieBar ;

	CDlgLayer			*m_pDlgLayer ;
	CDlgActorList	    *m_pDlgActorList ;
	CDlgKey			    *m_pDlgKey ;
	CDlgShotList	    *m_pDlgShotList ;
    CDlgMoviePreview    *m_pDlgMoviePreview ;

	bool			    m_MoveX ;							// Move on X Axis
	bool			    m_MoveY ;							// Move on Y Axis
	bool			    m_MoveZ ;							// Move on Z Axis

	bool			    m_MouseOverKey ;					// Mouse is over a Key
	bool			    m_MouseOverActor ;					// Mouse is over an Actor
	vector3 		    m_DragPlane ;						// Drag Plane Coefficients
	f32				    m_DragPlaned ;						// Drag Plane Coefficients
	int				    m_DragOldViewState ;				// Old View State for Dragging
	CKey			    *m_pDragKey ;						// Key to Drag
	vector3 		    m_DragOffset ;						// Offset to Item Dragging

	int				    m_ActorType ;						// Current Actor Type

	s32					m_ViewFlags ;
	s32					m_ViewLayer ;						// which layer to view

    bool                m_Playing ;
	bool		        m_TimeAccuratePlay ;
	bool		        m_StartTiming ;
	s32			        m_LastTime ;
	f32			        m_LastFrac ;
    s32                 m_TimerEvent ;
    s32                 m_nTimerMessages ;

	qraw			    m_QRaw ;

// Operations
public:

	CMovieFrame *GetFrameWnd		( ) ;
	virtual void UpdateMenuItems	( CMenu *pMenu ) ;

	CActor		*GetCurActor		( ) ;

	void	DrawSkeleton			( CRenderContext *pRC, matrix4& rTransform, COLORREF C, CCharacter *pCharacter );
	void	DrawView				( CRenderContext *pRC ) ;

	void	UpdateCursorState		( ) ;
	void	SetViewState			( int ViewState ) ;
	void	SetViewLayer			( s32 ViewLayer ) ;
	void	ModifyViewFlags			( s32 ClearFlags, s32 SetFlags );

	void	SetDragPlane			( vector3  *pVector ) ;				// Set Drag Plane
	bool	GetCursorOnDragPlane	( vector3  *pCursor, CPoint p,
									  f32 y ) ;							// Get 3d Cursor Position

	void	MovieLayerDeleted		( int Layer );
	void	MovieLayerSelected		( void );
	void	MovieLayerChanged		( void );

	CActor	*ActorNew				( ) ;								// New Actor
	void	ActorDel				( CActor *pActor,
									  CActor *pNewCurrentActor ) ;		// Delete Actor
	void	ActorSetCurrent			( CActor *pActor ) ;				// Set Current Actor
	void	ActorUpdateList			( ) ;								// Update Actor List
	void	ActorLookAt				( ) ;								// Make Camera Look at Actor
	void	ActorSelectAll			( bool Select ) ;					// Select All Actors

	CKey	*KeyAdd					( ) ;								// Add Key
	void	KeyDelete				( CKey *pKey,
									  CKey *pNewCurrentKey ) ;			// Delete Key

	void	KeyUpdateList			( ) ;								// Update Key List
	void	KeySetCurrent			( CKey *pKey ) ;					// Set Current Key
	CKey	*KeyGetCurrent			( ) ;								// Get Current Key
	void	KeyMove					( f32 dx, f32 dy, f32 dz ) ;		// Modify the current Key Position
	void	KeyMoveV				( f32 dx, f32 dy, f32 dz ) ;		// Modify the current Key Vector
	void	KeyMoveR				( f32 dx, f32 dy, f32 dz ) ;		// Modify the current Key Rotation
	void	KeyUpdateDetails		( CKey *pKey ) ;					// Update Details
	void	KeySetX					( f32 x ) ;
	void	KeySetY					( f32 y ) ;
	void	KeySetZ					( f32 z ) ;
	void	KeySetVX				( f32 x ) ;
	void	KeySetVY				( f32 y ) ;
	void	KeySetVZ				( f32 z ) ;
	void	KeySetRX				( f32 x ) ;
	void	KeySetRY				( f32 y ) ;
	void	KeySetRZ				( f32 z ) ;
	void	KeySetFrame				( int Frame ) ;
	void	KeyLookAt				( ) ;
	void	KeySelectAll			( bool Select ) ;
	void	KeySetScale				( f32 scale );						// Set Scale
	f32		KeyGetScale				( ) ;								// Get Scale

	void	KeyUpdateMotions		( ) ;								// Update List of Motions
	void	KeySetMotion			( CMotion *pMotion ) ;
	void	KeySetStartFrame		( int StartFrame ) ;
	void	KeySetFrameRate			( float Rate ) ;
	void	KeySetBlendFrames		( float Frames ) ;
	void	KeySetMirror			( bool Mirror ) ;

	void	KeySetAutoLinkDir		( bool setting );

	void	ShotListInitialize		( ) ;
	CShot	*ShotAdd				( ) ;
	void	ShotDelete				( CShot *pShot ) ;
	void	ShotSetFrame			( CShot *pShot, int Frame ) ;

	int		SetFrame				( int nFrame ) ;					// Set Current Frame
	int		GetFrame				( ) ;								// Get Frame
	int		SetNumFrames			( int NumFrames ) ;					// Set Number of Frames
	int		GetNumFrames			( ) ;								// Get Number of Frames
	void	UpdateMovieBar			( ) ;								// Update the Movie Bar

	void	SetMoveState			( int ID ) ;						// Set Movestate & Toolbar

	void	SetActorType			( int ActorType ) ;					// Set Actor type
	int		GetActorType			( ) ;								// Set Actor type

	CKey	*PickGetKey				( int *pSubMode, CPoint p,
									  bool VectorPriority ) ;			// Check picking keys

    void    SetUseShotList          ( bool State ) ;                    // Set Use Shot List Flag
    bool    GetUseShotList          ( ) ;                               // Get Use Shot List Flag

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMovieView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMovieView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CMovieView)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnStateSelect();
	afx_msg void OnStateRotate();
	afx_msg void OnStateZoom();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnAxisxz();
	afx_msg void OnAxisx();
	afx_msg void OnAxisy();
	afx_msg void OnAxisz();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnStateMove();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMovieInsertframes();
	afx_msg void OnMovieDeletefames();
	afx_msg void OnActorInsertframes();
	afx_msg void OnActorDeleteframes();
	afx_msg void OnActorCopy();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMovieExportGAMECUBE();
	afx_msg void OnMovieExportPC();
	afx_msg void OnMovieExportXBOX();
	afx_msg void OnMovieExportPS2();
	afx_msg void OnMovieMerge();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMoviesExportmovieasascii();
	afx_msg void OnMovieviewLayerselect();
	afx_msg void OnUpdateMovieviewLayerselect(CCmdUI* pCmdUI);
	afx_msg void OnMovieExportproperties();
	afx_msg void OnMovieExport();
	afx_msg void OnUpdateEnableD3d(CCmdUI* pCmdUI);
	afx_msg void OnEnableD3d();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVIEVIEW_H__A86D09A2_FDD3_11D1_9507_00207811EE70__INCLUDED_)
