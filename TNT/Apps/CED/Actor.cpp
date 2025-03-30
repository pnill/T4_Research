// Actor.cpp : implementation file
//

#include "stdafx.h"

#include "Actor.h"
#include "ieArchive.h"
#include "Key.h"
#include "RenderContext.h"
#include "CeDDoc.h"
#include "Mesh.h"


#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CActor Implementation

IMPLEMENT_DYNCREATE(CActor, CObject)

/////////////////////////////////////////////////////////////////////////////
// CActor Constructors

void CActor::Initialize()
{
	m_Name = "New Actor" ;
	m_pCharacter = NULL ;
	m_pMesh = NULL ;
	m_pCurKey = NULL ;

	m_nCache = 0 ;
	m_pCache = NULL ;
	m_CacheFirstFrame = 0 ;

	m_Type = ACTOR_CHARACTER ;
	m_DisplayPath = true ;
	m_Selected = false ;

	m_Layer = -1;
}

CActor::CActor()
{
	//---	Reset Data Members
	Initialize () ;
}

CActor::~CActor()
{
	//---	Destroy the Texture
	Destroy () ;
}

/////////////////////////////////////////////////////////////////////////////
// CActor Implementation

bool CActor::Create()
{
	return true ;
}

void CActor::Destroy()
{
	if( m_pCache ) x_free( m_pCache ) ;

	//---	Initialize Data Members
	Initialize () ;
}

void CActor::Copy (CActor &Dst, CActor &Src)
{
	Dst.m_pCharacter		= Src.m_pCharacter ;
	Dst.m_Skin				= Src.m_Skin ;

	Dst.m_Name				= Src.m_Name ;
	Dst.m_Type				= Src.m_Type ;

	Dst.m_Keys				= Src.m_Keys ;
	Dst.m_pCurKey			= Src.m_pCurKey ;
	Dst.m_pMesh				= Src.m_pMesh ;

	Dst.m_Layer				= Src.m_Layer ;

	CalculateSplineCache( ) ;
}

/////////////////////////////////////////////////////////////////////////////
// CActor Overridden operators

CActor &CActor::operator=( CActor &ptActor )
{
	Copy (*this, ptActor) ;

    return *this;  // Assignment operator returns left side.
}

/////////////////////////////////////////////////////////////////////////////
// CActor serialization

void CActor::Serialize(CieArchive& a)
{
	CCeDDoc *pDoc = (CCeDDoc*)a.m_pDocument ;

	if (a.IsStoring())
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_ACTOR) ;

		//---	Write Details
		a.WriteTaggedString( IE_TAG_ACTOR_NAME, m_Name ) ;
		m_Keys.Serialize( a ) ;
		if( m_pCharacter )
		{
			int nCharacter = pDoc->m_CharacterList.PtrToIndex( m_pCharacter ) ;
			a.WriteTaggeds32( IE_TAG_ACTOR_CHARACTER, nCharacter ) ;
		}
		if( m_pMesh )
		{
			int nMesh = pDoc->m_MeshList.PtrToIndex( m_pMesh ) ;
			a.WriteTaggeds32( IE_TAG_ACTOR_MESH, nMesh ) ;
		}
		a.WriteTaggeds32( IE_TAG_ACTOR_TYPE, m_Type ) ;
		a.WriteTaggedbool( IE_TAG_ACTOR_DISPLAYPATH, m_DisplayPath ) ;
		a.WriteTaggeds32( IE_TAG_ACTOR_LAYER, m_Layer );

		a.WriteTaggeds32( IE_TAG_ACTOR_SKIN, m_Skin );

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_ACTOR_END ) ;
	}
	else
	{
		//---	Load
		BOOL done = FALSE ;
		s32 nCharacter ;
		s32 nMesh ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_ACTOR_NAME:
				a.ReadString(m_Name) ;
				break ;
			case IE_TAG_KEYLIST:
				m_Keys.Serialize( a ) ;
				break ;
			case IE_TAG_ACTOR_CHARACTER:
				a.Reads32( nCharacter ) ;
				m_pCharacter = (CCharacter*)pDoc->m_CharacterList.IndexToPtr( nCharacter ) ;
				break ;
			case IE_TAG_ACTOR_MESH:
				a.Reads32( nMesh ) ;
				m_pMesh = (CMesh*)pDoc->m_MeshList.IndexToPtr( nMesh ) ;
				break ;
			case IE_TAG_ACTOR_TYPE:
				a.Reads32( m_Type ) ;
				break ;
			case IE_TAG_ACTOR_DISPLAYPATH:
				a.Readbool( m_DisplayPath ) ;
				break ;
			case IE_TAG_ACTOR_END:
				done = TRUE ;
				break ;
			case IE_TAG_ACTOR_SCALE:
				f32 dummy;
				a.Readf32 (dummy);
				break;
			case IE_TAG_ACTOR_LAYER:
				a.Reads32( m_Layer );
				break;
			case IE_TAG_ACTOR_SKIN:
				a.Reads32( m_Skin );
				break;

			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CActor diagnostics

#ifdef _DEBUG
void CActor::AssertValid() const
{
	CObject::AssertValid();
}

void CActor::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// Name Functions

void CActor::SetName( const char *pName )
{
	m_Name = pName ;
}

const char *CActor::GetName( )
{
	return m_Name ;
}

/////////////////////////////////////////////////////////////////////////////
// Character Functions

void CActor::SetCharacter( CCharacter *pCharacter )
{
	if( pCharacter == m_pCharacter )
		return;

	m_pCharacter = pCharacter ;

	//---	look for the equivilent motion within the new character's keys
	POSITION pos = m_Keys.GetHeadPosition();
	CKey* pKey;
	while( pos )
	{
		pKey = m_Keys.GetNext( pos );
		pKey->m_pCharacter = pCharacter;

		//---	if the key has a motion, attempt to find it
		if( pKey->GetMotion() )
		{
			CString PathName = pKey->GetMotion()->GetPathName();
			CString ExportName = pKey->GetMotion()->GetExportName();
			CMotion* pCurMotion = NULL;

			POSITION motpos = m_pCharacter->m_MotionList.GetHeadPosition();
			CMotion* pMotion;
			while( motpos )
			{
				pMotion = m_pCharacter->m_MotionList.GetNext( motpos );
				if( pMotion->GetPathName() == PathName )
				{
					pCurMotion = pMotion;
					break;
				}
				else if( pMotion->GetExportName() == ExportName )
					pCurMotion = pMotion;
			}

			pKey->m_pMotion = pCurMotion;
		}
	}
}

CCharacter *CActor::GetCharacter( )
{
	return m_pCharacter ;
}

/////////////////////////////////////////////////////////////////////////////
// Mesh Functions

void CActor::SetMesh( CMesh *pMesh )
{
	m_pMesh = pMesh ;
}

CMesh *CActor::GetMesh( )
{
	return m_pMesh ;
}

/////////////////////////////////////////////////////////////////////////////
// Key Functions

CKey *CActor::SetCurKey( CKey *pKey )
{
	m_pCurKey = pKey ;
	return m_pCurKey ;
}

CKey *CActor::GetCurKey( )
{
	return m_pCurKey ;
}

CKey *CActor::KeyNew( int Frame )
{
	CKey *pKey = new CKey ;
	ASSERT( pKey ) ;
	pKey->SetFrame( Frame ) ;
	vector3  v ;
	GetPositionAtTime( &v, Frame ) ;
	pKey->SetPosition( &v ) ;
	m_Keys.InsertSorted( pKey ) ;
	return pKey ;
}

void CActor::KeyDelete( CKey *pKey )
{
	CKey *pScan = NULL ;
	CKey *pPrev = NULL ;

	POSITION Pos = m_Keys.GetHeadPosition() ;
	while( Pos )
	{
		POSITION Pos2 = Pos ;
		pPrev = pScan ;
		pScan = m_Keys.GetNext( Pos ) ;

		if( pScan == pKey )
		{
			m_Keys.RemoveAt( Pos2 ) ;
			delete pKey ;
			if( GetCurKey() == pKey )
				SetCurKey( NULL ) ;
			CalculateSplineCache( ) ;
			break ;
		}
	}
}

void CActor::KeySelectAll( bool Select )
{
	CKey *pScan ;

	POSITION Pos = m_Keys.GetHeadPosition() ;
	while( Pos )
	{
		pScan = m_Keys.GetNext( Pos ) ;
		ASSERT( pScan ) ;
		pScan->m_Selected = Select ;
	}
}

void CActor::KeyDrag( vector3  *pDelta )
{
	CKey *pScan ;

	POSITION Pos = m_Keys.GetHeadPosition() ;
	while( Pos )
	{
		pScan = m_Keys.GetNext( Pos ) ;
		ASSERT( pScan ) ;
		if( pScan->m_Selected )
		{
			vector3  Position ;
			pScan->GetPosition( &Position ) ;
			Position += *pDelta;
			//V3_Add( &Position, &Position, pDelta ) ;
			pScan->SetPosition( &Position ) ;
		}
	}
}

bool CActor::KeySyncMotion( CKey* pKey )
{
	s32	KeyIndex;
	bool KeyChanged = FALSE;

	//---	exit now if the key has no motion
	if( pKey->GetMotion() == NULL )
		return KeyChanged;

	//---	find the index of this key
	for( KeyIndex=0; KeyIndex<m_Keys.GetCount(); KeyIndex++ )
		if( pKey == (CKey*)m_Keys.GetAt( m_Keys.FindIndex(KeyIndex) ))
			break;

	//---	if the key wasn't found return
	if( KeyIndex==m_Keys.GetCount() )
		return KeyChanged;

	//---	move to the first, previous key with this motion
	while( --KeyIndex >= 0 )
		if( ((CKey*)m_Keys.GetAt( m_Keys.FindIndex(KeyIndex) ))->GetMotion() != pKey->GetMotion() )
			break;

	//---	loop through all keys which have this same motion and set their start frame
	//		use the first keys start frame
	POSITION pos = m_Keys.FindIndex( ++KeyIndex );
	CKey* pStepKey = (CKey*)m_Keys.GetAt( pos );
	ASSERT(pos);
	f32 CurFrame = (f32)pStepKey->GetStartFrame();
	s32 CurTime = pStepKey->GetFrame();
	f32 CurRate = pStepKey->GetFrameRate();
	while( pos && ((pStepKey=((CKey*)m_Keys.GetNext(pos)))->GetMotion() == pKey->GetMotion()) )
	{
		CurFrame += CurRate*(pStepKey->GetFrame() - CurTime);
		pStepKey->SetStartFrame( (s32)CurFrame % pStepKey->GetMotion()->GetNumFrames() );
		CurTime = pStepKey->GetFrame();
		CurRate = pStepKey->GetFrameRate();

		if( pStepKey == pKey )
			KeyChanged = TRUE;
	}

	return KeyChanged;
}


bool CActor::KeySyncDirection( CKey* pKey )
{
	s32	KeyIndex;
	bool KeyChanged = FALSE;
return FALSE;
	//---	find the index of this key
	for( KeyIndex=0; KeyIndex<m_Keys.GetCount(); KeyIndex++ )
		if( pKey == (CKey*)m_Keys.GetAt( m_Keys.FindIndex(KeyIndex) ))
			break;

	//---	if the key wasn't found, return
	if( KeyIndex == m_Keys.GetCount() )
		return KeyChanged;

#if 1
	//---	if this key has its AutoLinkDir flag set, start with the previous key, if it doesn't start with this one.
	if( pKey->m_AutoLinkDir && (KeyIndex > 0))
		KeyIndex--;
#else
	//---	find the first key from this one looking backwards which doesn't have the AutoLinkDir flag set
	for( ; KeyIndex>0; --KeyIndex )
		if( !((CKey*)m_Keys.GetAt( m_Keys.FindIndex(KeyIndex) ))->m_AutoLinkDir )
			break;
#endif

	//---	starting from this key, set the key direction until a key is reached which doesn't have the auto-link set
	POSITION pos = m_Keys.FindIndex( KeyIndex );
	CKey* pStepKey = (CKey*)m_Keys.GetAt( pos );
	ASSERT(pos);
	vector3  v, cv;
	pStepKey->GetVector( &v ) ;
	radian CurFaceDir = x_atan2( v.X, v.Z );
	f32	StartFrame = (f32)pStepKey->GetStartFrame();
	s32 CurTime = pStepKey->GetFrame();
	f32 CurRate = pStepKey->GetFrameRate();
	CMotion*pMotion = pStepKey->GetMotion();
	bool FirstKey = TRUE;
	while( pos )
	{
		pStepKey = (CKey*)m_Keys.GetNext(pos);
		if( !FirstKey && (!pStepKey->m_AutoLinkDir || !pStepKey->m_AMCbased) )
			break;
		FirstKey = FALSE;

		//---	calculate the change in motion
		int	rate					= (int)(CurRate*256.0f);
		int	FramesScaledByAnimRate	= (pStepKey->GetFrame() - CurTime)*rate/256;
		f32	EndFrame				= FramesScaledByAnimRate + StartFrame;
		radian DDir = 0.0f;

		if( pMotion )
		{
//			while( StartFrame >= pMotion->GetNumFrames() )
//			{
//				StartFrame -= pMotion->GetNumFrames();
//				EndFrame -= pMotion->GetNumFrames();
//			}
//			radian CycleDDir = pMotion->GetFaceDir( pMotion->GetNumFrames()-1 ) - pMotion->GetFaceDir( 0 );
//			while( (EndFrame - StartFrame) >= pMotion->GetNumFrames() )
//			{
//				EndFrame	-= pMotion->GetNumFrames();
//				DDir		+= CycleDDir;
//			}
//			if( EndFrame >= pMotion->GetNumFrames() )
//			{
//				DDir		+= pMotion->GetFaceDir( (s32)EndFrame % pMotion->GetNumFrames() ) - pMotion->GetFaceDir( 0 );
//				EndFrame	= (f32)pMotion->GetNumFrames();
//			}
//
//			if( EndFrame > StartFrame )
//				DDir += pMotion->GetFaceDir( (s32)EndFrame-1 ) - pMotion->GetFaceDir( (s32)StartFrame );
			DDir = pMotion->GetFaceDir( (s32)EndFrame % pMotion->GetNumFrames() ) - pMotion->GetFaceDir( (s32)StartFrame % pMotion->GetNumFrames() );
		}

		//---	set this frames direction
		pStepKey->GetVector( &cv );
		f32 Length = x_sqrt(cv.X*cv.X + cv.Z*cv.Z);
		if( Length < 1.0f )	Length = 1.0f;
		CurFaceDir += DDir ;
		v.X = Length*x_sin( CurFaceDir );
		v.Z = Length*x_cos( CurFaceDir );
		
		//---	if the new direction is different, set the new value.
		if(( v.X != cv.X )||( v.Z != cv.Z ))
		{
			pStepKey->SetVector( &v );

			if( pStepKey == pKey )
				KeyChanged = TRUE;
		}

		//---	set the values to this keys values before going on to the next key
		StartFrame	= (f32)pStepKey->GetStartFrame();
		CurTime		= pStepKey->GetFrame();
		CurRate		= pStepKey->GetFrameRate();
		pMotion		= pStepKey->GetMotion();
	}

	return KeyChanged;
}

/////////////////////////////////////////////////////////////////////////////
// Render Functions

void CActor::CalcSpline( vector3  *p1, vector3  *p2, vector3  *v1, vector3  *v2, int num, vector3  *points)
{
	static double	hermite[4][4] =	{			/* Hermite spline matrix */
						{  2, -2,  1,  1 },
						{ -3,  3, -2, -1 },
						{  0,  0,  1,  0 },
						{  1,  0,  0,  0 } } ;
	static double	tmat[4] = { 0, 0, 0, 1 } ;	/* Parameter matrix for hermite calculation */
	double			rmat[4] ;					/* Result matrix for hermite calculation */
	double			t ;							/* Parameter variable, varies 0 to 1 */
	int				pt ;						/* Current point on spline */
	int				row ;						/* Row for matrix multiply */
	int				col ;						/* Column for matrix multiply */

	/*---	Store the initial point in the points array & decrement the number left */
	points->X = p1->X ;
	points->Y = p1->Y ;
	points->Z = p1->Z ;
	points++ ;
	num-- ;

	//---	Loop through all the points between the start and end
	for (pt = 1 ; pt < num ; pt++)
	{
		//---	Calculate the parameter variable
        t = (double)pt / (double)num ;

		//---	Store variable terms in the matrix
		tmat[2] = t ;
		tmat[1] = t * t ;
		tmat[0] = t * t * t ;

		//---	Perform the matrix multiply to factor 't' into the hermite
		for (row = 3 ; row >= 0 ; row--)
		{
           	rmat[row] = 0 ;
			for (col = 3 ; col >= 0 ; col--)
				rmat[row] += tmat[col] * hermite[col][row] ;
		}

		//---	Calculate x, y, z coords of spline point & store
		points->X = (f32)(p1->X * rmat[0] + p2->X * rmat[1] + v1->X * rmat[2] + v2->X * rmat[3]) ;
		points->Y = (f32)(p1->Y * rmat[0] + p2->Y * rmat[1] + v1->Y * rmat[2] + v2->Y * rmat[3]) ;
		points->Z = (f32)(p1->Z * rmat[0] + p2->Z * rmat[1] + v1->Z * rmat[2] + v2->Z * rmat[3]) ;
		points++ ;
	}

	//---	Store the final point in the points array
	if( num > 1 )
	{
		points->X = p2->X ;
		points->Y = p2->Y ;
		points->Z = p2->Z ;
	}
}

void CActor::CalculateSplineCache( )
{
	//---	Get First Frame
	POSITION Pos = m_Keys.GetHeadPosition() ;
	if( Pos )
	{
		//---	Read Head Key
		CKey *pKey1 = (CKey*)m_Keys.GetAt( Pos ) ;
		Pos = m_Keys.GetTailPosition() ;
		if( Pos )
		{
			//---	Read Tail Key
			CKey *pKey2 = (CKey*)m_Keys.GetAt( Pos ) ;

			//---	Set First & Number of Cache Points
			m_CacheFirstFrame = pKey1->GetFrame() ;
			m_nCache = pKey2->GetFrame() - pKey1->GetFrame() + 1 ;

			//---	Allocate Cache Space
			if( m_pCache ) x_free( m_pCache ) ;
			m_pCache = (vector3 *)x_malloc( (m_nCache) * sizeof(vector3 ) ) ;
			if( m_pCache )
			{
				//---	Loop through Keys and Fill in Cache
				if( m_nCache == 1 )
				{
					//---	Only 1 entry, so just set it from the First Key Position
					pKey1->GetPosition( m_pCache ) ;
				}
				else
				{
					//---	Segment to Segment persistant storage
					CMotion		*pCurrentMotion = NULL ;
					int			CurrentMotionFrameOffset = 0 ;

					bool		LastSegmentAMC = false ;
					radian		LastFaceDir = 0 ;

					vector3 	LastSegEnd(0,0,0) ;
					bool		LastSegEndValid = false ;

					//---	Loop through All Keys
					POSITION Pos = m_Keys.GetHeadPosition() ;
					ASSERT( Pos ) ;
					CKey *pKey1 = (CKey*)m_Keys.GetNext( Pos ) ;
					ASSERT( pKey1 ) ;
					while( Pos )
					{
						//---	Get Next Key
						CKey *pKey2 = m_Keys.GetNext( Pos ) ;

						//---	Get Segment Details
						int fs = pKey1->GetFrame() ;
						int fe = pKey2->GetFrame() ;
						int fd = fe - fs + 1 ;
						ASSERT( ((fs-m_CacheFirstFrame)+fd) <= m_nCache ) ;
						vector3  p1, p2 ;
						vector3  v1, v2 ;
						pKey1->GetPosition( &p1 ) ;
						pKey2->GetPosition( &p2 ) ;
						pKey1->GetVector( &v1 ) ;
						pKey2->GetVector( &v2 ) ;
						v1.Scale( 2 );
						v2.Scale( 2 );
//						V3_Scale( &v1, &v1, 2 ) ;
//						V3_Scale( &v2, &v2, 2 ) ;

						//---	Adjust Positions if AutoLinked to previous segment
						if( LastSegEndValid && pKey1->m_AutoLinkPos )
						{
							//---	Get Delta from Start to End of this segment
							vector3  d ;
							d = p2 - p1 ;
//							V3_Sub( &d, &p2, &p1 ) ;

							//---	Lock Start point to End of last segment
							p1 = LastSegEnd;
							p2 = p1 + d;
//							V3_Copy( &p1, &LastSegEnd ) ;
//							V3_Add( &p2, &p1, &d ) ;
						}

						//---	Generate Segment SPLINE or AMC
						if( !pKey1->m_AMCbased )
						{
							//---	Generate Spline based motion
							CalcSpline( &p1, &p2, &v1, &v2, fd, &m_pCache[fs-m_CacheFirstFrame] ) ;
							LastSegmentAMC = false ;
							LastSegEnd = p2 ;
//							V3_Copy( &LastSegEnd, &p2 ) ;
							LastSegEndValid = true ;
						}
						else
						{
							//---	Get Current Position, will be updated by move deltas
							vector3 	p ;
							p = p1;
//							V3_Copy( &p, &p1 ) ;

							//---	Get First Frame Details
							CMotion		*pMotion ;
							int			nFrame ;
							vector3 	DeltaMove ;
							vector3 	tDeltaMove ;
							radian		FaceDir ;
							radian		MoveDir ;
							GetMotionInfoAtTime( &pMotion, &nFrame, &DeltaMove, &FaceDir, &MoveDir, fs ) ;

							//---	Calculate Alignment Matrix for this segment, use FaceDir of
							//		first Motion Frame and align that to v1
							matrix4	m ;

							m.Identity();
//							M4_Identity( &m ) ;

//							M4_RotateYOn( &m, x_atan2(v1.X,v1.Z) - FaceDir ) ;

                            radian3  Yaw(0,0,0);
							GetOrientationAtTime( &Yaw.Yaw, fs ) ;
							m.SetRotationXYZ( Yaw );
//							M4_SetRotationsXYZ( &m, &Yaw ) ;

							//---	Generate AMC based motion
							for( int i = 0 ; i < fd ; i++ )
							{
								//---	Read Motion info at time
								GetMotionInfoAtTime( &pMotion, &nFrame, &DeltaMove, &FaceDir, &MoveDir, fs + i );

								//---	Align Delta Move using matrix & apply
								DeltaMove.Y = 0 ;
								m_pCache[fs - m_CacheFirstFrame + i] = p;
								m.Transform( &tDeltaMove, &DeltaMove, 1 );
								p += tDeltaMove;
//								V3_Copy( &m_pCache[fs - m_CacheFirstFrame + i], &p );
//								M4_TransformVerts( &m, &tDeltaMove, &DeltaMove, 1 );
//								V3_Add( &p, &p, &tDeltaMove );

/*
								//---	Set Last Face Direction
								vector3  v, tv ;
								V3_Zero( &v ) ;
								v.X = x_sin( FaceDir ) ;
								v.Z = x_cos( FaceDir ) ;
								V3_Transform( &tv, &v, 1, &m ) ;
								LastFaceDir = x_atan2( tv.X, tv.Z ) ;
*/
							}

							p -= tDeltaMove;
							LastSegEnd = p;
//							V3_Sub( &p, &p, &tDeltaMove ) ;
//							V3_Copy( &LastSegEnd, &p ) ;
							LastSegEndValid = true ;
							LastSegmentAMC = true ;
						}

						//---	Advance
						pKey1 = pKey2 ;
					}
				}
			}
		}
	}
}

void CActor::DrawPath( CRenderContext *pRC, bool Selected )
{
	matrix4		m ;
	f32			Len;

	//---	Exit if flagged not to display path
	if( !m_DisplayPath && !Selected )
		return ;

	//---	Ensure Spline Cache is valid
	if( m_pCache == NULL )
	{
		CalculateSplineCache( ) ;
	}

	//---	If Points to Draw then Draw them
	if( m_pCache && (m_nCache > 0) )
	{
		//---	Draw Points in Cache ( or lines )
		pRC->RENDER_TransProjPointArray( m_pCache, m_nCache, RGB( 0, 0, 0 ), false ) ; //Selected ) ;
	}

	//---	Draw All Keys
	if( Selected )
	{
		POSITION Pos = m_Keys.GetHeadPosition() ;
		while( Pos )
		{
			CKey *pKey = m_Keys.GetNext( Pos ) ;

			//---	Setup Matrix
			m.Identity();

			//---	Calc & Push Translation Matrix
			vector3 	v ;
			pKey->GetPosition( &v ) ;
			m.Translate( v );

			pRC->PushMatrix( &m ) ;

			//---	Draw Key
			if( pKey->m_Selected )
				pRC->RENDER_Marker( 8, RGB(255,0,0), RGB(0,255,0), RGB(0,0,255) ) ;
			else
				pRC->RENDER_Marker( 8, RGB(192,192,192), RGB(192,192,192), RGB(192,192,192) ) ;
			pRC->PopMatrix( ) ;

			//---	Calc & Push Vector Matrix
			pKey->GetVector( &v ) ;
			radian RX, RY ;
			
			v.ZToVecRot( &RX, &RY );
			Len = v.Length();

			m.Identity();
			m.RotateX( RX );
			m.RotateY( RY );
			pKey->GetPosition( &v );
			m.Translate( v );

			pRC->PushMatrix( &m ) ;

			//---	Draw the Vector
			pRC->RENDER_Arrow( Len, RGB(128,128,128) ) ;

			//---	Remove translation Matrix
			pRC->PopMatrix( ) ;
		}
	}
}

CKey *CActor::GetKeyAtTime( int t )
{
	CKey *pActiveKey = NULL ;

	//---	Limit t
	if( (t <= (m_CacheFirstFrame+m_nCache-1)) &&
		(t >= m_CacheFirstFrame) )
	{
//		if( m_pCharacter && m_pCharacter->GetSkeleton() )
		{
			//---	Determine Active Motion Key at time t
			POSITION Pos = m_Keys.GetHeadPosition( ) ;
			while( Pos )
			{
				CKey *pKey = m_Keys.GetNext( Pos ) ;
				if( pKey->GetFrame() > t )
					break ;
				pActiveKey = pKey ;
			}
		}
	}

	return pActiveKey ;
}

CKey *CActor::GetMotionKeyAtTime( int t )
{
	CKey	*pActiveKey = NULL ;

	//---	Limit t
//	t = min( m_CacheFirstFrame+m_nCache-1, t ) ;
//	t = max( m_CacheFirstFrame, t ) ;

	if( (t >= m_CacheFirstFrame) && (t <= (m_CacheFirstFrame+m_nCache)) )
	{
		if( m_pCharacter && m_pCharacter->GetSkeleton() )
		{
			//---	Determine Active Motion Key at time t
			POSITION Pos = m_Keys.GetHeadPosition( ) ;
			while( Pos )
			{
				CKey *pKey = m_Keys.GetNext( Pos ) ;
				if( pKey->GetFrame() > t )
					break ;
				if( pKey->GetMotion() )
				{
					pActiveKey = pKey ;
				}
			}
		}
	}

	//---	Key Found ?
	return pActiveKey ;
}

void CActor::GetRotationAtTime( vector3  *pRot, int t )
{
	pRot->Zero();
	CKey *pKey = GetKeyAtTime( t ) ;
	if( pKey )
	{
		POSITION Pos = m_Keys.Find( pKey ) ;
		CKey *pNextKey = NULL ;
		if( Pos )
		{
			m_Keys.GetNext( Pos ) ;
			if( Pos )
				pNextKey = m_Keys.GetNext( Pos ) ;
		}
		if( pNextKey )
		{
			//--	Interpolate
			vector3  r1, r2 ;
			f32 dt = (f32)(pNextKey->GetFrame() - pKey->GetFrame()) ;
			f32 it = 0 ;
			if( dt > 0 )
				it = (f32)(t - pKey->GetFrame()) / dt ;
			pKey->GetRotation( &r1 ) ;
			pNextKey->GetRotation( &r2 ) ;
			r1.Scale( 1-it );
			r2.Scale( it );
			*pRot = r1 + r2;

//			V3_Scale( &r1, &r1, 1-it ) ;
//			V3_Scale( &r2, &r2, it ) ;
//			V3_Add( pRot, &r1, &r2 ) ;
		}
		else
		{
			//--	Last Key or Only Key
			pKey->GetRotation( pRot ) ;
		}
		pRot->Scale( R_360/360 );
//		V3_Scale( pRot, pRot, R_360/360 ) ;
	}
}

f32 CActor::GetScaleAtTime( int t )
{
	f32 scale;

	CKey *pKey = GetKeyAtTime( t ) ;
	if( pKey )
	{
		POSITION Pos = m_Keys.Find( pKey ) ;
		CKey *pNextKey = NULL ;
		if( Pos )
		{
			m_Keys.GetNext( Pos ) ;
			if( Pos )
				pNextKey = m_Keys.GetNext( Pos ) ;
		}

		if( pNextKey )
		{
			//--	Interpolate
			f32 s1,s2;

			f32 dt = (f32)(pNextKey->GetFrame() - pKey->GetFrame()) ;
			f32 it = 0 ;
			if( dt > 0 )
				it = (f32)(t - pKey->GetFrame()) / dt ;
			s1 = pKey->GetScale( ) * (1-it);
			s2 = pNextKey->GetScale( ) * it;
			scale = s1 + s2;
		}
		else
		{
			//--	Last Key or Only Key
			scale = pKey->GetScale( ) ;
		}
		return scale;
	}
	else
	{
		return 1.0f;
	}
}

void CActor::PoseSkeleton( int t )
{
	if( m_pCharacter && m_pCharacter->GetSkeleton() )
	{
		CKey *pActiveKey = GetMotionKeyAtTime( t ) ;

		if( pActiveKey && pActiveKey->GetMotion() )
		{
			CMotion *pMotion = pActiveKey->GetMotion() ;
			int  FramesSinceStartOfKey = t - pActiveKey->GetFrame();
			int  rate = (int)(pActiveKey->GetFrameRate()*256.0f);
			int  FramesScaledByRate = FramesSinceStartOfKey*rate/256;

			m_pCharacter->GetSkeleton()->SetPoseMovie(
				pActiveKey->GetMotion(),
				(float)((FramesScaledByRate+pActiveKey->GetStartFrame())%(pMotion->GetNumFrames()-1)),
				pActiveKey->GetMirror(),
				false) ;
		}
		else
		{
			m_pCharacter->GetSkeleton()->SetNeutralPose( ) ;
		}
	}
}

void CActor::GetMotionInfoAtTime( CMotion **ppMotion, int *pnFrame, vector3  *pDeltaMove, radian *pFaceDir, radian *pMoveDir, int t )
{
	if( m_pCharacter && m_pCharacter->GetSkeleton() )
	{
		CKey *pActiveKey = GetMotionKeyAtTime( t ) ;

		if( pActiveKey && pActiveKey->GetMotion() )
		{
			CMotion *pMotion = pActiveKey->GetMotion() ;
			*ppMotion	= pMotion ;
			int  FramesSinceStartOfKey = t - pActiveKey->GetFrame();
			int  r = (int)(pActiveKey->GetFrameRate()*256.0f);
			float rate = (float)r/256.0f;
			int  FramesScaledByRate = (int)((float)FramesSinceStartOfKey*rate);
			*pnFrame	= ( FramesScaledByRate + pActiveKey->GetStartFrame() ) % ( pMotion->GetNumFrames() - 1 ) ;
			
			vector3  v1, v2 ;
			pMotion->GetTranslation( v1, *pnFrame ) ;
			pMotion->GetTranslation( v2, *pnFrame+1 ) ;
			*pDeltaMove = v2 - v1;
//			V3_Sub( pDeltaMove, &v2, &v1 ) ;
			*pFaceDir = pMotion->GetFaceDir( *pnFrame ) ;
			*pMoveDir = pMotion->GetMoveDir( *pnFrame ) ;
			pDeltaMove->X *= rate;
			pDeltaMove->Y *= rate;
			pDeltaMove->Z *= rate;

			if( pActiveKey->GetMirror() )
			{
				radian Yaw = pMotion->GetFaceDir( 0 );
//				*pFaceDir = Yaw - (*pFaceDir - Yaw);
//				*pMoveDir = Yaw - (*pMoveDir - Yaw);
				*pFaceDir = -*pFaceDir + 2*Yaw;
				*pMoveDir = -*pMoveDir + 2*Yaw;
				float len = x_sqrt( pDeltaMove->X*pDeltaMove->X + pDeltaMove->Z*pDeltaMove->Z );
				pDeltaMove->X = len*x_sin(*pMoveDir);
				pDeltaMove->Z = len*x_cos(*pMoveDir);
			}
		}
		else
		{
			*ppMotion	= NULL ;
			*pnFrame	= 0 ;
			pDeltaMove->Zero();
//			V3_Zero( pDeltaMove ) ;
			*pFaceDir = 0 ;
			*pMoveDir = 0 ;
		}
	}
}

void CActor::GetPositionAtTime( vector3  *pv, int t )
{
	//---	Ensure Spline Cache is valid
	if( m_pCache == NULL )
	{
		CalculateSplineCache( ) ;
	}

	//---	Limit t to within correct range for cache
	t = min( m_CacheFirstFrame+m_nCache-1, t ) ;
	t = max( m_CacheFirstFrame, t ) ;

	//---	Read Value
	if( m_pCache )
	{
		*pv = m_pCache[t-m_CacheFirstFrame];
//		V3_Copy( pv, &m_pCache[t-m_CacheFirstFrame] ) ;
	}
	else
	{
		pv->Zero();
//		V3_Zero( pv ) ;
	}
}

void CActor::GetDirectionAtTime( vector3  *pv, int t )
{
	vector3 	v1, v2 ;

	//---	Limit t
	t = min( m_CacheFirstFrame+m_nCache-1, t ) ;
	t = max( m_CacheFirstFrame, t ) ;

	GetPositionAtTime( &v1, t ) ;
	if( t == m_CacheFirstFrame+m_nCache-1 )
	{
		v2 = v1;
//		V3_Copy( &v2, &v1 ) ;
		if( (t-1) >= 0 )
		{
			GetPositionAtTime( &v1, t-1 ) ;
		}
	}
	else
	{
		GetPositionAtTime( &v2, t+1 ) ;
	}

	v2 -= v1;
	if( v2.Length() == 0 )
		v2.Z = 1;
	v2.Normalize();

	*pv = v2;

//	V3_Sub( &v2, &v2, &v1 ) ;
//	if( V3_Length( &v2 ) == 0 )
//	{
//		v2.Z = 1 ;
//	}
//	V3_Normalize( &v2 ) ;
//
//	//---	Return Value
//	V3_Copy( pv, &v2 ) ;
}

void CActor::GetOrientationAtTime( radian *pYRot, int t )
{
	*pYRot = 0 ;

	CKey *pKey = GetMotionKeyAtTime( t ) ;

	if( pKey )
	{
		if( pKey->m_AMCbased )
		{
			CKey *pKey = GetMotionKeyAtTime( t ) ;
			if( pKey )
			{
				vector3 	kv ;
				int			FramesSinceStartOfKey	= t - pKey->m_Frame;
				int			rate					= (int)(pKey->m_FrameRate*256.0f);
				int			FramesScaledByAnimRate	= FramesSinceStartOfKey*rate/256;
				int			at = (FramesScaledByAnimRate + pKey->m_StartFrame) % (pKey->GetMotion()->GetNumFrames()-1) ;
/*
				CKey *pDirKey = pKey;
				if( pDirKey->m_AutoLinkDir )
				{
					POSITION pos = m_Keys.GetHeadPosition();
					s32 Index=0;
					for( Index=0; Index<m_Keys.GetCount(); Index++ )
						if( m_Keys.GetAt(m_Keys.FindIndex(Index)) == pKey )
							break;

					s32 EndIndex;
					s32 StartIndex;
					EndIndex = StartIndex = Index;
					for(; StartIndex>0; StartIndex-- )
					{
						pDirKey = (CKey*)m_Keys.GetAt(m_Keys.FindIndex(Index));
						if( !pDirKey->m_AutoLinkDir )
							break;
					}

					pos = m_Keys.FindIndex( StartIndex );
					pDirKey = m_Keys.GetAt( pos );

					pDirKey->GetVector( &kv );
					*pYRot = x_atan2(kv.X, kv.Z) - pDirKey->GetMotion()->GetFaceDir( 0 );
					int Frame = pDirKey->GetFrame();

					while( pos )
					{
						pDirKey = (CKey*)m_Keys.GetNext( pos );
						if( pDirKey == pKey )
							break;

						if( pDirKey->GetMotion() )
						{
							int FramesInMotion			= pDirKey->GetMotion()->GetNumFrames();
							int FramesInKey				= pDirKey->GetFrame() - Frame;
							int	rate					= (int)(pDirKey->m_FrameRate*256.0f);
							int	FramesScaledByAnimRate	= FramesInKey*rate/256;
							int StartFrame				= pDirKey->GetStartFrame();
							int LastFrame				= StartFrame + FramesScaledByAnimRate;
							radian Dir;
							radian tDDir;
							radian DDir;

							if( LastFrame > (pDirKey->GetMotion()->GetNumFrames()-1) )
							{
								Dir = pDirKey->GetMotion()->GetFaceDir( FramesInMotion-1 );
								DDir = Dir - pDirKey->GetMotion()->GetFaceDir( StartFrame % FramesInMotion );
								StartFrame += FramesInMotion - (StartFrame % FramesInMotion);

								tDDir = Dir - pDirKey->GetMotion()->GetFaceDir( 0 );
								while( (LastFrame - StartFrame) > FramesInMotion )
								{
									DDir += tDDir;
									StartFrame += FramesInMotion;
								}
								Dir = pDirKey->GetMotion()->GetFaceDir( LastFrame % FramesInMotion );
								DDir += Dir - pDirKey->GetMotion()->GetFaceDir( 0 );
							}
							else
							{
								Dir = pDirKey->GetMotion()->GetFaceDir( LastFrame );
								DDir = Dir - pDirKey->GetMotion()->GetFaceDir( StartFrame );
							}

							*pYRot -= DDir;
						}

						Frame = pDirKey->GetFrame();
					}

					*pYRot += pKey->GetMotion()->GetFaceDir( at );
				}
				else*/
				{
					pKey->GetVector( &kv ) ;
					*pYRot = x_atan2(kv.X,kv.Z)-pKey->GetMotion()->GetFaceDir( at ) ;
				}
			}
			else
			{
				*pYRot = 0 ;
			}
		}
		else
		{
			//---	Limit t
			t = min( m_CacheFirstFrame+m_nCache-1, t ) ;
			t = max( m_CacheFirstFrame, t ) ;

			vector3 	v1, v2 ;

			GetPositionAtTime( &v1, t ) ;
			if( t == m_CacheFirstFrame+m_nCache-1 )
			{
				v2 = v1;
//				V3_Copy( &v2, &v1 ) ;
				if( (t-1) >= 0 )
				{
					GetPositionAtTime( &v1, t-1 ) ;
				}
			}
			else
			{
				GetPositionAtTime( &v2, t+1 ) ;
			}

			v2 -= v1;
			if( v2.Length() == 0 )
				v2.Z = 1;
			v2.Normalize();

//			V3_Sub( &v2, &v2, &v1 ) ;
//			if( V3_Length( &v2 ) == 0 )
//			{
//				v2.Z = 1 ;
//			}
//			V3_Normalize( &v2 ) ;

			//---	Return Value
			*pYRot = x_atan2( v2.X, v2.Z ) ;
		}
	}
}

void CActor::InsertFrames( int t, int nFrames )
{
	//---	Loop through all Keys and Add nFrames to Frame if >= t
	POSITION Pos = m_Keys.GetHeadPosition() ;
	while( Pos )
	{
		CKey *pKey = (CKey*)m_Keys.GetNext( Pos ) ;
		if( pKey->GetFrame() >= t )
		{
			int NewFrame = pKey->GetFrame() + nFrames ;
			if( nFrames < 0 )
				NewFrame = max( t, NewFrame ) ;
			pKey->SetFrame( NewFrame ) ;
		}
	}

	CalculateSplineCache( ) ;
}

void CActor::DeleteFrames( int t, int nFrames )
{
	InsertFrames( t, -nFrames ) ;
}

s32 CActor::SetType( int Type )
{
	m_Type = Type ;
	return m_Type ;
}

s32 CActor::GetType( )
{
	return m_Type ;
}

void CActor::MotionDeleted( CMotion *pMotion )
{
	bool modified = false ;

	//---	Loop through all Keys and Validate Motions
	POSITION Pos = m_Keys.GetHeadPosition() ;
	while( Pos )
	{
		CKey *pKey = (CKey*)m_Keys.GetNext( Pos ) ;
		if( pKey->GetMotion() == pMotion )
		{
			pKey->SetMotion( NULL ) ;
			modified = true ;
		}
	}

	//---	Recalculate the Spline Cache
	if( modified )
		CalculateSplineCache( ) ;
}

void CActor::SetLayer( int Layer )
{
	m_Layer = Layer;
}

int CActor::GetLayer( void )
{
	return m_Layer;
}
