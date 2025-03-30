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

	skel		m_Skel ;										// Quagmire Skeleton Structure
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
	skel_bone	*GetBoneFromIndex( int iBone ) ;				// Get Pointer to Bone
	skel_bone	*GetBoneFromName( const CString& Name );		// Get Pointer to Bone by Name
	int			GetBoneNumParents( skel_bone *pBone ) ;			// Get Number of Parents a Bone has
	int			GetBoneMirror( int iBone ) ;					// Get Mirror of Bone

	void		SetNeutralPose( ) ;								// Set Neutral Pose for Skeleton
	void		SetPose( CMotion *pMotion, float nFrame, bool Blend=false ) ;		// Set Pose for Skeleton
	void		SetPoseMovie( CMotion *pMotion, float nFrame, int Mirror, bool Blend=false ) ;	// Set Pose for Movie (No Translate)
	
	matrix4		*GetPosedBoneMatrix( int iBone ) ;				// Get Posed Bone Matrix
	skel		*GetSkel() ;									// Get Pointer to skel
	matrix4		*GetWorldMatrix() ;								// Get Pointer to World Matrix

	void		Render( CRenderContext *pRC,
						COLORREF Color = RGB(0,0,0),
						bool DrawAxes = false ) ;	// Draw Skeleton

// Implementation
protected:
	void Initialize() ;									// Initialize Data
	void Destroy () ;									// Destroy
	void Copy (CSkeleton &Dst, CSkeleton &Src) ;		// Copy

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

#endif // !defined(AFX_SKELETON_H__B8064083_E009_11D1_AEC0_00A024569FF3__INCLUDED_)
