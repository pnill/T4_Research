// Skeleton.cpp: implementation of the CSkeleton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "Skeleton.h"
#include "RenderContext.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "Skel.h"

#define	ANM_VERSION	1

/////////////////////////////////////////////////////////////////////////////
// CSkeleton Implementation

IMPLEMENT_DYNCREATE(CSkeleton, CObject)

/////////////////////////////////////////////////////////////////////////////
// CSkeleton Constructors

void CSkeleton::Initialize()
{
	m_pMatrices = NULL ;
	SKEL_InitInstance( &m_Skel ) ;
}

CSkeleton::CSkeleton()
{
	//---	Reset Data Members
	Initialize () ;
}

CSkeleton::~CSkeleton()
{
	//---	Destroy the Texture
	Destroy () ;
}

/////////////////////////////////////////////////////////////////////////////
// CSkeleton Implementation

void CSkeleton::Destroy()
{
	if( m_pMatrices != NULL )
		free( m_pMatrices ) ;

	//---	Initialize Data Members
	Initialize () ;
}

void CSkeleton::Copy (CSkeleton &Dst, CSkeleton &Src)
{
}

bool CSkeleton::CreateFromFile( const char *pFileName )
{
	char	Ext[_MAX_EXT] ;

	_splitpath( pFileName, NULL, NULL, NULL, Ext ) ;
	if( stricmp( Ext, ".ASF" ) == 0 )
	{
		return CreateFromASF( pFileName ) ;
	}
	else
	{
		return false ;
	}
}

bool CSkeleton::CreateFromASF( const char *pFileName )
{
	err Error = SKEL_ImportASF( &m_Skel, pFileName ) ;

	if( Error != ERR_SUCCESS )
	{
		CString Message;
		Message.Format( "Error loading skeleton file '%s'", pFileName );
		MessageBox( NULL, Message, "ERROR Loading File", MB_ICONERROR );
		return false;
	}

	SKEL_DelDummyBones( &m_Skel ) ;

	//---	Allocate Storage for Matrices
	m_pMatrices = (matrix4*)malloc( sizeof(matrix4) * GetNumBones() ) ;
	ASSERT( m_pMatrices ) ;

	//---	Set Neutral Pose
	SetNeutralPose() ;

	m_PathName = pFileName ;

	return true ;
}

const char *CSkeleton::GetPathName( )
{
	return m_PathName ;
}

skel *CSkeleton::GetSkel()
{
	return &m_Skel ;
}

int	CSkeleton::GetNumBones()
{
	return m_Skel.NBones ;
}

skel_bone *CSkeleton::GetBoneFromIndex( int iBone )
{
	ASSERT( (iBone >= 0) && (iBone < m_Skel.NBones ) ) ;

	if( (iBone >= 0) && (iBone < GetNumBones()) )
	{
		return &m_Skel.Bone[iBone] ;
	}
	else
	{
		return NULL ;
	}
}

skel_bone *CSkeleton::GetBoneFromName( const CString& Name )
{
	s32 iBone;

	for( iBone=0; iBone < GetNumBones(); iBone++ )
		if( m_Skel.Bone[iBone].BoneName == Name )
			return &m_Skel.Bone[iBone];

	return NULL;
}

s32 CSkeleton::FindBoneFromName( const CString& Name )
{
	s32 iBone;

	for( iBone=0; iBone < GetNumBones(); iBone++ )
		if( m_Skel.Bone[iBone].BoneName == Name )
			return iBone;

	return -1;
}

int CSkeleton::GetBoneNumParents( skel_bone *pBone )
{
	int	nParents = 0 ;
	
	while( pBone->ParentID != -1 )
	{
		pBone = &m_Skel.Bone[pBone->ParentID] ;
		nParents++ ;
	}

	return nParents ;
}

int CSkeleton::GetBoneMirror( int iBone )
{
	skel_bone *pBone = GetBoneFromIndex( iBone ) ;
    return pBone->MirrorID;    

/*
	int		iMirror = iBone ;

	ASSERT( (iBone >= 0) && (iBone < m_Skel.NBones ) ) ;

	//---	Generate Mirror Name by swapping L / R in first character
	skel_bone *pBone = GetBoneFromIndex( iBone ) ;
	ASSERT( pBone ) ;
	CString	MirrorName = pBone->BoneName ;
	if( (toupper(MirrorName[0]) == 'R') || (toupper(MirrorName[0]) == 'L') )
	{
		char *pName = MirrorName.GetBuffer( MirrorName.GetLength() ) ;
		ASSERT( pName ) ;
		pName[0] ^= ('L' ^ 'R') ;
		MirrorName.ReleaseBuffer() ;
	}

	//---	Search for Bone
	for( int i = 0 ; i < m_Skel.NBones ; i++ )
	{
		if( strcmp( pBone->BoneName, m_Skel.Bone[i].BoneName ) == 0 )
		{
			iMirror = i ;
			break ;
		}
	}

	//---	Return Mirror Bone
	return iMirror ;
*/
}

void CSkeleton::SetPose( CMotion *pMotion, float nFrame, bool Blend )
{
	vector3d	Rotation ;

	if( !pMotion )
	{
		SetNeutralPose();
		return;
	}

	//---	Build a World Orientation Matrix for the Skeleton
	V3_Set( &Rotation, 0, 0, 0 ) ;
	anim *pAnim = pMotion->GetAnim() ;
	if( pAnim && (GetNumBones() > 0) )
	{
		if( nFrame > (f32)(pMotion->m_Anim.NFrames-1) )
			nFrame = (f32)(pMotion->m_Anim.NFrames-1);

		if( !Blend )
		{
			vector3d	Translation ;

			nFrame = (float)((int)(nFrame+0.49f));
			ANIM_GetTransValue( pMotion->GetAnim(), (int)nFrame, &Translation ) ;
			SKEL_BuildWorldOrient( &m_WorldMatrix, (f32)1, Translation.X, Translation.Y, Translation.Z,
								   Rotation.X, Rotation.Y, Rotation.Z ) ;

			//---	Build Matrices given Anim and Frame
			SKEL_BuildMatrices( &m_Skel, pMotion->GetAnim(), (int)nFrame, m_pMatrices, &m_WorldMatrix ) ;
		}
		else
		{
			vector3d	Translation ;
			vector3d	Translation1 ;
			int Frame0 = (int)nFrame;
			int Frame1 = Frame0+1;
			float Frac = nFrame - (f32)Frame0;

			//---	For the motion to wrap correctly, nFrame should not be passed in >= (NumFrames-1),
			//		however, when viewing the motions with the scroll bar, the motion may get set to
			//		NFrames-1.  In that case we would want Frame1 to be set to the same value as Frame0
			//		rather than wrapping it to the beginning.
			if( Frame1 > (pMotion->m_Anim.NFrames-1) )
				Frame1 = pMotion->m_Anim.NFrames-1;

			ANIM_GetTransValue( pMotion->GetAnim(), Frame0, &Translation ) ;
			ANIM_GetTransValue( pMotion->GetAnim(), Frame1, &Translation1 ) ;

			Translation.X += (Translation1.X-Translation.X)*Frac;
			Translation.Y += (Translation1.Y-Translation.Y)*Frac;
			Translation.Z += (Translation1.Z-Translation.Z)*Frac;

			SKEL_BuildWorldOrient( &m_WorldMatrix, (f32)1, Translation.X, Translation.Y, Translation.Z,
								   Rotation.X, Rotation.Y, Rotation.Z ) ;

			//---	Build Matrices given Anim and Frame
			SKEL_BuildMatricesBlend( &m_Skel, pMotion->GetAnim(), Frame0, Frame1, Frac, m_pMatrices, &m_WorldMatrix ) ;
		}
	}
}

void CSkeleton::SetPoseMovie( CMotion *pMotion, float nFrame, int Mirror, bool Blend )
{
	if( !pMotion )
	{
		SetNeutralPose();
		return;
	}

	//---	Build a World Orientation Matrix for the Skeleton
	vector3d	Translation ;
	vector3d	Rotation ;

	V3_Set( &Rotation, 0, 0, 0 ) ;
	anim *pAnim = pMotion->GetAnim() ;
	if( pAnim && (GetNumBones() > 0) )
	{
		nFrame = (float)((int)(nFrame+0.5f));
		radian Yaw = pMotion->GetFaceDir( 0 );

		ANIM_GetTransValue( pMotion->GetAnim(), (s32)nFrame, &Translation ) ;
		SKEL_BuildWorldOrient( &m_WorldMatrix, (f32)1, 0, Translation.Y, 0,
			Rotation.X, Mirror ? Rotation.Y + 2*Yaw : Rotation.Y, Rotation.Z ) ;

		//---	Build Matrices given Anim and Frame
		if( Mirror )
			SKEL_BuildMatricesMir( &m_Skel, pMotion->GetAnim(), (s32)nFrame, m_pMatrices, &m_WorldMatrix ) ;
		else
			SKEL_BuildMatrices( &m_Skel, pMotion->GetAnim(), (s32)nFrame, m_pMatrices, &m_WorldMatrix ) ;
	}
}

void CSkeleton::SetNeutralPose( )
{
	//---	Build a World Orientation Matrix for the Skeleton
	vector3d	Translation ;
	vector3d	Rotation ;

	V3_Set( &Rotation, 0, 0, 0 ) ;
	V3_Set( &Translation, 0, 0, 0 ) ;
		SKEL_BuildWorldOrient( &m_WorldMatrix, (f32)1, Translation.X, Translation.Y, Translation.Z,
							   Rotation.X, Rotation.Y, Rotation.Z ) ;

	//---	Build Matrices given Anim and Frame
	if( GetNumBones() > 0 )
	{
		radian3d	*pRotations = new radian3d[GetNumBones()] ;
		for( int i = 0 ; i < GetNumBones() ; i++ )
			V3_Zero( (vector3d*)&pRotations[i] ) ;

		SKEL_BuildMatrices2( &m_Skel, pRotations, m_pMatrices, &m_WorldMatrix ) ;

		delete []pRotations ;
	}
}

matrix4 *CSkeleton::GetPosedBoneMatrix( int iBone )
{
	ASSERT( (iBone >= 0) && (iBone < m_Skel.NBones ) ) ;
	return &m_pMatrices[iBone] ;
}

matrix4 *CSkeleton::GetWorldMatrix( )
{
	return &m_WorldMatrix ;
}

void CSkeleton::Render( CRenderContext *pRC, COLORREF Color, bool DrawAxes )
{
	vector3d	p[100] ;

	ASSERT( GetNumBones() < 100 ) ;

	for( int i = 0 ; i < GetNumBones() ; i++ )
	{
		int ID = GetBoneFromIndex(i)->BoneID ;
		M4_GetTranslation( GetPosedBoneMatrix(ID), &p[ID] ) ;
	}

	//---	Set Pen to Draw in
	CPen PenBone( PS_SOLID, 1, Color ) ;
	CPen *pOldPen = pRC->m_pDC->SelectObject( &PenBone ) ;

	//---	Loop through each Bone
	for( i = 0 ; i < GetNumBones() ; i++ )
	{
		//---	Draw Bone Axis if required
		if( DrawAxes )
		{
			matrix4	m ;
			M4_Copy( &m, GetPosedBoneMatrix( i ) ) ;
			pRC->PushMatrix( &m ) ;
			pRC->RENDER_Axes( 2, RGB(255,0,0), RGB(0,255,0), RGB(0,0,255) ) ;
			pRC->PopMatrix( ) ;
		}

		//---	Draw Bone to Parent
		if( GetBoneFromIndex(i)->ParentID != -1 )
		{
			pRC->RENDER_TransClipProjLine3d( &p[GetBoneFromIndex(i)->ParentID], &p[GetBoneFromIndex(i)->BoneID] ) ;
		}
	}

	//---	Restore Pen
	pRC->m_pDC->SelectObject( pOldPen ) ;
}

/////////////////////////////////////////////////////////////////////////////
// CSkeleton Overridden operators

CSkeleton &CSkeleton::operator=( CSkeleton &ptSkeleton )
{
	Copy (*this, ptSkeleton) ;

    return *this;  // Assignment operator returns left side.
}

/////////////////////////////////////////////////////////////////////////////
// CSkeleton serialization

void CSkeleton::Serialize(CieArchive& a)
{
//	CScrEdDoc *pDoc = (CScrEdDoc*)a.m_pDocument ;

	if (a.IsStoring())
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_SKELETON) ;

		//---	Write Details
		a.WriteTaggedString( IE_TAG_SKELETON_PATHNAME, m_PathName ) ;

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_SKELETON_END ) ;
	}
	else
	{
		//---	Load Skeleton
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_SKELETON_PATHNAME:
				a.ReadString(m_PathName) ;
				break ;

			case IE_TAG_SKELETON_END:
				done = TRUE ;
				CreateFromFile( m_PathName ) ;
				break ;

			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSkeleton diagnostics

#ifdef _DEBUG
void CSkeleton::AssertValid() const
{
	CObject::AssertValid();
}

void CSkeleton::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG
