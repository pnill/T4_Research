// CeDDoc.h : interface of the CCeDDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CEDDOC_H__68F53CF1_D3EC_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_CEDDOC_H__68F53CF1_D3EC_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "FileTags.h"

//#include "Skeleton.h"
#include "CharacterList.h"
#include "MovieList.h"
#include "MeshList.h"

////////////////////////////////////////////////////////////////////////////

class CProjectView ;
class CMotionEditView ;
class CMovieView ;
class CRenderContext ;

////////////////////////////////////////////////////////////////////////////


//---	enumerations for search and replace
enum
{
	FILEPATHS_LOOKFROMSTART,
	FILEPATHS_LOOKFROMEND,
	FILEPATHS_LOOKANYWHERE,
};

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_DOC_FLAGDEFINE = IE_TAG_DOC+1,
	IE_TAG_DOC_FLAGDESCRIPTION,

	IE_TAG_DOC_CHARACTER_INCLUDE,
} ;

//////////////////////////////////////////////////////////////////////////////

enum
{
	HINT_CHARACTER_NEW		= 0x1000,
	HINT_MOTION_NEW,
	HINT_MOTION_DELETE,
	
	HINT_EVENT_NEW,
	HINT_EVENT_DELETE,
	HINT_SKELPOINT_NEW,
	HINT_SKELPOINT_DELETE,

	HINT_MOVIE_NEW,
	HINT_MESH_NEW,
	HINT_ACTOR_NEW,
	HINT_ACTOR_DELETE,

	HINT_REDRAWALL,

	HINT_D3DENABLE,
	HINT_D3DDISABLE,
} ;


class CCeDDoc : public CDocument
{
protected: // create from serialization only
	CCeDDoc();
	DECLARE_DYNCREATE(CCeDDoc)

// Attributes
public:
	bool			m_DrawGroundPlane ;
	bool			m_DrawConstructionPlane ;
	vector3 		m_ConPlanePos ;

	CCharacterList	m_CharacterList ;
	CMovieList		m_MovieList ;
	CMeshList		m_MeshList ;

	CString			m_CharacterIncludeFile ;				// Character Include File for Export
	CString			m_FlagDefine[MOTION_FLAG_BITS] ;		// #define for Flag Bits
	CString			m_FlagDescription[MOTION_FLAG_BITS] ;	// Description of Flag Bits

// Operations
public:
	void ActivateDocument						( ) ;
	void UpdateMenu								( ) ;
	void UpdateMenuItems						( CMenu *pMenu ) ;
	void ClosingFrameWindow						( CFrameWnd *pFrame ) ;

	CProjectView	*GetProjectView				( ) ;

	//---	Character Functions
	void		CreateCharacterView				( CCharacter *pCharacter ) ;
	CCharacter	*CreateNewCharacter				( ) ;

	void		CharacterProperties				( CCharacter* pCharacter );

	void		CharacterImportSkeleton			( CCharacter* pCharacter ) ;
	bool		CharacterImportSkeletonFromFile	( CCharacter *pCharacter,
												  const char *pPathName ) ;
	void		CharacterImportSkin				( CCharacter* pCharacter ) ;
	bool		CharacterImportSkinFromFile		( CCharacter *pCharacter,
												  const char *pPathName ) ;
	void		CharacterDeleteSkin				( CCharacter* pCharacter ) ;
	void		CharacterNewMotion				( CCharacter* pCharacter ) ;
	void		CharacterImportMotion			( CCharacter* pCharacter ) ;
	CMotion		*CharacterImportMotionFromFile	( CCharacter *pCharacter,
												  const char *pPathName ) ;
	void		CharacterExportAll				( void );
	void		CharacterExportFull				( CCharacter *pCharacter ) ;
	bool		CharacterExport					( int Target,
												  const char *pFileName,
												  CCharacter *pCharacter ) ;
	void		CharacterLoadAllMotions			( CCharacter* pCharacter ) ;
	CCharacter	*GetMatchingCharacter			( CCharacter *pCharacter ) ;


	//---	Movie Functions
	void		CreateMovieView					( CMovie *pMovie ) ;
	CMovie		*CreateNewMovie					( ) ;
	bool		MovieExport						( int Target,
												  CMovie *pMovie ) ;
	bool		MovieExportProperties			( CMovie* pMovie ) ;
	bool		MovieExportASCII				( const char *pFileName,
												  CMovie *pMovie ) ;

	//---
	void		MotionDeleted					( CMotion *pMotion ) ;			// Motion was deleted
	void		CharacterDeleted				( CCharacter *pCharacter ) ;	// Character was deleted

	//---	Mesh Functions
	CMesh		*CreateNewMesh					( ) ;

	void		CharacterEditSkins				( CCharacter* pCharacter );
	void		CharacterEditFaceDirRecipe		( CCharacter* pCharacter );

	//---	document file datestamp checking functions
	DWORD		MakeTimeStamp					( CTime &rTime );
	BOOL		CheckFileChange					( void );

	BOOL		m_bD3DEnable;

	//---	functions to allow the D3D button to turn off D3D for all of the application
	//		this function just visits all of the views of this document to make the change
	void EnableViewD3D( bool bEnableDisable );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeDDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnChangedViewList();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

	//---	old format loading
	void Serialize2(CArchive& ar);

// Implementation
public:
	void ShowDefinedEventsDlg();
	CEventList* GetProjectEventList();
	virtual ~CCeDDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CEventList	m_ProjectEventList;		// List of user defined events usable 
										// by any character/motion within the document/project scope
	DWORD		m_TimeStamp;

// Generated message map functions
protected:
	//{{AFX_MSG(CCeDDoc)
	afx_msg void OnViewNew();
	afx_msg void OnViewGroundPlane();
	afx_msg void OnViewConstructionPlane();
	afx_msg void OnViewProject();
	afx_msg void OnViewMotionedit();
	afx_msg void OnMovieImport();
	afx_msg void OnEventsDefinedevents();
	afx_msg void OnEditFilepaths();
	afx_msg void OnMovieExportall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CEDDOC_H__68F53CF1_D3EC_11D1_AEC0_00A024569FF3__INCLUDED_)
