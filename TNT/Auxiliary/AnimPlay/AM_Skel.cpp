///////////////////////////////////////////////////////////////////////////
//
//  AM_SKEL.CPP
//
///////////////////////////////////////////////////////////////////////////
#include "x_debug.hpp"
#include "am_play.h"
#include "AM_MocapPlayer.h"
#include "AM_Stats.h"

///////////////////////////////////////////////////////////////////////////
//
//	BuildLocalMatrix
//
//		This function builds a bone matrix by transforming a set of rotations
//	by a parent matrix.  Everything is completely local
//
inline void BuildLocalMatrix(		  matrix4&	M, 
								const radian3&	R, 
								const vector3&	T, 
								const matrix4&	PM )
{

#if 0
	M.Identity();
	M.RotateX( R.Pitch );
	M.RotateZ( R.Roll );
	M.RotateY( R.Yaw );
	M.Translate( T );
#else
    register f32 sx, sy, sz;
    register f32 cx, cy, cz;
    register f32 cxcy,sxcy,cxsy,sxsy;

    // calc basic values
	x_sincos( R.Pitch, sx, cx );
	x_sincos( R.Yaw, sy, cy );
	x_sincos( R.Roll, sz, cz );

    cxcy = cx*cy;
    sxcy = sx*cy;
    sxsy = sx*sy;
    cxsy = cx*sy;

    M.M[0][0] = cy*cz;
    M.M[1][0] = -cxcy*sz + sxsy;
    M.M[2][0] = sxcy*sz + cxsy;
    M.M[0][1] = sz;
    M.M[1][1] = cx*cz;
    M.M[2][1] = -sx*cz;
    M.M[0][2] = -sy*cz;
    M.M[1][2] = cxsy*sz + sxcy;
    M.M[2][2] = -sxsy*sz + cxcy;

    //---	set the translation
    M.M[3][0] = T.X;
    M.M[3][1] = T.Y;
    M.M[3][2] = T.Z;

    M.M[0][3] = M.M[1][3] = M.M[2][3] = 0.0f;
    M.M[3][3] = 1.0f;    
#endif

	M.Transform( PM );
}

///////////////////////////////////////////////////////////////////////////
//
//	BuildGlobalMatrix
//
//		This function builds a matrix from a set of rotations which are
//	themselves global and need not be transformed by a parent matrix.
//	The translation is still assumed to need to be transformed by the
//	parent translation.
//
static inline void BuildGlobalMatrix( matrix4&			M, 
                                      const radian3&    R, 
                                      const vector3&    T, 
                                      const matrix4&    PM,
                                      radian			WY)
{
    register f32 sx, sy, sz;
    register f32 cx, cy, cz;
    register f32 cxcy,sxcy,cxsy,sxsy;

    // calc basic values
	x_sincos( R.Pitch, sx, cx );
	x_sincos( R.Yaw+WY, sy, cy );
	x_sincos( R.Roll, sz, cz );

    cxcy = cx*cy;
    sxcy = sx*cy;
    sxsy = sx*sy;
    cxsy = cx*sy;

    M.M[0][0] = cy*cz;
    M.M[1][0] = -cxcy*sz + sxsy;
    M.M[2][0] = sxcy*sz + cxsy;
    M.M[0][1] = sz;
    M.M[1][1] = cx*cz;
    M.M[2][1] = -sx*cz;
    M.M[0][2] = -sy*cz;
    M.M[1][2] = cxsy*sz + sxcy;
    M.M[2][2] = -sxsy*sz + cxcy;

    //solve translations
//#ifndef VERSION_PS2
#if 1
    M.M[3][0] = (PM.M[0][0]*T.X) + (PM.M[1][0]*T.Y) + (PM.M[2][0]*T.Z) + PM.M[3][0];
    M.M[3][1] = (PM.M[0][1]*T.X) + (PM.M[1][1]*T.Y) + (PM.M[2][1]*T.Z) + PM.M[3][1];
    M.M[3][2] = (PM.M[0][2]*T.X) + (PM.M[1][2]*T.Y) + (PM.M[2][2]*T.Z) + PM.M[3][2];
#else // PS2 assembly code which should perform the previous calculation - try it out sometime
    // Using VU0 Assembly
    asm __volatile__
    ("
        lqc2    vf11, 0x30(%1) # load pParent col 3
        lqc2    vf12, 0x00(%2) # load the Trans vector
        lqc2    vf10, 0x20(%1) # load pParent col 2
        lqc2    vf09, 0x10(%1) # load pParent col 1
        lqc2    vf08, 0x00(%1) # load pParent col 0
        vmulaw.xyz  ACC, vf11, vf00w    # translate
        vmaddaz.xyz ACC, vf10, vf12z    # translate
        vmadday.xyz ACC, vf09, vf12y    # translate
        vmaddx.xyz vf02, vf08, vf12x    # translate
        sqc2    vf02, 0x30(%0) # store result in col 3 of pM

    ": : "r" (&M.M[0][0]) , "r" (&PM.M[0][0]) , "r" (&T) );
#endif

    //fill out edges
    M.M[0][3] = M.M[1][3] = M.M[2][3] = 0;
    M.M[3][3] = 1;
}


///////////////////////////////////////////////////////////////////////////
//
//	BuildLocalMatrixQuat
//
//		This function builds a bone matrix by transforming a set of rotations
//	by a parent matrix.  Everything is completely local
//
inline void BuildLocalMatrixQuat(	matrix4&	M, 
									const quaternion&	Q, 
									const vector3&		T, 
									const matrix4&		PM )
{

#if 0

	Q.BuildMatrix( M );
	M.Translate( T );

#else

    f32 tx  = 2.0f*Q.X;    
    f32 ty  = 2.0f*Q.Y;    
    f32 tz  = 2.0f*Q.Z;
    f32 twx = tx*Q.W;    
    f32 twy = ty*Q.W;    
    f32 twz = tz*Q.W;
    f32 txx = tx*Q.X;    
    f32 txy = ty*Q.X;    
    f32 txz = tz*Q.X;
    f32 tyy = ty*Q.Y;   
    f32 tyz = tz*Q.Y;   
    f32 tzz = tz*Q.Z;

    M.M[0][0] = 1.0f-(tyy+tzz);   
    M.M[1][0] = txy-twz;          
    M.M[2][0] = txz+twy;
    M.M[0][1] = txy+twz;          
    M.M[1][1] = 1.0f-(txx+tzz);   
    M.M[2][1] = tyz-twx;
    M.M[0][2] = txz-twy;          
    M.M[1][2] = tyz+twx;          
    M.M[2][2] = 1.0f-(txx+tyy);

    M.M[3][0] = T.X;
	M.M[3][1] = T.Y;
	M.M[3][2] = T.Z;

    M.M[0][3] = M.M[1][3] = M.M[2][3] = 0.0f;
    M.M[3][3] = 1.0f;

#endif

	M.Transform( PM );
}


///////////////////////////////////////////////////////////////////////////
//
//	BuildGlobalMatrixQuat
//
//		This function builds a bone matrix by transforming a set of rotations
//	by a parent matrix.  Everything is completely local
//
inline void BuildGlobalMatrixQuat(	matrix4&	M, 
									const quaternion&	Q,
									const vector3&	T, 
									const matrix4&	PM,
									const quaternion&	WY)
{
	quaternion q;

#if 0

	//---	rotate the quaternion about the world yaw
	q = Q;
	q.RotateY( WY );

#else

	//---	rotate the quaternion about the world yaw
    q.W	=	WY.W*Q.W - WY.Y*Q.Y;
    q.X =	WY.W*Q.X + WY.Y*Q.Z;
    q.Y =	WY.W*Q.Y + WY.Y*Q.W;
    q.Z =	WY.W*Q.Z - WY.Y*Q.X;


#endif

    f32 tx  = 2.0f*q.X;    
    f32 ty  = 2.0f*q.Y;    
    f32 tz  = 2.0f*q.Z;
    f32 twx = tx*q.W;    
    f32 twy = ty*q.W;    
    f32 twz = tz*q.W;
    f32 txx = tx*q.X;    
    f32 txy = ty*q.X;    
    f32 txz = tz*q.X;
    f32 tyy = ty*q.Y;   
    f32 tyz = tz*q.Y;   
    f32 tzz = tz*q.Z;

    M.M[0][0] = 1.0f-(tyy+tzz);   
    M.M[1][0] = txy-twz;          
    M.M[2][0] = txz+twy;
    M.M[0][1] = txy+twz;          
    M.M[1][1] = 1.0f-(txx+tzz);   
    M.M[2][1] = tyz-twx;
    M.M[0][2] = txz-twy;          
    M.M[1][2] = tyz+twx;          
    M.M[2][2] = 1.0f-(txx+tyy);

    //solve translations
    M.M[3][0] = (PM.M[0][0]*T.X) + (PM.M[1][0]*T.Y) + (PM.M[2][0]*T.Z) + PM.M[3][0];
    M.M[3][1] = (PM.M[0][1]*T.X) + (PM.M[1][1]*T.Y) + (PM.M[2][1]*T.Z) + PM.M[3][1];
    M.M[3][2] = (PM.M[0][2]*T.X) + (PM.M[1][2]*T.Y) + (PM.M[2][2]*T.Z) + PM.M[3][2];

    //fill out edges
    M.M[0][3] = M.M[1][3] = M.M[2][3] = 0;
    M.M[3][3] = 1;
}



///////////////////////////////////////////////////////////////////////////
void AMSKL_GetBoneMatrices  (const anim_group*	AnimGroup,
                             matrix4*			DestMs,
                             const URotData*	pBoneData,
							 u32				BoneDataFormat,
                             xbool				bMirror,
							 xbool				bUniformScale,
                             f32				WorldScale,
                             radian				WorldYaw,
                             const vector3&		WorldPos,
							 const matrix4*		pTransform )
{
	//=======================
	AMSTATS_BUILDMATS_START//==== PROFILING
	//=======================

    matrix4     WorldOrientM;
    s32         i;
	matrix4*    M;
	matrix4*	End;
	s32			NBones = AnimGroup->NBones;
	anim_bone*	pBones = AnimGroup->Bone;
	anim_bone*	pBone;
    radian3     MirRot;
    vector3     MirP2B;
	quaternion	MirQuat;
	quaternion	WorldYawQ;
	f32			sy, cy;

	if( BoneDataFormat&ANIMINFO_FLAG_QUATERNION )
	{
		if( BoneDataFormat&ANIMINFO_FLAG_GLOBALROTS )
		{
			//---	default the origin's parent to the transform supplied.
			WorldOrientM.Identity();

			//---	build world rotation quaternion to be applied to all matrices
			x_sincos( WorldYaw*0.5f, sy, cy );
			WorldYawQ.W = cy;
			WorldYawQ.X = 0;
			WorldYawQ.Y = sy;
			WorldYawQ.Z = 0;

			if( bMirror )
			{
				// Decide on rotation and translation values for bone
				ASSERT(pBones[0].MirrorID==0); // assume that the first bone has no mirror
				MirrorQuat( pBoneData[0]._q, MirQuat );
				MirrorTrans( pBones[0].ParentToBone, MirP2B );
				BuildGlobalMatrixQuat( DestMs[0], MirQuat, MirP2B, WorldOrientM, WorldYawQ );

				// Loop through the rest of the bones
				for (i=1; i<NBones; i++)
				{
					pBone		= &pBones[i];
					MirrorQuat( pBoneData[pBone->MirrorID]._q, MirQuat );
					MirrorTrans( pBones[pBone->MirrorID].ParentToBone, MirP2B );

					// Build matrix for this bone
					BuildGlobalMatrixQuat( DestMs[i], MirQuat, MirP2B, DestMs[ pBone->ParentID ], WorldYawQ );
				}
			}
			else
			{
				// Decide on rotation and translation values for bone
				BuildGlobalMatrixQuat( DestMs[0], pBoneData[0]._q, pBones[0].ParentToBone, WorldOrientM, WorldYawQ );

				// Loop through all bones hitting parents before children
				for (i=1; i<NBones; i++)
				{
					pBone		= &pBones[i];

					// Build matrix for this bone
					BuildGlobalMatrixQuat( DestMs[i], pBoneData[i]._q, pBone->ParentToBone, DestMs[pBone->ParentID], WorldYawQ );
				}
			}
		}
		else
		{
			// Build WorldOrientM 
			WorldOrientM.Identity();

			x_sincos( WorldYaw, sy, cy );
			WorldOrientM.M[0][0] =  cy;
			WorldOrientM.M[2][0] =  sy;
			WorldOrientM.M[0][2] = -sy;
			WorldOrientM.M[2][2] =  cy;

			// Decide on rotation and translation values for bone
			if (bMirror)
			{
				ASSERT(pBones[0].MirrorID==0); // assume that the first bone has no mirror
				MirrorQuat( pBoneData[0]._q, MirQuat );
				MirrorTrans( pBones[0].ParentToBone, MirP2B );
				BuildLocalMatrixQuat( DestMs[0], MirQuat, MirP2B, WorldOrientM );

				// Loop through all bones hitting parents before children
				for (i=1; i<NBones; i++)
				{
					pBone		= &pBones[i];
					MirrorQuat( pBoneData[pBone->MirrorID]._q, MirQuat );
					MirrorTrans( pBones[pBone->MirrorID].ParentToBone, MirP2B );

					// Build matrix for this bone
					BuildLocalMatrixQuat( DestMs[i], MirQuat, MirP2B, DestMs[pBone->ParentID] );
				}
			}
			else
			{
				BuildLocalMatrixQuat( DestMs[0], pBoneData[0]._q, pBones[0].ParentToBone, WorldOrientM );

				// Loop through all bones hitting parents before children
				for (i=1; i<NBones; i++)
				{
					pBone		= &pBones[i];

					// Build matrix for this bone
					BuildLocalMatrixQuat( DestMs[i], pBoneData[i]._q, pBone->ParentToBone, DestMs[pBone->ParentID] );
				}
			}
		}
	}
	else // Euler rotations
	{
		if( BoneDataFormat&ANIMINFO_FLAG_GLOBALROTS )
		{
			// Build WorldOrientM 
			WorldOrientM.Identity();

			if( bMirror )
			{
				// Decide on rotation and translation values for bone
				ASSERT(pBones[0].MirrorID==0); // assume that the first bone has no mirror
				MirrorRot( pBoneData[0]._r, MirRot );
				MirrorTrans( pBones[0].ParentToBone, MirP2B );
				BuildGlobalMatrix( DestMs[0], MirRot, MirP2B, WorldOrientM, WorldYaw );

				// Loop through the rest of the bones
				for (i=1; i<NBones; i++)
				{
					pBone		= &pBones[i];
					MirrorRot( pBoneData[pBone->MirrorID]._r, MirRot );
					MirrorTrans( pBones[pBone->MirrorID].ParentToBone, MirP2B );

					// Build matrix for this bone
					BuildGlobalMatrix( DestMs[i], MirRot, MirP2B, DestMs[ pBone->ParentID ], WorldYaw );
				}
			}
			else
			{
				// Decide on rotation and translation values for bone
				BuildGlobalMatrix( DestMs[0], pBoneData[0]._r, pBones[0].ParentToBone, WorldOrientM, WorldYaw );

				// Loop through all bones hitting parents before children
				for (i=1; i<NBones; i++)
				{
					pBone		= &pBones[i];

					// Build matrix for this bone
					BuildGlobalMatrix( DestMs[i], pBoneData[i]._r, pBone->ParentToBone, DestMs[pBone->ParentID], WorldYaw );
				}
			}
		}
		else
		{
			// Build WorldOrientM 
			WorldOrientM.Identity();

			x_sincos( WorldYaw, sy, cy );
			WorldOrientM.M[0][0] =  cy;
			WorldOrientM.M[2][0] =  sy;
			WorldOrientM.M[0][2] = -sy;
			WorldOrientM.M[2][2] =  cy;

			// Decide on rotation and translation values for bone
			if( bMirror )
			{
				ASSERT(pBones[0].MirrorID==0); // assume that the first bone has no mirror
				MirrorRot( pBoneData[0]._r, MirRot );
				MirrorTrans( pBones[0].ParentToBone, MirP2B );
				BuildLocalMatrix( DestMs[0], MirRot, MirP2B, WorldOrientM );

				// Loop through all bones hitting parents before children
				for (i=1; i<NBones; i++)
				{
					pBone		= &pBones[i];
					MirrorRot( pBoneData[pBone->MirrorID]._r, MirRot );
					MirrorTrans( pBones[pBone->MirrorID].ParentToBone, MirP2B );

					// Build matrix for this bone
					BuildLocalMatrix( DestMs[i], MirRot, MirP2B, DestMs[pBone->ParentID] );
				}
			}
			else
			{
				BuildLocalMatrix( DestMs[0], pBoneData[0]._r, pBones[0].ParentToBone, WorldOrientM );

				// Loop through all bones hitting parents before children
				for (i=1; i<NBones; i++)
				{
					pBone		= &pBones[i];

					// Build matrix for this bone
					BuildLocalMatrix( DestMs[i], pBoneData[i]._r, pBone->ParentToBone, DestMs[pBone->ParentID] );
				}
			}
		}
	}

	//
	//	perform scaling (NOTE: all this should be able to be placed into the matrix building above in some form.
	//	uniform scaling can be put into local rotation matrix building easily, and non linear can be put into
	//	global rotation matrix building easily.
	//
	if( WorldScale != 1.0f )
	{
		if( bUniformScale )
		{
			M	= &DestMs[0];
			End = &DestMs[NBones];
			while( M != End )
			{
				M->UniScale( WorldScale );
				M->M[3][0] += WorldPos.X;
				M->M[3][1] += WorldPos.Y;
				M->M[3][2] += WorldPos.Z;

				M++;
			}
		}
		else
		{
			M	= &DestMs[0];
			End = &DestMs[NBones];
			while( M != End )
			{
				M->M[3][0] = (M->M[3][0]*WorldScale) + WorldPos.X;
				M->M[3][1] = (M->M[3][1]*WorldScale) + WorldPos.Y;
				M->M[3][2] = (M->M[3][2]*WorldScale) + WorldPos.Z;
				M++;
			}
		}
	}
	else
	{
		M	= &DestMs[0];
		End = &DestMs[NBones];
		while( M != End )
		{
			M->M[3][0] += WorldPos.X;
			M->M[3][1] += WorldPos.Y;
			M->M[3][2] += WorldPos.Z;

			M++;
		}
	}

	//
	//	transform all of the matricies by the given transform, note that this process
	//	can be reduced for local bone rots by transformiming the WorldOrientM, if everything
	//	is already going.
	//
	if( pTransform )
	{
		matrix4* pBoneMat	= DestMs;
		matrix4* pEnd		= &DestMs[NBones];
		while( pBoneMat != pEnd )
		{
			pBoneMat->Transform( *pTransform );
			++pBoneMat;
		}
	}

	//=====================
	AMSTATS_BUILDMATS_END//==== PROFILING
	//=====================
}

///////////////////////////////////////////////////////////////////////////
static matrix4 s_AMSKL_GetBoneMatrix_Matrices[30];
void AMSKL_GetBoneMatrix    (const anim_group*	AnimGroup,
                             matrix4&			rDestM,
                             s32				BoneIndex,
                             const URotData*	pBoneData,
							 u32				BoneDataFormat,
                             xbool				bMirror,
							 xbool				bUniformScale,
                             f32				WorldScale,
                             radian				WorldYaw,
                             const vector3&		WorldPos,
							 const matrix4*		pTransform )
{
	//======================
	AMSTATS_BUILDMAT_START//==== PROFILING
	//======================

    radian3			Rot;
//    vector3			Trans;
    matrix4			WorldOrientM;
    s32				i;
	anim_bone*		pBones = AnimGroup->Bone;
	anim_bone*		pBone;
	const radian3*	pMirRots;
	s32				BoneStackIndex;
	s32				BoneStack[30];
	quaternion		MirQuat;
	quaternion		WorldYawQ;
	f32				sy, cy;

    ASSERT(AnimGroup && pBoneData);
    ASSERT(AnimGroup->NBones < 30);

    // Build path from bone to root
	i = BoneIndex;
    BoneStackIndex=0;
    while (i!=-1)
    {
        // Push bone on stack
        BoneStack[BoneStackIndex++] = i;
        i = AnimGroup->Bone[i].ParentID;
    }

	//---	initialize pointers to be stepped through the bone stack processing bone matrices.  Start
	//		processing after the root node because the root node is going to be done outside the loop
	//		to handle special cases.  ASSERT that the skipped bone matrix is the root.
	ASSERT(BoneStack[BoneStackIndex-1]==0);
	s32* pCur	= &BoneStack[BoneStackIndex-2];
	s32* pEnd	= &BoneStack[0];

	if( BoneDataFormat&ANIMINFO_FLAG_QUATERNION )
	{
		if( BoneDataFormat&ANIMINFO_FLAG_GLOBALROTS )
		{
			WorldOrientM.Identity();

			//---	build world rotation quaternion to be applied to all matrices
			x_sincos( WorldYaw*0.5f, sy, cy );
			WorldYawQ.W = cy;
			WorldYawQ.X = 0;
			WorldYawQ.Y = sy;
			WorldYawQ.Z = 0;

			if( bMirror )
			{
				// Decide on rotation and translation values for bone
				MirrorQuat( pBoneData[0]._q, MirQuat );
				BuildGlobalMatrixQuat( s_AMSKL_GetBoneMatrix_Matrices[0], MirQuat, pBones[0].ParentToBone, WorldOrientM, WorldYawQ );

				// Loop through the rest of the bones
				while( pCur >= pEnd )
				{
					i			= *pCur--;
					pBone		= &pBones[i];
					MirrorQuat( pBoneData[pBone->MirrorID]._q, MirQuat );

					// Build matrix for this bone
					BuildGlobalMatrixQuat( s_AMSKL_GetBoneMatrix_Matrices[i], MirQuat, pBone->ParentToBone, s_AMSKL_GetBoneMatrix_Matrices[ pBone->ParentID ], WorldYawQ );
				}
			}
			else
			{
				// Decide on rotation and translation values for bone
				BuildGlobalMatrixQuat( s_AMSKL_GetBoneMatrix_Matrices[0], pBoneData[0]._q, pBones[0].ParentToBone, WorldOrientM, WorldYawQ );

				// Loop through all bones hitting parents before children
				while( pCur >= pEnd )
				{
					i			= *pCur--;
					pBone		= &pBones[i];

					// Build matrix for this bone
					BuildGlobalMatrixQuat( s_AMSKL_GetBoneMatrix_Matrices[i], pBoneData[i]._q, pBone->ParentToBone, s_AMSKL_GetBoneMatrix_Matrices[pBone->ParentID], WorldYawQ );
				}
			}
		}
		else
		{
			// Build WorldOrientM 
			WorldOrientM.Identity();
			x_sincos( WorldYaw, sy, cy );
			WorldOrientM.M[0][0] =  cy;
			WorldOrientM.M[2][0] =  sy;
			WorldOrientM.M[0][2] = -sy;
			WorldOrientM.M[2][2] =  cy;

			// Decide on rotation and translation values for bone
			if (bMirror)
			{
				MirrorQuat( pBoneData[0]._q, MirQuat );
				BuildLocalMatrixQuat( s_AMSKL_GetBoneMatrix_Matrices[0], MirQuat, pBones[0].ParentToBone, WorldOrientM );

				// Loop through all bones hitting parents before children
				while( pCur >= pEnd )
				{
					i			= *pCur--;
					pBone		= &pBones[i];
					MirrorQuat( pBoneData[pBone->MirrorID]._q, MirQuat );

					// Build matrix for this bone
					BuildLocalMatrixQuat( s_AMSKL_GetBoneMatrix_Matrices[i], MirQuat, pBone->ParentToBone, s_AMSKL_GetBoneMatrix_Matrices[pBone->ParentID] );
				}
			}
			else
			{
				BuildLocalMatrixQuat( s_AMSKL_GetBoneMatrix_Matrices[0], pBoneData[0]._q, pBones[0].ParentToBone, WorldOrientM );

				// Loop through all bones hitting parents before children
				while( pCur >= pEnd )
				{
					i			= *pCur--;
					pBone		= &pBones[i];

					// Build matrix for this bone
					BuildLocalMatrixQuat( s_AMSKL_GetBoneMatrix_Matrices[i], pBoneData[i]._q, pBone->ParentToBone, s_AMSKL_GetBoneMatrix_Matrices[pBone->ParentID] );
				}
			}
		}
	}
	else
	{
		if( BoneDataFormat&ANIMINFO_FLAG_GLOBALROTS )
		{
			// Build WorldOrientM 
			WorldOrientM.Identity();

			if( bMirror )
			{
				// Decide on rotation and translation values for bone
				Rot.Pitch	=  pBoneData[0]._r.Pitch;
				Rot.Yaw		= -pBoneData[0]._r.Yaw;
				Rot.Roll	= -pBoneData[0]._r.Roll;
				BuildGlobalMatrix( s_AMSKL_GetBoneMatrix_Matrices[0], Rot, pBones[0].ParentToBone, WorldOrientM, WorldYaw );

				// Loop through the rest of the bones
				while( (u32)pCur >= (u32)pEnd )
				{
					i			= *pCur--;
					pBone		= &pBones[i];
					pMirRots	= &pBoneData[pBone->MirrorID]._r;

					// Decide on rotation and translation values for bone
					Rot.Pitch	=  pMirRots->Pitch;
					Rot.Yaw		= -pMirRots->Yaw;
					Rot.Roll	= -pMirRots->Roll;

					// Build matrix for this bone
					BuildGlobalMatrix( s_AMSKL_GetBoneMatrix_Matrices[i], Rot, pBone->ParentToBone, s_AMSKL_GetBoneMatrix_Matrices[ pBone->ParentID ], WorldYaw );
				}
			}
			else
			{
				// Decide on rotation and translation values for bone
				Rot		= pBoneData[0]._r;
				BuildGlobalMatrix( s_AMSKL_GetBoneMatrix_Matrices[0], Rot, pBones[0].ParentToBone, WorldOrientM, WorldYaw );

				// Loop through all bones hitting parents before children
				while( (u32)pCur >= (u32)pEnd )
				{
					i			= *pCur--;
					pBone		= &pBones[i];

					// Build matrix for this bone
					BuildGlobalMatrix( s_AMSKL_GetBoneMatrix_Matrices[i], pBoneData[i]._r, pBone->ParentToBone, s_AMSKL_GetBoneMatrix_Matrices[pBone->ParentID], WorldYaw );
				}
			}
		}
		else
		{
			// Build WorldOrientM 
			WorldOrientM.Identity();

			//
			//	add in the world yaw
			//
			x_sincos( WorldYaw, sy, cy );
			WorldOrientM.M[0][0] =  cy;
			WorldOrientM.M[2][0] =  sy;
			WorldOrientM.M[0][2] = -sy;
			WorldOrientM.M[2][2] =  cy;

			// Decide on rotation and translation values for bone
			if (bMirror)
			{
				Rot.Pitch	=  pBoneData[0]._r.Pitch;
				Rot.Yaw		= -pBoneData[0]._r.Yaw;
				Rot.Roll	= -pBoneData[0]._r.Roll;
				BuildLocalMatrix( s_AMSKL_GetBoneMatrix_Matrices[0], Rot, pBones[0].ParentToBone, WorldOrientM );

				// Loop through all bones hitting parents before children
				while( (u32)pCur >= (u32)pEnd )
				{
					i			= *pCur--;
					pBone		= &pBones[i];
					pMirRots	= &pBoneData[pBone->MirrorID]._r;

					// Decide on rotation and translation values for bone
					Rot.Pitch	=  pMirRots->Pitch;
					Rot.Yaw		= -pMirRots->Yaw;
					Rot.Roll	= -pMirRots->Roll;

					// Build matrix for this bone
					BuildLocalMatrix( s_AMSKL_GetBoneMatrix_Matrices[i], Rot, pBone->ParentToBone, s_AMSKL_GetBoneMatrix_Matrices[pBone->ParentID] );
				}
			}
			else
			{
				BuildLocalMatrix( s_AMSKL_GetBoneMatrix_Matrices[0], pBoneData[0]._r, pBones[0].ParentToBone, WorldOrientM );

				// Loop through all bones hitting parents before children
				while( (u32)pCur >= (u32)pEnd )
				{
					i			= *pCur--;
					pBone		= &pBones[i];

					// Build matrix for this bone
					BuildLocalMatrix( s_AMSKL_GetBoneMatrix_Matrices[i], pBoneData[i]._r, pBone->ParentToBone, s_AMSKL_GetBoneMatrix_Matrices[pBone->ParentID] );
				}
			}
		}
	}

	//---	shift the matricies to the objects actual world position
	//		for matricies which are built locally, it may be better to 
	//		tranlate the root matrix before building the rest, however
	//		it may also be better to add the world translation at the
	//		moment that matrices are needed (allowing for collision and
	//		other operations to change this object's position right up until
	//		it is rendered).  Leaving it here for for now. - Kevin
	rDestM = s_AMSKL_GetBoneMatrix_Matrices[BoneIndex];

	//
	//	perform scaling (uniform or position only)
	//
	if( WorldScale!=1.0f )
	{
		if( bUniformScale )
		{
			rDestM.UniScale( WorldScale );
			rDestM.M[3][0] += WorldPos.X;
			rDestM.M[3][1] += WorldPos.Y;
			rDestM.M[3][2] += WorldPos.Z;
		}
		else
		{
			rDestM.M[3][0] = (rDestM.M[3][0]*WorldScale) + WorldPos.X;
			rDestM.M[3][1] = (rDestM.M[3][1]*WorldScale) + WorldPos.Y;
			rDestM.M[3][2] = (rDestM.M[3][2]*WorldScale) + WorldPos.Z;
		}
	}
	else
	{
		rDestM.M[3][0] += WorldPos.X;
		rDestM.M[3][1] += WorldPos.Y;
		rDestM.M[3][2] += WorldPos.Z;
	}

	//
	//	transform all of the matricies by the given transform, note that this process
	//	can be reduced for local bone rots by transformiming the WorldOrientM, if everything
	//	is already going.
	//
	if( pTransform )
		rDestM.Transform( *pTransform );

	//====================
	AMSTATS_BUILDMAT_END//==== PROFILING
	//====================
}

/* function commented out because I don't have time to test it now.
///////////////////////////////////////////////////////////////////////////
void AMSKL_GetBoneOrient	( const anim_group*	AnimGroup,
							  URotData&			rOrient,
							  s32				BoneIndex,
							  const URotData*	pBoneData,
							  u32				BoneDataFormat,
							  xbool				bMirror,
							  radian			WorldYaw,
							  const matrix4*	pTransform )
{
	//======================
	AMSTATS_BUILDMAT_START//==== PROFILING
	//======================

    radian3     Rot;
    s32         i;
	anim_bone*	pBones = AnimGroup->Bone;
	s32			BoneStackIndex;
	s32			BoneStack[30];
	quaternion	Quat, Q;
	quaternion	MirQuat;
	radian3		MirRot;
	quaternion	WorldYawQ;
	f32			sy, cy;

    ASSERT(AnimGroup && pBoneData);
    ASSERT(AnimGroup->NBones < 30);

	if( BoneDataFormat&ANIMINFO_FLAG_GLOBALROTS )
	{
		if( BoneDataFormat&ANIMINFO_FLAG_QUATERNION )
		{
			//---	get the quaternion from this bone data assuming tha it is global
			if( bMirror )
				MirrorQuat( pBoneData[pBones[BoneIndex].MirrorID]._q, rOrient._q );
			else
				rOrient._q = pBoneData[BoneIndex]._q;

			//---	rotate the quaternion by the world yaw if one is given
			if( WorldYaw )
			{
				x_sincos( WorldYaw*0.5f, sy, cy );
				Quat = rOrient._q;
				rOrient._q.W = cy*Quat.W - sy*Quat.Y;
				rOrient._q.X = cy*Quat.X + sy*Quat.Z;
				rOrient._q.Y = cy*Quat.Y + sy*Quat.W;
				rOrient._q.Z = cy*Quat.Z - sy*Quat.X;
			}
		}
		else
		{
			//	get the bone's rotation value
			if( bMirror )
				MirrorRot( pBoneData[pBones[BoneIndex].MirrorID]._r, rOrient._r );
			else
				rOrient._r = pBoneData[BoneIndex]._r;

			rOrient._r.Yaw += WorldYaw;
		}
	}
	else
	{
		// Build path from bone to root
		i = BoneIndex;
		BoneStackIndex=0;
		while (i!=-1)
		{
			// Push bone on stack
			BoneStack[BoneStackIndex++] = i;
			i = AnimGroup->Bone[i].ParentID;
		}

		//---	initialize pointers to be stepped through the bone stack processing bone matrices.  Start
		//		processing after the root node because the root node is going to be done outside the loop
		//		to handle special cases.  ASSERT that the skipped bone matrix is the root.
		ASSERT(BoneStack[BoneStackIndex-1]==0);
		s32* pCur	= &BoneStack[BoneStackIndex-2];
		s32* pEnd	= &BoneStack[0];

		if( BoneDataFormat&ANIMINFO_FLAG_QUATERNION )
		{
			//	prepare the yaw rotation quaternion
			x_sincos( WorldYaw*0.5f, sy, cy );
			WorldYawQ.W = cy;
			WorldYawQ.X = 0;
			WorldYawQ.Y = sy;
			WorldYawQ.Z = 0;

			// Decide on rotation and translation values for bone
			if (bMirror)
			{
				MirrorQuat( pBoneData[0]._q, MirQuat );
				Quat = WorldYawQ*MirQuat;
				
				// Loop through all bones hitting parents before children
				while( pCur >= pEnd )
				{
					MirrorQuat( pBoneData[pBones[*pCur--].MirrorID]._q, MirQuat );

					// Build matrix for this bone
					Quat = Quat*MirQuat;
				}
			}
			else
			{
				Quat = WorldYawQ*pBoneData[0]._q;

				// Loop through all bones hitting parents before children
				while( pCur >= pEnd )
				{
					// Build matrix for this bone
					Quat = Quat*pBoneData[*pCur--]._q;
				}
			}

			//	save the requested value
			rOrient._q = Quat;
		}
		else
		{
			// Decide on rotation and translation values for bone
			if( bMirror )
			{
				MirrorRot( pBoneData[0]._r, Rot );
				Rot.Yaw += WorldYaw;
				RotToQuat( Rot, Quat );

				// Loop through all bones hitting parents before children
				while( (u32)pCur >= (u32)pEnd )
				{
					MirrorRot( pBoneData[pBones[*pCur--].MirrorID]._r, MirRot );
					RotToQuat( MirRot, Q );
					Quat = Quat * Q;
				}
			}
			else
			{
				Rot = pBoneData[0]._r;
				Rot.Yaw += WorldYaw;
				RotToQuat( Rot, Quat );

				// Loop through all bones hitting parents before children
				while( (u32)pCur >= (u32)pEnd )
				{
					RotToQuat( pBoneData[*pCur--]._r, Q );
					Quat = Quat*Q;
				}
			}

			//	save the requested value
			QuatToRot( Quat, rOrient._r );
		}
	}

	//
	//	transform all of the matricies by the given transform, note that this process
	//	can be reduced for local bone rots by transformiming the WorldOrientM, if everything
	//	is already going.
	//
	if( pTransform )
	{
		matrix4 M;
		RotXZYToMat( M, Rot );
		M.Transform( *pTransform );
		MatToRotXZY( M, Rot );
	}

	//====================
	AMSTATS_BUILDMAT_END//==== PROFILING
	//====================
}

*/

///////////////////////////////////////////////////////////////////////////

