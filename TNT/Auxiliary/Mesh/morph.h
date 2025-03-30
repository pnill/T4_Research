#ifndef __MORPH_H
#define __MORPH_H

/*****************************************************************************/
#include "x_types.hpp"
#include "x_math.hpp"

#include "CTextFile.h"

/*****************************************************************************/
/*
*MORPH_TARGETS[2] = "head_targets"
{
    *REFERENCE = "mesh_filename"

    *MORPH_TARGET[4] = "head_target1"
    {
        "chunk_name",

    	; iVert,	vDelta
    	0,		0.3, 0.234, 0.634
    	5		0.5, 0.34, 0.24
    	6,		0.1, 0.4, 0.46
    	9,		0.22, 0.14, 0.87
    }

    *MORPH_TARGET[4] = "head_target2"
    {
        "chunk_name",

    	; iVert,	vDelta
    	0,		0.3, 0.234, 0.634
    	5		0.5, 0.34, 0.24
    	6,		0.1, 0.4, 0.46
    	9,		0.22, 0.14, 0.87
    }
}

*MORPH = "head talk"
{
	*MORPH_AUDIO
	{
		; Filename, Offset
		"talking_head.wav", 10.0
	}

	*NUM_MORPH_STREAMS = 2
	*MORPH_STREAM[30] = "head_talk_00"
	{
        "head_target1",

		; Value...
		0.0, 0.0, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.8, 0.9,
		0.0, 0.0, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.8, 0.9,
		0.0, 0.0, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.8, 0.9,
	}

	*MORPH_STREAM[30] = "head_talk_01"
	{
        "head_target2",

		; Value...
		1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1,
		0.0, 0.0, 0.2, 0.3, 0.5, 0.6, 0.7, 0.8, 0.8, 0.9,
		0.9, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
	}
}
*/
/*****************************************************************************/
namespace morph_target_set
{

struct delta
{
// construction
	delta() : m_iVert(-1) {}

// data
	s32				m_iVert;
	vector3			m_vDelta;
};



struct target
{
// construction
	target() : m_nDeltas(0), m_pDelta(NULL) {}
	~target() { delete [] m_pDelta; m_pDelta = NULL; }

// data
	char			m_Name[256];
	char			m_ChunkName[256];

	s32				m_nDeltas;
	delta			*m_pDelta;
};


/*****************************************************************************
*
*	Class:	morph_target_set::object
*
*****************************************************************************/
class object
{
// construction
public:
	object();
	~object();

// operations
public:
	xbool Save(CTextFile& Tf);
	xbool Load(CTextFile& Tf);

protected:

// data
public:
	char			m_Name[ 256 ];
	char			m_MeshName[256];

	s32				m_nTargets;
	target			*m_pTarget;
};

}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
namespace morph_anim
{

struct stream
{
// construction
	stream() : m_nKeys(0), m_pKey(NULL) {}
	~stream() { delete [] m_pKey; m_pKey = NULL; }

// data
	char			m_TargetName[ 256 ];

	s32				m_nKeys;
	f32				*m_pKey;
};


/*****************************************************************************
*
*	Class:	morph_anim::object
*
*****************************************************************************/
struct object
{
// construction
public:
	object();
	~object();

// operations
public:
	xbool Save(CTextFile& Tf);
	xbool Load(CTextFile& Tf);

protected:


// data
public:
	char			m_Name[ 256 ];
	char			m_MorphTargetSetName[ 256 ];
//JONNY: need this !?

	char			m_AudioName[256];
	f32				m_AudioOffset;							// offset to start
//JONNY: need this !?			

	s32				m_Duration;								// length of animation in (1/60)

	s32				m_nStreams;
	stream			*m_pStream;
};

}

/*****************************************************************************/

#endif // __MORPH_H