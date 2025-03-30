#ifndef __BASIC_MORPH_H
#define __BASIC_MORPH_H

//****************************************************************************
#include "x_types.hpp"
#include "x_math.hpp"

#include "CTextFile.h"



/*****************************************************************************
*
* forward declarations
*
*****************************************************************************/

class CMaxExport;
class CMayaExport;



//****************************************************************************
/*
*MORPH_TARGET_SET[2] = "head_targets"
{
    *REFERENCE = "mesh_filename"

    *TARGET[4] = "head_target1"
    {
        "chunk_name",

    	; iVert,	vDelta
    	0,		0.3, 0.234, 0.634
    	5		0.5, 0.34, 0.24
    	6,		0.1, 0.4, 0.46
    	9,		0.22, 0.14, 0.87
    }

    *TARGET[4] = "head_target2"
    {
        "chunk_name",

    	; iVert,	vDelta
    	0,		0.3, 0.234, 0.634
    	5		0.5, 0.34, 0.24
    	6,		0.1, 0.4, 0.46
    	9,		0.22, 0.14, 0.87
    }
}

*MORPH_STREAM = "head talk"
{
	*AUDIO
	{
		; Filename, Offset
		"talking_head.wav", 10.0
	}

	*NUM_STREAMS = 2
	*SUBSTREAM[30] = "head_talk_00"
	{
        "head_target1",

		; Value...
		0.0, 0.0, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.8, 0.9,
		0.0, 0.0, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.8, 0.9,
		0.0, 0.0, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.8, 0.9,
	}

	*SUBSTREAM[30] = "head_talk_01"
	{
        "head_target2",

		; Value...
		1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1,
		0.0, 0.0, 0.2, 0.3, 0.5, 0.6, 0.7, 0.8, 0.8, 0.9,
		0.9, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
	}
}
*/


//****************************************************************************
namespace BasicMorph
{

/*****************************************************************************
*
*	Class:	BasicMorph::CDelta
*
*****************************************************************************/
class CDelta
{
	friend class CTargetSet;
	friend class ::CMaxExport;

// construction
public:
	CDelta() : m_iVert(-1) {}

// data
public:
	s32				m_iVert;
	vector3			m_vDelta;
};



/*****************************************************************************
*
*	Class:	BasicMorph::CTarget
*
*****************************************************************************/
class CTarget
{
	friend class CTargetSet;
	friend class ::CMaxExport;

// construction
public:
	CTarget() : m_nDeltas(0), m_pDeltas(NULL) {}
	~CTarget() { delete [] m_pDeltas; m_pDeltas = NULL; }

// operations
public:
    const char* GetName(void) const;
    const char* GetChunkName(void) const;

	xbool	IsValidDeltaId	(s32 iDelta) const;
	s32		GetNumDeltas	(void) const;
	CDelta	GetDelta		(s32 iDelta) const;

// data
protected:
	char			m_Name[256];
	char			m_ChunkName[256];

	s32				m_nDeltas;
	CDelta			*m_pDeltas;
};



/*****************************************************************************
*
*	Class:	BasicMorph::CTargetSet
*
*****************************************************************************/
class CTargetSet
{
	friend class ::CMaxExport;

// construction
public:
	CTargetSet();
	~CTargetSet();

// operations
public:
	xbool			IsValidTargetId	(s32 iTarget) const;

    const char*     GetName         (void) const;
    const char*     GetMeshName     (void) const;
	s32				GetNumTargets	(void) const;
	const CTarget&	GetTarget		(s32 iTarget) const;
	CTarget*		GetTargetPtr	(s32 iTarget) const;

	xbool			Save			(CTextFile& Tf) const;
	xbool			Load			(CTextFile& Tf);

// data
protected:
	char			m_Name[256];
	char			m_MeshName[256];

	s32				m_nTargets;
	CTarget			*m_pTargets;
};



/*****************************************************************************
*
*	Class:	BasicMorph::CSubstream
*
*****************************************************************************/
class CSubstream
{
	friend	class CStream;

// construction
public:
	CSubstream() : m_nKeys(0), m_pKey(NULL) {}
	~CSubstream() { delete [] m_pKey; m_pKey = NULL; }

// data
protected:
	char			m_TargetName[256];

	s32				m_nKeys;
	f32				*m_pKey;
};



/*****************************************************************************
*
*	Class:	BasicMorph::CAnimation
*
*****************************************************************************/
class CStream
{
// construction
public:
	CStream();
	~CStream();

// operations
public:
	xbool Save(CTextFile& Tf) const;
	xbool Load(CTextFile& Tf);

protected:

// data
protected:
	char			m_Name[256];
	char			m_MorphTargetSetName[256];
//JONNY: need this !?

	char			m_AudioName[256];
	f32				m_AudioOffset;							// offset to start
//JONNY: need this !?			

	s32				m_Duration;								// length of animation in (1/60)

	s32				m_nSubstreams;
	CSubstream		*m_pSubstreams;
};

//****************************************************************************

}

//****************************************************************************

#endif // __BASIC_MORPH_H
