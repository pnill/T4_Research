///////////////////////////////////////////////////////////////////////////
//
//  AM_Skel.hpp
//
///////////////////////////////////////////////////////////////////////////

#ifndef _AM_SKEL_HPP_
#define _AM_SKEL_HPP_


///////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
///////////////////////////////////////////////////////////////////////////
struct URotData;


///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////
#include "AM_Export.h"
#include "x_types.hpp"


///////////////////////////////////////////////////////////////////////////
// DEFINES / MACROS
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
// PRIVATE PROTOTYPES
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
//	AMSKL_GetBoneMatrices	- gets an array of matrices based on supplied
//							  rotation data and transformation information	
//
//	AMSKL_GetBoneMatrix		- same as AMSKL_GetBoneMatrices() except for
//							  one bone specifically.
//
//							  NOTE: This function is more expensive for
//									bones further from the root bone.
//
//
//	anim_group*		AnimGroup		- group containing the skeleton information
//	matrix4*		DestMs			- array of matricies to recieve the 
//	matrix4&		rDestM			- matrix of the bone to retreive
//	s32				BoneIndex		- index of the bone to retreive
//	URotData*		pBoneData		- array of bone rotations
//	u32				BoneDataFormat	- flags describing the format of the data in the bone data array
//	xbool			bMirror			- T/F should the pose be mirrored
//	xbool			bUniformScale	- T/F should the mesh be uniformly scaled around the bones, or stretched to fit.
//	f32				WorldScale		- World scale of the bones
//	radian			WorldYaw		- World yaw of the pose.
//	const vector3&	WorldPos		- world position
//	matrix4*		pTransform		- if not NULL, this is used to transform the entire pose to a new location independant
//									  from any motion playback.
//
//									  NOTE: this transformation is performed in addition to the WorldYaw,
//											WorldPos, and WorldScale transformations already provided.
//
//									  NOTE: Also, this transformation has a much hire cost when performed on global
//											rotaiton data compared to local rotation data.  However, without this
//											step, global rotation data should be faster than local.
//	
//
///////////////////////////////////////////////////////////////////////////
void    AMSKL_GetBoneMatrices       (const anim_group*	AnimGroup,
                                     matrix4*			DestMs,
                                     const URotData*	pBoneData,
									 u32				BoneDataFormat,
                                     xbool				bMirror,
									 xbool				bUniformScale,
                                     f32				WorldScale,
                                     radian				WorldYaw,
                                     const vector3&		WorldPos,
									 const matrix4*		pTransform = NULL);

///////////////////////////////////////////////////////////////////////////
void    AMSKL_GetBoneMatrix         (const anim_group*	AnimGroup,
                                     matrix4&			rDestM,
                                     s32				BoneIndex,
                                     const URotData*	pBoneData,
									 u32				BoneDataFormat,
                                     xbool				bMirror,
									 xbool				bUniformScale,
                                     f32				WorldScale,
                                     radian				WorldYaw,
                                     const vector3&		WorldPos,
									 const matrix4*		pTransform = NULL );

/* function commented out because it isn't yet tested, and I don't have time to test it now.
///////////////////////////////////////////////////////////////////////////
void	AMSKL_GetBoneOrient			(const anim_group*	AnimGroup,
									 URotData&			rOrient,
									 s32				BoneIndex,
									 const URotData*	pBoneData,
									 u32				BoneDataFormat,
									 xbool				bMirror,
									 radian				WorldYaw,
									 const matrix4*		pTransform = NULL )
*/
///////////////////////////////////////////////////////////////////////////
//  FINISHED
///////////////////////////////////////////////////////////////////////////


#endif
