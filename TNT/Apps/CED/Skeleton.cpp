// Skeleton.cpp: implementation of the CSkeleton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "Skeleton.h"
#include "RenderContext.h"
#include "CeDView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#include "Skel.h"

#define	ANM_VERSION	1

#define USE_QUAT_BLENDING


radian3 BlendXYZUsingQuaternions( const radian3& FromXYZ, const radian3& ToXYZ, f32 t );


/////////////////////////////////////////////////////////////////////////////
// CSkeleton Implementation

IMPLEMENT_DYNCREATE(CSkeleton, CObject)

/////////////////////////////////////////////////////////////////////////////
// CSkeleton Constructors

void CSkeleton::Initialize()
{
	m_pMatrices = NULL ;
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
		x_free( m_pMatrices ) ;

	//---	Initialize Data Members
	Initialize () ;
}

void CSkeleton::Copy (CSkeleton &Dst, CSkeleton &Src)
{
}

//==========================================================================
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

//==========================================================================
bool CSkeleton::CreateFromASF( const char *pFileName )
{
	err Error = m_Skel.ImportASF( pFileName ) ;

	if( Error != ERR_SUCCESS )
	{
		CString Message;
		Message.Format( "Error loading skeleton file '%s'", pFileName );
		MessageBox( NULL, Message, "ERROR Loading File", MB_ICONERROR );
		return false;
	}

	m_Skel.DelDummyBones() ;

	//---	Allocate Storage for Matrices
	m_pMatrices = (matrix4*)x_malloc( sizeof(matrix4) * GetNumBones() ) ;
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

CSkel *CSkeleton::GetSkel()
{
	return &m_Skel ;
}

int	CSkeleton::GetNumBones()
{
	return m_Skel.m_NBones ;
}

SSkelBone *CSkeleton::GetBoneFromIndex( int iBone )
{
	ASSERT( (iBone >= 0) && (iBone < m_Skel.m_NBones ) ) ;

	if( (iBone >= 0) && (iBone < GetNumBones()) )
	{
		return &m_Skel.m_pBone[iBone] ;
	}
	else
	{
		return NULL ;
	}
}

SSkelBone *CSkeleton::GetBoneFromName( const CString& Name )
{
	s32 iBone;

	for( iBone=0; iBone < GetNumBones(); iBone++ )
		if( m_Skel.m_pBone[iBone].BoneName == Name )
			return &m_Skel.m_pBone[iBone];

	return NULL;
}

s32 CSkeleton::FindBoneFromName( const CString& Name )
{
	s32 iBone;

	for( iBone=0; iBone < GetNumBones(); iBone++ )
		if( m_Skel.m_pBone[iBone].BoneName == Name )
			return iBone;

	return -1;
}

int CSkeleton::GetBoneNumParents( SSkelBone *pBone )
{
	int	nParents = 0 ;
	
	while( pBone->ParentID != -1 )
	{
		pBone = &m_Skel.m_pBone[pBone->ParentID] ;
		nParents++ ;
	}

	return nParents ;
}

int CSkeleton::GetBoneMirror( int iBone )
{
	SSkelBone *pBone = GetBoneFromIndex( iBone ) ;
    return pBone->MirrorID;    

/*
	int		iMirror = iBone ;

	ASSERT( (iBone >= 0) && (iBone < m_Skel.m_NBones ) ) ;

	//---	Generate Mirror Name by swapping L / R in first character
	SSkelBone *pBone = GetBoneFromIndex( iBone ) ;
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
	for( int i = 0 ; i < m_Skel.m_NBones ; i++ )
	{
		if( strcmp( pBone->BoneName, m_Skel.m_pBone[i].BoneName ) == 0 )
		{
			iMirror = i ;
			break ;
		}
	}

	//---	Return Mirror Bone
	return iMirror ;
*/
}

////////////////////////////////////////////////////////////////////////////
void CSkeleton::SetPose( matrix4 WorldBase, CMotion *pMotion, float nFrame, bool Blend, bool Mirror )
{
	vector3 	Translation ;
	vector3 	Translation1 ;
	radian		RotationY ;
//	radian		RotationY1 ;

	if( !pMotion )
	{
		SetNeutralPose( Mirror );
		return;
	}

	anim *pAnim = pMotion->GetAnim() ;
	if( pAnim && (GetNumBones() > 0) )
	{
		if( nFrame > (f32)(pMotion->GetNumFrames()-1) )
			nFrame = (f32)(pMotion->GetNumFrames()-1);

		if( !Blend )
		{
			nFrame = (float)((int)(nFrame+0.49f));
			ANIM_GetTransValue( pMotion->GetAnim(), (int)nFrame, &Translation ) ;
			RotationY = pMotion->GetFaceDir( (s32)nFrame );

			m_WorldMatrix.Identity();
			m_WorldMatrix.Rotate( radian3( 0.0f, RotationY, 0.0f ) );
			m_WorldMatrix.Translate( Translation );
			m_WorldMatrix.Transform( WorldBase );

			//---	Build Matrices given Anim and Frame
			BuildMatrices( pMotion->GetAnim(), (int)nFrame, m_pMatrices, &m_WorldMatrix ) ;
		}
		else
		{
			int Frame0 = (int)nFrame;
			int Frame1 = Frame0+1;
			float Frac = nFrame - (f32)Frame0;

			//---	For the motion to wrap correctly, nFrame should not be passed in >= (NumFrames-1),
			//		however, when viewing the motions with the scroll bar, the motion may get set to
			//		NFrames-1.  In that case we would want Frame1 to be set to the same value as Frame0
			//		rather than wrapping it to the beginning.
			if( Frame1 > (pMotion->GetNumFrames()-1) )
				Frame1 = pMotion->GetNumFrames()-1;

			ANIM_GetTransValue( pMotion->GetAnim(), Frame0, &Translation ) ;
			ANIM_GetTransValue( pMotion->GetAnim(), Frame1, &Translation1 ) ;
			Translation.X += (Translation1.X-Translation.X)*Frac;
			Translation.Y += (Translation1.Y-Translation.Y)*Frac;
			Translation.Z += (Translation1.Z-Translation.Z)*Frac;

			RotationY = pMotion->GetFaceDirBlend( nFrame );

			m_WorldMatrix.Identity();
			m_WorldMatrix.Rotate( radian3( 0.0f, RotationY, 0.0f ) );
			m_WorldMatrix.Translate( Translation );
			m_WorldMatrix.Transform( WorldBase );

			//---	Build Matrices given Anim and Frame
			BuildMatricesBlend( pMotion->GetAnim(), Frame0, Frame1, Frac, m_pMatrices, &m_WorldMatrix ) ;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CSkeleton::SetPose( const vector3& Pos, const radian3& Rot, CMotion *pMotion, float nFrame, bool Blend, bool Mirror )
{
	vector3 	Translation;
	vector3 	Translation1;

	if( pMotion )
	{
		anim *pAnim = pMotion->GetAnim() ;
		if( pAnim && (GetNumBones() > 0) )
		{
			if( nFrame > (f32)(pMotion->GetNumFrames()-1) )
				nFrame = (f32)(pMotion->GetNumFrames()-1);

			matrix4 Mat;
			Mat.Identity();

			if( !Blend )
			{
				//---	Build Matrices given Anim and Frame
				if( Mirror )
//					RecursivelyBuildMatrixMir( pMotion->GetAnim(), (s32)(nFrame+0.49f), m_pMatrices, 0, &Mat );
					BuildMatricesMir( pMotion->GetAnim(), (s32)(nFrame+0.49f), m_pMatrices, &Mat );
				else
//					RecursivelyBuildMatrix( pMotion->GetAnim(), (s32)(nFrame+0.49f), m_pMatrices, 0, &Mat );
					BuildMatrices( pMotion->GetAnim(), (s32)(nFrame+0.49f), m_pMatrices, &Mat );
			}
			else
			{
				s32 Frame0 = (s32) nFrame;
				s32 Frame1 = Frame0+1;
				float Frac = nFrame - (f32)Frame0;

				//---	For the motion to wrap correctly, nFrame should not be passed in >= (NumFrames-1),
				//		however, when viewing the motions with the scroll bar, the motion may get set to
				//		NFrames-1.  In that case we would want Frame1 to be set to the same value as Frame0
				//		rather than wrapping it to the beginning.
				if( Frame1 > (pMotion->GetNumFrames()-1) )
					Frame1 = pMotion->GetNumFrames()-1;

				//---	Build Matrices given Anim and Frame
				if( Mirror )
//					RecursivelyBuildMatrixMirBlend( pMotion->GetAnim(), Frame0, Frame1, Frac, m_pMatrices, 0, &Mat );
					BuildMatricesMirBlend( pMotion->GetAnim(), Frame0, Frame1, Frac, m_pMatrices, &Mat );
				else
//					RecursivelyBuildMatrixBlend( pMotion->GetAnim(), Frame0, Frame1, Frac, m_pMatrices, 0, &Mat );
					BuildMatricesBlend( pMotion->GetAnim(), Frame0, Frame1, Frac, m_pMatrices, &Mat );
			}
		}
	}
	else
		SetNeutralPose( Mirror );

	//---
	//
	//	transform the matrices node by the rot/pos values passed in.
	//
	//---
	matrix4 Mat;
	vector3 RootPos = m_pMatrices[0].GetTranslation();
	radian3 RootRot = m_pMatrices[0].GetRotationXYZ();
	Mat.Identity();
	Mat.Translate( -RootPos );
	Mat.RotateZ( -RootRot.Roll );
	Mat.RotateY( -RootRot.Yaw );
	Mat.RotateX( -RootRot.Pitch );
	Mat.RotateXYZ( Rot );
	Mat.Translate( Pos - RootPos );
	
	s32 i;
	for( i=0; i<m_Skel.m_NBones; i++ )
		m_pMatrices[i].Transform( Mat );
}

////////////////////////////////////////////////////////////////////////////
//
//	at this point, Ratio is how much of the primary motion is used.  1.0 ==
//	primary motion complete
//
void CSkeleton::SetPose2MotionBlend( matrix4	WorldBase,
									 CMotion*	pMotion,
									 f32		nFrame,
									 CMotion*	pSecondMotion,
									 f32		nSecondFrame,
									 f32		Ratio,
									 bool		bMergeMotion,
									 bool		bBlend,
									 bool		bMirror )
{
	vector3 	Translation ;
	vector3 	Translation1 ;
	radian		RotationY ;
	radian		RotationY1 ;
	radian		SecondRotY ;
	radian		SecondRotY1 ;
	vector3		SecondTrans;
	vector3		TransFrame0;
	vector3		SecondTransFrame0;
	radian		ConvertMotionRotY;

	ASSERTS(!bMirror,"not yet implemented");

	if( !pMotion )
	{
		SetNeutralPose();
		return;
	}

	//---	if there is no last motion, there is nothing to blend between, just set the pose
	//		based on the first motion
	if( !pSecondMotion )
	{
		SetPose( WorldBase, pMotion, nFrame, bBlend );
		return;
	}

	anim *pAnim = pMotion->GetAnim() ;
	if( pAnim && (GetNumBones() > 0) )
	{
		if( nFrame > (f32)(pMotion->GetNumFrames()-1) )
			nFrame = (f32)(pMotion->GetNumFrames()-1);

		if( !bBlend )
		{
			if( bMergeMotion )
			{
				//---	remove the interpolation from the frames
				nFrame			= (float)((int)(nFrame+0.49f));
				nSecondFrame	= (f32)pSecondMotion->GetNumFrames() * (nFrame / (f32)pMotion->GetNumFrames());

				//---	get the transition between the two motions
				ANIM_GetTransValue( pMotion->GetAnim(), 0, &TransFrame0 ) ;
				ANIM_GetTransValue( pSecondMotion->GetAnim(), 0, &SecondTransFrame0 ) ;
				ANIM_GetTransValue( pMotion->GetAnim(), (int)nFrame, &Translation ) ;
				ANIM_GetTransValue( pSecondMotion->GetAnim(), (int)nSecondFrame, &SecondTrans ) ;
				SecondTrans.X = TransFrame0.X - SecondTransFrame0.X;
				SecondTrans.Z = TransFrame0.Z - SecondTransFrame0.Z;
				Translation = SecondTrans + (Translation-SecondTrans)*Ratio;

				//---	get the rotationY between the two motions
				RotationY	= pMotion->GetFaceDir( (s32)nFrame );
				SecondRotY	= pSecondMotion->GetFaceDir( (s32)nSecondFrame );
				SecondRotY +=  pMotion->GetFaceDir(0) - pSecondMotion->GetFaceDir(0);
				f32 DR = SecondRotY - RotationY;
				while( DR > R_180 )		DR -= R_360;
				while( DR < -R_180 )	DR += R_360;
				RotationY	= SecondRotY + DR*Ratio;
			}
			else
			{
				nFrame = (float)((int)(nFrame+0.49f));
				ANIM_GetTransValue( pMotion->GetAnim(), (int)nFrame, &Translation ) ;
				RotationY = pMotion->GetFaceDir( (s32)nFrame );

				//---	blend the animation Y translation between the two motions
				ANIM_GetTransValue( pSecondMotion->GetAnim(), (int)nSecondFrame, &SecondTrans ) ;
				Translation.Y = SecondTrans.Y + (Translation.Y - SecondTrans.Y) * Ratio;
			}

			m_WorldMatrix.Identity();
			m_WorldMatrix.Rotate( radian3( 0.0f, RotationY, 0.0f ) );
			m_WorldMatrix.Translate( Translation );
			m_WorldMatrix.Transform( WorldBase );

			//---	Build Matrices given Anim and Frame
			BuildMatrices2MotionBlend( pMotion->GetAnim(), (int)nFrame, (int)nFrame, 0.0f,
									   pSecondMotion->GetAnim(), (s32)nSecondFrame, Ratio,
									   m_pMatrices, &m_WorldMatrix ) ;
		}
		else
		{
			int Frame0 = (int)nFrame;
			int Frame1 = Frame0+1;
			float Frac = nFrame - (f32)Frame0;

			//---	For the motion to wrap correctly, nFrame should not be passed in >= (NumFrames-1),
			//		however, when viewing the motions with the scroll bar, the motion may get set to
			//		NFrames-1.  In that case we would want Frame1 to be set to the same value as Frame0
			//		rather than wrapping it to the beginning.
			if( Frame1 > (pMotion->GetNumFrames()-1) )
				Frame1 = pMotion->GetNumFrames()-1;

			if( bMergeMotion )
			{
				int		SecondFrame0 = (int)nSecondFrame;
				int		SecondFrame1 = SecondFrame0+1;
				float	SecondFrac	= nSecondFrame - (f32)SecondFrame0;
				vector3	SecondTrans1;

				//---	don't let the second frame, next frame wrap to beginning or go past the end.
				if( SecondFrame1 > (pSecondMotion->GetNumFrames()-1) )
					SecondFrame1 = pSecondMotion->GetNumFrames()-1;

				//---	first motion translation
				ANIM_GetTransValue( pMotion->GetAnim(), Frame0, &Translation ) ;
				ANIM_GetTransValue( pMotion->GetAnim(), Frame1, &Translation1 ) ;
				Translation += (Translation1-Translation)*Frac;

				//---	second motion translation
				ANIM_GetTransValue( pSecondMotion->GetAnim(), SecondFrame0, &SecondTrans );
				ANIM_GetTransValue( pSecondMotion->GetAnim(), SecondFrame1, &SecondTrans1 );
				SecondTrans += (SecondTrans1-SecondTrans)*SecondFrac;

				//---	calculate the rotation difference between the first frames of the two animations
				ConvertMotionRotY = pMotion->GetFaceDir(0) - pSecondMotion->GetFaceDir(0);

				//---	adjust translation so that both motions are on the same spot
				ANIM_GetTransValue( pMotion->GetAnim(), 0, &TransFrame0 ) ;
				ANIM_GetTransValue( pSecondMotion->GetAnim(), 0, &SecondTransFrame0 ) ;
				SecondTrans.X -= SecondTransFrame0.X;
				SecondTrans.Z -= SecondTransFrame0.Z;
				SecondTrans.RotateY( ConvertMotionRotY );
				SecondTrans.X += TransFrame0.X;
				SecondTrans.Z += TransFrame0.Z;

				//---	blend between the two motion translations for final translation
				Translation = SecondTrans + (Translation - SecondTrans)*Ratio;

				//---	first motion rotation
				RotationY = pMotion->GetFaceDir( Frame0 );
				RotationY1 = pMotion->GetFaceDir( Frame1 );
				f32 D = RotationY1-RotationY;
				while( D > R_180 )		D -= R_360;
				while( D <= -R_180 )	D += R_360;
				RotationY = RotationY + Frac*D;

				//---	second motion rotation
				SecondRotY = pSecondMotion->GetFaceDir( SecondFrame0 );
				SecondRotY1 = pSecondMotion->GetFaceDir( SecondFrame1 );
				D = SecondRotY1-SecondRotY;
				while( D > R_180 )		D -= R_360;
				while( D <= -R_180 )	D += R_360;
				SecondRotY = SecondRotY + SecondFrac*D;

				///---	get second rotation reletive to the start of the first motion
				SecondRotY += ConvertMotionRotY;

				//---	blend between the two motion rotations for final rotation
				D = RotationY-SecondRotY;
				while( D > R_180 )		D -= R_360;
				while( D <= -R_180 )	D += R_360;
				RotationY = SecondRotY + Ratio*D;
			}
			else
			{
				ANIM_GetTransValue( pMotion->GetAnim(), Frame0, &Translation ) ;
				ANIM_GetTransValue( pMotion->GetAnim(), Frame1, &Translation1 ) ;
				Translation.X += (Translation1.X-Translation.X)*Frac;
				Translation.Y += (Translation1.Y-Translation.Y)*Frac;
				Translation.Z += (Translation1.Z-Translation.Z)*Frac;

				//---	blend the animation Y translation between the two motions
				ANIM_GetTransValue( pSecondMotion->GetAnim(), (int)nSecondFrame, &SecondTrans ) ;
				Translation.Y = SecondTrans.Y + (Translation.Y - SecondTrans.Y) * Ratio;

				RotationY = pMotion->GetFaceDir( Frame0 );
				RotationY1 = pMotion->GetFaceDir( Frame1 );
				f32 D = RotationY1-RotationY;
				while( D > R_180 )		D -= R_360;
				while( D <= -R_180 )	D += R_360;
				RotationY = RotationY + Frac*D;
			}

			m_WorldMatrix.Identity();
			m_WorldMatrix.Rotate( radian3( 0.0f, RotationY, 0.0f ) );
			m_WorldMatrix.Translate( Translation );
			m_WorldMatrix.Transform( WorldBase );

			//---	Build Matrices given Anim and Frame
			BuildMatrices2MotionBlend( pMotion->GetAnim(), Frame0, Frame1, Frac,
									   pSecondMotion->GetAnim(), (s32)nSecondFrame, Ratio,
									   m_pMatrices, &m_WorldMatrix ) ;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CSkeleton::SetPoseMovie( CMotion *pMotion, float nFrame, int Mirror, bool Blend )
{
	if( !pMotion )
	{
		SetNeutralPose();
		return;
	}

	//---	Build a World Orientation Matrix for the Skeleton
	vector3 	Translation ;
	vector3 	Rotation ;

	Rotation.Zero();

	anim *pAnim = pMotion->GetAnim() ;
	if( pAnim && (GetNumBones() > 0) )
	{
		nFrame = (float)((int)(nFrame+0.5f));
#if 1
		Rotation.Y = pMotion->GetFaceDir( (s32)nFrame );
		f32 Yaw = Rotation.Y - pMotion->GetFaceDir( 0 );
#else
		f32 Yaw = pMotion->GetFaceDir( 0 );
#endif

		ANIM_GetTransValue( pMotion->GetAnim(), (s32)nFrame, &Translation ) ;
		BuildWorldOrient( &m_WorldMatrix, (f32)1,
						  0, Translation.Y, 0,
						  Rotation.X, Mirror ? Rotation.Y - 2*Yaw : Rotation.Y, Rotation.Z ) ;

		//---	Build Matrices given Anim and Frame
		if( Mirror )
			BuildMatricesMir( pMotion->GetAnim(), (s32)nFrame, m_pMatrices, &m_WorldMatrix ) ;
		else
			BuildMatrices( pMotion->GetAnim(), (s32)nFrame, m_pMatrices, &m_WorldMatrix ) ;
	}
}


////////////////////////////////////////////////////////////////////////////
void CSkeleton::SetNeutralPose( bool bMirror )
{
	//---	Build a World Orientation Matrix for the Skeleton
	vector3 	Translation ;
	vector3 	Rotation ;

	Rotation.Zero();
	Translation.Zero();

	BuildWorldOrient( &m_WorldMatrix, (f32)1,
					   Translation.X, Translation.Y, Translation.Z,
					   Rotation.X, Rotation.Y, Rotation.Z ) ;

	//---	Build Matrices given Anim and Frame
	if( GetNumBones() > 0 )
	{
		if( bMirror )
			BuildMatricesMir( NULL, 0, m_pMatrices, &m_WorldMatrix ) ;
		else
			BuildMatrices( NULL, 0, m_pMatrices, &m_WorldMatrix ) ;
	}
}

//==========================================================================
matrix4 *CSkeleton::GetPosedBoneMatrix( int iBone )
{
	ASSERT( (iBone >= 0) && (iBone < m_Skel.m_NBones ) ) ;
	return &m_pMatrices[iBone] ;
}

//==========================================================================
matrix4 *CSkeleton::GetWorldMatrix( )
{
	return &m_WorldMatrix ;
}


//==========================================================================
void CSkeleton::Render( CRenderContext *pRC, COLORREF Color, bool DrawAxes )
{
	vector3 	p[100] ;

	ASSERT( GetNumBones() < 100 ) ;

	for( int i = 0 ; i < GetNumBones() ; i++ )
	{
		int ID = GetBoneFromIndex(i)->BoneID ;
		p[ID] = GetPosedBoneMatrix(ID)->GetTranslation();
//		M4_GetTranslation( GetPosedBoneMatrix(ID), &p[ID] ) ;
	}

	//---	Set Pen to Draw in
	CPen PenBone( PS_SOLID, 1, Color ) ;
	CPen *pOldPen = pRC->SelectObject( &PenBone ) ;

	//---	Loop through each Bone
	for( i = 0 ; i < GetNumBones() ; i++ )
	{
		//---	Draw Bone Axis if required
		if( DrawAxes )
		{
			matrix4	m ;
			m = *GetPosedBoneMatrix( i );
//			M4_Copy( &m, GetPosedBoneMatrix( i ) ) ;
			pRC->PushMatrix( &m ) ;
			pRC->RENDER_Axes( 2, RGB(255*MOD_COLOR,0,0), RGB(0,255*MOD_COLOR,0), RGB(0,0,255*MOD_COLOR) ) ;
			pRC->PopMatrix( ) ;
		}

		//---	Draw Bone to Parent
		if( GetBoneFromIndex(i)->ParentID != -1 )
		{
			pRC->RENDER_TransClipProjLine3d( &p[GetBoneFromIndex(i)->ParentID], &p[GetBoneFromIndex(i)->BoneID] ) ;
		}
	}

	//---	Restore Pen
	pRC->SelectObject( pOldPen ) ;
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


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  MATRICES
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void CSkeleton::BuildWorldOrient(matrix4*   M, 
								 f32        Scale, 
								 f32        Tx, 
								 f32        Ty, 
								 f32        Tz,
								 radian     Rx, 
								 radian     Ry, 
								 radian     Rz)
{
    vector3	S(Scale,Scale,Scale);
    radian3	R(Rx,Ry,Rz);
    vector3	T(Tx,Ty,Tz);
    ASSERT(M);

	M->SetRotationXYZ( R );
	M->Scale( S );
	M->Translate( T );
//    M4_SetupSRT(M,&S,&R,&T);
}

///////////////////////////////////////////////////////////////////////////
// THIS FUNCTION DOESN'T APPEAR TO WORK.  I AM LEAVING IT BECAUSE IT MIGHT BE
//	HANDY TO GET IT WORKING SOME DAY
void CSkeleton::RecursivelyBreakMatrix( anim*      Anim,
										s32        AnimFrame,
										matrix4*   DstMatrices,
										s32        BoneID,
										matrix4*   ParentMatrix)
{
    s32 i;
    matrix4 M;
    radian3 LR;
    vector3  LT;

	{
		vector3	T = ParentMatrix->GetTranslation();
		radian3 R = ParentMatrix->GetRotationXYZ();
		M.Identity();
		M.SetTranslation( -T );
		M.RotateXYZ( -R );

		matrix4 L = DstMatrices[BoneID];
		L.Transform( M );
		DstMatrices[BoneID] = L;
	}

    // Get local rotation and translation info
    LT   = m_Skel.m_pBone[BoneID].ParentToBone;

    if (Anim)
    {
	    LR.X = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+0 ) ;
	    LR.Y = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+1 ) ;
	    LR.Z = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+2 ) ;
    }
    else
    {
        LR.X = LR.Y = LR.Z = 0;
    }

    // Build matrix local to world matrix bone
	M.Identity();
	M.SetRotationXYZ( LR );
	M.Translate( LT );
	M.Transform( *ParentMatrix );

    // Call children with L2W matrix as parent matrix
    for (i=0; i<m_Skel.m_NBones; i++)
		if (m_Skel.m_pBone[i].ParentID == BoneID)
			RecursivelyBuildMatrix( Anim, AnimFrame, DstMatrices, i, &M );
}

///////////////////////////////////////////////////////////////////////////

void CSkeleton::RecursivelyBuildMatrix( anim*      Anim,
										s32        AnimFrame,
										matrix4*   DstMatrices,
										s32        BoneID,
										matrix4*   ParentMatrix)
{
    s32 i;
    matrix4 M;
    radian3 LR;
    vector3  LT;

    // Get local rotation and translation info
    LT   = m_Skel.m_pBone[BoneID].ParentToBone;

    if (Anim)
    {
	    LR.X = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+0 ) ;
	    LR.Y = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+1 ) ;
	    LR.Z = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+2 ) ;
    }
    else
    {
        LR.X = LR.Y = LR.Z = 0;
    }

    // Build matrix local to world matrix bone
	M.Identity();
	M.SetRotationXYZ( LR );
	M.Translate( LT );
	M.Transform( *ParentMatrix );

    // Call children with L2W matrix as parent matrix
    for (i=0; i<m_Skel.m_NBones; i++)
		if (m_Skel.m_pBone[i].ParentID == BoneID)
			RecursivelyBuildMatrix( Anim, AnimFrame, DstMatrices, i, &M );

    // Place L2W matrix in destination array 
    DstMatrices[BoneID] = M;
}

///////////////////////////////////////////////////////////////////////////

void CSkeleton::RecursivelyBuildMatrixBlend( anim*		Anim,
											 s32		AnimFrame0,
											 s32		AnimFrame1,
											 f32		AnimFrameFrac,
											 matrix4*	DstMatrices,
											 s32		BoneID,
											 matrix4*	ParentMatrix)
{
    s32 i;
    matrix4 M;
//	radian DR;
    radian3 LR;
    radian3 LR1;
    vector3  LT;

    // Get local rotation and translation info
    LT   = m_Skel.m_pBone[BoneID].ParentToBone;

    if (Anim)
    {
	    LR.X = ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+0 ) ;
	    LR.Y = ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+1 ) ;
	    LR.Z = ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+2 ) ;

		LR1.X = ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+0 ) ;
	    LR1.Y = ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+1 ) ;
	    LR1.Z = ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+2 ) ;

#ifdef USE_QUAT_BLENDING
		LR = BlendXYZUsingQuaternions( LR, LR1, AnimFrameFrac );
#else
		DR = LR1.X-LR.X;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.X = LR.X + DR*AnimFrameFrac;

		DR = LR1.Y-LR.Y;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.Y = LR.Y + DR*AnimFrameFrac;

		DR = LR1.Z-LR.Z;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.Z = LR.Z + DR*AnimFrameFrac;
#endif
    }
    else
    {
        LR.X = LR.Y = LR.Z = 0;
    }

    // Build matrix local to world matrix bone
	M.Identity();
	M.SetRotationXYZ( LR );
	M.Translate( LT );
	M.Transform( *ParentMatrix );

    // Call children with L2W matrix as parent matrix
    for (i=0; i<m_Skel.m_NBones; i++)
	    if (m_Skel.m_pBone[i].ParentID == BoneID)
	        RecursivelyBuildMatrixBlend( Anim, AnimFrame0, AnimFrame1, AnimFrameFrac, DstMatrices, i, &M );

    // Place L2W matrix in destination array 
    DstMatrices[BoneID] = M;
}


///////////////////////////////////////////////////////////////////////////

void CSkeleton::RecursivelyBuildMatrix2MotionBlend( anim*		Anim,
													s32			AnimFrame0,
													s32			AnimFrame1,
													f32			AnimFrameFrac,
													anim*		LastAnim,
													s32			LastFrame,
													f32			LastAnimFrac,
													matrix4*	DstMatrices,
													s32			BoneID,
													matrix4*	ParentMatrix)
{
    s32 i;
    matrix4 M;
//	radian DR;
    radian3 LR;
    radian3 LR1;
    vector3  LT;

    // Get local rotation and translation info
    LT   = m_Skel.m_pBone[BoneID].ParentToBone;

    if (Anim)
    {
	    LR.X = ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+0 ) ;
	    LR.Y = ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+1 ) ;
	    LR.Z = ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+2 ) ;

		LR1.X = ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+0 ) ;
	    LR1.Y = ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+1 ) ;
	    LR1.Z = ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+2 ) ;

#ifdef USE_QUAT_BLENDING
		LR = BlendXYZUsingQuaternions( LR, LR1, AnimFrameFrac );
#else
		DR = LR1.X-LR.X;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.X = LR.X + DR*AnimFrameFrac;

		DR = LR1.Y-LR.Y;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.Y = LR.Y + DR*AnimFrameFrac;

		DR = LR1.Z-LR.Z;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.Z = LR.Z + DR*AnimFrameFrac;
#endif

		if( LastAnim )
		{
			LR1.X = ANIM_GetRotation( LastAnim, LastFrame, (BoneID*3)+0 ) ;
			LR1.Y = ANIM_GetRotation( LastAnim, LastFrame, (BoneID*3)+1 ) ;
			LR1.Z = ANIM_GetRotation( LastAnim, LastFrame, (BoneID*3)+2 ) ;

#ifdef USE_QUAT_BLENDING
			LR = BlendXYZUsingQuaternions( LR1, LR, LastAnimFrac );
#else
			DR = LR.X - LR1.X;
			while (DR < -R_180) DR += R_360;
			while (DR >  R_180) DR -= R_360;
			LR.X = LR1.X + DR*LastAnimFrac;

			DR = LR.Y-LR1.Y;
			while (DR < -R_180) DR += R_360;
			while (DR >  R_180) DR -= R_360;
			LR.Y = LR1.Y + DR*LastAnimFrac;

			DR = LR.Z-LR1.Z;
			while (DR < -R_180) DR += R_360;
			while (DR >  R_180) DR -= R_360;
			LR.Z = LR1.Z + DR*LastAnimFrac;
#endif
		}
    }
    else
    {
        LR.X = LR.Y = LR.Z = 0;
    }

    // Build matrix local to world matrix bone
	M.Identity();
	M.SetRotationXYZ( LR );
	M.Translate( LT );
	M.Transform( *ParentMatrix );

    // Call children with L2W matrix as parent matrix
    for (i=0; i<m_Skel.m_NBones; i++)
	    if (m_Skel.m_pBone[i].ParentID == BoneID)
	        RecursivelyBuildMatrix2MotionBlend( Anim,
												AnimFrame0,
												AnimFrame1,
												AnimFrameFrac,
												LastAnim,
												LastFrame,
												LastAnimFrac,
												DstMatrices,
												i,
												&M );
//			RecursivelyBuildMatrixBlend( Anim, AnimFrame0, AnimFrame1, AnimFrameFrac, DstMatrices, i, &M );

    // Place L2W matrix in destination array 
    DstMatrices[BoneID] = M;
}

///////////////////////////////////////////////////////////////////////////

void CSkeleton::RecursivelyBuildMatrixMir( anim*      Anim,
										   s32        AnimFrame,
										   matrix4*   DstMatrices,
										   s32        BoneID,
										   matrix4*   ParentMatrix)
{
    s32 i;
    matrix4 M;
    radian3 LR;
    vector3  LT;

    // Get local rotation and translation info
    LT   = m_Skel.m_pBone[ m_Skel.m_pBone[BoneID].MirrorID ].ParentToBone;

    if (Anim)
    {
	    LR.X =  ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+0 ) ;
	    LR.Y = -ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+1 ) ;
	    LR.Z = -ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+2 ) ;
    }
    else
    {
        LR.X = LR.Y = LR.Z = 0;
    }

    // Build matrix local to world matrix bone
	M.Identity();
	M.SetRotationXYZ( LR );
	M.Translate( LT );
	M.Transform( *ParentMatrix );

    // Call children with L2W matrix as parent matrix
    for (i=0; i<m_Skel.m_NBones; i++)
		if (m_Skel.m_pBone[i].ParentID == BoneID)
			RecursivelyBuildMatrixMir( Anim, AnimFrame, DstMatrices, i, &M);

    // Place L2W matrix in destination array 
    DstMatrices[BoneID] = M;
}


///////////////////////////////////////////////////////////////////////////

void CSkeleton::RecursivelyBuildMatrixMirBlend( anim*		Anim,
											 s32		AnimFrame0,
											 s32		AnimFrame1,
											 f32		AnimFrameFrac,
											 matrix4*	DstMatrices,
											 s32		BoneID,
											 matrix4*	ParentMatrix)
{
    s32 i;
    matrix4 M;
//	radian DR;
    radian3 LR;
    radian3 LR1;
    vector3  LT;

    // Get local rotation and translation info
    LT   = m_Skel.m_pBone[ m_Skel.m_pBone[BoneID].MirrorID ].ParentToBone;

    if (Anim)
    {
	    LR.X = ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+0 ) ;
	    LR.Y = -ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+1 ) ;
	    LR.Z = -ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+2 ) ;

		LR1.X = ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+0 ) ;
	    LR1.Y = -ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+1 ) ;
	    LR1.Z = -ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+2 ) ;

#ifdef USE_QUAT_BLENDING
		LR = BlendXYZUsingQuaternions( LR, LR1, AnimFrameFrac );
#else
		DR = LR1.X-LR.X;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.X = LR.X + DR*AnimFrameFrac;

		DR = LR1.Y-LR.Y;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.Y = LR.Y + DR*AnimFrameFrac;

		DR = LR1.Z-LR.Z;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.Z = LR.Z + DR*AnimFrameFrac;
#endif
    }
    else
    {
        LR.X = LR.Y = LR.Z = 0;
    }

    // Build matrix local to world matrix bone
	M.Identity();
	M.SetRotationXYZ( LR );
	M.Translate( LT );
	M.Transform( *ParentMatrix );

    // Call children with L2W matrix as parent matrix
    for (i=0; i<m_Skel.m_NBones; i++)
	    if (m_Skel.m_pBone[i].ParentID == BoneID)
	        RecursivelyBuildMatrixMirBlend( Anim, AnimFrame0, AnimFrame1, AnimFrameFrac, DstMatrices, i, &M );

    // Place L2W matrix in destination array 
    DstMatrices[BoneID] = M;
}

///////////////////////////////////////////////////////////////////////////
//	Breaks the matrices back to their original local rotations
void CSkeleton::BreakMatrices( anim*      Anim,
							   s32        AnimFrame,
							   matrix4*   DstMatrices,
							   matrix4*   WorldOrientM)
{
    RecursivelyBreakMatrix( Anim, AnimFrame, DstMatrices, 0, WorldOrientM );
}


///////////////////////////////////////////////////////////////////////////
void CSkeleton::BuildMatrices( anim*      Anim,
							   s32        AnimFrame,
							   matrix4*   DstMatrices,
							   matrix4*   WorldOrientM )
{
    RecursivelyBuildMatrix( Anim, AnimFrame, DstMatrices, 0, WorldOrientM );
}

///////////////////////////////////////////////////////////////////////////
void CSkeleton::BuildMatricesBlend( anim*      Anim,
									s32        AnimFrame0,
									s32        AnimFrame1,
									f32        AnimFrameFrac,
									matrix4*   DstMatrices,
									matrix4*   WorldOrientM )
{
	RecursivelyBuildMatrixBlend( Anim, AnimFrame0, AnimFrame1, AnimFrameFrac, DstMatrices, 0, WorldOrientM );
}

///////////////////////////////////////////////////////////////////////////
void CSkeleton::BuildMatrices2MotionBlend( anim*      Anim,
										   s32        AnimFrame0,
										   s32        AnimFrame1,
										   f32        AnimFrameFrac,
										   anim*      LastAnim,
										   s32		  LastAnimFrame,
										   f32        LastAnimFrac,
										   matrix4*   DstMatrices,
										   matrix4*   WorldOrientM )
{
	RecursivelyBuildMatrix2MotionBlend( Anim, AnimFrame0, AnimFrame1, AnimFrameFrac, LastAnim, LastAnimFrame, LastAnimFrac, DstMatrices, 0, WorldOrientM );
}

///////////////////////////////////////////////////////////////////////////

void CSkeleton::GetBoneMatrix( anim*      Anim,
							   s32        AnimFrame,
							   matrix4*   DstMatrix,
							   matrix4*   WorldOrientM,
							   s32        BoneID)
{
    matrix4  LM;
    vector3  LT;
    radian3 LR;

    // Clear DstMatrix
	DstMatrix->Identity();
//    M4_Identity(DstMatrix);

    // Loop from bone to root along parents in skeleton
    while (BoneID != -1)
    {
        // Build local matrix of this bone
        LT   = m_Skel.m_pBone[BoneID].ParentToBone;
		LR.X = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+0 ) ;
		LR.Y = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+1 ) ;
		LR.Z = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+2 ) ;
		LM.SetRotationXYZ( LR );
		LM.Translate( LT );
//        M4_SetRotationsXYZ  (&LM,&LR);
//        M4_TranslateOn      (&LM,&LT);

        // Since this is a parent's matrix Premult onto dest matrix
		DstMatrix->Transform( LM );
//        M4_PreMultOn(DstMatrix,&LM);

        // Go to next parent
        BoneID = m_Skel.m_pBone[BoneID].ParentID;
    }

    // Use WorldOrientM as final parent matrix
	DstMatrix->Transform( *WorldOrientM );
//    M4_PreMultOn(DstMatrix,WorldOrientM);
}


///////////////////////////////////////////////////////////////////////////

void CSkeleton::BuildMatricesMir( anim*      Anim,
								  s32        AnimFrame,
								  matrix4*   DstMatrices,
								  matrix4*   WorldOrientM)
{
    RecursivelyBuildMatrixMir( Anim, AnimFrame, DstMatrices, 0, WorldOrientM );
}

///////////////////////////////////////////////////////////////////////////

void CSkeleton::BuildMatricesMirBlend( anim*      Anim,
									  s32        AnimFrame0,
									  s32        AnimFrame1,
									  f32        AnimFrameFrac,
									  matrix4*   DstMatrices,
									  matrix4*	 WorldOrientM )
{
	RecursivelyBuildMatrixMirBlend( Anim, AnimFrame0, AnimFrame1, AnimFrameFrac, DstMatrices, 0, WorldOrientM );
}

///////////////////////////////////////////////////////////////////////////

void CSkeleton::GetBoneMatrixMir( anim*      Anim,
								  s32        AnimFrame,
								  matrix4*   DstMatrix,
								  matrix4*   WorldOrientM,
								  s32        BoneID)
{
    matrix4  LM;
    vector3  LT;
    radian3 LR;

    // Clear DstMatrix
	DstMatrix->Identity();
//    M4_Identity(DstMatrix);

    // Loop from bone to root along parents in skeleton
    while (BoneID != -1)
    {
        // Build local matrix of this bone
        LT   = m_Skel.m_pBone[BoneID].ParentToBone;
		LR.X =  ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+0 ) ;
		LR.Y = -ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+1 ) ;
		LR.Z = -ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+2 ) ;
		LM.SetRotationXYZ( LR );
		LM.Translate( LT );
//        M4_SetRotationsXYZ  (&LM,&LR);
//        M4_TranslateOn      (&LM,&LT);

        // Since this is a parent's matrix Premult onto dest matrix
		DstMatrix->Transform( LM );
//        M4_PreMultOn(DstMatrix,&LM);

        // Go to next parent
        BoneID = m_Skel.m_pBone[ m_Skel.m_pBone[BoneID].MirrorID ].ParentID;
    }

    // Use WorldOrientM as final parent matrix
	DstMatrix->Transform( *WorldOrientM );
//    M4_PreMultOn(DstMatrix,WorldOrientM);
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
