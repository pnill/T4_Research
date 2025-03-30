#ifndef _Prop_h_
#define	_Prop_h_
// Motion.h : header file
//

#include "ieArchive.h"
#include "anim.h"
#include "FileTags.h"

class CCharacter ;
class CMotion ;


#define	MAX_LINK_CHARACTER_INDEX	255
#define	MAX_LINK_MOTION_INDEX		65535


/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_PROPDEF_END = IE_TAG_PROPDEF+1,

	IE_TAG_PROPDEF_ACTIVE,
	IE_TAG_PROPDEF_EXPORT,
	IE_TAG_PROPDEF_NAME,
	IE_TAG_PROPDEF_BONENAME,
	IE_TAG_PROPDEF_LINKCHARACTER,
	IE_TAG_PROPDEF_POSITION,
	IE_TAG_PROPDEF_STATICPOS,
	IE_TAG_PROPDEF_ROTATION,
	IE_TAG_PROPDEF_STATICROT,
	IE_TAG_PROPDEF_POSFACINGREL,
	IE_TAG_PROPDEF_SCALED,
	IE_TAG_PROPDEF_USELINKEDPOS,
	IE_TAG_PROPDEF_USELINKEDROT,
	IE_TAG_PROPDEF_ROTFACINGREL,
	IE_TAG_PROPDEF_MIRRORED,
	IE_TAG_PROPDEF_MIRRORWITHANIM,
	IE_TAG_PROPDEF_MIRRORROT,	// not used anymore , but must remain here
} ;

enum
{
	IE_TAG_PROPDATA_END = IE_TAG_PROPDATA+1,

	IE_TAG_PROPDATA_ACTIVE,
	IE_TAG_PROPDATA_EXPORT,
	IE_TAG_PROPDATA_LINKMOTION,
} ;


/////////////////////////////////////////////////////////////////////////////
// CPropDef - stored per prop per character
/////////////////////////////////////////////////////////////////////////////
class CPropDef
{
public:
	CPropDef();
	~CPropDef();

	/////////////////////////////////////////////////////////////////////////////
	//	ACCESS FUNCTIONS
	/////////////////////////////////////////////////////////////////////////////

	const s32			GetIndex				( void ) const;
	CCharacter*			GetCharacter			( void ) const;

	bool				IsActive				( void ) const;
	bool				IsExported				( void ) const;
	const CString&		GetPropName				( void ) const;
	const CString&		GetBoneName				( void ) const;
	s32					GetBone					( void );

	CCharacter*			GetLinkedCharacter		( void );

	bool				IsPositionStatic		( void ) const;
	const vector3&		GetPosition				( void ) const;

	bool				IsRotationStatic		( void ) const;
	const radian3&		GetRotation				( void ) const;

	bool				IsPosFacingRelative		( void ) const;
	bool				IsRotFacingRelative		( void ) const;

	bool				IsScaled				( void ) const;
	bool				IsMirrored				( void ) const;

	bool				IsUseLinkedPosition		( void ) const;
	bool				IsUseLinkedRotation		( void ) const;

	bool				IsMirrorWithAnim		( void ) const;


	/////////////////////////////////////////////////////////////////////////////
	//	SET FUNCTIONS
	/////////////////////////////////////////////////////////////////////////////
	void				SetIndex				( s32 Index );
	void				SetCharacter			( const CCharacter* pCharacter );

	void				SetActive				( bool bSetting );
	void				SetExported				( bool bSetting );
	void				SetPropName				( const CString& Name );
	void				SetBoneName				( const CString& Name );
	void				SetBone					( s32 iBone );
	void				SetLinkedCharacter		( const CCharacter* pCharacter );

	void				SetPositionStatic		( bool bSetting );
	void				SetPosition				( const vector3& rPosition );

	void				SetRotationStatic		( bool bSetting );
	void				SetRotation				( const radian3& rRotation );

	void				SetFacingRelativePos	( bool bSetting );
	void				SetFacingRelativeRot	( bool bSetting );

	void				SetScaled				( bool bSetting );
	void				SetMirrored				( bool bSetting );

	void				SetUseLinkedPosition	( bool bSetting );
	void				SetUseLinkedRotation	( bool bSetting );

	void				SetMirrorWithAnim		( bool bSetting );

	/////////////////////////////////////////////////////////////////////////////
	//	UTILITY FUNCTIONS
	/////////////////////////////////////////////////////////////////////////////

	void				Copy					( const CPropDef& Src );
	CPropDef&			operator=				( const CPropDef& Src );

	void				Serialize				( CieArchive& a );

	s32					GetRuntimeVersion		( void );

protected:
	/////////////////////////////////////////////////////////////////////////////
	//	MEMBER DATA
	/////////////////////////////////////////////////////////////////////////////

	s32					m_RuntimeVersion;	// value is ticked every time a value changes which would require
											// prop data to reset itself.  Prop data always checks its record of
											// its run-time version for a difference with this one and resets if required.

	//---	The following data is always valid to describe a prop.
	bool				m_bPropActive;		// is this prop active
	bool				m_bPropExported;	// is this prop exported
	s32					m_Index;			// which prop is this for this character
	CCharacter*			m_pCharacter;		// character which uses this prop definition
	CString				m_PropName;			// name string to be displayed to describe the use of this prop.
	CString				m_BoneName;			// name of the bone which this prop is relative to.
	s32					m_BoneID;			// number of the bone to which this prop is based

	//---	if the prop is based on a link to another character's motion, the following data will be valid
	//		
	CCharacter*			m_pLinkCharacter;

	//---	if the prop is based on rotation and offset, the following data will be valid.  Props are
	//		either relative to the bone by direction or offset.  If the prop is relative by direction
	//		the Distance value is used to store the distance that the prop is from the bone.  After the
	//		prop has been positioned, it can either have its own set rotation value, or one relative
	//		to the bone that it is tied to.  The m_bStaticRotation value is TRUE when the prop has a defined
	//		rotation value.
	bool				m_bStaticPosition;		// Is the prop given a static position, or is it rotated based on the bone's rotation?
	vector3				m_Position;				// value is either a relative position or direction depending on the mode
	bool				m_bUseLinkedPos;		// Use the position from the linked motion rather than the one provided
	bool				m_bStaticRotation;		// Is the prop given a static rotation, or is it rotated based on the bone's rotation?
	radian3				m_Rotation;				// prop rotation about local axis
	bool				m_bUseLinkedRot;		// Use the rotation from the linked motion rather than the one provided

	bool				m_bRotFacingRelative;	// prop facing is relative to the player's direction
	bool				m_bPosFacingRelative;	// prop facing is relative to the player's direction
	bool				m_bScaled;				// prop position is scaled relative to bone.
	bool				m_bMirrored;			// prop should be rendered mirrored (doesn't affect the exported data)
	bool				m_bMirrorWithAnim;		// mirror the prop when the played animation is mirrored
};


/////////////////////////////////////////////////////////////////////////////
// CPropData - stored per prop per motion
/////////////////////////////////////////////////////////////////////////////
class CPropData
{
public:
	CPropData();
	~CPropData();

	/////////////////////////////////////////////////////////////////////////////
	//	ACCESS FUNCTIONS
	/////////////////////////////////////////////////////////////////////////////

	bool				IsFullyActive		( void ) const;	// prop definition is active as well.
	bool				IsFullyExported		( void ) const; // prop definition is exported as well

	bool				IsActive			( void ) const;
	bool				IsExported			( void ) const;

	bool				IsGeneratedPropData	( void ) const;	// T/F, is there stream data which has been merged. (and cannot therefore be recreated internally)
	bool				HasUniquePropData	( void );

		  CMotion*		GetMotion			( void ) const;
	      CCharacter*	GetCharacter		( void ) const;
	const s32			GetIndex			( void ) const;

		  CPropDef*		GetDefinition		( void ) const;

		  CCharacter*	GetLinkedCharacter	( void ) const;
		  CMotion*		GetLinkedMotion		( void );

	const vector3*		GetPositionStream	( void );
	const radian3*		GetRotationStream	( void );

	vector3				GetPosition			( f32 Frame );
	radian3				GetRotation			( f32 Frame );


	/////////////////////////////////////////////////////////////////////////////
	//	SET FUNCTIONS
	/////////////////////////////////////////////////////////////////////////////

	void				SetIndex			( s32 Index );
	void				SetMotion			( const CMotion* pMotion );

	void				SetActive			( bool bSetting );
	void				SetExported			( bool bSetting );

	void				SetLinkedMotion		( const CMotion* pMotion );


	/////////////////////////////////////////////////////////////////////////////
	//	UTILITY FUNCTIONS
	/////////////////////////////////////////////////////////////////////////////

	void				Copy				( const CPropData& Src );
	CPropData&			operator=			( const CPropData& Src );

	void				MergeProp			( CPropData& rMergeProp, f32 fRatio );
	void				ResampleProp		( s32 OldNFrames, s32 NewNFrames );
	void				MimicFinalExportQuantizationResult	( void );
						
	void				Serialize			( CieArchive& a );
						
	void				ResetMotionData		( void );
	void				GeneratePropMotion	( void );

	xbool				CheckVersion		( void );

protected:

	/////////////////////////////////////////////////////////////////////////////
	//	MEMBER DATA
	/////////////////////////////////////////////////////////////////////////////

	s32					m_RuntimeVersion;	// version of the prop definition the last time this
											// prop data was reset.
	bool				m_bGeneratedStreams;// T/F, is this prop data the result of merging multiple props together?
											// if it is, it will no longer be able to regenerate itself when there is a
											// change in version number.

	//---	The following data is always valid to describe a prop.
	bool				m_bPropActive;		// is this prop active
	bool				m_bPropExported;	// is this prop exported
	s32					m_Index;			// prop index so that the definition can be determined
	CMotion*			m_pMotion;			// motion that this prop is used within.

	//---	if the prop is based on a link to another character's motion, the following data will be valid
	//		
	CMotion*			m_pLinkMotion;

	//---	Generated prop motion to be used for playback and export.  The length of these arrays is based on the
	//		number of frames in the motion.  If the number of frames change, this data must be cleared and recalculated.
	vector3*			m_pPosition;
	radian3*			m_pRotation;
};


//=============================================================================
inline CPropDef& CPropDef::operator=( const CPropDef& Src )
{
	Copy( Src );
	return *this;
}

//=================================================================================
inline void CPropDef::SetIndex( s32 Index )
{
	m_Index		= Index;
}

//=================================================================================
inline void	CPropDef::SetCharacter( const CCharacter* pCharacter )
{
	m_pCharacter	= (CCharacter*)pCharacter;
}

//=================================================================================
inline bool CPropDef::IsActive( void ) const
{
	return m_bPropActive;
}

//=================================================================================
inline bool CPropDef::IsExported( void ) const
{
	return m_bPropExported;
}

//=================================================================================
inline const s32 CPropDef::GetIndex( void ) const
{
	return m_Index;
}

//=================================================================================
inline const CString& CPropDef::GetPropName( void ) const
{
	return m_PropName;
}

//=================================================================================
inline CCharacter* CPropDef::GetCharacter( void ) const
{
	ASSERT(m_pCharacter);
	return m_pCharacter;
}

//=================================================================================
inline const CString& CPropDef::GetBoneName( void ) const
{
	return m_BoneName;
}

//=================================================================================
inline bool CPropDef::IsPositionStatic( void ) const
{
	return m_bStaticPosition;
}

//=================================================================================
inline const vector3& CPropDef::GetPosition( void ) const
{
	return m_Position;
}

//=================================================================================
inline bool CPropDef::IsRotationStatic( void ) const
{
	return m_bStaticRotation;
}

//=================================================================================
inline const radian3& CPropDef::GetRotation( void ) const
{
	return m_Rotation;
}

//=================================================================================
inline bool CPropDef::IsUseLinkedPosition( void ) const
{
	return m_bUseLinkedPos;
}

//=================================================================================
inline bool CPropDef::IsUseLinkedRotation( void ) const
{
	return m_bUseLinkedRot;
}

//=================================================================================
inline bool CPropDef::IsMirrorWithAnim( void ) const
{
	return m_bMirrorWithAnim;
}

//=================================================================================
inline void CPropDef::SetActive( bool bSetting )
{
	m_bPropActive	= bSetting;
}

//=================================================================================
inline void CPropDef::SetExported( bool bSetting )
{
	m_bPropExported	= bSetting;
}

//=================================================================================
inline void CPropDef::SetPropName( const CString& Name )
{
	m_PropName	= Name;
}

//=================================================================================
inline void CPropDef::SetPositionStatic( bool bSetting )
{
	m_bStaticPosition	= bSetting;
}

//=================================================================================
inline void CPropDef::SetPosition( const vector3& rPosition )
{
	++m_RuntimeVersion;
	m_Position	= rPosition;
}

//=================================================================================
inline void CPropDef::SetRotationStatic( bool bSetting )
{
	m_bStaticRotation	= bSetting;
}

//=================================================================================
inline void CPropDef::SetRotation( const radian3& rRotation )
{
	++m_RuntimeVersion;
	m_Rotation		= rRotation;
}

//=================================================================================
inline s32 CPropDef::GetRuntimeVersion( void )
{
	return m_RuntimeVersion;
}

//=================================================================================
inline void CPropDef::SetUseLinkedPosition( bool bSetting )
{
	++m_RuntimeVersion;
	m_bUseLinkedPos	= bSetting;
}

//=================================================================================
inline void CPropDef::SetUseLinkedRotation( bool bSetting )
{
	++m_RuntimeVersion;
	m_bUseLinkedRot	= bSetting;
}

//=================================================================================
inline void CPropDef::SetMirrorWithAnim( bool bSetting )
{
	++m_RuntimeVersion;
	m_bMirrorWithAnim	= bSetting;
}

//=================================================================================
//=================================================================================
//=================================================================================
//					PROP DATA INLINED FUNCTIONS
//=================================================================================
//=================================================================================
//=================================================================================

//=============================================================================
inline CPropData& CPropData::operator=( const CPropData& Src )
{
	Copy( Src );
	return *this;
}

//=================================================================================
inline bool CPropData::IsActive( void ) const
{
	return m_bPropActive;
}

//=================================================================================
inline bool CPropData::IsExported( void ) const
{
	return m_bPropExported;
}

//=================================================================================
inline bool CPropDef::IsPosFacingRelative( void ) const
{
	return m_bPosFacingRelative;
}

//=================================================================================
inline bool CPropDef::IsRotFacingRelative( void ) const
{
	return m_bRotFacingRelative;
}

//=================================================================================
inline bool CPropDef::IsScaled( void ) const
{
	return m_bScaled;
}

//=================================================================================
inline bool CPropDef::IsMirrored( void ) const
{
	return m_bMirrored;
}

//=================================================================================
inline bool CPropData::IsFullyActive( void ) const
{
	return IsActive() && GetDefinition() && GetDefinition()->IsActive();
}

//=================================================================================
inline bool CPropData::IsFullyExported( void ) const
{
	return IsExported() && GetDefinition() && GetDefinition()->IsActive() && GetDefinition()->IsExported();
}

//=================================================================================
inline CMotion* CPropData::GetMotion( void ) const
{
	return m_pMotion;
}

//=================================================================================
inline const s32 CPropData::GetIndex( void ) const
{
	return m_Index;
}

//=================================================================================
inline CCharacter* CPropData::GetCharacter( void ) const
{
	if( GetDefinition() )
		return GetDefinition()->GetCharacter();

	return NULL;
}

//=================================================================================
inline CCharacter* CPropData::GetLinkedCharacter( void ) const
{
	if( GetDefinition() )
		return GetDefinition()->GetLinkedCharacter();

	return NULL;
}

//=================================================================================
inline void CPropData::SetIndex( s32 Index )
{
	m_Index			= Index;
}

//=================================================================================
inline void CPropData::SetMotion( const CMotion* pMotion )
{
	m_pMotion		= (CMotion*)pMotion;
}

//=================================================================================
inline void CPropData::SetActive( bool bSetting )
{
	m_bPropActive	= bSetting;
}

//=================================================================================
inline void CPropData::SetExported( bool bSetting )
{
	m_bPropExported	= bSetting;
}

//=================================================================================
inline void CPropDef::SetFacingRelativePos( bool bSetting )
{
	m_bPosFacingRelative	= bSetting;
}

//=================================================================================
inline void CPropDef::SetFacingRelativeRot( bool bSetting )
{
	m_bRotFacingRelative	= bSetting;
}

//=================================================================================
inline void CPropDef::SetScaled( bool bSetting )
{
	m_bScaled	= bSetting;
}

//=================================================================================
inline void CPropDef::SetMirrored( bool bSetting )
{
	m_bMirrored	= bSetting;
}

//=================================================================================
inline void CPropData::SetLinkedMotion( const CMotion* pMotion )
{
	m_pLinkMotion	= (CMotion*)pMotion;
	ResetMotionData();
}

//=================================================================================
inline xbool CPropData::CheckVersion( void )
{
	CPropDef* pPropDef = GetDefinition();

	if( !pPropDef )
		return TRUE;

	return (m_RuntimeVersion == pPropDef->GetRuntimeVersion());
}

//=================================================================================
inline bool CPropData::IsGeneratedPropData( void ) const
{
	return m_bGeneratedStreams;
}

//=================================================================================
inline bool CPropData::HasUniquePropData( void )
{
	return GetLinkedMotion() || IsGeneratedPropData();
}

//=================================================================================
#endif // __PROPS_H__