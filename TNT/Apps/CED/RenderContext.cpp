// RenderContext.cpp: implementation of the CRenderContext class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "RenderContext.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRenderContext::CRenderContext( CCamera *pCamera )
{
	m_pCamera = pCamera;

	m_MatrixStackIndex	= 0 ;
	m_NodeStackIndex	= 0 ;

	m_pCurPen			= 0 ;

}

CRenderContext::~CRenderContext()
{
}

//////////////////////////////////////////////////////////////////////
// Geometry Functions

void CRenderContext::PushMatrix( matrix4 *pMatrix )
{
	ASSERT( m_MatrixStackIndex < MATRIX_STACK_SIZE ) ;

	m_MatrixStack[m_MatrixStackIndex++] = *pMatrix;
}

void CRenderContext::PopMatrix( )
{
	m_MatrixStackIndex-- ;
	ASSERT( m_MatrixStackIndex >= 0 ) ;
}

const matrix4& CRenderContext::GetMatrix( )
{
	return m_MatrixStack[m_MatrixStackIndex-1];
}


//////////////////////////////////////////////////////////////////////
// Node Functions

void CRenderContext::PushNode( CeNode *pNode )
{
	ASSERT( m_NodeStackIndex < NODE_STACK_SIZE ) ;
	m_NodeStack[m_NodeStackIndex] = pNode ;
	m_NodeStackIndex++ ;
}

void CRenderContext::PopNode( )
{
	m_NodeStackIndex-- ;
	ASSERT( m_NodeStackIndex >= 0 ) ;
}



//===================================================
// V3_ClipLineSegment
//===================================================
void CRenderContext::V3_ClipLineSegment( vector3* CV, vector3* V0, vector3* V1,
                         f32 NX, f32 NY, f32 NZ, f32 D )
{
    vector3    dp;
    f32         Numerator;
    f32         Denominator;
    f32         t;

    ASSERT(CV && V0 && V1);

    // compute delta vector from p0 -> p1
    dp.X = V1->X - V0->X;
    dp.Y = V1->Y - V0->Y;
    dp.Z = V1->Z - V0->Z;

    // compute parametric t-value of intersection with plane
    Numerator   = -(NX*V0->X + NY*V0->Y + NZ*V0->Z + D);
    Denominator = NX*dp.X + NY*dp.Y + NZ*dp.Z;

    // if Denominator is close to 0, then V0->V1 is parallel to plane.
    // Use t=0.5 as approximation.
    if (Denominator < 0.001 && Denominator > -0.001) t=0.5f;
    else t = (Numerator / Denominator);

    // confirm that points straddle plane
    //ASSERT(t<=1.0001 && t>=-0.0001);

    // compute coordinates of clipped point
    CV->X = V0->X + t*dp.X;
    CV->Y = V0->Y + t*dp.Y;
    CV->Z = V0->Z + t*dp.Z;
}
