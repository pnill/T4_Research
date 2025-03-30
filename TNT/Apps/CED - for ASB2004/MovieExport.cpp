// MovieExport.cpp: implementation of the CMovieExport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "MovieExport.h"
#include "Movie.h"
#include "Actor.h"
#include "Key.h"
#include "shot.h"
#include "mesh.h"
#include "CeDDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "mfcutils.h"

#define MAJOR_VERSION_NUMBER '1'
#define MINOR_VERSION_NUMBER '2'

CExportProgress	*CMovieExport::m_pProgressDialog = NULL ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMovieExport::CMovieExport( int Target )
{
	m_Target            = Target ;
	m_DefineID          = 1 ;
}

CMovieExport::~CMovieExport()
{
}




CString CMovieExport::MakeDefine( CString String )
{
	CString Define = "ID_" ;

	//---	Build DEFINE with no Illegal characters
	for( int i = 0 ; i < String.GetLength() ; i++ )
	{
		char c = String[i] ;
		if( !__iscsym(c) )
			c = '_' ;
		Define += c ;
	}

	return Define ;
}


bool CMovieExport::FindDefine( s16 *pID, CString String )
{
	bool NewID = true ;
	s16 FoundID = 0 ;

	//---	Try to find a match in the list of Strings
	for( int i = 1 ; i < m_DefineID ; i++ )
	{
		if( m_DefineArray[i] == String )
		{
			FoundID = i ;
			NewID = false ;
			break ;
		}
	}

	//---	Not found, so add new element
	if( FoundID == 0 )
	{
		FoundID = m_DefineID ;
		m_DefineArray.SetAtGrow( m_DefineID, String ) ;
		m_DefineID++ ;
	}

	*pID = FoundID ;
	return NewID ;
}


void CMovieExport::Export( const char *pFileName, CMovie *pMovie, CCeDDoc *pDoc )
{
	int		nErrors = 0 ;
	s32		nActors = 0 ;

	//---	Create Progress Dialog
	if( m_pProgressDialog ) delete m_pProgressDialog ;
	m_pProgressDialog = new CExportProgress ;
	ASSERT( m_pProgressDialog ) ;
	m_pProgressDialog->Create( IDD_EXPORT_PROGRESS, NULL ) ;
	m_pProgressDialog->EnableOk( false ) ;

	//---	Create ExportFiles necessary
	movie_Header.Create( m_Target ) ;
	movie_ActorTable.Create( m_Target ) ;
	movie_Actors.Create( m_Target ) ;
	movie_Final.Create( m_Target ) ;

	m_pProgressDialog->Fmt( "Exporting Movie..." ) ;
	m_pProgressDialog->Fmt( " " ) ;

//////////////////////////////////////////////////////////////////////////////
//	ACTORS

	m_pProgressDialog->Fmt( "Exporting Actors..." ) ;
	m_pProgressDialog->Fmt( " " ) ;

	m_pProgressDialog->SetRange( 0, pMovie->m_ActorList.GetCount() ) ;

	//---	Loop through Each Actor
	POSITION Pos = pMovie->m_ActorList.GetHeadPosition( ) ;
	while( Pos )
	{
		CActor *pActor = pMovie->m_ActorList.GetNext( Pos ) ;
		char	*pName;
		ASSERT( pActor ) ;
		if( pActor )
		{
			s16		nKeys = 0 ;

			//---	Force calculation of Spline Cache
			pActor->CalculateSplineCache( ) ;

			//---	Output Actor Data to Table
			movie_ActorTable.Writes32( movie_Actors.GetPosition() ) ;		// Address of Keys
			{
				u8 t = (u8)pActor->m_Type ;
				movie_ActorTable.Writeu8( t ) ;
			}
			{
//				s8 t = (s8)pDoc->m_CharacterList.PtrToIndex( pActor->m_pCharacter ) ;
				movie_ActorTable.Writes8( 0 ) ;									// Character ID
			}

			//---	Output Key Data
			POSITION PosKey = pActor->m_Keys.GetHeadPosition( ) ;
			while( PosKey )
			{
				CKey *pKey = pActor->m_Keys.GetNext( PosKey ) ;
				ASSERT( pKey ) ;
				if( pKey )
				{
					//---	If this key has an animation, store the Y value from the animation.
					if( pKey->m_pCharacter && pKey->m_pMotion && pKey->m_AMCbased )
					{
						vector3d p, t;
						pKey->GetPosition( &p );
						pKey->m_pMotion->GetTranslation( &t, pKey->m_StartFrame );
						p.Y = t.Y;
					}

					//---	Write Key Info
					vector3d	p, v, r ;
					pKey->GetPosition( &p ) ;
					pKey->GetVector( &v ) ;
					pKey->GetRotation( &r ) ;
					s16		t ;
					t = pKey->GetFrame() ;	movie_Actors.Writes16( t ) ;
					pActor->GetPositionAtTime( &p, t ) ;						// Force real Key position for export
					t = (s16)( p.X+0.5f ) ;	movie_Actors.Writes16( t ) ;
					t = (s16)( p.Y+0.5f ) ;	movie_Actors.Writes16( t ) ;
					t = (s16)( p.Z+0.5f ) ;	movie_Actors.Writes16( t ) ;
					t = (s16)( v.X+0.5f ) ;	movie_Actors.Writes16( t ) ;
					t = (s16)( v.Y+0.5f ) ;	movie_Actors.Writes16( t ) ;
					t = (s16)( v.Z+0.5f ) ;	movie_Actors.Writes16( t ) ;

					t = (s16)( r.X+0.5f ) ;	movie_Actors.Writes16( t ) ;
					t = (s16)( r.Y+0.5f ) ;	movie_Actors.Writes16( t ) ;
					t = (s16)( r.Z+0.5f ) ;	movie_Actors.Writes16( t ) ;
					
					//---	The next so many bytes are part of a union with the
					if( pKey->m_pCharacter && pKey->m_pMotion )
					{
						t = (s16)pKey->m_pCharacter->MotionToExportIndex( pKey->m_pMotion );

						if( t == -1 )
						{
							CString msg;
							if( pKey->m_pCharacter->m_MotionList.PtrToIndex( pKey->m_pMotion ) >= 0 )
								msg.Format( "Anim '%s' loaded, but flagged not to be exported.", pKey->m_pMotion->GetExportName() );
							else
								msg.Format( "Anim '%s' does not exist in the character list.", pKey->m_pMotion->GetExportName() );
							MessageBox( NULL, msg, "MISSING MOTION", MB_ICONERROR );
						}
					}
					else
						t = -1 ;
					movie_Actors.Writes16( t ) ;

					t = pKey->GetStartFrame( ) ;
					movie_Actors.Writes16( t ) ;

					t = (s16)( pKey->GetFrameRate()*256.0f ) ;
					movie_Actors.Writes16( t ) ;

					t = (s16)( pKey->GetBlendFrames()*256.0f ) ;
					movie_Actors.Writes16( t ) ;
					//---	end of union

					//---	Output flags field. Set bit 0 for AMC anim type
					t = 0;
					if (pKey->m_AMCbased)
						t |= 0x0001;				// Bit 0, AMC flag
					if ( pKey->m_AutoLinkPos )
						t |= 0x0004;				// Bit 2, Autolink flag
					if ( pKey->m_Visible )
						t |= 0x0008;				// Bit 3, Visibility flag (scenery object only)
					if ( pKey->m_CameraLock )
						t |= 0x0010;				// Bit 4, Relative to camera
					if( pKey->m_Mirror )
						t |= 0x0020;				// Bit 5, Mirrored animation
					if ( pKey->m_AutoLinkDir )
						t |= 0x0040;				// Bit 6, Autolink direction flag

					movie_Actors.Writes16( t ) ;
					//---	Output event field
					t = (s16)pKey->GetEvent();	movie_Actors.Writes16( t );
					//---	Write out scale (only used for scenery object)
					t = (s16) (pKey->GetScale() * 256.0f);
					movie_Actors.Writes16( t ) ;
					
					//---	Increment Number of Keys Exported
					nKeys++ ;
				}

			}

			//---	Write Number of Keys in Actor KeyData
			movie_ActorTable.Writes16( nKeys ) ;							// Count of Keys
			
			//--- Dump out name of this player object
			if (pActor->GetType() == ACTOR_SCENERY)
			{
				pName = (char *)pActor->GetMesh()->GetFileName();
			}
			else
			{
				pName = (char *)pActor->GetName();
			}
			s32 i;
			char ch;
			for (i=0;i<15;i++)
			{
				ch = *pName;
				
				if (ch)
				{
					// Force it to uppercase
					if ( (ch>='a') && (ch<='z') ) ch -= 'a'-'A';
					movie_ActorTable.Writeu8( ch );
					pName++;
				}
				else
				{
					movie_ActorTable.Writeu8( 0 );
				}
			}

			//--- Write Null terminator for string
			movie_ActorTable.Writeu8( 0 );

			//---	Increment Number of Actors Exported
			nActors++ ;
			m_pProgressDialog->SetPos( nActors ) ;
		}
	}

	//--- BISC Added 6/23
	m_pProgressDialog->Fmt( "Exporting Shots..." ) ;
	m_pProgressDialog->Fmt( " " ) ;

	//--- Dump out the shot actors
	s16 nShots = 0;
	Pos = pMovie->m_ShotList.GetHeadPosition( ) ;
	if (Pos)
	{
		//---	Output Actor Data to Table
		movie_ActorTable.Writes32( movie_Actors.GetPosition() ) ;		// Address of Shot list
		movie_ActorTable.Writeu8( ACTOR_SHOT ) ;
		movie_ActorTable.Writes8( 0 ) ;									// Character ID

		while( Pos )
		{
			CShot *pShot = pMovie->m_ShotList.GetNext( Pos ) ;
			ASSERT( pShot ) ;
			if( pShot )
			{
				//---	Output Shot Data
				s16 t;

				t = (s16)pShot->GetFrame();	movie_Actors.Writes16( t );			// Start frame # for this shot
				t = (s16)pShot->GetFOV();	movie_Actors.Writes16( t );			// Store FOV in X, it will get splined!
				t = (s16)pShot->GetSpeed();	movie_Actors.Writes16( t );			// Store Speed in Y, it will get splined!
											movie_Actors.Writes16( 0 );			// Dummy Z value
											movie_Actors.Writes16( 0 );			// Dummy DX value
											movie_Actors.Writes16( 0 );			// Dummy DY value
											movie_Actors.Writes16( 0 );			// Dummy DZ value
											movie_Actors.Writes16( 0 );			// Dummy RX value
											movie_Actors.Writes16( 0 );			// Dummy RY value
											movie_Actors.Writes16( 0 );			// Dummy RZ value

				//---	The next few bytes are a union with the regular character (above) and must match for count
				t = (s16)pMovie->m_ActorList.PtrToIndex(pShot->GetEye());
											movie_Actors.Writes16( t );			// Eye position for camera
				t = (s16)pMovie->m_ActorList.PtrToIndex(pShot->GetTarget());		
											movie_Actors.Writes16( t );			// Target position for camera
											movie_Actors.Writes16( 0 );
											movie_Actors.Writes16( 0 );
				//---	end of union


											movie_Actors.Writes16( 0x0002 ); 	// Flags field - This character uses linear blend

											movie_Actors.Writes16( 0 );			// Event field
											movie_Actors.Writes16( 0 );			// Scale field

				nShots++;
			}
		}

		//---	Write Number of Shots in Actor KeyData
		movie_ActorTable.Writes16( nShots ) ;							// Count of Keys
		//--- Dump out name of this shot object

		s32 i;
		char ch;
		char *pName = "SHOT LIST";

		for (i=0;i<15;i++)
		{
			ch = *pName;
			
			if (ch)
			{
				// Force it to uppercase
				if ( (ch>='a') && (ch<='z') ) ch -= 'a'-'A';
				movie_ActorTable.Writeu8( ch );
				pName++;
			}
			else
			{
				movie_ActorTable.Writeu8( 0 );
			}
		}

			//--- Write Null terminator for string
		movie_ActorTable.Writeu8( 0 );

		//---	Increment Number of Actors Exported
		nActors++ ;
		m_pProgressDialog->SetPos( nActors ) ;
	}
	//--- End BISC Added 6/23

	m_pProgressDialog->SetPos( pMovie->m_ActorList.GetCount() ) ;

	//---	Build Final Output File
	movie_Final.Writeu8('V');
	movie_Final.Writeu8(MAJOR_VERSION_NUMBER);
	movie_Final.Writeu8('.');
	movie_Final.Writeu8(MINOR_VERSION_NUMBER);
	movie_Final.Writes32( nActors ) ;
	{
		s16 t;
		t = (s16)pMovie->GetNumFrames();	movie_Final.Writes32( t );
	}

	movie_Final.Cat( &movie_ActorTable ) ;
	movie_Final.Cat( &movie_Actors ) ;

//////////////////////////////////////////////////////////////////////////////
//	SAVE

	movie_Final.WriteToDisk( pFileName ) ;
//	anim_Header.WriteToDisk( ::MFCU_SetExtension( pFileName, ".h" ) ) ;

	//---	Print Errors recorded
	m_pProgressDialog->Fmt( " " ) ;
	if( nErrors == 0 )
		m_pProgressDialog->Fmt( "No Errors during Export!", nErrors ) ;
	else if( nErrors == 1 )
		m_pProgressDialog->Fmt( "%d Error during Export!", nErrors ) ;
	else
		m_pProgressDialog->Fmt( "%d Errors during Export!", nErrors ) ;


//////////////////////////////////////////////////////////////////////////////
//	COMPLETE

	//---	Close Progress Dialog
	m_pProgressDialog->EnableOk( true ) ;
}
