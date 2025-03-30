// CharacterExport.cpp: implementation of the CCharacterExport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "CharacterExport.h"
#include "acccmp.h"
#include "CeDDoc.h"
#include "Auxiliary\\AnimPlay\\AM_Export.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#include "mfcutils.h"


//#define MISC_MOVEDIR            0
//#define MISC_FACEDIR            1
//#define MISC_PROP_TX            2
//#define MISC_PROP_TY            3
//#define MISC_PROP_TZ            4
//#define MISC_PROP_RX            5
//#define MISC_PROP_RY            6
//#define MISC_PROP_RZ            7
//#define MISC_BUBBLE_TORSO       8
//#define MISC_BUBBLE_LLEG        9
//#define MISC_BUBBLE_RLEG        10

#define ANIM_FRAMES_PER_BLOCK	32

#define	UNCOMPRESS_MOTION_DATA	0

#define ANIMGROUP_VERSION       7

CExportProgress	*CCharacterExport::m_pProgressDialog = NULL ;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCharacterExport::CCharacterExport( int Target )
{
	m_nErrors				= 0 ;
							
	m_Target				= Target ;
	m_DefineID				= 1 ;
	m_nBones				= 0 ;
	m_BlockOffset			= 0 ;
	m_EventIndex			= 0 ;
	m_nAnimBytes			= 0 ;
	m_nAnimations			= 0 ;
	m_nAnimEvents			= 0 ;
	m_nAnimBlocks			= 0 ;
	m_nAnimStreams			= 0 ;
							
	m_CompMotionsArray		= NULL;
	m_CompBlockSizesArray	= NULL;
}

//////////////////////////////////////////////////////////////////////

CCharacterExport::~CCharacterExport()
{
}

//////////////////////////////////////////////////////////////////////

CString CCharacterExport::MakeDefine( const CString& Prefix, const CString& String )
{
	CString Define = Prefix ;

	//---	Build DEFINE with no Illegal characters
	for( int i = 0 ; i < String.GetLength() ; i++ )
	{
		char c = String[i] ;
		if( !__iscsym(c) )
			c = '_' ;
		Define += c ;
	}

	return Define ;
}

//////////////////////////////////////////////////////////////////////

bool CCharacterExport::FindDefine( s16 *pID, CString String )
{
	bool NewID = true ;
	s16 FoundID = 0 ;

	//---	Try to find a match in the list of Strings
	for( int i = 1 ; i < m_DefineID ; i++ )
	{
		if( m_DefineArray[i] == String )
		{
			FoundID = i ;
			NewID = false ;
			break ;
		}
	}

	//---	Not found, so add new element
	if( FoundID == 0 )
	{
		FoundID = m_DefineID ;
		m_DefineArray.SetAtGrow( m_DefineID, String ) ;
		m_DefineID++ ;
	}

	*pID = FoundID ;
	return NewID ;
}


//////////////////////////////////////////////////////////////////////
inline void AddAngleNormalized( f32& Dir, f32 RotY )
{
	Dir += RotY;
	while( Dir>R_360 )	Dir -= R_360;
	while( Dir<0.0f )	Dir += R_360;
}

//////////////////////////////////////////////////////////////////////
void DisplayAnimMB( CMotion* pMotion, anim* pAnim, s32 Mode, const char* ModeName );

//////////////////////////////////////////////////////////////////////
void CCharacterExport::ExportMotionBlocks( s32			MotionIndex,
										   CMotion*		pMotion,
										   CCharacter*	pCharacter,
										   s32			FramesPerBlock,
										   s32&			rNumExportedFrames,
										   s32&			rCompDataSize,
										   s32&			rCompDataBlocks,
										   u8			PropsExported,
										   matrix4&		Mat )
{
    s16*			RawData;
    byte*			CompData;
    s32				RawDataSize;
    s32				CompDataSize;
    s32				i,j;
    anim*			pAnim;
    CSkel*			pSkel;
	CExportFile*	pCompData		= &m_CompMotionsArray[ MotionIndex ];
	CByteArray*		pCompBlockSizes	= &m_CompBlockSizesArray[ MotionIndex ];
	radian			RotY = Mat.GetRotation().Yaw;
	vector3			vTrans;
	f32				W;

	//---	if the data has already been compressed, add the compressed block sizes to the export, then return
	if( pCompData->GetLength() )
	{
		for( i=0; i<pCompBlockSizes->GetSize(); i++ )
			anim_BlockSize.Writeu8( pCompBlockSizes->GetAt(i) ) ;
	
		rCompDataBlocks = pCompBlockSizes->GetSize() ;

		rCompDataSize = pCompData->GetLength();
	}


	//****************************************************************************************
	//*	RESAMPLE SAMPLE THE MOTION DATA TO THE EXPORT FPS
	//****************************************************************************************

	//---	copy the motion to a new motion which will be compressed
	CMotion ExportMotion;
	ExportMotion = *pMotion;
	ExportMotion.MakeIntoCompressedVersion( FALSE );
	pMotion = &ExportMotion;
	pAnim = pMotion->GetAnim();

	s32 MoveDirStream = 3*(pAnim->NBones+1);
	s32 FaceDirStream = MoveDirStream+1;
	for( j = 0 ; j < pMotion->GetNumFrames() ; j++ )
	{
		ANIM_SetRotation( pAnim, j, MoveDirStream, pMotion->GetMoveDir(j) ) ;
		ANIM_SetRotation( pAnim, j, FaceDirStream, pMotion->GetFaceDir(j) ) ;
	}

	rNumExportedFrames = pAnim->NFrames;

	//---	alias pointers to the skeleton information
    CSkeleton *pSkeleton = pCharacter->GetSkeleton() ;
    pSkel = pSkeleton->GetSkel();

	//---	prepare the compression buffer for the data
	pCompBlockSizes->RemoveAll();
	pCompData->Create( m_Target );
	pCompData->SeekToBegin();
	pCompData->SetLength( 0 );


	//****************************************************************************************
	//*	CONVERT ROTATION FORMAT INTO EXPORTED ROTATION FORMAT
	//****************************************************************************************

    //---   Convert bone rotations from local XYZ to global XZY
    matrix4 DestM[50];
    matrix4 WorldOrientM;
	f32		ExportAngleConversion	= (f32)(1<<pMotion->GetExportBits())/R_360;
	ASSERT(pAnim->NBones<=50);

    //---   Build WorldOrientM
	WorldOrientM.Identity();

    //---   Build matrices and compute global XZY rotations
	s32 NFrames = pAnim->NFrames;
	s32 NBones = pAnim->NBones;
	radian3	Rot;
	quaternion Quat;
	s32 qX, qY, qZ, qW;

	//---	if the export is to contain world rotations, put the root rotations back.
	if( pMotion->GetExportWorldRots() )
	{
		//---	lets put the original root rotations back into the animation before compressing it.
		for( i=0; i<pMotion->GetNumFrames(); i++ )
		{
			ANIM_SetRotation( pAnim, i, 0, pMotion->m_pRootRotsSave[i].X );
			ANIM_SetRotation( pAnim, i, 1, pMotion->m_pRootRotsSave[i].Y );
			ANIM_SetRotation( pAnim, i, 2, pMotion->m_pRootRotsSave[i].Z );
		}
	}

//DisplayAnimMB( pMotion, pMotion->GetAnim(), 1, "Before Export" );

	if( pMotion->GetExportQuaternion() )
	{
		//---	calculate the 
		f32	ExportQuatConversion	= (f32)( (1L<<pMotion->GetExportBits())/2 - 1 );
		f32 ExportQuatWConversion	= (f32)( (1L<<pMotion->GetExportBits()) - 1 );
		f32	OneOverQuatConversion	= (ExportQuatConversion == 0.0f) ? 0.0f : 1.0f / ExportQuatConversion;
		s32	StreamsPerBone			= pMotion->GetExportQuatW() ? 4 : 3;

		//---	add enough streams to hold the new 4 value quaternions if the W stream is to be exported
		if( pMotion->GetExportQuatW() )
			ANIM_AddStreams( pAnim, 3*NBones, NBones, FALSE ); // not really rotation streams, but...

		for (i=0; i<NFrames; ++i)
		{
			// Compute local to world matrix
			if( pMotion->GetExportGlobalRots() )
				pCharacter->m_pSkeleton->BuildMatrices( pAnim, i, DestM, &WorldOrientM );

			//---	Create Quaternions for each euler rotation set.  Work backward from the end of the frame so that
			//		data isn't overwritten before it is referenced.
			for (j=NBones-1; j>=0; --j)
			{
				if( pMotion->GetExportGlobalRots() )
				{
					Quat.SetupMatrix( DestM[j] );

					//---	if we are using world based rotations, we will want to apply the export transformation
					if( pMotion->GetExportWorldRots() )
						Quat.RotateY( RotY );
				}
				else
				{
					Rot.Pitch	= ANIM_GetRotation( pAnim, i, 3*j + 0 );
					Rot.Yaw		= ANIM_GetRotation( pAnim, i, 3*j + 1 );
					Rot.Roll	= ANIM_GetRotation( pAnim, i, 3*j + 2 );
					Quat.SetupRotateXYZ( Rot );

					//---	if we are using world based rotations, we will want to apply the export transformation,
					//		but only if this is the root bone because it isn't global rotations
					if( (j==0) && pMotion->GetExportWorldRots() )
						Quat.RotateY( RotY );
				}

				//---	if the W value is negative, negate the quaternion (this doesn't change the direction of the quaternion
				//		but it will allow us to export the quaternion W value 
				if( Quat.W < 0.0f )
					Quat.Negate();

				//---	make sure the quaternion is normalized so that each value is between -1.0f..1.0f.
				Quat.Normalize();

				qX	= (s16)((Quat.X * ExportQuatConversion) + ExportQuatConversion);
				qY	= (s16)((Quat.Y * ExportQuatConversion) + ExportQuatConversion);
				qZ	= (s16)((Quat.Z * ExportQuatConversion) + ExportQuatConversion);

				//---	if we are going to export the quaternion W stream, calculate the exported W value using the newly quantized
				//		X, Y, Z values and then quantize W using all of the export bits relying on its value being from 0.0 to 1.0
				if( pMotion->GetExportQuatW() )
				{
					//---	recalculate W based on the X, Y, Z values so that the quat's length will still be 1.0 even with quantization
					Quat.X	= ((f32)qX - ExportQuatConversion) * OneOverQuatConversion;
					Quat.Y	= ((f32)qY - ExportQuatConversion) * OneOverQuatConversion;
					Quat.Z	= ((f32)qZ - ExportQuatConversion) * OneOverQuatConversion;

					W		= 1.0f - (Quat.X*Quat.X + Quat.Y*Quat.Y + Quat.Z*Quat.Z);
					if( W > 0.0f )	Quat.W	= (f32)sqrt( W );
					else			Quat.W	= 0.0f;
					qW	= (s16)(Quat.W*ExportQuatWConversion);
				}

				//---	Place quaternion data over the old rotation data.  Since quats are from -1 to 1, they conversion
				//		is a bit different from the angle conversion.
				ANIM_SetMiscValue( pAnim, i, StreamsPerBone*j + 0, (s16)qX );
				ANIM_SetMiscValue( pAnim, i, StreamsPerBone*j + 1, (s16)qY );
				ANIM_SetMiscValue( pAnim, i, StreamsPerBone*j + 2, (s16)qZ );
				if( pMotion->GetExportQuatW() )
				{
					ASSERT(StreamsPerBone==4);
					ANIM_SetMiscValue( pAnim, i, StreamsPerBone*j + 3, (s16)qW );
				}
			}
		}

		//---	the quternion streams are no longer rotations
		for( i=0; i<3*NBones; i++ )
			ANIM_SetStreamType( pAnim, i, FALSE );

		//---	transform all of the motion translations
		for( i=0; i<NFrames; i++ )
		{
			vTrans.X = (f32)ANIM_GetMiscValue( pAnim, i, StreamsPerBone*NBones+0 );
			vTrans.Y = (f32)ANIM_GetMiscValue( pAnim, i, StreamsPerBone*NBones+1 );
			vTrans.Z = (f32)ANIM_GetMiscValue( pAnim, i, StreamsPerBone*NBones+2 );
			vTrans.Scale( 1.0f / 16.0f );
			vTrans = Mat.Transform( vTrans );
			vTrans.Scale( 16.0f );
			ANIM_SetMiscValue( pAnim, i, StreamsPerBone*NBones+0, (s32)vTrans.X );
			ANIM_SetMiscValue( pAnim, i, StreamsPerBone*NBones+1, (s32)vTrans.Y );
			ANIM_SetMiscValue( pAnim, i, StreamsPerBone*NBones+2, (s32)vTrans.Z );
		}

		//---	convert MoveDir FaceDir
		s32 MoveDirStream = StreamsPerBone*NBones+3;
		s32 FaceDirStream = StreamsPerBone*NBones+4;
		f32 FaceDir, MoveDir;
		for( i=0; i<NFrames; i++ )
		{
			MoveDir = ANIM_GetRotation( pAnim, i, MoveDirStream );
			FaceDir = ANIM_GetRotation( pAnim, i, FaceDirStream );
			AddAngleNormalized( MoveDir, RotY );
			AddAngleNormalized( FaceDir, RotY );
			ANIM_SetMiscValue( pAnim, i, MoveDirStream, (s16)(MoveDir * ExportAngleConversion) );
			ANIM_SetMiscValue( pAnim, i, FaceDirStream, (s16)(FaceDir * ExportAngleConversion) );
		}
		
		s32		PropStream = StreamsPerBone*NBones+3/* anim trans x,y,z */+2/* face dir/move dir */;
		s32		NPropRotStreams	=	pMotion->GetExportQuatW() ? 4 : 3;
		s32		PropID = 0;
		bool	bTransformProp;

		//---	convert any exported prop-rotations to quaternions as well.
		while( PropsExported )
		{
			//---	If the first bit is on, there is an active prop;
			if( PropsExported&1 )
			{
				//---	determine if this prop needs to be transformed
				bTransformProp = pMotion->GetProp( PropID ).GetDefinition()->GetBone() == -1 ? TRUE : FALSE;

				//---	add a stream to make room for the quaternion format
				if( pMotion->GetExportQuatW() )
					ANIM_AddStreams( pAnim, PropStream+3, 1, FALSE );

				for( i=0; i<NFrames; i++ )
				{
					Rot.Pitch	= ANIM_GetRotation( pAnim, i, PropStream+0 );
					Rot.Yaw		= ANIM_GetRotation( pAnim, i, PropStream+1 );
					Rot.Roll	= ANIM_GetRotation( pAnim, i, PropStream+2 );
					Quat.SetupRotateXYZ( Rot );
					if( bTransformProp )
						Quat.RotateY( RotY );

					if( Quat.W < 0.0f )
						Quat.Negate();

					Quat.Normalize();

					qX	= (s16)((Quat.X * ExportQuatConversion) + ExportQuatConversion);
					qY	= (s16)((Quat.Y * ExportQuatConversion) + ExportQuatConversion);
					qZ	= (s16)((Quat.Z * ExportQuatConversion) + ExportQuatConversion);

					//---	if the W stream is being exported, recalcuate the W value based on the quantized X, Y, Z values and
					//		then quantize W assuming a value between 0.0 and 1.0
					if( pMotion->GetExportQuatW() )
					{
						//---	recalculate W based on the X, Y, Z values so that the quat's length will still be 1.0 even with quantization
						Quat.X	= ((f32)qX - ExportQuatConversion) * OneOverQuatConversion;
						Quat.Y	= ((f32)qY - ExportQuatConversion) * OneOverQuatConversion;
						Quat.Z	= ((f32)qZ - ExportQuatConversion) * OneOverQuatConversion;

						W		= 1.0f - (Quat.X*Quat.X + Quat.Y*Quat.Y + Quat.Z*Quat.Z);
						if( W > 0.0f )	Quat.W	= (f32)sqrt( W );
						else			Quat.W	= 0.0f;
						qW	= (s16)(Quat.W*ExportQuatWConversion);
					}

					//---	Place quaternion data over the old rotation data.  Since quats are from -1 to 1, they conversion
					//		is a bit different from the angle conversion.
					ANIM_SetMiscValue( pAnim, i, PropStream+0, (s16)qX );
					ANIM_SetMiscValue( pAnim, i, PropStream+1, (s16)qY );
					ANIM_SetMiscValue( pAnim, i, PropStream+2, (s16)qZ );
					if( pMotion->GetExportQuatW() )
						ANIM_SetMiscValue( pAnim, i, PropStream+3, (s16)qW );

					//---	transform the translation
    				vTrans.X = (f32)ANIM_GetMiscValue( pAnim, i, PropStream+NPropRotStreams+0 );
    				vTrans.Y = (f32)ANIM_GetMiscValue( pAnim, i, PropStream+NPropRotStreams+1 );
    				vTrans.Z = (f32)ANIM_GetMiscValue( pAnim, i, PropStream+NPropRotStreams+2 );

					if( bTransformProp )
					{
						vTrans.Scale( 1.0f / 16.0f );
						vTrans = Mat.Transform( vTrans );
						vTrans.Scale( 16.0f );
					}

    				ANIM_SetMiscValue( pAnim, i, PropStream+NPropRotStreams+0, (s32)vTrans.X );
    				ANIM_SetMiscValue( pAnim, i, PropStream+NPropRotStreams+1, (s32)vTrans.Y );
    				ANIM_SetMiscValue( pAnim, i, PropStream+NPropRotStreams+2, (s32)vTrans.Z );
				}

				//---	the first 3 streams are no longer rotation streams.
				ANIM_SetStreamType( pAnim, PropStream+0, FALSE );
				ANIM_SetStreamType( pAnim, PropStream+1, FALSE );
				ANIM_SetStreamType( pAnim, PropStream+2, FALSE );

				PropStream += NPropRotStreams + 3;	// Rots streams are either 3 or 4 depending on wether W stream
													// is exported, plus 3 for translation streams.
			}

			//---	shift the bits down (there is one bit on per exported prop,
			//		each prop has a set of rotations streams followed by a set of
			//		translations streams).
			PropsExported >>= 1;
			PropID++;
		}
	}
	else // export euler rotations
	{
		for (i=0; i<NFrames; ++i)
		{
			// Compute local to world matrix
			if( pMotion->GetExportGlobalRots() )
				pCharacter->m_pSkeleton->BuildMatrices( pAnim, i, DestM, &WorldOrientM );

			// Pull global XYZ rotations from matrix and convert to global XZY
			for (j=0; j<NBones; ++j)
			{
				if( pMotion->GetExportGlobalRots() )
				{
					Rot = DestM[j].GetRotationXYZ();

					Rot.XYZ_to_XZY(); // XZY because it is more efficient for building matrices
	
					//---	if we are using world based rotations, we will want to apply the export transformation
					if( pMotion->GetExportWorldRots() )
						Rot.Y += RotY;
				}
				else
				{
					Rot.Pitch	= ANIM_GetRotation( pAnim, i, 3*j + 0 );
					Rot.Yaw		= ANIM_GetRotation( pAnim, i, 3*j + 1 );
					Rot.Roll	= ANIM_GetRotation( pAnim, i, 3*j + 2 );

					Rot.XYZ_to_XZY(); // XZY because it is more efficient for building matrices

					//---	if we are using world based rotations, we will want to apply the export transformation
					//		but only if this isn't the root bone.
					if( (j==0) && pMotion->GetExportWorldRots() )
						Rot.Y += RotY;
				}

				// Place new rotation in animation
				ANIM_SetMiscValue( pAnim, i, 3*j + 0, (s16)(Rot.Pitch * ExportAngleConversion) );
				ANIM_SetMiscValue( pAnim, i, 3*j + 1, (s16)(Rot.Yaw * ExportAngleConversion) );
				ANIM_SetMiscValue( pAnim, i, 3*j + 2, (s16)(Rot.Roll * ExportAngleConversion) );
			}
		}
//DisplayAnimMB( pMotion, pMotion->GetAnim(), 2, "After Conversion" );

		//---	transform all of the motion translations
		for( i=0; i<NFrames; i++ )
		{
			ANIM_GetTransValue( pAnim, i, &vTrans );
			vTrans = Mat.Transform( vTrans );
			ANIM_SetTransValue( pAnim, i, &vTrans );
		}

		//---	convert MoveDir FaceDir
		s32 MoveDirStream = 3*NBones+3;
		s32 FaceDirStream = 3*NBones+4;
		f32 FaceDir, MoveDir;
		for( i=0; i<NFrames; i++ )
		{
			MoveDir = ANIM_GetRotation( pAnim, i, MoveDirStream );
			FaceDir = ANIM_GetRotation( pAnim, i, FaceDirStream );
			AddAngleNormalized( MoveDir, RotY );
			AddAngleNormalized( FaceDir, RotY );
			ANIM_SetMiscValue( pAnim, i, MoveDirStream, (s16)(MoveDir * ExportAngleConversion) );
			ANIM_SetMiscValue( pAnim, i, FaceDirStream, (s16)(FaceDir * ExportAngleConversion) );
		}
		
		s32	PropStream = 3*NBones+3/* anim trans x,y,z */+2/* face dir/move dir */;
		s32	PropID = 0;
		bool bTransformProp;

		//---	convert any exported prop-rotations to XZY as well.
		while( PropsExported )
		{
			//---	If the first bit is on, there is an active prop;
			if( PropsExported&1 )
			{
				//---	determine if this prop needs to be transformed
				bTransformProp = pMotion->GetProp( PropID ).GetDefinition()->GetBone() == -1 ? TRUE : FALSE;

				for( i=0; i<NFrames; i++ )
				{
					ASSERT(ANIM_StreamIsRotation( pAnim, PropStream+0 ));
					ASSERT(ANIM_StreamIsRotation( pAnim, PropStream+1 ));
					ASSERT(ANIM_StreamIsRotation( pAnim, PropStream+2 ));
					Rot.Pitch	= ANIM_GetRotation( pAnim, i, PropStream+0 );
					Rot.Yaw		= ANIM_GetRotation( pAnim, i, PropStream+1 );
					Rot.Roll	= ANIM_GetRotation( pAnim, i, PropStream+2 );
					Rot.XYZ_to_XZY(); // XZY because it is more efficient for building matrices
					if( bTransformProp )
						Rot.Y += RotY;

					ANIM_SetMiscValue( pAnim, i, PropStream + 0, (s16)(Rot.Pitch * ExportAngleConversion) );
					ANIM_SetMiscValue( pAnim, i, PropStream + 1, (s16)(Rot.Yaw * ExportAngleConversion) );
					ANIM_SetMiscValue( pAnim, i, PropStream + 2, (s16)(Rot.Roll * ExportAngleConversion) );

					//---	transform the translation
    				vTrans.X = (f32)ANIM_GetMiscValue( pAnim, i, PropStream+3 ) ;
    				vTrans.Y = (f32)ANIM_GetMiscValue( pAnim, i, PropStream+4 ) ;
    				vTrans.Z = (f32)ANIM_GetMiscValue( pAnim, i, PropStream+5 ) ;
					if( bTransformProp )
					{
						vTrans.Scale( 1.0f / 16.0f );
						vTrans = Mat.Transform( vTrans );
						vTrans.Scale( 16.0f );
					}
    				ANIM_SetMiscValue( pAnim, i, PropStream+3, (s32)vTrans.X ) ;
    				ANIM_SetMiscValue( pAnim, i, PropStream+4, (s32)vTrans.Y ) ;
    				ANIM_SetMiscValue( pAnim, i, PropStream+5, (s32)vTrans.Z ) ;
				}
	//DisplayAnimMB( pMotion, pMotion->GetAnim(), 3, "Prop Rotations" );

				PropStream += 6; // (3 Rot/3 Trans).
			}

			//---	shift the bits down (there is one bit on per exported prop,
			//		each prop has a set of rotations streams followed by a set of
			//		translations streams).
			PropsExported >>= 1;
			PropID++;
		}
	}
//DisplayAnimMB( pMotion, pMotion->GetAnim(), 4, "Display" );

	//---   Minimize deltas in rotation streams for maximum compression
	for (i=0; i<pAnim->NStreams; i++)
		if( ANIM_StreamIsRotation( pAnim, i ) )
			ANIM_MinimizeAngleDiff( pAnim, i, 1<<pMotion->GetExportBits() );



	//****************************************************************************************
	//*	EXPORT THE MOTION BLOCKS
	//****************************************************************************************

	m_nAnimStreams = pAnim->NStreams;

	f32 fNumBlocks = (f32)pAnim->NFrames / FramesPerBlock;
	s32 NumBlocks = (s32)fNumBlocks;
	if( fNumBlocks != (f32)NumBlocks )
		++NumBlocks;
    
    //---   Allocate Raw and Comp data blocks
    RawData = (s16*)x_malloc(sizeof(s16)*NumBlocks*FramesPerBlock*pAnim->NStreams);
    CompData = (byte*)x_malloc(sizeof(s16)*NumBlocks*FramesPerBlock*pAnim->NStreams);
    ASSERT(RawData && CompData);

	//---	Loop through Blocks
	s32 BlockCount=0;
	s32 IncFrames = FramesPerBlock-1;
	for( j = 0; j < pAnim->NFrames ; j+=IncFrames )
	{
		ASSERT(BlockCount < NumBlocks);

		//---	Determine First and Last Frame to Export to this Block
		s32	L			= j;
		s32	R			= L+IncFrames;
		s16 Value ;
		if( R >= pAnim->NFrames )
			R = pAnim->NFrames-1;

		//---	Export Block of Frames
        RawDataSize  = 0;
        CompDataSize = 0;

        /////////////////////////////////////////////////////////////
		//---	Loop through all Streams and load into RawData
        /////////////////////////////////////////////////////////////
		for( int nStream = 0 ; nStream < pAnim->NStreams ; nStream++ )
		{
			//---	Loop through All Frames in this Block and load into RawData
			for( int nFrame = L ; nFrame <= R ; nFrame++ )
			{

				ANIM_GetStreamValue( pAnim, nFrame, nStream, &Value ) ;
                RawData[RawDataSize++] = Value;
			}
		}

        /////////////////////////////////////////////////////////////
        //---   Compress RawData into CompData buffer
        /////////////////////////////////////////////////////////////
#if !UNCOMPRESS_MOTION_DATA
        CompDataSize = ACCCMP_Encode(CompData,
                                     RawData,
                                     pAnim->NStreams,
                                     (RawDataSize / pAnim->NStreams));
#else
        CompDataSize = RawDataSize*sizeof(s16);
        x_memcpy(CompData,RawData,CompDataSize);
#endif

        /////////////////////////////////////////////////////////////
		//---	Keep Record of beginning of Data in File
        /////////////////////////////////////////////////////////////
		s32 FilePosStart = pCompData->GetPosition();

        //---   Write the CompData buffer to file one byte at a time
		pCompData->Write( CompData, CompDataSize );

        //---   Pad block to 16 byte bounds
		pCompData->PadToBoundry( 16 );

		//---	Determine File Size
		s32 FilePosEnd = pCompData->GetPosition() ;
		CompDataSize = FilePosEnd - FilePosStart;

		//---	Add BlockSize to Table
        ASSERT( (CompDataSize/16) < 255);
        ASSERT( (CompDataSize%16) == 0 );
		pCompBlockSizes->Add( (u8)(CompDataSize/16) );
		if( (s32)(CompDataSize/16) >= 256 )
			m_pProgressDialog->FmtError( EXPORT_ERROR_COMPBLOCKTOOBIG, "\"%s\": Compressed animation block >= 4096 bytes", "ERROR - Block size overflow", pMotion->GetExportName() );
	}

	//---	store the block sizes into the export buffer
	for( i=0; i<pCompBlockSizes->GetSize(); i++ )
		anim_BlockSize.Writeu8( pCompBlockSizes->GetAt(i) ) ;
	rCompDataBlocks = pCompBlockSizes->GetSize() ;

    x_free(RawData);
    x_free(CompData);

	rCompDataSize = pCompData->GetLength();
}

//////////////////////////////////////////////////////////////////////
void CCharacterExport::ComputePropInfo(	CMotion*        pMotion, 
										CCharacter*     pCharacter,
										u8*             PropsUsedFlags,
										u8*				PropsWithStreamsFlags )
{
	s32 i;

	*PropsUsedFlags = 0;
	*PropsWithStreamsFlags = 0;
	for( i=0; i<MAX_NUM_PROPS; i++ )
	{
		if( pMotion->GetProp(i).IsFullyExported() )
			*PropsUsedFlags |= 1<<i;

		if( ComputeSpecificPropInfo( pMotion, pCharacter, i ) )
			*PropsWithStreamsFlags |= 1<<i;
	}
}
    
//////////////////////////////////////////////////////////////////////
bool CCharacterExport::ComputeSpecificPropInfo(	CMotion*        pMotion, 
												CCharacter*     pCharacter,
												s32				PropIndex )
{
    s32 j;
	anim*			pAnim = pMotion->GetAnim() ;
    CMotion*		pPropMotion;
    CCharacter*		pPropCharacter;
    anim*			pPropAnim;
	CPropData&		rProp		= (CPropData&) pMotion->GetProp(PropIndex);
	CPropDef*		pPropDef	= (CPropDef*) rProp.GetDefinition();
	radian3			R;
	vector3			T;

	//---	if this prop isn't supposed to be exported, exit now
	if( !rProp.IsFullyActive() || !rProp.IsFullyExported() )
		return FALSE;

	//---   Find next prop, fill out these three values, then
	//---   Execute the code below
	pPropCharacter = (CCharacter*)rProp.GetLinkedCharacter();
	pPropMotion    = (CMotion*)rProp.GetLinkedMotion();
	if ((pPropCharacter == NULL) || (pPropMotion == NULL))
	{
		return FALSE;
	}
	else
	{
		//---   Load the prop motion
		pPropMotion->m_bSupressWarningBoxes = TRUE;		// supress all possible warning boxes which may result from the next call
		pPropAnim  = pPropMotion->GetAnim();
		pPropMotion->m_bSupressWarningBoxes = FALSE;

		//---	if the prop motion wasn't loadable, inform the user and return FALSE
		if( !pPropAnim )
		{
			m_pProgressDialog->FmtError( EXPORT_ERROR_CANNOTLOADPROP, "\"%s\": Linked prop motion \"%s\" unloadable.", pMotion->GetExportName(), pPropMotion->GetExportName() ) ;
			return FALSE;
		}

		//---	if the prop motion doesn't have at least one bone, inform the user and return.
		if( pPropAnim->NBones < 1 )
		{
			m_pProgressDialog->FmtError( EXPORT_ERROR_NOBONESINPROP, "\"%s\": Linked prop motion \"%s\" has no bones.", pMotion->GetExportName(), pPropMotion->GetExportName() ) ;
			return FALSE;
		}

		//---   Send warning if prop and anim don't overlap correctly
		if ( ((pPropAnim->StartFrame + pPropAnim->NFrames) > (pAnim->StartFrame + pAnim->NFrames)) ||
			 (pPropAnim->StartFrame < pAnim->StartFrame) )
		{
			//---	Add Error Line
			m_pProgressDialog->Fmt( "WARNING - Motion and linked prop motion do not overlap properly" ) ;
			m_pProgressDialog->FmtError( EXPORT_WARNING_PROPBADOVERLAP, "\"%s\": Linked prop motion \"%s\" doesn't overlap properly.", pMotion->GetExportName(), pPropMotion->GetExportName() );
			m_nErrors++ ;
		}

		//---	since the export is to contain world rotations, put the root rotations back.
		pPropMotion->RestoreRootRots();

		//---   Add streams for prop info
		s32 RXS = ANIM_AddStream( pAnim, TRUE ) ;	// RX
		s32 RYS = ANIM_AddStream( pAnim, TRUE ) ;	// RY
		s32 RZS = ANIM_AddStream( pAnim, TRUE ) ;	// RZ
		s32 TXS = ANIM_AddStream( pAnim, FALSE ) ;	// TX
		s32 TYS = ANIM_AddStream( pAnim, FALSE ) ;	// TY
		s32 TZS = ANIM_AddStream( pAnim, FALSE ) ;	// TZ

		//---   ADD PROP FRAMES TO ANIM
		for( j = 0 ; j < pMotion->GetNumFrames() ; j++ )
		{
			s32   PropFrame;

			PropFrame = (j+pAnim->StartFrame) - pPropAnim->StartFrame;
			if ((PropFrame>=0) && (PropFrame<pPropAnim->NFrames))
			{
				R = rProp.GetRotation( (f32)j );
				T = rProp.GetPosition( (f32)j );
			}
			else
			{
				R = pPropDef->GetRotation();
				T = pPropDef->GetPosition();
			}

			T.Scale( 16 );

    		ANIM_SetRotation( pAnim, j, RXS, R.X ) ;
    		ANIM_SetRotation( pAnim, j, RYS, R.Y ) ;
    		ANIM_SetRotation( pAnim, j, RZS, R.Z ) ;
    		ANIM_SetMiscValue( pAnim, j, TXS, (s32)T.X ) ;
    		ANIM_SetMiscValue( pAnim, j, TYS, (s32)T.Y ) ;
    		ANIM_SetMiscValue( pAnim, j, TZS, (s32)T.Z ) ;
		}

		pPropMotion->SaveRootRots();

	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////

s16 BuildPointPosition(anim* Anim, CSkeleton* pSkeleton, s32 Frame, vector3 * Pt, s32 BoneID)
{
    s32         i;
    radian3     BoneRot[100];
    matrix4     BoneM[100];
    matrix4     WorldOrientM;
    vector3     PtRelativeToBone;
    vector3     PtInWorld;
    u32         X,Y,Z;
    s16         P;
    bool        UpBit;
	CSkel*		Skel = pSkeleton->GetSkel();

    // Load rotation values into array
    for (i=0; i<Skel->m_NBones*3; i+=3)
    {
        BoneRot[(i/3)].Pitch	= ANIM_GetRotation( Anim, Frame, i+0 );
        BoneRot[(i/3)].Yaw		= ANIM_GetRotation( Anim, Frame, i+1 );
        BoneRot[(i/3)].Roll		= ANIM_GetRotation( Anim, Frame, i+2 );
    }

    // Build WorldOrientM
	WorldOrientM.Identity();

    // Build Matrices for this frame
    pSkeleton->BuildMatrices( Anim, Frame, BoneM, &WorldOrientM );

    // Get point relative to bone
	PtRelativeToBone = *Pt - Skel->m_pBone[BoneID].OriginToBone;

    // Transform point into world using matrix
	BoneM[BoneID].Transform( &PtInWorld, &PtRelativeToBone, 1 );


    if (PtInWorld.Z > 0) 
    {
        UpBit = 1;
        PtInWorld.Z -= 20.0f;
    }
    else
    {
        PtInWorld.Z += 20.0f;
        UpBit = 0;
    }

    // Map range of -40<->40 into 0<->31
    {
        f32 L = -40.0f;
        f32 R =  40.0f;

        // Snap P0 into range of L<->R inches
        if (PtInWorld.X >  R) PtInWorld.X = R;
        if (PtInWorld.X <  L) PtInWorld.X = L;
        if (PtInWorld.Y >  R) PtInWorld.Y = R;
        if (PtInWorld.Y <  L) PtInWorld.Y = L;
        if (PtInWorld.Z >  R) PtInWorld.Z = R;
        if (PtInWorld.Z <  L) PtInWorld.Z = L;

        // Map range (L<->R) into (0-31)
        X = (u32)(( 31.0f * ((PtInWorld.X-L)/(R-L)) ) + 0.5f);
        Y = (u32)(( 31.0f * ((PtInWorld.Y-L)/(R-L)) ) + 0.5f);
        Z = (u32)(( 31.0f * ((PtInWorld.Z-L)/(R-L)) ) + 0.5f);

        // Confirm correct range
        ASSERT(X>=0 && X<=31);
        ASSERT(Y>=0 && Y<=31);
        ASSERT(Z>=0 && Z<=31);
    }

    P = (s16)( (UpBit<<15) | ((X&0x1F)<<10) | ((Y&0x1F)<<5) | (Z&0x1F) );

    // Return value
    return P;
}

//////////////////////////////////////////////////////////////////////

void CCharacterExport::ComputePointInfo (CMotion*       pMotion, 
                                         CCharacter*    pCharacter, 
                                         u8*            PointUsedFlags )
{
    s32     iPoint;
    s32     Frame;
	anim*   pAnim = pMotion->GetAnim() ;
    CSkel*   pSkel = pCharacter->GetSkeleton()->GetSkel();

    *PointUsedFlags = 0;

    for( iPoint = 0 ; iPoint < MIN(8,pCharacter->m_SkelPointList.GetCount()) ; iPoint++ )
    {
        CSkelPoint *pSkelPoint = (CSkelPoint*)pCharacter->m_SkelPointList.IndexToPtr( iPoint ) ;

        // Read point from SkelPoint
        vector3  Pt;
        pSkelPoint->GetPosition(&Pt);

        if( pMotion->SkelPointGetCheck( iPoint ) )
        {
            // Point iPoint is present in this animation, add to streams
            (*PointUsedFlags) |= (1<<iPoint);

            // Add stream for point info
			s32 Stream = ANIM_AddStream( pAnim, FALSE ) ;

            // Loop through frames and compute point values
	        for( Frame = 0 ; Frame < pMotion->GetNumFrames() ; Frame++ )
            {
                s16 EncodedPoint;
				s32 Bone = pSkelPoint->GetBone() ;
				if( Bone == 0 )
				{
					Bone = 1 ;
				}
                EncodedPoint = BuildPointPosition( pAnim, pCharacter->m_pSkeleton, Frame, &Pt, Bone-1 );
    	        ANIM_SetStreamValue( pAnim, Frame, Stream, (s16)EncodedPoint ) ;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////

void CCharacterExport::ComputeUserStreamInfo (CMotion*      pMotion, 
                                              CCharacter*   pCharacter, 
                                              u8*           UserStreamUsedFlags )
{
/*
    s32 j;
	anim *pAnim = pMotion->GetAnim() ;

	for( j = 0 ; j < pMotion->GetNumFrames() ; j++ )
    {
    	ANIM_SetMiscValue( pAnim, j, StartStream + 0, (s16)j ) ;
    }
*/
    *UserStreamUsedFlags = 0x00;
}

//////////////////////////////////////////////////////////////////////

void CCharacterExport::ResetMotionCompressionBuffers( CCharacter* pCharacter )
{
	//===	This function will force the motion stream data to be re-compressed
	//		at export.  If it is not called, the previously compressed data will be exported.

	//---	remove the compressed motion data
	if( m_CompMotionsArray ) delete [] m_CompMotionsArray;
	m_CompMotionsArray		= new CExportFile[ pCharacter->GetNumMotions() ];

	//---	remove the compressed block size array
	if( m_CompBlockSizesArray ) delete [] m_CompBlockSizesArray;
	m_CompBlockSizesArray	= new CByteArray[ pCharacter->GetNumMotions() ];
}

//////////////////////////////////////////////////////////////////////
void CCharacterExport::Export( const char *pFileName, CCharacter *pCharacter )
{
    s32		patch_CompDataOffset;
    s32     MaxStreams=0;
    s32     NBasicStreams;
    u8      PropsUsed;
	u8		PropsWithStreams;
    u8      PointsUsed;
    u8      UserStreamsUsed;
	s32		TotalAnimsExported;
	s32		TotalFrames;
	s32		TotalFramesExported;
	s32		TotalEventsExported;
	s32		TotalBlocksExported;
	s32		TotalAnimsAttempted;
	s32		TotalBlockFrames;
	s32		CompMotionDataSize;
    s32     i, j;
	
    //---	Create Progress Dialog
	if( m_pProgressDialog ) delete m_pProgressDialog ;
	m_pProgressDialog = new CExportProgress ;
	m_nErrors = 0 ;
	ASSERT( m_pProgressDialog ) ;
	m_pProgressDialog->Create( IDD_EXPORT_PROGRESS, NULL ) ;
	m_pProgressDialog->EnableOk( false ) ;

	//---	Create ExportFiles necessary
	anim_group.Create( m_Target ) ;
	anim_info.Create( m_Target ) ;
	anim_bone.Create( m_Target ) ;
	anim_events.Create( m_Target ) ;
	anim_BlockSize.Create( m_Target ) ;
//	anim_Blocks.Create( m_Target ) ;
	anim_Header.Create( m_Target ) ;

	anim_HeaderEvent.Create( m_Target ) ;
	anim_HeaderFlags.Create( m_Target ) ;
	anim_HeaderProps.Create( m_Target ) ;
	anim_HeaderPoints.Create( m_Target ) ;
	anim_HeaderStreams.Create( m_Target ) ;

	anim_Text.Create( m_Target );

    patch_CompDataOffset = anim_group.GetPosition();

	//---	supress any warning while exporting this character's motions
	for( i = 0 ; i < pCharacter->GetNumMotions() ; i++ )
		if( pCharacter->GetMotion(i) )
			pCharacter->GetMotion(i)->m_bSupressWarningBoxes = TRUE;

    ///////////////////////////////////////////////////////////////////////
    //	SKELETON
    ///////////////////////////////////////////////////////////////////////

	m_pProgressDialog->Fmt( "Exporting Skeleton..." ) ;
	m_pProgressDialog->Fmt( " " ) ;

	//---	Write Skeleton
	CSkeleton *pSkeleton = pCharacter->GetSkeleton() ;
	for( i = 0 ; i < pSkeleton->GetNumBones() ; i++ )
	{
		//---	Get Pointer to Bone
		SSkelBone *pBone = pSkeleton->GetBoneFromIndex( i ) ;
		ASSERT( pBone ) ;

		anim_bone.PadToBoundry( 4 ) ;

		//---	Bone Name
		anim_bone.WritePaddedString( pBone->BoneName, 20 ) ;

		//---	Bone Details
		anim_bone.Writef32( pBone->OriginToBone.X ) ;
		anim_bone.Writef32( pBone->OriginToBone.Y ) ;
		anim_bone.Writef32( pBone->OriginToBone.Z ) ;
		anim_bone.Writef32( pBone->ParentToBone.X ) ;
		anim_bone.Writef32( pBone->ParentToBone.Y ) ;
		anim_bone.Writef32( pBone->ParentToBone.Z ) ;
		anim_bone.Writes16( (s16)pBone->BoneID ) ;
		anim_bone.Writes16( (s16)pBone->ParentID ) ;
		anim_bone.Writes16( (s16)pSkeleton->GetBoneMirror( pBone->BoneID ) ) ;

		m_nBones++ ;
	}
    /*
    typedef struct
    {
        char        BoneName[20];       // Bone name, all uppercase
        vector3     OriginToBone;       // Global translation from origin to bone pos
        vector3     ParentToBone;       // Local translation from parent to bone pos
        s16         BoneID;             // Index of this bone in array
        s16         ParentID;           // Index of parent bone in array or -1
        s16         MirrorID;           // This bone's mirror image lhand = rhand
    } anim_bone;
    */

    ///////////////////////////////////////////////////////////////////////
    //	PLAN EVENT VALUES
    ///////////////////////////////////////////////////////////////////////

	//---	add all project level events first
	POSITION pos = pCharacter->m_pDoc->GetProjectEventList()->GetHeadPosition();
	CEvent* pEvent;
	s16 FoundID;
	while( pos )
	{
		pEvent = pCharacter->m_pDoc->GetProjectEventList()->GetNext( pos );

		//---	Create or Find a Define for the Event
		CString Define = MakeDefine( pCharacter->m_ExportEventPrefix, pEvent->GetName() ) ;
		FindDefine( &FoundID, Define ) ;
	}

	//---	now add all events from specific characters
	pos = pCharacter->m_pDoc->m_CharacterList.GetHeadPosition();
	CCharacter* pEventCharacter;
	while( pos )
	{
		pEventCharacter = pCharacter->m_pDoc->m_CharacterList.GetNext( pos );
		POSITION pos2 = pEventCharacter->GetAttachedEvents()->GetHeadPosition();
		while( pos2 )
		{
			pEvent = pCharacter->GetAttachedEvents()->GetNext( pos2 );

			//---	Create or Find a Define for the Event
			CString Define = MakeDefine( pCharacter->m_ExportEventPrefix, pEvent->GetName() ) ;
			FindDefine( &FoundID, Define ) ;
		}
	}


    ///////////////////////////////////////////////////////////////////////
    //	MOTIONS
    ///////////////////////////////////////////////////////////////////////

	m_pProgressDialog->Fmt( "Exporting Motions..." ) ;
	m_pProgressDialog->Fmt( " " ) ;

	//---	count exported motions and set the range
	s32 TotalMotionsExported = 0;
	for( i = 0 ; i < pCharacter->GetNumMotions() ; i++ )
		if( pCharacter->GetMotion(i) && pCharacter->GetMotion(i)->m_bExport )
			TotalMotionsExported++;
	m_pProgressDialog->SetRange( 0, TotalMotionsExported ) ;

	//---	Write Text to Header
	if( pCharacter->m_bExportHeaderFile )
	{
		anim_Header.WriteString(        "/////////////////////////////////////////////////////////////////////////// \015\012"
										"//                 CED ANIMATION HEADER FILE - DO NOT EDIT               // \015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"\015\012" ) ;

		anim_HeaderEvent.WriteString(   "\015\012\015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"//                                EVENTS                                 // \015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"\015\012" ) ;
		anim_HeaderEvent.WriteString(   "#define ID_NULL 0\015\012" );


		anim_HeaderFlags.WriteString(   "\015\012\015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"//                          USER DEFINED FLAGS                           // \015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"\015\012" ) ;

		anim_HeaderProps.WriteString(   "\015\012\015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"//                                PROPS                                  // \015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"\015\012" ) ;

		anim_HeaderPoints.WriteString(  "\015\012\015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"//                            POINT TRACKING                             // \015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"\015\012" ) ;

		anim_HeaderStreams.WriteString( "\015\012\015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"//                         USER DEFINED STREAMS                          // \015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"\015\012" ) ;
	}

	//---	Check for Points that are not attached to any Bone
	POSITION Pos = pCharacter->m_SkelPointList.GetHeadPosition() ;
	while( Pos )
	{
		CSkelPoint *pSkelPoint = pCharacter->m_SkelPointList.GetNext( Pos ) ;
		if( pSkelPoint->GetBone() == 0 )
		{
			m_pProgressDialog->Fmt( "WARNING - Point '%s' not attached to Bone, using ROOT", pSkelPoint->GetName() ) ;
			m_pProgressDialog->FmtError( EXPORT_WARNING_POINTNOTATTACHED, "Point '%s' not attached to Bone, using ROOT", pSkelPoint->GetName() ) ;
			m_nErrors++ ;
		}
	}

	//---	prepare the motion compression buffers
	ResetMotionCompressionBuffers( pCharacter );
	CompMotionDataSize = 0;

	//---	Write All Motions
	TotalFrames = 0 ;
	TotalBlockFrames = 0;
	TotalFramesExported = 0 ;
	TotalEventsExported = 0 ;
	TotalBlocksExported = 0 ;
    TotalAnimsExported  = 0 ;
	TotalAnimsAttempted = 0 ;
	for( i = 0 ; i < pCharacter->GetNumMotions() ; i++ )
	{
        //---   Sum up basic streams
        {
            CSkeleton *pSkeleton = pCharacter->GetSkeleton() ;
            NBasicStreams = 0;
            NBasicStreams += pSkeleton->GetNumBones() * 3;  // ROT
            NBasicStreams += 3;                             // TRANS
        }

		//---	Get Pointer to CMotion and anim
		CMotion	*pMotion = pCharacter->GetMotion( i ) ;
		if( pMotion )
		{
			f32		ExportAngleConversion = (f32)(1<<pMotion->GetExportBits())/R_360;

			//---	create the experted animation define
			CString Define = pMotion->GetExportName( ) ;
			if( pCharacter->m_ExportOptForceUpper )
				Define.MakeUpper( ) ;

			//---	if this motion is not exported, continue to the next one now
			if( !pMotion->m_bExport )
			{
				//---	Write Header File Info
				if( pCharacter->m_bExportHeaderFile )
				{
					if( pCharacter->m_bExportHeaderFilePath )
						anim_Header.WriteStringFmt( "//#define %s%s -1 // %s\015\012", pCharacter->m_ExportMotionPrefix, Define, pMotion->GetPathName() ) ;
					else
						anim_Header.WriteStringFmt( "//#define %s%s -1\015\012", pCharacter->m_ExportMotionPrefix, Define ) ;
				}

				//---	Write to the text file
				if( pCharacter->m_bExportTextFile )
					anim_Text.WriteStringFmt( "// %s\015\012", pMotion->GetPathName() );

				m_CompMotionsArray[ i ].SetLength(0);
				m_CompBlockSizesArray[ i ].SetSize(0);
				continue;
			}

			//---	Add Line to Export Dialog
			m_pProgressDialog->SetPos( ++TotalAnimsAttempted ) ;
			m_pProgressDialog->Fmt( "%s", pMotion->GetPathName() ) ;

			//---	supress the warning boxes while exporting this motion
			long Result=0;
			bool bLoaded = pMotion->EnsureLoaded( &Result );
			if( Result )
			{
				if( Result & CMotion::LOAD_RESULT_ERROR )
					m_pProgressDialog->FmtError( EXPORT_ERROR_CANNOTLOAD, "\"%s\": AMC DID NOT LOAD", pMotion->GetExportName() ) ;
				if( Result & CMotion::LOAD_RESULT_ONEFRAME )
					m_pProgressDialog->FmtError( EXPORT_WARNING_MOTIONHASONEFRAME, "\"%s\": Motion has only one frame. It must have two.", pMotion->GetExportName() ) ;
				if( Result & CMotion::LOAD_RESULT_FRAMESCHANGED )
					m_pProgressDialog->FmtError( EXPORT_WARNING_NUMFRAMESCHANGED, "\"%s\": The number of frames have changed since last loaded.", pMotion->GetExportName() ) ;

				//---	this warning is best done while exporting the events later one.  this one wouldn't be hit if the animation was already loaded for instance.
//				if( Result & CMotion::LOAD_RESULT_EVENTOUTSIDEANIM )
//					m_pProgressDialog->FmtError( EXPORT_WARNING_EVENTSPASTEND, "\"%s\": One or more events are past the end of the motion.", pMotion->GetExportName() ) ;
			}

			//---	Ensure Anim is loaded and get pointer to it
			anim* pAnim = pMotion->GetAnim() ;
			if( !bLoaded || !pAnim )
			{
				//---	Write Header File Info
				if( pCharacter->m_bExportHeaderFile )
				{
					if( pCharacter->m_bExportHeaderFilePath )
						anim_Header.WriteStringFmt( "#define %s%s -1 // %s\015\012", pCharacter->m_ExportMotionPrefix, Define, pMotion->GetPathName() ) ;
					else
						anim_Header.WriteStringFmt( "#define %s%s -1\015\012", pCharacter->m_ExportMotionPrefix, Define ) ;
				}

				//---	Write to the text file
				if( pCharacter->m_bExportTextFile )
					anim_Text.WriteStringFmt( "%s\015\012", pMotion->GetPathName() );

				m_pProgressDialog->Fmt( "ERROR - AMC DID NOT LOAD" ) ;
				m_nErrors++ ;

				m_CompMotionsArray[ i ].SetLength(0);
				m_CompBlockSizesArray[ i ].SetSize(0);

				continue;
			}

			//---	Add to Total Frames Exported
            TotalAnimsExported++;
			TotalFrames += pMotion->GetNumFrames( ) ;
//            TotalBlocksExported += (pMotion->GetNumFrames( ) / ANIM_FRAMES_PER_BLOCK);

			//---	Write Header File Info
			if( pCharacter->m_bExportHeaderFile )
			{
				if( pCharacter->m_bExportHeaderFilePath )
					anim_Header.WriteStringFmt( "#define %s%s %d // %s\015\012", pCharacter->m_ExportMotionPrefix, Define, m_nAnimations, pMotion->GetPathName() ) ;
				else
					anim_Header.WriteStringFmt( "#define %s%s %d\015\012", pCharacter->m_ExportMotionPrefix, Define, m_nAnimations ) ;
			}

			//---	Write to the text file
			if( pCharacter->m_bExportTextFile )
				anim_Text.WriteStringFmt( "%s\015\012", pMotion->GetPathName() );


			if( pCharacter->m_bExportDataFile )
			{
				///////////////////////////////////////////////////////////
				//---	Add MoveDir and RootDir
				///////////////////////////////////////////////////////////
				s32 MoveDirStream = ANIM_AddStream( pAnim, TRUE ) ;     // MOVEDIR
				s32 FaceDirStream = ANIM_AddStream( pAnim, TRUE ) ;     // FACEDIR
				for( j = 0 ; j < pMotion->GetNumFrames() ; j++ )
				{
					ANIM_SetRotation( pAnim, j, MoveDirStream, pMotion->GetMoveDir(j) ) ;
					ANIM_SetRotation( pAnim, j, FaceDirStream, pMotion->GetFaceDir(j) ) ;
				}


				///////////////////////////////////////////////////////////
				//---   Add prop streams
				///////////////////////////////////////////////////////////
				PropsUsed = 0;
				ComputePropInfo( pMotion, pCharacter, &PropsUsed, &PropsWithStreams );

				///////////////////////////////////////////////////////////
				//---   Add point streams
				///////////////////////////////////////////////////////////
				PointsUsed = 0;
				ComputePointInfo( pMotion, pCharacter, &PointsUsed );

				///////////////////////////////////////////////////////////
				//---   Add user streams
				///////////////////////////////////////////////////////////
				UserStreamsUsed = 0;
				ComputeUserStreamInfo( pMotion, pCharacter, &UserStreamsUsed );

				///////////////////////////////////////////////////////////
				//---   Check if this number of streams beats MaxStreams
				///////////////////////////////////////////////////////////
				if (pMotion->CountNumExportedStreams( PropsWithStreams ) > MaxStreams)
					MaxStreams = pMotion->CountNumExportedStreams( PropsWithStreams );

				///////////////////////////////////////////////////////////
				//---	Calculate the rotation and translation to apply to
				//		the motion so that on frame zero it is on zero facing
				//		zero.
				///////////////////////////////////////////////////////////
				f32		AntiRotY	= -pMotion->GetFaceDir(0);
				vector3	AntiTrans;
				matrix4	AntiMat;
				ANIM_GetTransValue( pMotion->GetAnim(), 0, &AntiTrans );
				AntiTrans.X = -AntiTrans.X;
				AntiTrans.Y = 0.0f;
				AntiTrans.Z = -AntiTrans.Z;
				AntiMat.Identity();
				AntiMat.Translate( AntiTrans );
				AntiMat.RotateY( AntiRotY );

//#define	NO_TRANSFORM_AT_EXPORT
#ifdef NO_TRANSFORM_AT_EXPORT
				AntiRotY	= 0.0f;
				AntiTrans.Set( 0.0f, 0.0f, 0.0f );
				AntiMat.Identity();
#endif

				///////////////////////////////////////////////////////////
				//---   Export the compressed motion data
				///////////////////////////////////////////////////////////
				s32 CompSize, CompBlocks;
				s32 NumExportedFrames;
				if( pCharacter->m_bExportDataFile )
				{
					ExportMotionBlocks( i,
										pMotion,
										pCharacter,
										pCharacter->m_ExportFramesPerBlock,
										NumExportedFrames,
										CompSize,
										CompBlocks,
										PropsWithStreams,
										AntiMat ) ;
				}
	//			m_pProgressDialog->Fmt( "I=%5d, off=%10d, size=%6d, ID_%s", m_nAnimations, CompMotionDataSize, CompSize, Define );

				///////////////////////////////////////////////////////////
				//---   Export the motion header
				///////////////////////////////////////////////////////////

				//---	Calculate Total Translation
				vector3 	tEnd, tStart ;
				ANIM_GetTransValue( pAnim, 0, &tStart ) ;
				ANIM_GetTransValue( pAnim, pMotion->GetNumFrames()-1, &tEnd ) ;
				tStart	= AntiMat.Transform( tStart );
				tEnd	= AntiMat.Transform( tEnd );
				tStart.Scale( 16 );
				tEnd.Scale( 16 );

				//---	Calc RootDir and MoveDir
				u16		RootDirAtFrame0 = (u16)((pMotion->GetFaceDir(0)+AntiRotY)							* ExportAngleConversion) ;
				u16		RootDirAtFrameN = (u16)((pMotion->GetFaceDir(pMotion->GetNumFrames()-1)+AntiRotY)	* ExportAngleConversion) ;
				u16		MoveDirAtFrame0 = (u16)((pMotion->GetMoveDir(0)+AntiRotY)							* ExportAngleConversion) ;
				u16		MoveDirAtFrameN = (u16)((pMotion->GetMoveDir(pMotion->GetNumFrames()-1)+AntiRotY)	* ExportAngleConversion) ;

				anim_info.PadToBoundry( 4 ) ;

//				anim_info.Writes32( anim_Blocks.GetPosition() ) ;	// BlockOffset
				anim_info.Writes32( CompMotionDataSize ) ;				// BlockOffset
				anim_info.Writes16( pMotion->GetNumFrames() ) ;			// NFrames
				anim_info.Writes16( m_EventIndex ) ;					// EventIndex
				anim_info.Writes16( (s16)TotalBlocksExported ) ;		// BlockSizeIndex
//				anim_info.Writeu16( RootDirAtFrame0 ) ;					// RootDirAtFrame0
				anim_info.Writeu16( RootDirAtFrameN ) ;					// RootDirAtFrameN
				anim_info.Writeu16( MoveDirAtFrame0 ) ;					// MoveDirAtFrame0
				anim_info.Writeu16( MoveDirAtFrameN ) ;					// MoveDirAtFrameN
//				anim_info.Writes16( (s16)tStart.X ) ;					// TransXAtFrame0
				anim_info.Writes16( (s16)tStart.Y ) ;					// TransYAtFrame0
//				anim_info.Writes16( (s16)tStart.Z ) ;					// TransZAtFrame0
				anim_info.Writes16( (s16)tEnd.X ) ;						// TransXAtFrameN
				anim_info.Writes16( (s16)tEnd.Y ) ;						// TransYAtFrameN
				anim_info.Writes16( (s16)tEnd.Z ) ;						// TransZAtFrameN
				anim_info.Writes16( m_nAnimations ) ;  					// ID

				CompMotionDataSize += CompSize;
				TotalBlocksExported += CompBlocks;
				TotalFramesExported += NumExportedFrames;
				TotalBlockFrames += pCharacter->m_ExportFramesPerBlock*(CompBlocks-1) + NumExportedFrames%pCharacter->m_ExportFramesPerBlock;

				//---   Export user flags
				{
					s32 f;
					u16 Flags = 0;
					for (f=0; f<MOTION_FLAG_BITS; f++) 
						Flags |= ((u16)(pMotion->m_FlagBit[f]&0x01)) << f;
					anim_info.Writes16( (s16)Flags );                   
				}

				//---	Exported frames
				anim_info.Writes16( (s16)NumExportedFrames );		// Number of exported frames

				//---	Export Bits ( High 4 bits hold number of quat bits minues 1, Low 4 bits holds number of normal bits minus 1.).
				anim_info.Writeu8( (u8)pMotion->GetExportBits() );

				//---   Export NEvents
				anim_info.Writeu8( pMotion->GetNumEvents() ) ;      // Number of Events
				TotalEventsExported += pMotion->GetNumEvents();

				//---   Export Props, Points, UserStreams Present
				anim_info.Writeu8( PropsUsed );
				anim_info.Writeu8( PropsWithStreams );
				anim_info.Writeu8( PointsUsed );
				anim_info.Writeu8( UserStreamsUsed );

				//---   Count total number of streams in use and export
				{
					s32 NStreams;
					s32	NStreamsPerProp;
					s32 k;
					if( pMotion->GetExportQuatW() )
					{
						NStreams = pCharacter->GetSkeleton()->GetNumBones()*4;
						NStreamsPerProp	= 7;
					}
					else
					{
						NStreams = pCharacter->GetSkeleton()->GetNumBones()*3;
						NStreamsPerProp	= 6;
					}
					NStreams += 3 + 2;
					for (k=0; k<8; k++)
					{
						if (PropsWithStreams& (1<<k)) NStreams += NStreamsPerProp;
						if (PointsUsed      & (1<<k)) NStreams += 1;
						if (UserStreamsUsed & (1<<k)) NStreams += 1;
					}

					anim_info.Writeu8( (u8)NStreams );
				}

				u8 Flags;
				Flags = 0;
				Flags |= pMotion->GetExportGlobalRots()		? 0x01 : 0;	// rotations are local/global (global do not need to be combined with parent rotations) (root is always local)
				Flags |= pMotion->GetExportWorldRots()		? 0x02 : 0; // rotations are frame/world relative to this frame's face dir OR are global rotations.
				Flags |= pMotion->GetExportQuaternion()		? 0x04 : 0; // export is in quaternions (4 samples per rotation)
				Flags |= pMotion->GetExportQuatW()			? 0x08 : 0; // export includes W streams for the quaternion values to speed up decompression.
				Flags |= UNCOMPRESS_MOTION_DATA				? 0x10 : 0; // data is uncompressed (good for debugging)
				Flags |= pMotion->GetExportQuatBlend()		? 0x20 : 0;	// force quaternion blending at run-time to fix some problems (ignored for quaternion export)
				Flags |= pMotion->GetExportPropQuatBlend()	? 0x40 : 0;	// force quaternion blending at run-time for the props
				anim_info.Writeu8( Flags );

				//---	pad byte
				anim_info.Writeu8( 0 );
				anim_info.Writeu8( 0 );

				/*
				typedef struct
				{
					s32 BlockOffset;                // Offset in bytes to beginning of comp data
					s16 NFrames;                    // Num frames in the animation
					s16 EventIndex;                 // Index into the event array to first event
					s16 BlockSizeIndex;             // Index into BlockSize array to first block
					s16 RootDirAtFrame0;            // RootDir for first frame of animation
					s16 RootDirAtFrameN;            // RootDir for last frame of animation
					s16 MoveDirAtFrame0;            // MoveDir for first frame of animation
					s16 MoveDirAtFrameN;            // MoveDir for last frame of animation
					s16 TransXAtFrame0;             // TransX at first frame of animation in 12.4
					s16 TransYAtFrame0;             // TransY at first frame of animation in 12.4
					s16 TransZAtFrame0;             // TransZ at first frame of animation in 12.4
					s16 TransXAtFrameN;             // TransX at last  frame of animation in 12.4
					s16 TransYAtFrameN;             // TransY at last  frame of animation in 12.4
					s16 TransZAtFrameN;             // TransZ at last  frame of animation in 12.4
					s16 ID;                         // Index of this animation in the array
					u16 UserFlags;
					u16	FrameRate;					// frame rate in 8.8
					u8	ExportBits;					// there are 2 values here.  4/4 (quat bits/data bits)
					u8  NEvents;                    // Num Events in animation
					u8  PropsPresent;               // Which of the 8 props are present in anim
					u8  PointsPresent;              // Which of the 8 points are present in anim
					u8  UserStreamsPresent;         // Which user streams are present in anim
					u8	NStreams;					// number of streams
					u8	Flags;						// animation flags
					u8  Pad;
				} anim_info;
				*/

				///////////////////////////////////////////////////////////
				//---	Write Events
				///////////////////////////////////////////////////////////
				for( j = 0 ; j < pMotion->GetNumEvents() ; j++ )
				{
					CEvent *pEvent = pMotion->GetEvent( j ) ;
					if( pEvent )
					{
						//---	Read Event Details
						vector3  v ;
						vector3  RootP;
						pEvent->UpdateEventPosition();
						pEvent->GetPosition( &v ) ;
						v = AntiMat.Transform( v );	// transform the point into the exported motion space
						int iFrame = pEvent->GetFrame() ;
						if( iFrame < pMotion->GetNumFrames() )
						{
							pMotion->GetTranslation( RootP, iFrame );
							RootP = AntiMat.Transform( RootP );// transform the point into the exported motion space
						}
						else
							RootP.Set( 0.0f, 0.0f, 0.0f );
						
						//---	Create or Find a Define for the Event
						CString Define = MakeDefine( pCharacter->m_ExportEventPrefix, pEvent->GetName() ) ;
						s16 FoundID ;
						FindDefine( &FoundID, Define ) ;
//							if( pCharacter->m_ExportOptForceUpper )
//								Define.MakeUpper( ) ;
//	
//							//---	Add Define to Header File
//							if( NewID )
//								anim_HeaderEvent.WriteStringFmt( "#define %s %d\015\012", Define, FoundID ) ;

						//---	Add Error if Event is outside frame range of motion
						if( (iFrame < 0) || (iFrame >= pMotion->GetNumFrames()) )
						{
							m_pProgressDialog->Fmt( "ERROR - EVENT '%s' OUTSIDE ANIMATION FRAME RANGE", pEvent->GetName() ) ;
							m_pProgressDialog->FmtError( EXPORT_WARNING_EVENTSPASTEND, "\"%s\": Event '%s' outside animation frame range.", pMotion->GetExportName(), pEvent->GetName() );
							m_nErrors++ ;
							iFrame = pMotion->GetNumFrames()-1;
						}

						//---	Write the Event
						anim_events.Writes16( iFrame ) ;
						anim_events.Writes16( FoundID ) ;
						anim_events.Writes16( (s16)(v.X*16) ) ;
						anim_events.Writes16( (s16)(v.Y*16) ) ;
						anim_events.Writes16( (s16)(v.Z*16) ) ;
						anim_events.Writes16( (s16)(RootP.X*16) ) ;
						anim_events.Writes16( (s16)(RootP.Y*16) ) ;
						anim_events.Writes16( (s16)(RootP.Z*16) ) ;
						m_EventIndex++ ;
                    
						/*
						typedef struct
						{
							s16 Frame;                      // Frame event occurs on in (1/60s)
							s16 EventID;                    // Event ID
							s16 PX,PY,PZ;                   // Position of event in 12.4 fixed
							s16 RPX,RPY,RPZ;                // Position of root in 12.4 fixed
						} anim_event;
						*/

						//---   Increase number of events in animgroup
						m_nAnimEvents++;
					}
				}
			}

			//---	Delete Misc Streams
			while( pAnim->NStreams > ((pAnim->NBones+1)*3) )
				ANIM_DelStream( pAnim, pAnim->NStreams-1 );

			//---	Increment Number of Animations Exported
			m_nAnimations++ ;
		}
	}


	if( pCharacter->m_bExportHeaderFile )
	{
		///////////////////////////////////////////////////////////
		//	EVENT DEFINES
		///////////////////////////////////////////////////////////
		for( i=1; i<m_DefineID; i++ )
		{
			//---	get the define
			CString Define = m_DefineArray[i];
			if( pCharacter->m_ExportOptForceUpper )
				Define.MakeUpper( ) ;

			//---	export the define
			anim_HeaderEvent.WriteStringFmt( "#define %s %d\015\012", Define, i ) ;
		}


		///////////////////////////////////////////////////////////////////////
		//	FLAG BIT DEFINES
		///////////////////////////////////////////////////////////////////////

		CCeDDoc	*pDoc = pCharacter->m_pDoc ;
		ASSERT( pDoc ) ;

		int MaxLen = 0 ;
		for( i = 0 ; i < MOTION_FLAG_BITS ; i++ )
		{
			int Len = pDoc->m_FlagDefine[i].GetLength() ;
			if( Len > MaxLen ) MaxLen = Len ;
		}

		//---	Write motion flags
		for( i = 0 ; i < MOTION_FLAG_BITS ; i++ )
		{
			CString Define = pDoc->m_FlagDefine[i];

			if( pCharacter->m_ExportOptForceUpper )
				Define.MakeUpper( ) ;

			//---	Add Define to Header File
			int Len = Define.GetLength() ;
			if( Len > 0 )
			{
				Len = MaxLen - Len ;
				anim_HeaderFlags.WriteStringFmt( "#define %s_FLAG_%s %*s (1<<%d)         // %s\015\012", pCharacter->m_ExportMotionPrefix, Define, Len, "", i, pDoc->m_FlagDescription[i] ) ;
			}
		}

		///////////////////////////////////////////////////////////////////////
		//	PROPS DEFINES
		///////////////////////////////////////////////////////////////////////
		s32 iProp;
		for( iProp=0; iProp<MAX_NUM_PROPS; iProp++ )
		{
			if( pCharacter->GetPropDef(iProp).IsActive() &&
				pCharacter->GetPropDef(iProp).IsExported() )
			{
				//---	make a define for this prop by adding the charcters name to the prop name.
				CString CharacterName( pCharacter->GetName() );
				CString PropName = pCharacter->GetPropDef(iProp).GetPropName();
				CharacterName.MakeUpper();
				PropName.MakeUpper();
				PropName.Replace( ' ', '_' );
				anim_HeaderProps.WriteStringFmt( "#define %sPROP_%s_%s %d\015\012", pCharacter->m_ExportMotionPrefix, CharacterName, PropName, iProp );
			}
		}

		///////////////////////////////////////////////////////////////////////
		//	POINT TRACKING DEFINES
		///////////////////////////////////////////////////////////////////////
		s32 iPoint;
		for( iPoint = 0 ; iPoint < MIN(8,pCharacter->m_SkelPointList.GetCount()) ; iPoint++ )
		{
			CSkelPoint *pSkelPoint = (CSkelPoint*)pCharacter->m_SkelPointList.IndexToPtr( iPoint ) ;
			CString Define = pSkelPoint->GetName() ;

			if( pCharacter->m_ExportOptForceUpper )
				Define.MakeUpper( ) ;

			anim_HeaderPoints.WriteStringFmt( "#define %sPOINT_%s %1d\015\012",pCharacter->m_ExportMotionPrefix,Define,iPoint);
		}

		///////////////////////////////////////////////////////////////////////
		//	USER STREAM DEFINES
		///////////////////////////////////////////////////////////////////////
		anim_HeaderStreams.WriteString( "\015\012\015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"//                              FINISHED                                 // \015\012"
										"/////////////////////////////////////////////////////////////////////////// \015\012"
										"\015\012" ) ;


	}

    ///////////////////////////////////////////////////////////////////////
    //	ANIMGROUP
    ///////////////////////////////////////////////////////////////////////

	anim_group.Writeu32( 0 ) ;                                          // AnimCompData
    anim_group.Writes32( (s32)ANIMGROUP_VERSION );                      // Version
	anim_group.WritePaddedString( pCharacter->GetName(), 32 ) ;         // Name[32]
	s32 patch_AnimBlockSize = anim_group.GetPosition() ;                // AnimBlockSize
	anim_group.Writeu32( 0 ) ;                                          
	s32 patch_AnimEvent = anim_group.GetPosition() ;                    // AnimEvent
	anim_group.Writeu32( 0 ) ;                                          
	s32 patch_AnimInfo = anim_group.GetPosition() ;                     // AnimInfo
	anim_group.Writeu32( 0 ) ;                                          
	s32 patch_Bone = anim_group.GetPosition() ;                         // Bone
	anim_group.Writeu32( 0 ) ;
	anim_group.Writes32( m_nAnimBytes ) ;                               // NAnimBytes
	anim_group.Writes16( m_nAnimations ) ;                              // NAnimations
	anim_group.Writes16( m_nAnimEvents ) ;                              // NAnimEvents
	anim_group.Writes16( m_nAnimBlocks ) ;                              // NAnimBlocks
	anim_group.Writes16( m_nAnimStreams ) ;                             // NAnimStreams
	anim_group.Writes16( m_nBones ) ;                                   // NBones
    anim_group.Writes16( (s16)MaxStreams );
	anim_group.Writes16( (s16)pCharacter->m_ExportFramesPerBlock );
	anim_group.PadToBoundry( 4 ) ;

	for( i=0; i<MAX_NUM_PROPS; i++ )
	{
		CPropDef& rPropDef = pCharacter->GetPropDef(i);
		prop_def  exportPropDef;

		memset( &exportPropDef, 0, sizeof(prop_def) );
		xbool bExported = (rPropDef.IsActive() && rPropDef.IsExported()) ? 1 : 0;
		if( bExported )
		{
			ASSERT((rPropDef.GetBone()>=-1)&&(rPropDef.GetBone()<100));

			//
			//	calculate the prop flags
			//
			u16 Flags = 0;
			Flags |= bExported						? prop_def::ACTIVE			: 0;
			Flags |= rPropDef.IsScaled()			? prop_def::SCALED			: 0;
			Flags |= rPropDef.IsMirrored()			? prop_def::MIRRORED		: 0;
			Flags |= rPropDef.IsPositionStatic()	? prop_def::STATIC_POS		: 0;
			Flags |= rPropDef.IsRotationStatic()	? prop_def::STATIC_ROT		: 0;
			Flags |= rPropDef.IsPosFacingRelative()	? prop_def::FACEREL_POS		: 0;
			Flags |= rPropDef.IsRotFacingRelative()	? prop_def::FACEREL_ROT		: 0;
			Flags |= rPropDef.IsMirrorWithAnim()	? prop_def::MIRRORWITHANIM	: 0;

			//
			//	calculate the prop quaternion
			//
			quaternion Quat;
			Quat.Identity();
			Quat.RotateX( rPropDef.GetRotation().X );
			Quat.RotateZ( rPropDef.GetRotation().Z );
			Quat.RotateY( rPropDef.GetRotation().Y );

			//
			//	initialize the exported prop structure
			//
			exportPropDef.Flags		= Flags;
			exportPropDef.BoneID	= rPropDef.GetBone();
			exportPropDef.Pos		= rPropDef.GetPosition();
			exportPropDef.Rot		= rPropDef.GetRotation();
			exportPropDef.Quat		= Quat;
		}
		else
		{
			exportPropDef.Quat.Identity();
		}

		anim_group.Writeu16( exportPropDef.Flags );
		anim_group.Writes16( exportPropDef.BoneID );
		anim_group.Writef32( exportPropDef.Pos.X );
		anim_group.Writef32( exportPropDef.Pos.Y );
		anim_group.Writef32( exportPropDef.Pos.Z );
		anim_group.Writef32( exportPropDef.Rot.Pitch );
		anim_group.Writef32( exportPropDef.Rot.Yaw );
		anim_group.Writef32( exportPropDef.Rot.Roll );
		anim_group.Writef32( exportPropDef.Quat.X );
		anim_group.Writef32( exportPropDef.Quat.Y );
		anim_group.Writef32( exportPropDef.Quat.Z );
		anim_group.Writef32( exportPropDef.Quat.W );
	}

    /*

    typedef struct
    {
        void*       AnimCompData;       // Ptr to compressed binary data. FilePtr on N64
        s32         Version;            // Version number
        char        Name[32];           // Group name
        u8*         AnimBlockSize;      // In multiples of 16 bytes
        anim_event* AnimEvent;          // Events
        anim_info*  AnimInfo;           // Info on each animation
        anim_bone*  Bone;               // Skeleton animations use
        s32         NAnimBytes;         // Size of compressed animation data
        s16         NAnimations;        // Num animations in group
        s16         NAnimEvents;        // Total events in group
        s16         NAnimBlocks;        // Total compressed anim blocks in group
        s16         NAnimStreams;       // Total streams each anim has
        s16         NBones;             // Total bones each anim uses
        s16         MaxStreams;         // Max streams ever needed
		s16			FramesPerBlock;		// Max frames compressed within any block
		s16			pad;

		struct prop_def
		{
			u16			Flags;
			s16			BoneID;
			vector3		Pos;
			radian3		Rot;
			quaternion	Quat;
		} PropDefs[8];

    } anim_group;

    */

    ///////////////////////////////////////////////////////////////////////
    //	JOIN ALL FILES
    ///////////////////////////////////////////////////////////////////////

	//---	Append AnimBlockSize and Patch
	anim_group.PadToBoundry( 4 ) ;
	anim_group.PatchPtr( patch_AnimBlockSize, anim_group.GetPosition() ) ;
	anim_group.Cat( &anim_BlockSize ) ;

	//---	Append AnimEvent and Patch
	anim_group.PadToBoundry( 4 ) ;
	anim_group.PatchPtr( patch_AnimEvent, anim_group.GetPosition() ) ;
	anim_group.Cat( &anim_events ) ;

	//---	Append AnimInfo and Patch
	anim_group.PadToBoundry( 4 ) ;
	anim_group.PatchPtr( patch_AnimInfo, anim_group.GetPosition() ) ;
	anim_group.Cat( &anim_info ) ;

	//---	Append Bone and Patch
	anim_group.PadToBoundry( 4 ) ;
	anim_group.PatchPtr( patch_Bone, anim_group.GetPosition() ) ;
	anim_group.Cat( &anim_bone ) ;

	//---	Append Blocks and Patch
	anim_group.PadToBoundry( 16 ) ;
    anim_group.PatchPtr( patch_CompDataOffset, anim_group.GetPosition() ) ;
//	anim_group.Cat( &anim_Blocks ) ;

	//---	Join all the Header Files
	anim_Header.Cat( &anim_HeaderEvent ) ;
	anim_Header.Cat( &anim_HeaderFlags ) ;
	anim_Header.Cat( &anim_HeaderProps ) ;
	anim_Header.Cat( &anim_HeaderPoints ) ;
	anim_Header.Cat( &anim_HeaderStreams ) ;

	//---	remove the supression any warning while exporting this character's motions
	for( i = 0 ; i < pCharacter->GetNumMotions() ; i++ )
		if( pCharacter->GetMotion(i) )
			pCharacter->GetMotion(i)->m_bSupressWarningBoxes = FALSE;

    ///////////////////////////////////////////////////////////////////////
    //	SAVE
    ///////////////////////////////////////////////////////////////////////

    //---   Display file size
	s32 HeaderSize = anim_group.GetPosition();
	s32 FileSize = HeaderSize + CompMotionDataSize;
	s32 TotalMotions = pCharacter->GetNumMotions();

	//---	Save Binary Data
	if( pCharacter->m_bExportDataFile )
	{
		if( !anim_group.WriteToDisk( pFileName ) )
			return;

		//---	concatinate motion data
		for( i=0; i<TotalMotions; i++ )
			if( !m_CompMotionsArray[ i ].AppendToDisk( pFileName ) )
				break;
	}

	//---	Save Header
	bool bHeaderFileUpdated;
	if( pCharacter->m_bExportHeaderFile )
		anim_Header.WriteToDiskIfDifferent( ::MFCU_SetExtension( pFileName, ".h" ), &bHeaderFileUpdated );

	//---	Save the text file
	bool bTextFileUpdated;
	if( pCharacter->m_bExportTextFile )
		anim_Text.WriteToDiskIfDifferent( ::MFCU_SetExtension( pFileName, ".txt"), &bTextFileUpdated );

	//---	Print Total Frames Exported
	m_pProgressDialog->Fmt( "Total Anims exported  = %d  (of %d)", TotalAnimsExported, TotalMotions );
	m_pProgressDialog->Fmt( "Total Frames          = %d  (of %d)", TotalFramesExported, TotalFrames );
	m_pProgressDialog->Fmt( "Total Events          = %d", TotalEventsExported );
	m_pProgressDialog->Fmt( "Total Blocks          = %d  (+%d frames for %d total exported frames)", TotalBlocksExported, TotalBlockFrames-TotalFramesExported, TotalBlockFrames );
	m_pProgressDialog->Fmt( "" );
	m_pProgressDialog->Fmt( "Total Header Data     = %d", HeaderSize );
	m_pProgressDialog->Fmt( "Total Compressed Data = %d", CompMotionDataSize );
	m_pProgressDialog->Fmt( "Total File Size       = %d", FileSize ) ;
	m_pProgressDialog->Fmt( "" );
	m_pProgressDialog->Fmt( " Header File Updated  = %s", bHeaderFileUpdated ? "TRUE" : "FALSE" );
	if( pCharacter->m_bExportTextFile )
		m_pProgressDialog->Fmt( " Text File Updated    = %s", bTextFileUpdated ? "TRUE" : "FALSE" );

	//---	Print Errors recorded
	m_pProgressDialog->Fmt( " " ) ;
	if( m_nErrors == 0 )
		m_pProgressDialog->Fmt( "No Errors during Export!", m_nErrors ) ;
	else if( m_nErrors == 1 )
		m_pProgressDialog->Fmt( "%d Error during Export!", m_nErrors ) ;
	else
		m_pProgressDialog->Fmt( "%d Errors during Export!", m_nErrors ) ;

	//---	Close Progress Dialog
	m_pProgressDialog->EnableOk( true ) ;
//	m_pProgressDialog->DestroyWindow( ) ;
} // Export
