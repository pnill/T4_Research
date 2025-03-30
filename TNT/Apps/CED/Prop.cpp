#include "stdafx.h"
#include "CEDDoc.h"
#include "Prop.h"

//=============================================================================
//=============================================================================
//=============================================================================
//		PROP DEFINITION FUNCTIONS
//=============================================================================
//=============================================================================
//=============================================================================

//=============================================================================
CPropDef::CPropDef()
{
	m_RuntimeVersion		= 0;

	m_bPropActive			= FALSE;
	m_bPropExported			= TRUE;
	m_PropName				= "";
	m_BoneName				= "";
	m_BoneID				= -1;

	m_pLinkCharacter		= NULL;

	m_bStaticPosition		= FALSE;
	m_Position.Set( 0.0f, 0.0f, 0.0f );
	m_bUseLinkedPos			= TRUE;
	m_bStaticRotation		= FALSE;
	m_Rotation.Set( 0.0f, 0.0f, 0.0f );
	m_bUseLinkedRot			= TRUE;
	m_bMirrorWithAnim		= TRUE;

	m_bPosFacingRelative	= TRUE;
	m_bRotFacingRelative	= TRUE;
	m_bScaled				= TRUE;
	m_bMirrored				= FALSE;
}
//=============================================================================
CPropDef::~CPropDef()
{
}

//=============================================================================
void CPropDef::Copy( const CPropDef& Src )
{
	m_bPropActive			= Src.m_bPropActive;
	m_bPropExported			= Src.m_bPropExported;
	m_BoneName				= Src.m_BoneName;
	m_BoneID				= -1;	// this will cause it to find the bone ID when it is next accessed.
	m_pLinkCharacter		= Src.m_pLinkCharacter;
	m_bStaticPosition		= Src.m_bStaticPosition;
	m_Position				= Src.m_Position;
	m_bStaticRotation		= Src.m_bStaticRotation;
	m_Rotation				= Src.m_Rotation;
	m_bPosFacingRelative	= Src.m_bPosFacingRelative;
	m_bRotFacingRelative	= Src.m_bRotFacingRelative;
	m_bScaled				= Src.m_bScaled;
	m_bMirrored				= Src.m_bMirrored;
	m_bUseLinkedPos			= Src.m_bUseLinkedPos;
	m_bUseLinkedRot			= Src.m_bUseLinkedRot;
	m_bMirrorWithAnim		= Src.m_bMirrorWithAnim;
}

//=============================================================================
void CPropDef::Serialize( CieArchive& a )
{
	ASSERT(m_pCharacter!=NULL);

	if( a.IsStoring() )
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_PROPDEF);

		a.WriteTaggedbool( IE_TAG_PROPDEF_ACTIVE, m_bPropActive );
		a.WriteTaggedbool( IE_TAG_PROPDEF_EXPORT, m_bPropExported );
		a.WriteTaggedString( IE_TAG_PROPDEF_NAME, m_PropName );
		a.WriteTaggedString( IE_TAG_PROPDEF_BONENAME, m_BoneName );
		a.WriteTaggedbool( IE_TAG_PROPDEF_STATICPOS, m_bStaticPosition );
		a.WriteTaggedvector3( IE_TAG_PROPDEF_POSITION, m_Position );
		a.WriteTaggedbool( IE_TAG_PROPDEF_STATICROT, m_bStaticRotation );
		a.WriteTaggedvector3( IE_TAG_PROPDEF_ROTATION, *((vector3*)&m_Rotation) );
		a.WriteTaggedbool( IE_TAG_PROPDEF_POSFACINGREL, m_bPosFacingRelative );
		a.WriteTaggedbool( IE_TAG_PROPDEF_ROTFACINGREL, m_bRotFacingRelative );
		a.WriteTaggedbool( IE_TAG_PROPDEF_SCALED, m_bScaled );
		a.WriteTaggedbool( IE_TAG_PROPDEF_MIRRORED, m_bMirrored );
		a.WriteTaggedbool( IE_TAG_PROPDEF_USELINKEDPOS, m_bUseLinkedPos );
		a.WriteTaggedbool( IE_TAG_PROPDEF_USELINKEDROT, m_bUseLinkedRot );
		a.WriteTaggedbool( IE_TAG_PROPDEF_MIRRORWITHANIM, m_bMirrorWithAnim );

		//---	if there is a linked character, save it out now
		if( m_pLinkCharacter )
		{
			s32 Index = m_pCharacter->m_pDoc->m_CharacterList.PtrToIndex( (CObject*)m_pLinkCharacter ) ;
			if( Index != -1 )
				a.WriteTaggeds32( IE_TAG_PROPDEF_LINKCHARACTER, Index+1 ) ;
		}
		
		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_PROPDEF_END ) ;
	}
	else
	{
		bool dummy_bool;

		//---	Load Prop definition
		BOOL	done = FALSE ;

		while (!done)
		{
			s32		Index ;
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_PROPDEF:
				break;

			case IE_TAG_PROPDEF_ACTIVE:
				a.Readbool( m_bPropActive );
				break;

			case IE_TAG_PROPDEF_EXPORT:
				a.Readbool( m_bPropExported ) ;
				break ;

			case IE_TAG_PROPDEF_NAME:
				a.ReadString( m_PropName ) ;
				break ;

			case IE_TAG_PROPDEF_BONENAME:
				a.ReadString( m_BoneName ) ;
				break ;

			case IE_TAG_PROPDEF_STATICPOS:
				a.Readbool( m_bStaticPosition) ;
				break ;

			case IE_TAG_PROPDEF_POSITION:
				a.Readvector3( m_Position ) ;
				break ;

			case IE_TAG_PROPDEF_STATICROT:
				a.Readbool( m_bStaticRotation ) ;
				break ;

			case IE_TAG_PROPDEF_ROTATION:
				a.Readvector3( *((vector3*)&m_Rotation) ) ;
				break ;

			case IE_TAG_PROPDEF_LINKCHARACTER:
				a.Reads32( Index ) ;
				m_pLinkCharacter = (CCharacter*)Index ;
				break ;

			case IE_TAG_PROPDEF_POSFACINGREL:
				a.Readbool( m_bPosFacingRelative );
				break ;

			case IE_TAG_PROPDEF_ROTFACINGREL:
				a.Readbool( m_bRotFacingRelative );
				break ;

			case IE_TAG_PROPDEF_SCALED:
				a.Readbool( m_bScaled );
				break ;

			case IE_TAG_PROPDEF_MIRRORED:
				a.Readbool( m_bMirrored );
				break ;

			case IE_TAG_PROPDEF_MIRRORWITHANIM:
				a.Readbool( m_bMirrorWithAnim );
				break ;

			case IE_TAG_PROPDEF_MIRRORROT:
				a.Readbool( dummy_bool );
				break ;

			case IE_TAG_PROPDEF_USELINKEDPOS:
				a.Readbool( m_bUseLinkedPos );
				break ;

			case IE_TAG_PROPDEF_USELINKEDROT:
				a.Readbool( m_bUseLinkedRot );
				break ;

			case IE_TAG_PROPDEF_END:
				done = TRUE ;
				break ;

			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}

//=================================================================================
s32 CPropDef::GetBone( void )
{
	ASSERT((m_BoneID < 100)&&(m_BoneID >= -1));

	//---	if the bone index hasn't yet been found, find it now
	if( (m_BoneID == -1) && (m_BoneName.GetLength() > 0) )
	{
		if( m_pCharacter->GetSkeleton() )
			m_BoneID = m_pCharacter->GetSkeleton()->FindBoneFromName( m_BoneName );
	}

	return m_BoneID;
}

//=================================================================================
CCharacter* CPropDef::GetLinkedCharacter( void )
{
	//---	if the linked character is holding an index value, convert it into a character pointer.
	if( m_pLinkCharacter && ( (u32)m_pLinkCharacter <= MAX_LINK_CHARACTER_INDEX ))
		m_pLinkCharacter = (CCharacter*)m_pCharacter->m_pDoc->m_CharacterList.IndexToPtr( ((s32)m_pLinkCharacter)-1 ) ;

	return m_pLinkCharacter;
}

//=================================================================================
void CPropDef::SetBoneName( const CString& Name )
{
	m_BoneName	= Name;
	if( GetCharacter()->GetSkeleton() )
	{
		m_BoneID	= GetCharacter()->GetSkeleton()->FindBoneFromName( Name );
		ASSERT((m_BoneID < 100)&&(m_BoneID >= -1));
	}

	++m_RuntimeVersion;
}

//=================================================================================
void CPropDef::SetBone( s32 iBone )
{
	SSkelBone* pBone	= NULL;

	ASSERT((iBone<100)&&(iBone>=-1));

	if( GetCharacter()->GetSkeleton() && (iBone!=-1) )
		pBone = GetCharacter()->GetSkeleton()->GetBoneFromIndex( iBone );
	else
		pBone = NULL;

	if( pBone )
	{
		m_BoneName	= pBone->BoneName;
		m_BoneID	= iBone;
	}
	else
	{
		m_BoneName	= "";
		m_BoneID	= -1;
	}

	++m_RuntimeVersion;
}

//=================================================================================
void CPropDef::SetLinkedCharacter( const CCharacter* pCharacter )
{
	if( m_pLinkCharacter == pCharacter )
		return;

	//---	set the new character
	m_pLinkCharacter	= (CCharacter*)pCharacter;

	//---	go through every motion in this character and reset its motion data.
	//		Try to find the equivilent motion within the new character.
	s32 NumMotions	= m_pCharacter->GetNumMotions();
	s32 i;
	CMotion*	pMotion;
	CMotion*	pLinkMotion;
	for( i=0; i<NumMotions; i++ )
	{
		pMotion = m_pCharacter->GetMotion( i );

		if( m_pLinkCharacter )
			pLinkMotion = m_pLinkCharacter->FindMotionByExportName( pMotion->GetExportName() );
		else
			pLinkMotion	= NULL;

		pMotion->GetProp( GetIndex() ).SetLinkedMotion( pLinkMotion );
	}
}



//=============================================================================
//=============================================================================
//=============================================================================
//		PROP DATA FUNCTIONS
//=============================================================================
//=============================================================================
//=============================================================================

//=============================================================================
CPropData::CPropData()
{
	m_RuntimeVersion	= -1;
	m_bGeneratedStreams	= FALSE;

	m_bPropActive		= FALSE;
	m_Index				= -1;
	m_pMotion			= NULL;

	m_pLinkMotion		= NULL;

	m_pPosition			= NULL;
	m_pRotation			= NULL;
}

//=============================================================================
CPropData::~CPropData()
{
	ResetMotionData();
}

//=============================================================================
void CPropData::ResetMotionData( void )
{
	if( m_pPosition )
	{
		free( m_pPosition );
		m_pPosition	= NULL;
	}

	if( m_pRotation )
	{
		free( m_pRotation );
		m_pRotation = NULL;
	}
}

//=============================================================================
void CPropData::Copy( const CPropData &Src )
{
	m_bPropActive	= Src.m_bPropActive;
	m_bPropExported	= Src.m_bPropExported;
	m_pLinkMotion	= Src.m_pLinkMotion;

	//---	reset the motion data so that it will be recalculated the next time
	//		that it is accessed.
	ResetMotionData();
}

//=============================================================================
void CPropData::Serialize( CieArchive& a )
{
	ASSERT(m_pMotion!=NULL);

	if( a.IsStoring() )
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_PROPDATA);

		a.WriteTaggedbool( IE_TAG_PROPDATA_ACTIVE, m_bPropActive );
		a.WriteTaggedbool( IE_TAG_PROPDATA_EXPORT, m_bPropExported );
		
		//---	if there is a linked character, save it out now
		if( m_pLinkMotion )
		{
			s32 Index = GetLinkedCharacter()->m_MotionList.PtrToIndex( m_pLinkMotion ) ;
			if( Index != -1 )
				a.WriteTaggeds32( IE_TAG_PROPDATA_LINKMOTION, Index+1 ) ;	// add one so that any boolean checks will come up TRUE when loaded
		}
		
		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_PROPDATA_END ) ;
	}
	else
	{
		//---	Load Motion
		BOOL	done = FALSE ;

		while (!done)
		{
			s32		Index ;
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_PROPDATA:
				break;

			case IE_TAG_PROPDATA_ACTIVE:
				a.Readbool( m_bPropActive ) ;
				break ;

			case IE_TAG_PROPDATA_EXPORT:
				a.Readbool( m_bPropExported ) ;
				break ;

			case IE_TAG_PROPDATA_LINKMOTION:
				a.Reads32( Index ) ;
				m_pLinkMotion = (CMotion*)Index ;	// do not subtract the 1 that was added at save.  It will be removed when the pointer is fixed up.
				break ;

			case IE_TAG_PROPDATA_END:
				done = TRUE ;
				break ;

			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}

//=================================================================================
const vector3* CPropData::GetPositionStream( void )
{
	//---	recalculate the positions
	if( !m_pPosition || !CheckVersion() )
	{
		ResetMotionData();
		GeneratePropMotion();
	}

	return m_pPosition;
}

//=================================================================================
const radian3* CPropData::GetRotationStream( void )
{
	//---	recalculate the rotations
	if( !m_pPosition || !CheckVersion() )
	{
		ResetMotionData();
		GeneratePropMotion();
	}

	return m_pRotation;
}

//=================================================================================
vector3 CPropData::GetPosition( f32 Frame )
{
	//---	if the prop data was requested outside of this motion, return zero
	if( (Frame < 0.0f) || (Frame >= (m_pMotion->GetNumFrames()-1)) )
		return GetPositionStream()[ m_pMotion->GetNumFrames()-1 ];

	//---	recalculate the positions
	s32 Frame0		= (s32)Frame;
	f32 Frac		= Frame - (f32)Frame0;
	vector3 Pos0	= GetPositionStream()[ Frame0 ];
	vector3 Pos1	= GetPositionStream()[ Frame0+1 ];
	vector3 RetPos	= Pos0 + (Pos1-Pos0)*Frac;

	return RetPos;
}

//=================================================================================
radian3 BlendXYZUsingQuaternions( const radian3& FromXYZ, const radian3& ToXYZ, f32 t );

//=================================================================================
radian3 CPropData::GetRotation( f32 Frame )
{
	//---	if the prop data was requested outside of this motion, return zero
	if( (Frame < 0.0f) || (Frame >= (m_pMotion->GetNumFrames()-1)) )
		return GetRotationStream()[ m_pMotion->GetNumFrames()-1 ];

	//---	recalculate the positions
	s32 Frame0		= (s32)Frame;
	f32 Frac		= Frame - (f32)Frame0;

	//---	some angles weren't being turned into quaternions and back properly, this makes sure
	//		that we don't do it unless necessary.
	if( Frac == 0.0f )
		return GetRotationStream()[ Frame0 ];

	radian3 Rot0	= GetRotationStream()[ Frame0 ];
	radian3 Rot1	= GetRotationStream()[ Frame0+1 ];
	
	radian3 RetPos	= BlendXYZUsingQuaternions( Rot0, Rot1, Frac );

	return RetPos;
}

//=================================================================================
void CPropData::GeneratePropMotion( void )
{
	s32	i;
	vector3	Pos;
	radian3	Rot;
	matrix4	Mat;

	//---	clear any previously allocated prop motion data
	ResetMotionData();

	//---	if there is no motion, the prop motion cannot be generated
	if( !m_pMotion )
		return;

	//---	if the prop streams were generated from merging several props, they cannot be 
	//		regenerated here.  ASSERT that they are not currently holding generated motion
	ASSERT(!m_bGeneratedStreams);
	if( m_bGeneratedStreams )
		return;

	//---	get the number of frames of data which will be generated
	s32	NFrames	= m_pMotion->GetNumFrames();

	//---	allocate space for the new data
	m_pPosition	= (vector3*)malloc( NFrames*sizeof(vector3) );
	m_pRotation	= (radian3*)malloc( NFrames*sizeof(radian3) );

	//---
	//
	//	Generate new prop motion data.  This prop data is the position and rotation
	//	of the prop relative to a bone or (if bone is -1) the animation origin.  At
	//	render time, the bone's actual position and rotation must be applied to these
	//	position/rotation values to place the prop into the correct location.
	//
	//---

	//
	//	If there is no definition, clear the data and return now.
	//
	CPropDef* pPropDef = GetDefinition();
	if( !pPropDef )
	{
		memset( m_pPosition, 0, NFrames*sizeof(vector3) );
		memset( m_pRotation, 0, NFrames*sizeof(radian3) );
		return;
	}

	//
	//	Store the real-time version of the prop data
	//
	m_RuntimeVersion = pPropDef->GetRuntimeVersion();

	//
	//	If the prop has been given a linked motion, the motion data may have to be
	//	transformed if the prop is also relative to a bone.  First, load all of the
	//	animation data into the streams.
	//
	s32	NumLinkedMotionFrames	= 0;
	if( GetLinkedMotion() )
	{
		//---	save the current motion and frame of the character
		CMotion* pCharCurMotionSave	= GetCharacter()->GetCurMotion();
		f32		 CharCurFrameSave	= GetCharacter()->GetCurFrame();

		//---	Get the number of frames.
		NumLinkedMotionFrames	= GetLinkedMotion()->GetNumFrames();
		if( NumLinkedMotionFrames > NFrames )
			NumLinkedMotionFrames = NFrames;

		//---	copy/convert the motion data
		for( i=0; i<NumLinkedMotionFrames; i++ )
		{
			//---	get the position/rotation of the prop on this frame of the motion.
			GetLinkedCharacter()->SetPose( GetLinkedMotion(), (f32)i );
			Mat = *GetLinkedCharacter()->GetSkeleton()->GetPosedBoneMatrix( 0 );
			Pos = Mat.GetTranslation();
			Rot = Mat.GetRotationXYZ();


			//---	if the prop is supposed to be relative to a bone, convert the data
			if( pPropDef->GetBone() != -1 )
			{
				GetCharacter()->SetPose( GetMotion(), (f32)i );
				Mat = *GetCharacter()->GetSkeleton()->GetPosedBoneMatrix( pPropDef->GetBone() );

				vector3 BonePos = Mat.GetTranslation();
				radian3 BoneRot = Mat.GetRotationXYZ();

				//---	if the rotation should be exactly the same as the motion provides, convert the motion's rotation to be
				//		relative to the bone's using quaternions for the best result.
				if( pPropDef->IsUseLinkedRotation() )
				{
#if 0
					Rot -= BoneRot;
#else
					quaternion p, b, q;
					p.SetupRotateXYZ( Rot );
					b.SetupRotateXYZ( BoneRot );

					// starting with p = b*q;  -OR- finalproprot = bonerot*relativerot.
					// try solving for q by the following: q = b.Inverse()*p
					quaternion ib = b;
					ib.Invert();
					matrix4 m;
					p.Transform( ib );
					p.BuildMatrix( m );
					Rot = m.GetRotationXYZ();
#endif
				}

				//---	if the position should be exactly the same as the motion provides, convert the motion's position to be
				//		relative to the bone's
				if( pPropDef->IsUseLinkedPosition() )
				{
					Pos -= BonePos;
			
					//---	reverse rotate the relative position to the bone so that it can be re-calculated later.
					Pos.RotateZ( -BoneRot.Z );
					Pos.RotateY( -BoneRot.Y );
					Pos.RotateX( -BoneRot.X );
				}
			}

			//---	store the position/rotation of the prop for this frame
			m_pPosition[i] = Pos;
			m_pRotation[i] = Rot;
		}

		//---	restore the current motion and frame of the character
		GetCharacter()->SetPose( pCharCurMotionSave, CharCurFrameSave, TRUE );
	}

	//
	//	Make sure that any position/rotation values which have not yet been set are set to
	//	the prop definition's regular values.
	//
	for( i=NumLinkedMotionFrames; i<NFrames; i++ )
	{
		m_pPosition[i] = pPropDef->GetPosition();
		m_pRotation[i] = pPropDef->GetRotation();
	}
}

//=================================================================================
CPropDef* CPropData::GetDefinition( void ) const
{
	if( GetMotion() && GetMotion()->m_pCharacter )
		return &GetMotion()->m_pCharacter->m_PropDefs[GetIndex()];

	return NULL;
}

//=================================================================================
CMotion* CPropData::GetLinkedMotion( void )
{
	//---	If an index value is currently sitting in the m_pLinkMotion pointer (as would be the case right after a file
	//		load), convert it into an actual pointer.
	if( m_pLinkMotion && ((u32)m_pLinkMotion <= MAX_LINK_MOTION_INDEX ) )
	{
		if( !GetDefinition() || !GetDefinition()->GetLinkedCharacter() )
			m_pLinkMotion	= NULL;
		else
			m_pLinkMotion	= (CMotion*)GetDefinition()->GetLinkedCharacter()->m_MotionList.IndexToPtr( ((s32)m_pLinkMotion)-1 ) ;
	}

	return (CMotion*)m_pLinkMotion;
}

//=================================================================================
void CPropData::MergeProp( CPropData& rMergeProp, f32 fRatio )
{
	//---	if neither motion has unique prop data, there is nothing to be blended
	if( !HasUniquePropData() && !rMergeProp.HasUniquePropData() )
		return;

	//---	get storage space for the new position, rotation streams
	vector3*	pPosStream	= (vector3*) malloc( sizeof(vector3)*GetMotion()->GetNumFrames() );
	radian3*	pRotStream	= (radian3*) malloc( sizeof(radian3)*GetMotion()->GetNumFrames() );

	//---	Blend the data from each prop motion into new data streams
	ASSERT(GetMotion());
	ASSERT(rMergeProp.GetMotion());
	f32	FrameStep		= (f32)(rMergeProp.GetMotion()->GetNumFrames()-1) / (f32)(GetMotion()->GetNumFrames()-1);
	f32	InterpFrame;
	for( s32 i=0; i<GetMotion()->GetNumFrames(); i++ )
	{
		InterpFrame	= i*FrameStep;
		vector3	Pos0	= GetPosition( (f32)i );
		vector3 Pos1	= rMergeProp.GetPosition( InterpFrame );
		pPosStream[i]	= Pos0 + fRatio*( Pos0 - Pos1 );

		radian3	Rot0	= GetRotation( (f32)i );
		radian3 Rot1	= rMergeProp.GetRotation( (f32)i );
		pRotStream[i]	= BlendXYZUsingQuaternions( Rot0, Rot1, fRatio );
	}

	//---	store the new position, rotation stream data into this prop's motion streams
	if( m_pPosition ) free( m_pPosition );
	if( m_pRotation ) free( m_pRotation );
	m_pPosition	= pPosStream;
	m_pRotation	= pRotStream;

	//---	flag that this prop's data has been computated as the result of a merge so that we will know that it cannot
	//		be reset from within, but must be recreated.
	m_bGeneratedStreams	= TRUE;
}

//=================================================================================
void CPropData::ResampleProp( s32 OldNFrames, s32 NewNFrames )
{
	//---	The number of frames of the motion cannot have been changed before this resampling process is done because
	//		the GetPosition() and GetRotation() functions will not work properly
	ASSERT( OldNFrames == m_pMotion->GetNumFrames() );

	//---	if the motion doesn't have unique prop data, there is nothing to be done here,
	//		just reset the version number so that it will be recalculated
	if( !HasUniquePropData() )
	{
		ResetMotionData();
		return;
	}

	//---	get storage space for the new position, rotation streams
	vector3*	pPosStream	= (vector3*) malloc( sizeof(vector3)*NewNFrames );
	radian3*	pRotStream	= (radian3*) malloc( sizeof(radian3)*NewNFrames );

	//---	Blend the data from each prop motion into new data streams
	ASSERT(GetMotion());
	f32	FrameStep		= (f32)(OldNFrames-1) / (f32)(NewNFrames-1);
	f32	InterpFrame;
	for( s32 i=0; i<NewNFrames; i++ )
	{
		InterpFrame		= i*FrameStep;

		pPosStream[i]	= GetPosition( InterpFrame );
		pRotStream[i]	= GetRotation( InterpFrame );
	}

	//---	store the new position, rotation stream data into this prop's motion streams
	free( m_pPosition );
	free( m_pRotation );
	m_pPosition	= pPosStream;
	m_pRotation	= pRotStream;

	//---	the prop data has been sampled down, from now on it cannot be regenerated from its original sources properly, mark
	//		it as such so that we know that this is the case.
	m_bGeneratedStreams	= TRUE;
}

//=================================================================================
void CPropData::MimicFinalExportQuantizationResult( void )
{
	matrix4		Mat;
	quaternion	Quat;
	radian3		Rot;
	vector3		Pos;

	//---	if the prop data isn't unique, it will be always read from the character header a default relationship which is not be compressed,
	//		so there is nothing to do here, return.
	if( !HasUniquePropData() )
		return;

	//---	get the number of export bits for this motion
	s32	ExportBits = m_pMotion->GetExportBits();

	f32	ExportAngleConversion	= ((f32)(1<<ExportBits))/R_360;
	f32	ExportQuatConversion	= (f32)( (1L<<m_pMotion->GetExportBits())/2 - 1 );
	f32	ExportQuatWConversion	= (f32)( (1L<<m_pMotion->GetExportBits()) - 1 );
	f32	OneOverExportAngleConversion;
	f32	OneOverExportQuatConversion;
	f32	OneOverExportQuatWConversion;

	if( ExportAngleConversion == 0.0f )	OneOverExportAngleConversion = 0.0f;
	else								OneOverExportAngleConversion = 1.0f/ExportAngleConversion;
	if( ExportQuatConversion == 0.0f )	OneOverExportQuatConversion = 0.0f;
	else								OneOverExportQuatConversion = 1.0f/ExportQuatConversion;
	if( ExportQuatWConversion == 0.0f )	OneOverExportQuatWConversion = 0.0f;
	else								OneOverExportQuatWConversion = 1.0f/ExportQuatWConversion;

    //---   Build matrices and compute global XZY rotations
    for( s32 i=0; i<m_pMotion->GetNumFrames(); i++)
    {
        // Place new rotation in animation
		Rot = m_pRotation[i];

		if( m_pMotion->GetExportQuaternion() )
		{
			Quat.SetupRotateXYZ( Rot );
			if( Quat.W < 0.0f )
				Quat.Negate();
			Quat.Normalize();
			
			//---	mimic compression on the quaternion.
			Quat.X = (((f32)((s32)((Quat.X*ExportQuatConversion)+ExportQuatConversion)))-ExportQuatConversion)*OneOverExportQuatConversion;
			Quat.Y = (((f32)((s32)((Quat.Y*ExportQuatConversion)+ExportQuatConversion)))-ExportQuatConversion)*OneOverExportQuatConversion;
			Quat.Z = (((f32)((s32)((Quat.Z*ExportQuatConversion)+ExportQuatConversion)))-ExportQuatConversion)*OneOverExportQuatConversion;

			//---	calculate the W value based on the X,Y,Z values of the quaterion.
			Quat.W = (f32)sqrt( 1.0f - (Quat.X*Quat.X + Quat.Y*Quat.Y + Quat.Z*Quat.Z) );

			//---	if this W value is to be exported, quantize it as well.
			if( m_pMotion->GetExportQuatW() )
				Quat.W = ((f32)((s32)(Quat.W*ExportQuatWConversion)))*OneOverExportQuatWConversion;

			Quat.BuildMatrix( Mat );
			Rot = Mat.GetRotationXYZ();
		}
		else
		{
			//---	shift the values by the bit compression
			Rot.X = ((s32)(Rot.X * ExportAngleConversion))*OneOverExportAngleConversion;
			Rot.Y = ((s32)(Rot.Y * ExportAngleConversion))*OneOverExportAngleConversion;
			Rot.Z = ((s32)(Rot.Z * ExportAngleConversion))*OneOverExportAngleConversion;
		}

        // Place new rotation in animation
		m_pRotation[i] = Rot;

		// quantize the position value (should have very little affect)
		Pos = m_pPosition[i];
		Pos.Scale( 16.0f );
		Pos.X = (f32)((s16)Pos.X);
		Pos.Y = (f32)((s16)Pos.Y);
		Pos.Z = (f32)((s16)Pos.Z);
		Pos.Scale( 1.0f / 16.0f );
		m_pPosition[i] = Pos;
    }


	//---	the prop data has been modified from its original version, it will no longer be able to be reset and computed on the fly,
	//		flag that this is the case.
	m_bGeneratedStreams	= TRUE;
}
