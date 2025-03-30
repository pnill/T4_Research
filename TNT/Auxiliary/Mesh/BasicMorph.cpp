/*****************************************************************************
*
*	BasicMorph.cpp - generic morph structure
*
*	5/8/00
*
*****************************************************************************/

#include "BasicMorph.h"

#include "x_plus.hpp"


//****************************************************************************
const char	kIdReference[]			= "REFERENCE";
									
const char	kIdMorphTargetSet[]		= "MORPH_TARGET_SET";
const char	kIdMorphTarget[]		= "TARGET";
									
const char	kIdMorphStream[]		= "MORPH_STREAM";
const char	kIdMorphAudio[]			= "AUDIO";

const char	kIdNumMorphSubstreams[]	= "NUM_SUBSTREAMS";
const char	kIdMorphSubstream[]		= "SUBSTREAM";



//****************************************************************************

namespace BasicMorph
{

/*****************************************************************************
*
*	Class:	BasicMorph::CTarget
*
*****************************************************************************/

const char* CTarget::GetName(void) const
{
    return(m_Name);
}

//****************************************************************************
const char* CTarget::GetChunkName(void) const
{
    return(m_ChunkName);
}

//****************************************************************************
xbool CTarget::IsValidDeltaId(s32 iDelta) const
{
	return((iDelta >= 0) && (iDelta < m_nDeltas));
}

//****************************************************************************
s32 CTarget::GetNumDeltas(void) const
{
    return(m_nDeltas);
}

//****************************************************************************
CDelta CTarget::GetDelta(s32 iDelta) const
{
	ASSERT(IsValidDeltaId(iDelta));

	return(m_pDeltas[iDelta]);
}



/*****************************************************************************
*
*	Class:	BasicMorph::CTargetSet
*
*****************************************************************************/
CTargetSet::CTargetSet() : m_nTargets(0), m_pTargets(NULL) { }

CTargetSet::~CTargetSet()
{
	delete [] m_pTargets;
	m_pTargets = NULL;
}

//****************************************************************************
const char* CTargetSet::GetName(void) const
{
    return(m_Name);
}

//****************************************************************************
const char* CTargetSet::GetMeshName(void) const
{
    return(m_MeshName);
}

//****************************************************************************
xbool CTargetSet::IsValidTargetId(s32 iTarget) const
{
	return((iTarget >= 0) && (iTarget < m_nTargets));
}

//****************************************************************************
s32 CTargetSet::GetNumTargets(void) const
{
	return(m_nTargets);
}

//****************************************************************************
const CTarget& CTargetSet::GetTarget(s32 iTarget) const
{
	ASSERT(IsValidTargetId(iTarget));
	return(m_pTargets[iTarget]);
}

//****************************************************************************
CTarget *CTargetSet::GetTargetPtr(s32 iTarget) const
{
	ASSERT(IsValidTargetId(iTarget));
	return(&m_pTargets[iTarget]);
}

//****************************************************************************
// save a target set

xbool CTargetSet::Save(CTextFile& Tf) const
{
	if (m_pTargets == FALSE)
		return(TRUE);


	Tf.BeginToken(kIdMorphTargetSet, m_nTargets, m_Name);

	Tf.Token(kIdReference, m_MeshName);

	for(s32 cTarget = 0; cTarget < m_nTargets; cTarget++)
	{
		CTarget	*pTarget = &m_pTargets[cTarget];

		Tf.BeginToken(kIdMorphTarget, pTarget->m_nDeltas, pTarget->m_Name);
		Tf.Token(kIdReference, pTarget->m_ChunkName);

    	Tf.Put("\n; iVert,	vDelta\n");
		for(s32 cDelta = 0; cDelta < pTarget->m_nDeltas; cDelta++)
		{
			Tf.Put(pTarget->m_pDeltas[cDelta].m_iVert);
			Tf.Put("\t");
			Tf.Put(pTarget->m_pDeltas[cDelta].m_vDelta);
			Tf.Put("\n");
		}

		Tf.EndToken();
	}

	Tf.EndToken();

	return(TRUE);
}

//****************************************************************************
// load a target set

xbool CTargetSet::Load(CTextFile& Tf)
{
	CTextFile::tokentype	Token;
	xbool					bFound = FALSE,
							bSuccess = FALSE;
	s32						iTarget = 0;


	ASSERT(m_pTargets == NULL);

	Tf.SeekStart();
	while(Tf.GetToken(&Token))
	{
		// -------------------------------------------------------------------
		// read target set
		if (Token == kIdMorphTargetSet)
		{
			ASSERTS((bFound == FALSE), "file contains more than one target set");
			bFound = TRUE;

			// get the object name
			x_strncpy(m_Name, Token.String, sizeof(m_Name));


			// alloc the targets
			m_nTargets = Token.Count;
			ASSERT(m_nTargets);

			m_pTargets = new CTarget [m_nTargets];


			Tf.MarkSection();
			while(Tf.GetSectionToken(&Token))
			{
				// -----------------------------------------------------------
				// read mesh name
				if (Token == kIdReference)
				{
					x_strncpy(m_MeshName, Token.String, sizeof(m_MeshName));
				}

				// -----------------------------------------------------------
				// read targets
				else if (Token == kIdMorphTarget)
				{
					CTarget	*pTarget = &m_pTargets[iTarget];


					// get the target name
					x_strncpy(pTarget->m_Name, Token.String, sizeof(pTarget->m_Name));


					// alloc the deltas
					pTarget->m_nDeltas = Token.Count;
					pTarget->m_pDeltas = new CDelta [pTarget->m_nDeltas];


					// get the chunk name					
					bSuccess = Tf.GetToken(&Token);
					ASSERT(bSuccess);
					ASSERT(Token == kIdReference);

					x_strncpy(pTarget->m_ChunkName, Token.String, sizeof(pTarget->m_ChunkName));


					// get the deltas
					for(s32 cDelta = 0; cDelta < pTarget->m_nDeltas; cDelta++)
					{
						Tf.Get(&pTarget->m_pDeltas[cDelta].m_iVert);
						Tf.Get(&pTarget->m_pDeltas[cDelta].m_vDelta);
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

//****************************************************************************
//****************************************************************************
//****************************************************************************


/*****************************************************************************
*
*	Class:	BasicMorph::CStream
*
*****************************************************************************/
CStream::CStream()
{
	m_nSubstreams = 0;
	m_pSubstreams = NULL;
}


CStream::~CStream()
{
	delete [] m_pSubstreams;
	m_pSubstreams = NULL;
}

//****************************************************************************
// save a morph anim object

xbool CStream::Save(CTextFile& Tf) const
{
	if (m_pSubstreams == NULL)
		return(TRUE);


	Tf.BeginToken(kIdMorphStream, m_Name);


	Tf.BeginToken(kIdMorphAudio, 0);

	Tf.Put("; Filename, Offset\n");
	Tf.Put(fs("\"%s\",\t", m_AudioName));
	Tf.Put(m_AudioOffset);

	Tf.EndToken();


	Tf.Token(kIdNumMorphSubstreams, m_nSubstreams);

	for(s32 cStream = 0; cStream < m_nSubstreams; cStream++)
	{
		CSubstream	*pSubstream = &m_pSubstreams[cStream];

		Tf.BeginToken(kIdMorphSubstream, 0);

		Tf.Put(fs("\"%s\",\n", pSubstream->m_TargetName));

		Tf.Put("; Value...\n");

		for(s32 cKey = 0; cKey < pSubstream->m_nKeys; cKey++)
		{
			Tf.Put(pSubstream->m_pKey[cKey]);

			if ((cKey % 10) == 0)
			{
				Tf.Put(",\n");
			}
			else
			{
				Tf.Put("\n");
			}
		}

		Tf.EndToken();
	}

	Tf.EndToken();


	return(TRUE);
}

//****************************************************************************
// load a morph anim object

xbool CStream::Load(CTextFile& Tf)
{
	ASSERT(FALSE);

	return(TRUE);
}

//****************************************************************************

}

//****************************************************************************
