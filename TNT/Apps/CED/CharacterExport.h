// CharacterExport.h: interface for the CCharacterExport class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHARACTEREXPORT_H__8E599862_E5B6_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_CHARACTEREXPORT_H__8E599862_E5B6_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ExportFile.h"
#include "Character.h"
#include "ExportProgress.h"

class CCharacterExport  
{
//---	Attributes
public:
	static CExportProgress	*m_pProgressDialog ;
	int				m_nErrors ;

	CExportFile		anim_Text ;		// text file listing all used animations
	CExportFile		anim_Header ;
	CExportFile		anim_HeaderEvent ;
	CExportFile		anim_HeaderFlags ;
	CExportFile		anim_HeaderProps ;
	CExportFile		anim_HeaderPoints ;
	CExportFile		anim_HeaderStreams ;
	CExportFile		anim_group ;
	CExportFile		anim_info ;
	CExportFile		anim_bone ;
	CExportFile		anim_events ;
	CExportFile		anim_BlockSize ;
//	CExportFile		anim_Blocks ;					

	s32				m_CompMotionCount ;			// stores the number of compressed motions in the array
	CExportFile*	m_CompMotionsArray ;		// stores the data once it has been compressed.
	CByteArray*		m_CompBlockSizesArray ;		// stores the sizes of the compressed blocks

	s16				m_DefineID ;				// New Define ID
	CStringArray	m_DefineArray ;				// String Array for Defines

	int				m_Target ;

	int				m_nBones ;
	
	int				m_BlockOffset ;
	int				m_EventIndex ;

	int				m_nAnimBytes ;
	int				m_nAnimations ;
	int				m_nAnimEvents ;
	int				m_nAnimBlocks ;
	int				m_nAnimStreams ;

//---	Operations
public:
			CCharacterExport				( int Target ) ;
	virtual ~CCharacterExport				() ;

	CString	MakeDefine						( const CString& Prefix, const CString& String ) ;
	bool	FindDefine						( s16 *pID, CString String ) ;

	void	ResetMotionCompressionBuffers	( CCharacter* pCharacter ) ;

	void	ExportMotionBlocks				( s32			MotionIndex,
											  CMotion*		pMotion,
											  CCharacter*	pCharacter,
											  s32			FramesPerBlock,
											  s32&			rNumExportedFrames,
											  s32&			rCompDataSize,
											  s32&			rCompDataBlocks,
											  u8			PropsExported,
											  matrix4&		Mat ) ;
	void	Export							( const char *pFileName, CCharacter *pCharacter ) ;

	bool	ComputeSpecificPropInfo			( CMotion*      pMotion, 
											  CCharacter*   pCharacter,
											  s32			PropIndex );

    void	ComputePropInfo					(CMotion* pMotion, CCharacter* pCharacter, u8* PropsUsedFlags, u8* PropsWithStreamsFlags ) ;
    void	ComputePointInfo				(CMotion* pMotion, CCharacter* pCharacter, u8* PointUsedFlags ) ;
    void	ComputeUserStreamInfo			(CMotion* pMotion, CCharacter* pCharacter, u8* UserStreamUsedFlags ) ;
};

#endif // !defined(AFX_CHARACTEREXPORT_H__8E599862_E5B6_11D1_AEC0_00A024569FF3__INCLUDED_)
