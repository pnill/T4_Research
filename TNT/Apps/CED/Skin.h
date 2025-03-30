// Skin.h: interface for the CSkin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKIN_H__F33F4C3A_13DE_46D9_8F13_7E0B4017032A__INCLUDED_)
#define AFX_SKIN_H__F33F4C3A_13DE_46D9_8F13_7E0B4017032A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ieArchive.h"
#include "FileTags.h"
#include "Skin_Mesh.h"
//#include "x_types.hpp"

////////////////////////////////////////////////////////////////////////////
//---	FORWARD DECLARATIONS
class CSkeleton;
class CCharacter;
class CSkel;
class mesh::object;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_SKIN_END = IE_TAG_SKIN+1,

	IE_TAG_SKIN_NAME,
	IE_TAG_SKIN_PATHNAME,
} ;


////////////////////////////////////////////////////////////////////////////
//	The CLASS
////////////////////////////////////////////////////////////////////////////
class CSkin  
{
public:
	CSkin();
	virtual ~CSkin();

	////////////////////////////////////////////////////////////////////////////
	//	Initialization functionality
	////////////////////////////////////////////////////////////////////////////

	xbool	CreateFromFile	( const char* pFileName );

	xbool	FitToSkeleton	( CSkel& rSkel );

	////////////////////////////////////////////////////////////////////////////
	//	Implementation
	////////////////////////////////////////////////////////////////////////////

			const	CString&	GetName		( void );
	virtual	void				Serialize	( CieArchive& a );
					CSkin&		operator=	( CSkin& rSkin );						// Right side is the argument.

			mesh::object*		GetMesh		( void ) { if(!m_pSkin)CreateFromFile(m_File); return m_pSkin; }

	////////////////////////////////////////////////////////////////////////////
	//	Protected implementation
	////////////////////////////////////////////////////////////////////////////
protected:

	xbool	CreateFromMTF	( const char* pFileName );

	//---	mesh management
	void	RemapVertWeights( CSkel& rSkel, mesh::object* pGeom );
	
	xbool	LimitMatricesPerFace( s32 MaxMats, s32& MaxFaceWeights, s32& MaxVertexWeights );
	xbool	LimitMatricesPerChunk( s32 MaxMats, s32& MaxFaceWeights, s32& MaxVertexWeights );

	////////////////////////////////////////////////////////////////////////////
	//	Data Members
	////////////////////////////////////////////////////////////////////////////
protected:

	CString			m_Name;
	CString			m_File;
	mesh::object*	m_pSkin;
};

#endif // !defined(AFX_SKIN_H__F33F4C3A_13DE_46D9_8F13_7E0B4017032A__INCLUDED_)
