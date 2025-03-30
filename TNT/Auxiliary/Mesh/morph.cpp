/*****************************************************************************
*
*	morph.cpp - generic morph structure
*
*	5/8/00
*
*****************************************************************************/

#include "morph.h"

#include "x_plus.hpp"

/*****************************************************************************/
#define ID_REFERENCE			"REFERENCE"

#define ID_MORPH_TARGET_SET		"MORPH_TARGET_SET"
#define ID_MORPH_TARGET			"MORPH_TARGET"

#define ID_MORPH				"MORPH"
#define ID_MORPH_AUDIO			"MORPH_AUDIO"

#define ID_NUM_MORPH_STREAMS	"NUM_MORPH_STREAMS"
#define ID_MORPH_STREAM			"MORPH_STREAM"


/*****************************************************************************/
/*
*MORPH_TARGET_SET[2] = "head_targets"
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

/*****************************************************************************
*
*	Class:	morph_target_set::object
*
*****************************************************************************/
object::object() : m_nTargets(0), m_pTarget(NULL) { }

object::~object()
{
	delete [] m_pTarget;
	m_pTarget = NULL;
}

/*****************************************************************************/
// save a target set object

xbool object::Save(CTextFile& Tf)
{
	if (m_pTarget == FALSE)
		return(TRUE);


	Tf.BeginToken(ID_MORPH_TARGET_SET, m_nTargets, m_Name);

	Tf.Token(ID_REFERENCE, m_MeshName);

	for(s32 cTarget = 0; cTarget < m_nTargets; cTarget++)
	{
		target	*pTarget = &m_pTarget[cTarget];

		Tf.BeginToken(ID_MORPH_TARGET, pTarget->m_nDeltas, pTarget->m_Name);
		Tf.Token(ID_REFERENCE, pTarget->m_ChunkName);

    	Tf.Put("\n; iVert,	vDelta\n");
		for(s32 cDelta = 0; cDelta < pTarget->m_nDeltas; cDelta++)
		{
			Tf.Put(pTarget->m_pDelta[cDelta].m_iVert);
			Tf.Put("\t");
			Tf.Put(pTarget->m_pDelta[cDelta].m_vDelta);
			Tf.Put("\n");
		}

		Tf.EndToken();
	}

	Tf.EndToken();

	return(TRUE);
}

/*****************************************************************************/
// load a target set object

xbool object::Load(CTextFile& Tf)
{
	CTextFile::tokentype	Token;
	xbool					bFound = FALSE,
							bSuccess = FALSE;
	s32						iTarget = 0;


	ASSERT(m_pTarget == NULL);

	while(Tf.GetToken(&Token))
	{
		// -------------------------------------------------------------------
		// read target set
		if (Token == ID_MORPH_TARGET_SET)
		{
			ASSERTS((bFound == FALSE), "file contains more than one target set");
			bFound = TRUE;

			// get the object name
			x_strncpy(m_Name, Token.String, sizeof(m_Name));


			// alloc the targets
			m_nTargets = Token.Count;
			ASSERT(m_nTargets);

			m_pTarget = new target [m_nTargets];


			Tf.MarkSection();
			while(Tf.GetSectionToken(&Token))
			{
				// -----------------------------------------------------------
				// read mesh name
				if (Token == ID_REFERENCE)
				{
					x_strncpy(m_MeshName, Token.String, sizeof(m_MeshName));
				}

				// -----------------------------------------------------------
				// read targets
				else if (Token == ID_MORPH_TARGET)
				{
					target	*pTarget = &m_pTarget[iTarget];


					// get the target name
					x_strncpy(pTarget->m_Name, Token.String, sizeof(pTarget->m_Name));


					// alloc the deltas
					pTarget->m_nDeltas = Token.Count;
					pTarget->m_pDelta = new delta [pTarget->m_nDeltas];


					// get the chunk name					
					bSuccess = Tf.GetToken(&Token);
					ASSERT(bSuccess);
					ASSERT(Token == ID_REFERENCE);

					x_strncpy(pTarget->m_ChunkName, Token.String, sizeof(pTarget->m_ChunkName));


					// get the deltas
					for(s32 cDelta = 0; cDelta < pTarget->m_nDeltas; cDelta++)
					{
						Tf.Get(&pTarget->m_pDelta[cDelta].m_iVert);
						Tf.Get(&pTarget->m_pDelta[cDelta].m_vDelta);
					}

					iTarget++;
				}
				// -----------------------------------------------------------
				// skip any other data
				else
				{
					Tf.SkipSection();
				}
			}
		}
		// -------------------------------------------------------------------
		// skip any other data
		else
		{
			Tf.SkipSection();
		}
	}


	// check we read all the targets
	if (bFound)
	{
		ASSERT(iTarget == m_nTargets);
	}


	return(bFound);
}

/*****************************************************************************/

}



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

namespace morph_anim
{

/*****************************************************************************
*
*	Class:	morph_anim::object
*
*****************************************************************************/
object::object()
{
	m_nStreams = 0;
	m_pStream = NULL;
}


object::~object()
{
	delete [] m_pStream;
	m_pStream = NULL;
}

/*
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

//	char			m_Name[ 256 ];
//	char			m_MorphTargetSetName[ 256 ];
//
//	char			m_AudioName[256];
//	f32				m_AudioOffset;							// offset to start
////JONNY: need this !?			
//
//	s32				m_Duration;								// length of animation in (1/60)

/*****************************************************************************/
// save a morph anim object

xbool object::Save(CTextFile& Tf)
{
	if (m_pStream == NULL)
		return(TRUE);


	Tf.BeginToken(ID_MORPH, m_Name);


	Tf.BeginToken(ID_MORPH_AUDIO, 0);

	Tf.Put("; Filename, Offset\n");
	Tf.Put(fs("\"%s\",\t", m_AudioName));
	Tf.Put(m_AudioOffset);

	Tf.EndToken();


	Tf.Token(ID_NUM_MORPH_STREAMS, m_nStreams);

	for(s32 cStream = 0; cStream < m_nStreams; cStream++)
	{
		stream	*pStream = &m_pStream[cStream];

		Tf.BeginToken(ID_MORPH_AUDIO, 0);

		Tf.Put(fs("\"%s\",\n", pStream->m_TargetName));

		Tf.Put("; Value...\n");

		for(s32 cKey = 0; cKey < pStream->m_nKeys; cKey++)
		{
			Tf.Put(pStream->m_pKey[cKey]);

			if ((cKey % 10) == 0)
				Tf.Put(",\n");
			else
				Tf.Put("\n");
		}

		Tf.EndToken();
	}

	Tf.EndToken();


	return(TRUE);
}

/*****************************************************************************/
// load a morph anim object

xbool object::Load(CTextFile& Tf)
{
	ASSERT(FALSE);

	return(TRUE);
}

/*****************************************************************************/

}

/*****************************************************************************/
