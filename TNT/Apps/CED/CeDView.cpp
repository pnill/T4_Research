// CeDView.cpp : implementation of the CCeDView class
//

#include "stdafx.h"
#include "CeD.h"

#include "CeDDoc.h"
#include "CeDView.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "RenderContextD3D.h"
#include "RenderContextCDC.h"


/////////////////////////////////////////////////////////////////////////////
// CCeDView

IMPLEMENT_DYNCREATE(CCeDView, CView)

BEGIN_MESSAGE_MAP(CCeDView, CView)
	//{{AFX_MSG_MAP(CCeDView)
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOVE()
	ON_WM_ACTIVATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCeDView construction/destruction

CCeDView::CCeDView()
{
	// TODO: add construction code here

	m_MouseLast = CPoint(0,0) ;

	m_pBackDC = NULL ;
	m_pBackBitmap = NULL ;
	m_pBackOldBitmap = NULL ;

	m_ViewState = 0 ;

	m_MouseOverItem = false ;

	m_pD3DRC = NULL;
}

CCeDView::~CCeDView()
{
	//---	Delete BackBuffer
	if( m_pBackDC != NULL )
	{
		m_pBackDC->SelectObject( m_pBackOldBitmap ) ;
		delete m_pBackBitmap ;
		delete m_pBackDC ;
		m_pBackBitmap = NULL ;
		m_pBackDC = NULL ;
	}

	if( m_pD3DRC )
		delete m_pD3DRC;
}

BOOL CCeDView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

void CCeDView::CreateBackBuffer( CDC *pDC )
{
	//---	Get Current Client Size
	CRect	r ;
	GetClientRect( &r ) ;

	//---	If Window was resized then destroy DC and Bitmap
	if( (m_pBackDC != NULL) && (m_BackRect != r) )
	{
		m_pBackDC->SelectObject( m_pBackOldBitmap ) ;
		delete m_pBackBitmap ;
		delete m_pBackDC ;
		m_pBackBitmap = NULL ;
		m_pBackDC = NULL ;
	}

	//---	If No BackDC then Create a BackDC
	if( m_pBackDC == NULL )
	{
		bool	freeDC = false ;
		bool	freeBitmap = false ;

		m_BackRect = r ;
		m_pBackDC = new CDC ;
		if( m_pBackDC )
		{
			if( m_pBackDC->CreateCompatibleDC( pDC ) )
			{
				m_pBackBitmap = new CBitmap ;
				if( m_pBackBitmap )
				{
					if( m_pBackBitmap->CreateCompatibleBitmap( pDC, r.Width(), r.Height() ) )
					{
						m_pBackOldBitmap = m_pBackDC->SelectObject( m_pBackBitmap ) ;
					}
					else
					{
						freeBitmap = TRUE ;
						freeDC = TRUE ;
					}
				}
				else
				{
					freeDC = TRUE ;
				}
			}
			else
			{
				freeDC = TRUE ;
			}
		}

		if( freeBitmap )
		{
			delete m_pBackBitmap ;
			m_pBackBitmap = NULL ;
		}

		if( freeDC )
		{
			delete m_pBackDC ;
			m_pBackDC = NULL ;
		}
	}
}

void CCeDView::UpdateMenuItems( CMenu *pMenu )
{

}

void CCeDView::GetCursorRay( vector3  *pRayBase, vector3  *pRay, CPoint p )
{
	f32			cx, cy ;
	vector3 	vCameraSrc, vScreenSrc ;
	vector3 	vCameraDst, vScreenDst ;
	matrix4		m ;
	vector3 	Target ;
    vector3     d(0,0,0);


	//---	Get Camera Details
	m_Camera.GetProjectCenter( &cx, &cy, NULL, NULL ) ;

	//---	Get Camera Point and Screen Corner Points
	vCameraSrc.Set( 0, 0, 0 ) ;
	vScreenSrc.Set( cx - p.x, cy - p.y, m_Camera.m_D ) ;

	//---	Build Camera to World Matrix
	m_Camera.GetTarget( &Target ) ;
	m.Identity();
	d.Z = -m_Camera.GetDistance();
	m.Translate( d );
	m.RotateX( -m_Camera.GetElevation() );
	m.RotateY( -m_Camera.GetRotation() );
	m.Translate( Target );

	//---	Transform Camera Point & Screen Point
	m.Transform( &vCameraDst, &vCameraSrc, 1 );
	m.Transform( &vScreenDst, &vScreenSrc, 1 );

	//---	Get Unit Vector from the 2 Points we have
	*pRayBase = vCameraDst;
	*pRay = vScreenDst - vCameraDst;
	pRay->Normalize();

}

//==========================================================================
void CCeDView::DrawView( CRenderContext *pRC )
{
}

//==========================================================================
void CCeDView::ReleaseDevices( void )
{
	m_pD3DRC->ReleaseDevices();
}

//==========================================================================
void CCeDView::AquireDevices( void )
{
	m_pD3DRC->AquireDevices();
}

//==========================================================================
void CCeDView::OnDraw(CDC* pDC)
{
	if( m_bD3DActive )
	{
		if( m_pD3DRC == NULL )
			m_pD3DRC = new CRenderContextD3D( this, &m_Camera );

		Prepare3DEnvironment( TRUE, BACKGROUND_COLOR );

//		CRect r ;
//		GetClientRect( &r );
//		m_pD3DRC->RENDER_FillSolidRect( r.left, r.top, r.Width(), r.Height(), BACKGROUND_COLOR ) ;
		m_pD3DRC->Reset();
		DrawView( (CRenderContext*) m_pD3DRC );

		Present3DEnvironment();
	}
	else
	{
		//---	Ensure we have a Back Buffer
		CreateBackBuffer( pDC ) ;

		//---	Render
		if( m_pBackDC )
		{
			CRect r ;
			GetClientRect( &r ) ;
			m_pBackDC->FillSolidRect( &r, BACKGROUND_COLOR ) ;
			CRenderContextCDC rc( m_pBackDC, &m_Camera ) ;
			DrawView( (CRenderContext*)&rc ) ;
			pDC->BitBlt( 0, 0, r.Width(), r.Height(), m_pBackDC, 0, 0, SRCCOPY ) ;
		}
		else
		{
			CRect r ;
			GetClientRect( &r ) ;
			pDC->FillSolidRect( &r, BACKGROUND_COLOR ) ;
			CRenderContextCDC rc( pDC, &m_Camera ) ;
			DrawView( (CRenderContext*)&rc ) ;
		}
	}
/*
	m_nTimerMessages = 0 ;
	if( m_Playing )
		::PostMessage( (HWND)GetSafeHwnd(), WM_TIMER, 0, 0 ) ;
*/
}

/////////////////////////////////////////////////////////////////////////////
// CCeDView printing

BOOL CCeDView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCeDView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCeDView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CCeDView diagnostics

#ifdef _DEBUG
void CCeDView::AssertValid() const
{
	CView::AssertValid();
}

void CCeDView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCeDDoc* CCeDView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCeDDoc)));
	return (CCeDDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCeDView message handlers

void CCeDView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	
	CView::OnLButtonDblClk(nFlags, point);
}

void CCeDView::OnInitialUpdate() 
{

	CD3DView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	
}

void CCeDView::OnMove(int x, int y) 
{
	CD3DView::OnMove(x, y);
	
	// TODO: Add your message handler code here
	
}

void CCeDView::OnSize(UINT nType, int cx, int cy) 
{
	CD3DView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	CRect r ;
	GetClientRect( &r ) ;
	m_Camera.SetProjectWindow( (f32)r.left, (f32)r.top, (f32)r.Width(), (f32)r.Height() );

	//---	for some reason this function is called once with zero set for its 
	if( r.Width() && r.Height() )
	{
		radian hfov = DEG_TO_RAD((f32)m_Camera.GetFOV());
		// SetProjection( m_Camera.m_NZ, m_Camera.m_FZ, hfov, r.Width(), r.Height() );
		SetProjection( m_Camera.ComputeProjMatrixD3D() ) ;
	}
}


void CCeDView::OnDestroy() 
{
	CD3DView::OnDestroy();
	
}

bool CCeDView::GetD3DPermission( void )
{
	return GetDocument()->m_bD3DEnable ? TRUE : FALSE;
}
