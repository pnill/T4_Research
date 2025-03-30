// Skeleton.h: interface for the CSkeleton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKELETON_H__B8064083_E009_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_SKELETON_H__B8064083_E009_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ieArchive.h"

#include "Skel.h"
#include "Anim.h"
#include "Motion.h"
#include "FileTags.h"

class CRenderContext ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_SKELETON_END = IE_TAG_SKELETON,

	IE_TAG_SKELETON_PATHNAME,
} ;

/////////////////////////////////////////////////////////////////////////////
// CCharacter

class CSkeleton : public CObject
{
	DECLARE_DYNCREATE(CSkeleton)

// Attributes
public:

private:
	CString		m_PathName ;									// PathName to Skeleton

	CSkel		m_Skel ;										// Quagmire Skeleton Structure
	matrix4		*m_pMatrices ;									// Pointer to Skeleton Matrices
	matrix4		m_WorldMatrix ;

// Constructors
public:
	CSkeleton();
	~CSkeleton();

// Operations
public:
	virtual void Serialize(CieArchive& a) ;
	CSkeleton &operator=( CSkeleton & ) ;						// Right side is the argument.

	bool		CreateFromFile( const char *pFileName ) ;		// Create Skeleton from File
	bool		CreateFromASF( const char *pFileName ) ;		// Create Skeleton from ASF File
	const char	*GetPathName() ;								// Get PathName

	int			GetNumBones() ;									// Get Number of Bones in Skeleton
	s32			FindBoneFromName( const CString& Name );		// Get bone index given a bone name
	SSkelBone	*GetBoneFromIndex( int iBone ) ;				// Get Pointer to Bone
	SSkelBone	*GetBoneFromName( const CString& Name );		// Get Pointer to Bone by Name
	int			GetBoneNumParents( SSkelBone *pBone ) ;			// Get Number of Parents a Bone has
	int			GetBoneMirror( int iBone ) ;					// Get Mirror of Bone

	void		SetNeutralPose( bool bMirror=FALSE ) ;			// Set Neutral Pose for Skeleton
	void		SetPose( matrix4 WorldBase, CMotion *pMotion, float nFrame, bool Blend=false, bool Mirror=false ) ;		// Set Pose for Skeleton
	void		SetPose( const vector3& Pos, const radian3& Rot, CMotion *pMotion, float nFrame, bool Blend=false, bool Mirror=false );
	void		SetPose2MotionBlend( matrix4 WorldBase, CMotion *pMotion, float nFrame, CMotion *pLastMotion, float nLastFrame, f32 Ratio, bool bMergeMotion, bool bBlend, bool Mirror=false ) ;
	void		SetPoseMovie( CMotion *pMotion, float nFrame, int Mirror, bool Blend=false ) ;	// Set Pose for Movie (No Translate)
	
	matrix4		*GetPosedBoneMatrix( int iBone ) ;				// Get Posed Bone Matrix
	CSkel		*GetSkel() ;									// Get Pointer to skel
	matrix4		*GetWorldMatrix() ;								// Get Pointer to World Matrix

	void		Render( CRenderContext *pRC,
						COLORREF Color = RGB(0,0,0),
						bool DrawAxes = false ) ;	// Draw Skeleton

	//---	build world orientation matrix
	void		BuildWorldOrient(matrix4*   M, 
								 f32        Scale, 
								 f32        Tx, 
								 f32        Ty, 
								 f32        Tz,
								 radian     Rx, 
								 radian     Ry, 
								 radian     Rz);

	//---	functions for building matrices
	void BuildMatrices			( anim*      Anim,
								  s32        AnimFrame,
								  matrix4*   DstMatrices,
								  matrix4*   WorldOrientM);

	void BuildMatricesBlend		( anim*      Anim,
								  s32        AnimFrame0,
								  s32        AnimFrame1,
								  f32        AnimFrameFrac,
								  matrix4*   DstMatrices,
								  matrix4*   WorldOrientM );

	void BuildMatrices2MotionBlend( anim*      Anim,
								   s32        AnimFrame0,
								   s32        AnimFrame1,
								   f32        AnimFrameFrac,
								   anim*      LastAnim,
								   s32		  LastAnimFrame,
								   f32        LastAnimFrac,
								   matrix4*   DstMatrices,
								   matrix4*   WorldOrientM );

	void BuildMatrices_WithPlacement( vector3&	Pos,
									 radian3&	Rot,
									 anim*      Anim,
								     s32        AnimFrame,
								     matrix4*   DstMatrices,
								     matrix4*   WorldOrientM );

	void BuildMatricesBlend_WithPlacement( vector3&	Pos,
										  radian3&	Rot,
										  anim*     Anim,
										  s32       AnimFrame0,
										  s32       AnimFrame1,
										  f32       AnimFrameFrac,
										  matrix4*  DstMatrices,
										  matrix4*  WorldOrientM );

	void BuildMatricesXZY		( anim*      Anim,
								  s32        AnimFrame,
								  matrix4*   DstMatrices,
								  matrix4*   WorldOrientM );

	void BuildMatrices2			( radian3*   Rotations,
								  matrix4*   DstMatrices,
								  matrix4*   WorldOrientM );

	void GetBoneMatrix			( anim*      Anim,
								  s32        AnimFrame,
								  matrix4*   DstMatrix,
								  matrix4*   WorldOrientM,
								  s32        BoneID);

	void BuildMatricesMir		( anim*      Anim,
								  s32        AnimFrame,
								  matrix4*   DstMatrices,
								  matrix4*   WorldOrientM);

	void BuildMatricesMirBlend	( anim*      Anim,
								  s32        AnimFrame0,
								  s32        AnimFrame1,
								  f32        AnimFrameFrac,
								  matrix4*   DstMatrices,
								  matrix4*   WorldOrientM );

	void GetBoneMatrixMir		( anim*      Anim,
								  s32        AnimFrame,
								  matrix4*   DstMatrix,
								  matrix4*   WorldOrientM,
								  s32        BoneID);

	//---	functions to break matrices back to their local versions
	void BreakMatrices			( anim*      Anim,
							      s32        AnimFrame,
							      matrix4*   DstMatrices,
							      matrix4*   WorldOrientM);

// Implementation
protected:
	void Initialize() ;									// Initialize Data
	void Destroy () ;									// Destroy
	void Copy (CSkeleton &Dst, CSkeleton &Src) ;		// Copy


	//---	functions for building matrices
	void RecursivelyBuildMatrix						( anim*     Anim,
													  s32       AnimFrame,
													  matrix4*  DstMatrices,
													  s32       BoneID,
													  matrix4*  ParentMatrix );

	void RecursivelyBuildMatrixBlend				( anim*		Anim,
													  s32		AnimFrame0,
													  s32		AnimFrame1,
													  f32		AnimFrameFrac,
													  matrix4*	DstMatrices,
													  s32		BoneID,
													  matrix4*	ParentMatrix );

	void RecursivelyBuildMatrix2MotionBlend			( anim*		Anim,
													  s32		AnimFrame0,
													  s32		AnimFrame1,
													  f32		AnimFrameFrac,
													  anim*		LastAnim,
													  s32		LastFrame,
													  f32		LastAnimFrac,
													  matrix4*	DstMatrices,
													  s32		BoneID,
													  matrix4*	ParentMatrix );

	void RecursivelyBuildMatrix2					( radian3*  Rotations,
													  matrix4*  DstMatrices,
													  s32       BoneID,
													  matrix4*  ParentMatrix );

	void RecursivelyBuildMatrixMir					( anim*     Anim,
													  s32       AnimFrame,
													  matrix4*  DstMatrices,
													  s32		BoneID,
													  matrix4*	ParentMatrix );

	void RecursivelyBuildMatrixMirBlend				( anim*		Anim,
													  s32		AnimFrame0,
													  s32		AnimFrame1,
													  f32		AnimFrameFrac,
													  matrix4*	DstMatrices,
													  s32		BoneID,
													  matrix4*	ParentMatrix );

	//---	functions to recursively break matrices back to their local versions
	void RecursivelyBreakMatrix						( anim*      Anim,
													  s32        AnimFrame,
													  matrix4*   DstMatrices,
													  s32        BoneID,
													  matrix4*   ParentMatrix);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

#endif // !defined(AFX_SKELETON_H__B8064083_E009_11D1_AEC0_00A024569FF3__INCLUDED_)
