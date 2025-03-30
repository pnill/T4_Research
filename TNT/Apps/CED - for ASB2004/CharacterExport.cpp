// CharacterExport.cpp: implementation of the CCharacterExport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "CharacterExport.h"
#include "acccmp.h"
#include "CeDDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "mfcutils.h"

const char*	EXPORT_PREFIX		= "ID_";
const char* EXPORT_PREFIX_FE	= "ID_FE_";

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

CString CCharacterExport::MakeDefine( CString String, bool FrontEndSpecific )
{
	CString Define;

	if (FrontEndSpecific == true)
		Define = EXPORT_PREFIX_FE;
	else
		Define = EXPORT_PREFIX;

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

//////////////////////////////////////////////////////////////////////

void CCharacterExport::ExportMotionBlocks( s32 MotionIndex,
										  CMotion *pMotion,
										  CCharacter *pCharacter,
										  s32 FramesPerBlock,
										  s32& rNumExportedFrames,
										  s32& rCompDataSize,
										  s32& rCompDataBlocks)
{
    s16*			RawData;
    byte*			CompData;
    s32				RawDataSize;
    s32				CompDataSize;
    s32				i,j;
    anim			Anim;
    anim*			pAnim;
    skel*			pSkel;
	CExportFile*	pCompData		= &m_CompMotionsArray[ MotionIndex ];
	CByteArray*		pCompBlockSizes	= &m_CompBlockSizesArray[ MotionIndex ];

	//---	if the data has already been compressed, add the compressed block sizes to the export, then return
	if( pCompData->GetLength() )
	{
		for( i=0; i<pCompBlockSizes->GetSize(); i++ )
			anim_BlockSize.Writeu8( pCompBlockSizes->GetAt(i) ) ;
	
		rCompDataBlocks = pCompBlockSizes->GetSize() ;

		rCompDataSize = pCompData->GetLength();
	}


    //---   Get the resampled animation for processing
    pMotion->GetResampledAnimation( Anim, pMotion->GetExportFrameRate() );
    pAnim = &Anim;
	rNumExportedFrames = pAnim->NFrames;

	//---	alias pointers to the skeleton information
    CSkeleton *pSkeleton = pCharacter->GetSkeleton() ;
    pSkel = pSkeleton->GetSkel();

	//---	prepare the compression buffer for the data
	pCompBlockSizes->RemoveAll();
	pCompData->Create( m_Target );
	pCompData->SeekToBegin();
	pCompData->SetLength( 0 );

    //---   Convert bone rotations from local XYZ to global XZY
    matrix4 DestM[50];
    matrix4 WorldOrientM;
	f32		ExportAngleConversion = (f32)(1<<pMotion->GetExportBits())/R_360;

	ASSERT(pAnim->NBones<=50);

    //---   Build WorldOrientM
    M4_Identity( &WorldOrientM );

    //---   Build matrices and compute global XZY rotations
	s32 NFrames = pAnim->NFrames;
	s32 NBones = pAnim->NBones;
    for (i=0; i<NFrames; ++i)
    {
        // Compute local to world matrix
        SKEL_BuildMatrices (pSkel, pAnim, i, DestM, &WorldOrientM);

        // Pull global XYZ rotations from matrix and convert to global XZY
        for (j=0; j<NBones; ++j)
        {
            radian3d Rot;
            M4_GetRotationsXYZ(&DestM[j], &Rot);
            x_XYZToXZYRot( &Rot );

            // Place new rotation in animation
            pAnim->Stream[ (NFrames*((j*3)+0)) + i ] = (s16)(Rot.X * ExportAngleConversion);
            pAnim->Stream[ (NFrames*((j*3)+1)) + i ] = (s16)(Rot.Y * ExportAngleConversion);
            pAnim->Stream[ (NFrames*((j*3)+2)) + i ] = (s16)(Rot.Z * ExportAngleConversion);
        }
    }
/*
	//---	Remove the Yaw of each frame from all of the bones except the root bone for that frame.
	//		this will make blending at run-time in the game easier.
	for( i=0; i<NFrames; ++i )
	{
		s16 RootYaw = pAnim->Stream[ NBones + i ];
		s16 Rot;
		for( j=1; j<NBones; ++j )
		{
			Rot = pAnim->Stream[ (j*3+1)*NFrames + i  ];
			Rot -= RootYaw;
			pAnim->Stream[ (j*3+1)*NFrames + i ] = (s16)Rot;
		}
	}
*/
    //---   Minimize deltas in rotation streams for maximum compression
    for (i=0; i<pAnim->NBones*3; i++)
        ANIM_MinimizeAngleDiff( pAnim, i, 1<<pMotion->GetExportBits() );

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
                RawData[RawDataSize] = Value;
                RawDataSize++;
			}
		}

        /////////////////////////////////////////////////////////////
        //---   Compress RawData into CompData buffer
        /////////////////////////////////////////////////////////////
        if (1)
        {
            CompDataSize = ACCCMP_Encode(CompData,
                                         RawData,
                                         pAnim->NStreams,
                                         (RawDataSize / pAnim->NStreams));
        }
        else
        {
            x_memcpy(CompData,RawData,sizeof(s16)*RawDataSize);
            CompDataSize = RawDataSize*sizeof(s16);
        }

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
			MessageBox( NULL, "Compressed aniamtion block >= 4096 bytes", "ERROR - Block size overflow", MB_ICONERROR );
	}

	//---	store the block sizes into the export buffer
	for( i=0; i<pCompBlockSizes->GetSize(); i++ )
		anim_BlockSize.Writeu8( pCompBlockSizes->GetAt(i) ) ;
	rCompDataBlocks = pCompBlockSizes->GetSize() ;

    x_free(RawData);
    x_free(CompData);
    ANIM_KillInstance (&Anim);

	rCompDataSize = pCompData->GetLength();
}

//////////////////////////////////////////////////////////////////////
    
void CCharacterExport::ComputePropInfo (CMotion*        pMotion, 
                                        CCharacter*     pCharacter, 
                                        s32*            NMiscStreams, 
                                        u8*             PropUsedFlags )
{
    s32 j;
	anim*           pAnim = pMotion->GetAnim() ;
    CMotion*        pPropMotion;
    CCharacter*     pPropCharacter;
    anim*           pPropAnim;
    s32             StartStream;
    s32             PropIndex;
	f32				AngleConversion = ((f32)(1<<pMotion->GetExportBits()))/R_360;

    //---   Find next prop, fill out these three values, then
    //---   Execute the code below
    PropIndex      = 0;
    pPropMotion    = pMotion->m_pLinkMotion;
    pPropCharacter = pMotion->m_pLinkCharacter;

    if ((pPropCharacter == NULL) || (pPropMotion == NULL) || !pMotion->m_bExportLink)
		return;

    //---   Copy root info into stream
	pPropAnim  = pPropMotion->GetAnim() ;
	if( !pPropAnim || (pPropAnim->NBones < 1) )
		return;

    (*PropUsedFlags) = (1<<PropIndex);

	//---   Send warning if prop and anim don't overlap correctly
	if ( ((pPropAnim->StartFrame + pPropAnim->NFrames) > (pAnim->StartFrame + pAnim->NFrames)) ||
		 (pPropAnim->StartFrame < pAnim->StartFrame) )
	{
		//---	Add Error Line
		m_pProgressDialog->Fmt( "ERROR - Anim and Linked Anim do not overlap properly" ) ;
		m_nErrors++ ;
    }


    //---   Add streams for prop info
    StartStream = *NMiscStreams;
    (*NMiscStreams) += 6;
	ANIM_AddMiscStream( pAnim, StartStream+0 ) ;   // RX
	ANIM_AddMiscStream( pAnim, StartStream+1 ) ;   // RY
	ANIM_AddMiscStream( pAnim, StartStream+2 ) ;   // RZ
	ANIM_AddMiscStream( pAnim, StartStream+3 ) ;   // TX
	ANIM_AddMiscStream( pAnim, StartStream+4 ) ;   // TY
	ANIM_AddMiscStream( pAnim, StartStream+5 ) ;   // TZ

    //---   ADD PROP FRAMES TO ANIM
	for( j = 0 ; j < pMotion->GetNumFrames() ; j++ )
    {
		s32   PropFrame;

		PropFrame = (j+pAnim->StartFrame) - pPropAnim->StartFrame;
		if ((PropFrame>=0) && (PropFrame<pPropAnim->NFrames))
		{
			radian RX,RY,RZ;
			vector3d T;

			RX = ANIM_GetRotation( pPropAnim, PropFrame, 0 )*AngleConversion;
			RY = ANIM_GetRotation( pPropAnim, PropFrame, 1 )*AngleConversion;
			RZ = ANIM_GetRotation( pPropAnim, PropFrame, 2 )*AngleConversion;

			ANIM_GetTransValue (pPropAnim,PropFrame,&T);
			V3_Scale(&T,&T,16);

    	    ANIM_SetMiscValue( pAnim, j, StartStream + 0, (s16)RX ) ;
    	    ANIM_SetMiscValue( pAnim, j, StartStream + 1, (s16)RY ) ;
    	    ANIM_SetMiscValue( pAnim, j, StartStream + 2, (s16)RZ ) ;
    	    ANIM_SetMiscValue( pAnim, j, StartStream + 3, (s32)T.X ) ;
    	    ANIM_SetMiscValue( pAnim, j, StartStream + 4, (s32)T.Y ) ;
    	    ANIM_SetMiscValue( pAnim, j, StartStream + 5, (s32)T.Z ) ;
		}
        else
        {
    	    ANIM_SetMiscValue( pAnim, j, StartStream + 0, 0 ) ;
    	    ANIM_SetMiscValue( pAnim, j, StartStream + 1, 0 ) ;
    	    ANIM_SetMiscValue( pAnim, j, StartStream + 2, 0 ) ;
    	    ANIM_SetMiscValue( pAnim, j, StartStream + 3, 0 ) ;
    	    ANIM_SetMiscValue( pAnim, j, StartStream + 4, 0 ) ;
    	    ANIM_SetMiscValue( pAnim, j, StartStream + 5, 0 ) ;
        }
	}

    ANIM_MinimizeAngleDiff( pAnim, pAnim->NStreams-6+0, (1<<pMotion->GetExportBits()) );
    ANIM_MinimizeAngleDiff( pAnim, pAnim->NStreams-6+1, (1<<pMotion->GetExportBits()) );
    ANIM_MinimizeAngleDiff( pAnim, pAnim->NStreams-6+2, (1<<pMotion->GetExportBits()) );
}

//////////////////////////////////////////////////////////////////////

s16 BuildPointPosition(anim* Anim, skel* Skel, s32 Frame, vector3d* Pt, s32 BoneID)
{
    s32         i;
    radian3d    BoneRot[100];
    matrix4     BoneM[100];
    matrix4     WorldOrientM;
    vector3d    PtRelativeToBone;
    vector3d    PtInWorld;
    u32         X,Y,Z;
    s16         P;
    bool        UpBit;

    // Load rotation values into array
    for (i=0; i<Skel->NBones*3; i+=3)
    {
        BoneRot[(i/3)].X = ANIM_GetRotation( Anim, Frame, i+0 );
        BoneRot[(i/3)].Y = ANIM_GetRotation( Anim, Frame, i+1 );
        BoneRot[(i/3)].Z = ANIM_GetRotation( Anim, Frame, i+2 );
    }

    // Build WorldOrientM
    M4_Identity(&WorldOrientM);

    // Build Matrices for this frame
    SKEL_BuildMatrices2 (Skel, BoneRot, BoneM, &WorldOrientM);

    // Get point relative to bone
    V3_Sub(&PtRelativeToBone,Pt,&Skel->Bone[BoneID].OriginToBone);

    // Transform point into world using matrix
    M4_TransformVerts ( &BoneM[BoneID], &PtInWorld, &PtRelativeToBone, 1 );


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
                                         s32*           NMiscStreams, 
                                         u8*            PointUsedFlags )
{
    s32     iPoint;
    s32     StartStream;
    s32     Frame;
	anim*   pAnim = pMotion->GetAnim() ;
    skel*   pSkel = pCharacter->GetSkeleton()->GetSkel();

    *PointUsedFlags = 0;

    for( iPoint = 0 ; iPoint < MIN(8,pCharacter->m_SkelPointList.GetCount()) ; iPoint++ )
    {
        CSkelPoint *pSkelPoint = (CSkelPoint*)pCharacter->m_SkelPointList.IndexToPtr( iPoint ) ;

        // Read point from SkelPoint
        vector3d Pt;
        pSkelPoint->GetPosition(&Pt);

        if( pMotion->SkelPointGetCheck( iPoint ) )
        {
            // Point iPoint is present in this animation, add to streams
            (*PointUsedFlags) |= (1<<iPoint);

            // Add stream for point info
            StartStream = *NMiscStreams;
            (*NMiscStreams) += 1;
			ANIM_AddMiscStream( pAnim, StartStream ) ;

            // Loop through frames and compute point values
	        for( Frame = 0 ; Frame < pMotion->GetNumFrames() ; Frame++ )
            {
                s16 EncodedPoint;
				s32 Bone = pSkelPoint->GetBone() ;
				if( Bone == 0 )
				{
					Bone = 1 ;
				}
                EncodedPoint = BuildPointPosition( pAnim, pSkel, Frame, &Pt, Bone-1 );
    	        ANIM_SetMiscValue( pAnim, Frame, StartStream, (s16)EncodedPoint ) ;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////

void CCharacterExport::ComputeUserStreamInfo (CMotion*      pMotion, 
                                              CCharacter*   pCharacter, 
                                              s32*          NMiscStreams, 
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
    s32     NMiscStreams;
    u8      PropsUsed;
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
    s32     j;
	char	pstrPrefix[8];

	memset(pstrPrefix, 0, 8);

	if (pCharacter->m_ExportFrontEndSpecific == true)
	{
		x_strcpy(pstrPrefix, EXPORT_PREFIX_FE);
	}
	else
	{
		x_strcpy(pstrPrefix, EXPORT_PREFIX);
	}
	
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
	anim_event.Create( m_Target ) ;
	anim_BlockSize.Create( m_Target ) ;
//	anim_Blocks.Create( m_Target ) ;
	anim_Header.Create( m_Target ) ;

	anim_HeaderEvent.Create( m_Target ) ;
	anim_HeaderFlags.Create( m_Target ) ;
	anim_HeaderProps.Create( m_Target ) ;
	anim_HeaderPoints.Create( m_Target ) ;
	anim_HeaderStreams.Create( m_Target ) ;

    patch_CompDataOffset = anim_group.GetPosition();

    ///////////////////////////////////////////////////////////////////////
    //	SKELETON
    ///////////////////////////////////////////////////////////////////////

	m_pProgressDialog->Fmt( "Exporting Skeleton..." ) ;
	m_pProgressDialog->Fmt( " " ) ;

	//---	Write Skeleton
	CSkeleton *pSkeleton = pCharacter->GetSkeleton() ;
	for( int i = 0 ; i < pSkeleton->GetNumBones() ; i++ )
	{
		//---	Get Pointer to Bone
		skel_bone *pBone = pSkeleton->GetBoneFromIndex( i ) ;
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
        vector3d    OriginToBone;       // Global translation from origin to bone pos
        vector3d    ParentToBone;       // Local translation from parent to bone pos
        s16         BoneID;             // Index of this bone in array
        s16         ParentID;           // Index of parent bone in array or -1
        s16         MirrorID;           // This bone's mirror image lhand = rhand
    } anim_bone;
    */

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
	anim_Header.WriteString(        "/////////////////////////////////////////////////////////////////////////// \015\012"
							        "//                 CED ANIMATION HEADER FILE - DO NOT EDIT               // \015\012"
                                    "/////////////////////////////////////////////////////////////////////////// \015\012"
							        "\015\012" ) ;

	anim_HeaderEvent.WriteString(   "\015\012\015\012"
								    "/////////////////////////////////////////////////////////////////////////// \015\012"
                                    "//                                EVENTS                                 // \015\012"
                                    "/////////////////////////////////////////////////////////////////////////// \015\012"
								    "\015\012" ) ;
	anim_HeaderEvent.WriteStringFmt(   "#define %sNULL 0\015\012", pstrPrefix );


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


	//---	Check for Points that are not attached to any Bone
	POSITION Pos = pCharacter->m_SkelPointList.GetHeadPosition() ;
	while( Pos )
	{
		CSkelPoint *pSkelPoint = pCharacter->m_SkelPointList.GetNext( Pos ) ;
		if( pSkelPoint->GetBone() == 0 )
		{
			m_pProgressDialog->Fmt( "ERROR - Point '%s' not attached to Bone, using ROOT", pSkelPoint->GetName() ) ;
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
				anim_Header.WriteStringFmt( "//#define %s%s -1\015\012", pstrPrefix, Define ) ;
				m_CompMotionsArray[ i ].SetLength(0);
				m_CompBlockSizesArray[ i ].SetSize(0);
				continue;
			}

			//---	Add Line to Export Dialog
			m_pProgressDialog->SetPos( ++TotalAnimsAttempted ) ;
			m_pProgressDialog->Fmt( "%s", pMotion->GetPathName() ) ;

			//---	Ensure Anim is loaded and get pointer to it
			anim *pAnim = pMotion->GetAnim() ;
			if( !pAnim || !pMotion->m_bExport )
			{
				//---	Write Header File Info
				anim_Header.WriteStringFmt( "#define %s%s -1\015\012", pstrPrefix, Define ) ;

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
//			anim_Header.WriteStringFmt( "#define %s%s %d\015\012", pstrPrefix, Define, m_nAnimations ) ;
            anim_Header.WriteStringFmt( "#define %s%s %d \t// %s\015\012", pstrPrefix, Define, m_nAnimations, pMotion->GetPathName() );

            ///////////////////////////////////////////////////////////
			//---	Add MoveDir and RootDir
            ///////////////////////////////////////////////////////////
            NMiscStreams = 0;
			ANIM_AddMiscStream( pAnim, NMiscStreams++   ) ;     // MOVEDIR
			ANIM_AddMiscStream( pAnim, NMiscStreams++   ) ;     // FACEDIR
			for( j = 0 ; j < pMotion->GetNumFrames() ; j++ )
			{
				ANIM_SetMiscValue( pAnim, j, 0, (u16)(pMotion->GetMoveDir(j) * ExportAngleConversion) ) ;
				ANIM_SetMiscValue( pAnim, j, 1, (u16)(pMotion->GetFaceDir(j) * ExportAngleConversion) ) ;
			}


            ///////////////////////////////////////////////////////////
            //---   Add prop streams
            ///////////////////////////////////////////////////////////
            PropsUsed = 0;
            ComputePropInfo(pMotion,pCharacter,&NMiscStreams,&PropsUsed);

            ///////////////////////////////////////////////////////////
            //---   Add point streams
            ///////////////////////////////////////////////////////////
            PointsUsed = 0;
            ComputePointInfo(pMotion,pCharacter,&NMiscStreams,&PointsUsed);

            ///////////////////////////////////////////////////////////
            //---   Add user streams
            ///////////////////////////////////////////////////////////
            UserStreamsUsed = 0;
            ComputeUserStreamInfo(pMotion,pCharacter,&NMiscStreams,&UserStreamsUsed);

            ///////////////////////////////////////////////////////////
            //---   Check if this number of streams beats MaxStreams
            ///////////////////////////////////////////////////////////
            if (NMiscStreams + NBasicStreams > MaxStreams)
                MaxStreams = (NMiscStreams + NBasicStreams);


            ///////////////////////////////////////////////////////////
            //---   Export the compressed motion data
            ///////////////////////////////////////////////////////////
			s32 CompSize, CompBlocks;
			s32 NumExportedFrames;
			ExportMotionBlocks( i, pMotion, pCharacter, pCharacter->m_ExportFramesPerBlock, NumExportedFrames, CompSize, CompBlocks ) ;
//			m_pProgressDialog->Fmt( "I=%5d, off=%10d, size=%6d, ID_%s", m_nAnimations, CompMotionDataSize, CompSize, Define );

            ///////////////////////////////////////////////////////////
            //---   Export the motion header
            ///////////////////////////////////////////////////////////
			//---	Calculate Total Translation
			vector3d	tEnd, tStart ;
			ANIM_GetTransValue( pAnim, 0, &tStart ) ;
			ANIM_GetTransValue( pAnim, pMotion->GetNumFrames()-1, &tEnd ) ;
			V3_Scale( &tStart, &tStart, 16 ) ;
			V3_Scale( &tEnd, &tEnd, 16 ) ;


			//---	Calc RootDir and MoveDir
			u16		RootDirAtFrame0 = (u16)(pMotion->GetFaceDir(0)							*ExportAngleConversion) ;
			u16		RootDirAtFrameN = (u16)(pMotion->GetFaceDir(pMotion->GetNumFrames()-1)	*ExportAngleConversion) ;
			u16		MoveDirAtFrame0 = (u16)(pMotion->GetMoveDir(0)							*ExportAngleConversion) ;
			u16		MoveDirAtFrameN = (u16)(pMotion->GetMoveDir(pMotion->GetNumFrames()-1)	*ExportAngleConversion) ;

			anim_info.PadToBoundry( 4 ) ;

//				anim_info.Writes32( anim_Blocks.GetPosition() ) ;	// BlockOffset
			anim_info.Writes32( CompMotionDataSize ) ;				// BlockOffset
			anim_info.Writes16( pMotion->GetNumFrames() ) ;			// NFrames
			anim_info.Writes16( m_EventIndex ) ;					// EventIndex
			anim_info.Writes16( (s16)TotalBlocksExported ) ;		// BlockSizeIndex
			anim_info.Writeu16( RootDirAtFrame0 ) ;					// RootDirAtFrame0
			anim_info.Writeu16( RootDirAtFrameN ) ;					// RootDirAtFrameN
			anim_info.Writeu16( MoveDirAtFrame0 ) ;					// MoveDirAtFrame0
			anim_info.Writeu16( MoveDirAtFrameN ) ;					// MoveDirAtFrameN
			anim_info.Writes16( (s16)tStart.X ) ;					// TransXAtFrame0
			anim_info.Writes16( (s16)tStart.Y ) ;					// TransYAtFrame0
			anim_info.Writes16( (s16)tStart.Z ) ;					// TransZAtFrame0
			anim_info.Writes16( (s16)tEnd.X ) ;						// TransXAtFrameN
			anim_info.Writes16( (s16)tEnd.Y ) ;						// TransYAtFrameN
			anim_info.Writes16( (s16)tEnd.Z ) ;						// TransZAtFrameN
//				anim_info.Writes16( i-1 ) ;							// ID
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

			//---	Export Bits
			anim_info.Writeu8( (u8)pMotion->GetExportBits() );

            //---   Export NEvents
			anim_info.Writeu8( pMotion->GetNumEvents() ) ;      // Number of Events
            TotalEventsExported += pMotion->GetNumEvents();

            //---   Export Props, Points, UserStreams Present
            anim_info.Writeu8( PropsUsed );
            anim_info.Writeu8( PointsUsed );
            anim_info.Writeu8( UserStreamsUsed );

            //---   Count total number of streams in use and export
            {
                s32 NStreams;
                s32 k;
                NStreams = pCharacter->GetSkeleton()->GetNumBones()*3;
                NStreams += 3 + 2;
                for (k=0; k<8; k++)
                {
                    if (PropsUsed       & (1<<k)) NStreams += 6;
                    if (PointsUsed      & (1<<k)) NStreams += 1;
                    if (UserStreamsUsed & (1<<k)) NStreams += 1;
                }

                anim_info.Writeu8( (u8)NStreams );
            }

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
				u8	ExportBits;					// number of bits the data is packed into
                u8  NEvents;                    // Num Events in animation
                u8  PropsPresent;               // Which of the 8 props are present in anim
                u8  PointsPresent;              // Which of the 8 points are present in anim
                u8  UserStreamsPresent;         // Which user streams are present in anim
				u8	NStreams;					// number of streams
				u8  Pad[2];
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
					vector3d v ;
                    vector3d RootP;
					pEvent->UpdateEventPosition();
					pEvent->GetPosition( &v ) ;
					int iFrame = pEvent->GetFrame() ;
                    pMotion->GetTranslation(&RootP,iFrame);
					
					//---	Create or Find a Define for the Event
					CString Define = MakeDefine( pEvent->GetName(), pCharacter->m_ExportFrontEndSpecific ) ;
					s16 FoundID ;
					FindDefine( &FoundID, Define ) ;
//						if( pCharacter->m_ExportOptForceUpper )
//							Define.MakeUpper( ) ;
//
//						//---	Add Define to Header File
//						if( NewID )
//							anim_HeaderEvent.WriteStringFmt( "#define %s %d\015\012", Define, FoundID ) ;

					//---	Add Error if Event is outside frame range of motion
					if( (iFrame < 0) || (iFrame >= pMotion->GetNumFrames()) )
					{
						m_pProgressDialog->Fmt( "ERROR - EVENT '%s' OUTSIDE ANIMATION FRAME RANGE", pEvent->GetName() ) ;
						m_nErrors++ ;
                        iFrame = pMotion->GetNumFrames()-1;
					}

					//---	Write the Event
					anim_event.Writes16( iFrame ) ;
					anim_event.Writes16( FoundID ) ;
					anim_event.Writes16( (s16)(v.X*16) ) ;
					anim_event.Writes16( (s16)(v.Y*16) ) ;
					anim_event.Writes16( (s16)(v.Z*16) ) ;
					anim_event.Writes16( (s16)(RootP.X*16) ) ;
					anim_event.Writes16( (s16)(RootP.Y*16) ) ;
					anim_event.Writes16( (s16)(RootP.Z*16) ) ;
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

            //---	Delete Misc Streams
            for (j=0; j<NMiscStreams; j++)
				ANIM_DelMiscStream( pAnim, 0 ) ;

			//---	Increment Number of Animations Exported
			m_nAnimations++ ;
		}
	}


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

    // ASB Hack only here.  Since for some reason we re-defined some of the event ID's, I am adding them
    // to the export so we don't have to each time.  Mike Skinner
    anim_HeaderEvent.WriteStringFmt( "\n\n");
    anim_HeaderEvent.WriteStringFmt( "#define EVENT%sRUNNER_HIT_BASE       %sRUN_HIT_BASE\n", pstrPrefix, pstrPrefix );
    anim_HeaderEvent.WriteStringFmt( "#define EVENT%sSPECIAL               %sSPECIAL_EVENT\n", pstrPrefix, pstrPrefix );
    anim_HeaderEvent.WriteStringFmt( "#define EVENT%sCONTACT_FOLLOW_L      %sBAT_CONTACT_FOLLOW_L\n", pstrPrefix, pstrPrefix );
    anim_HeaderEvent.WriteStringFmt( "#define EVENT%sCONTACT_FOLLOW_R      %sBAT_CONTACT_FOLLOW_R\n", pstrPrefix, pstrPrefix );
    anim_HeaderEvent.WriteStringFmt( "#define EVENT%sCONTACT_POINT         %sBAT_CONTACT_PIONT\n", pstrPrefix, pstrPrefix );
    anim_HeaderEvent.WriteStringFmt( "#define EVENT%sBALL_CAUGHT           %sFIE_BALL_CAUGHT\n", pstrPrefix, pstrPrefix );
    anim_HeaderEvent.WriteStringFmt( "#define EVENT%sBALL_RELEASE          %sFIE_BALL_RELEASE\n", pstrPrefix, pstrPrefix );
    anim_HeaderEvent.WriteStringFmt( "#define EVENT%sTAG_RUNNER            %sFIE_TAG_OUT\n", pstrPrefix, pstrPrefix );
    anim_HeaderEvent.WriteStringFmt( "#define EVENT%sPOSITION_X            %sPIT_POSITION_X\n", pstrPrefix, pstrPrefix );
    anim_HeaderEvent.WriteStringFmt( "#define EVENT%sBALL_SWITCH_HANDS     %sFIE_BALL_CHANGE_HANDS\n", pstrPrefix, pstrPrefix );
    anim_HeaderEvent.WriteStringFmt( "#define EVENT%sSWING_STRIKEOUT       %sBAT_STRIKEOUT\n", pstrPrefix, pstrPrefix );


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
			anim_HeaderFlags.WriteStringFmt( "#define %s%s %*s (1<<%d)         // %s\015\012", pstrPrefix, Define, Len, "", i, pDoc->m_FlagDescription[i] ) ;
		}
	}

    ///////////////////////////////////////////////////////////////////////
    //	PROPS DEFINES
    ///////////////////////////////////////////////////////////////////////


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

        anim_HeaderPoints.WriteStringFmt( "#define %s%s %1d\015\012",pstrPrefix, Define,iPoint);
    }

    ///////////////////////////////////////////////////////////////////////
    //	USER STREAM DEFINES
    ///////////////////////////////////////////////////////////////////////
	anim_HeaderStreams.WriteString( "\015\012\015\012"
								    "/////////////////////////////////////////////////////////////////////////// \015\012"
                                    "//                              FINISHED                                 // \015\012"
                                    "/////////////////////////////////////////////////////////////////////////// \015\012"
								    "\015\012" ) ;



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
	anim_group.Cat( &anim_event ) ;

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

    ///////////////////////////////////////////////////////////////////////
    //	SAVE
    ///////////////////////////////////////////////////////////////////////

    //---   Display file size
	s32 HeaderSize = anim_group.GetPosition();
	s32 FileSize = HeaderSize + CompMotionDataSize;
	s32 TotalMotions = pCharacter->GetNumMotions();

    //---	Save Binary Data
	if( !anim_group.WriteToDisk( pFileName ) )
		return;

	//---	concatinate motion data
	for( i=0; i<TotalMotions; i++ )
		if( !m_CompMotionsArray[ i ].AppendToDisk( pFileName ) )
			break;

	//---	Save Header
	anim_Header.WriteToDisk( ::MFCU_SetExtension( pFileName, ".h" ) );

	//---	Print Total Frames Exported
	m_pProgressDialog->Fmt( "Total Anims exported  = %d  (of %d)", TotalAnimsExported, TotalMotions );
	m_pProgressDialog->Fmt( "Total Frames          = %d  (of %d)", TotalFramesExported, TotalFrames );
	m_pProgressDialog->Fmt( "Total Events          = %d", TotalEventsExported );
	m_pProgressDialog->Fmt( "Total Blocks          = %d  (+%d frames for %d total exported frames)", TotalBlocksExported, TotalBlockFrames-TotalFramesExported, TotalBlockFrames );
	m_pProgressDialog->Fmt( "" );
	m_pProgressDialog->Fmt( "Total Header Data     = %d", HeaderSize );
	m_pProgressDialog->Fmt( "Total Compressed Data = %d", CompMotionDataSize );
	m_pProgressDialog->Fmt( "Total File Size       = %d", FileSize ) ;

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
