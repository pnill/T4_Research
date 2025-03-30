// CeDView.cpp : implementation of the CCeDView class
//

#include "stdafx.h"
#include "CeD.h"

#include "CeDDoc.h"
#include "CeDView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "RenderContext.h"

/////////////////////////////////////////////////////////////////////////////
// CCeDView

IMPLEMENT_DYNCREATE(CCeDView, CView)

BEGIN_MESSAGE_MAP(CCeDView, CView)
	//{{AFX_MSG_MAP(CCeDView)
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
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

void CCeDView::GetCursorRay( vector3d *pRayBase, vector3d *pRay, CPoint p )
{
	f32			cx, cy ;
	vector3d	vCameraSrc, vScreenSrc ;
	vector3d	vCameraDst, vScreenDst ;
	matrix4		m ;
	vector3d	Target ;
    vector3d    d = {0};


	//---	Get Camera Details
	m_Camera.GetProjectCenter( &cx, &cy, NULL, NULL ) ;

	//---	Get Camera Point and Screen Corner Points
	V3_Set( &vCameraSrc, 0, 0, 0 ) ;
	V3_Set( &vScreenSrc, cx - p.x, cy - p.y, m_Camera.m_DX ) ;

	//---	Build Camera to World Matrix
	m_Camera.GetTarget( &Target ) ;
	M4_Identity( &m ) ;
    d.Z = -m_Camera.GetDistance();
	M4_TranslateOn( &m, &d ) ;
	M4_RotateXOn( &m, -m_Camera.GetElevation() ) ;
	M4_RotateYOn( &m, -m_Camera.GetRotation() ) ;
	M4_TranslateOn( &m, &Target ) ;

	//---	Transform Camera Point & Screen Point
	M4_TransformVerts( &m, &vCameraDst, &vCameraSrc, 1 ) ;
	M4_TransformVerts( &m, &vScreenDst, &vScreenSrc, 1 ) ;

	//---	Get Unit Vector from the 2 Points we have
	V3_Copy( pRayBase, &vCameraDst ) ;
	V3_Sub( pRay, &vScreenDst, &vCameraDst ) ;
	V3_Normalize( pRay ) ;
}


void CCeDView::OnDraw(CDC* pDC)
{
	CCeDDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
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

void CCeDView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	//---	Call Document Activation Function
//	if( bActivate )
//		GetDocument()->ActivateDocument() ;

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CCeDView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	CRect r ;
	GetClientRect( &r ) ;
	m_Camera.m_DX = (r.Width()/2) / x_tan(DEG_TO_RAD(m_Camera.m_FOV)/2) ;
	m_Camera.m_DY = (r.Width()/2) / x_tan(DEG_TO_RAD(m_Camera.m_FOV)/2) ;
}

void CCeDView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	
	
	CView::OnLButtonDblClk(nFlags, point);
}
