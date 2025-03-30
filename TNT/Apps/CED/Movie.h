#ifndef _Movie_h_
#define	_Movie_h_
// Movie.h : header file
//

/////////////////////////////////////////////////////////////////////////////

#include "ieArchive.h"
#include "FileTags.h"
#include "ActorList.h"
#include "ShotList.h"

/////////////////////////////////////////////////////////////////////////////

class CActor ;
class CCharacter ;
class CMotion ;
class CCeDDoc ;

/////////////////////////////////////////////////////////////////////////////

enum
{
	EXPORT_TYPE_GAMECUBE,
	EXPORT_TYPE_PC,
	EXPORT_TYPE_XBOX,
	EXPORT_TYPE_PS2,

	NUM_EXPORT_TYPES
};



/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_MOVIE_END = IE_TAG_MOVIE+1,

	IE_TAG_MOVIE_NAME,
	IE_TAG_MOVIE_NUMFRAMES,
	IE_TAG_MOVIE_FRAME,
	IE_TAG_MOVIE_USESHOTLIST,
    IE_TAG_MOVIE_PREVIEWWIDTH,
    IE_TAG_MOVIE_PREVIEWHEIGHT,

	IE_TAG_MOVIE_LAYERS,
	IE_TAG_MOVIE_LAYER_NAME,

	IE_TAG_MOVIE_EXPORTPATHS,
//	IE_TAG_MOVIE_EXPORTPATH,

} ;


#define	MAX_MOVIE_LAYERS	256

/////////////////////////////////////////////////////////////////////////////
// CMovie

class CMovie : public CObject
{
	DECLARE_DYNCREATE(CMovie)

// Attributes
public:
	CString			m_Name ;								// Movie Name

	s32				m_NumFrames ;							// Number of Frames
	s32				m_nFrame ;								// Current Frame

	CActorList		m_ActorList ;							// List of Actors
	CActor			*m_pCurActor ;							// Pointer to current Actor

	CShotList		m_ShotList ;							// List of Camera Shots

	bool			m_UseShotList ;							// Use ShotList to View
    s32             m_PreviewWidth ;                        // Width of Preview Window
    s32             m_PreviewHeight ;                       // Height of Preview Window

	CString			m_LayerNames[MAX_MOVIE_LAYERS] ;		// View/Edit layer names
	s32				m_NumLayers ;							// Number of layers

	CString			m_ExportPaths[NUM_EXPORT_TYPES];		// Export paths for different movie types
	bool			m_ExportActive[NUM_EXPORT_TYPES];		// is this export active

    // Constructors
public:
	CMovie();
	~CMovie();

// Operations
public:
	virtual void Serialize(CieArchive& a) ;
	CMovie &operator=( CMovie & ) ;										// Right side is the argument.

	bool		Create					( ) ;							// Create Movie

	void		SetName					( const char *pName ) ;			// Set Name of Movie
	const char	*GetName				( ) ;							// Get Name of Movie

	int			GetNumLayers			( void ) { return m_NumLayers; }
	CString&	GetLayer				( int Layer ) ;
	void		SetLayer				( int Layer, CString& LayerName ) ;
	int			AddLayer				( CString& LayerName ) ;		// add a view/edit layer to the movie
	bool		RemoveLayer				( CString& LayerName ) ;		// add a view/edit layer to the movie
	bool		RemoveLayer				( int Layer ) ;					// remove a view/edit layer
	int			FindLayerByName			( CString& LayerName ) ;		// find a view/edit layer

	CActor		*GetCurActor			( ) ;							// Pointer to current Actor
	CActor		*SetCurActor			( CActor *pActor ) ;			// Set Pointer to current Actor

	int			SetFrame				( int nFrame ) ;				// Set Current Frame
	int			GetFrame				( ) ;							// Get Frame
	int			SetNumFrames			( int NumFrames ) ;				// Set Number of Frames
	int			GetNumFrames			( ) ;							// Get Number of Frames

	void		InsertFrames			( int AtFrame, int nFrames ) ;	// Insert Frames
	void		DeleteFrames			( int AtFrame, int nFrames ) ;	// Delete Frames

	void		CharacterDeleted		( CCharacter *pCharacter ) ;	// Delete a Character
	void		MotionDeleted			( CMotion *pMotion ) ;			// Delete a Motion

	void		ShotDelete				( CShot *pShot ) ;				// Delete Shot
	CShot		*GetCurShot				( ) ;							// Get Current Shot
	CActor		*ShotGetEye				( int nFrame ) ;				// Get Eye at Frame
	CActor		*ShotGetTarget			( int nFrame ) ;				// Get Target at Frame
	f32			ShotGetFOV				( int nFrame ) ;				// Get field of view at frame

	void		Merge					( CMovie *pMovie ) ;			// Merge in a Movie

    void        SetPreviewWidth         ( s32 Width ) ;
    void        SetPreviewHeight        ( s32 Height ) ;
    s32         GetPreviewWidth         ( ) ;
    s32         GetPreviewHeight        ( ) ;
	
// Implementation
protected:
	void Initialize() ;										// Initialize Data
	void Destroy () ;										// Destroy Movie
	void Copy (CMovie &Dst, CMovie &Src) ;					// Copy

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_Movie_h_
