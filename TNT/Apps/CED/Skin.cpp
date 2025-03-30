// Skin.cpp: implementation of the CSkin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ced.h"
#include "Skin.h"
#include "Skel.h"

#include "x_plus.hpp"
#include "x_memory.hpp"

//---	MTF file support includes
#include "skin_mesh.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define	MAX_NUM_BONES		100
#define	MAX_NUM_VERTS		16384
#define	MAX_NUM_CHUNKS		1024
#define	MAX_FACE_MATRICES	16

//using namespace mesh;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkin::CSkin()
{
	m_Name	= "<new skin>";
	m_File	= "";
	m_pSkin	= NULL;
}

CSkin::~CSkin()
{
	if( m_pSkin )
		delete m_pSkin;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//	Initialization functionality
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

xbool CSkin::CreateFromFile( const char* pFileName )
{
	char	Ext[_MAX_EXT] ;

	m_File = pFileName;

	_splitpath( pFileName, NULL, NULL, NULL, Ext ) ;
	if(( stricmp( Ext, ".MTF" ) == 0 )||
	   ( stricmp( Ext, ".ATF" ) == 0 ))
	{
		return CreateFromMTF( pFileName ) ;
	}
	else
	{
		return FALSE ;
	}
}

#include "d3d8.h"
#include "D3DView.h"

////////////////////////////////////////////////////////////////////////////
xbool CSkin::FitToSkeleton( CSkel& rSkel )
{
	if( m_pSkin )
	{
		//---	remap the vert weights to the new skeleton
		RemapVertWeights( rSkel, m_pSkin );

#if 1 // needed if we are going to use D3D to transform the verts into world space

		D3DCAPS8 caps = CD3DView::m_Adapters[CD3DView::m_dwAdapter].devices[CD3DView::m_Adapters[CD3DView::m_dwAdapter].dwCurrentDevice].d3dCaps;

		//---	rebuild the skin mesh so that there are no more than
		//		4 blended matrices per chunk
		s32 SourceMaxFaceWeights;
		s32 SourceMaxVertexWeights;
		LimitMatricesPerChunk( caps.MaxVertexBlendMatrices, SourceMaxFaceWeights, SourceMaxVertexWeights );

#endif
	}

	return TRUE;
}



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//	Implementation
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

const CString& CSkin::GetName( void )
{
	return m_Name;
}


void CSkin::Serialize( CieArchive& a )
{
	if (a.IsStoring())
	{
		//---	START
		a.WriteTag(IE_TYPE_START,IE_TAG_SKIN) ;

		//---	Write Details
		a.WriteTaggedString( IE_TAG_SKIN_NAME, m_Name ) ;

		//---	Write Details
		a.WriteTaggedString( IE_TAG_SKIN_PATHNAME, m_File ) ;

		//---	END
		a.WriteTag( IE_TYPE_END, IE_TAG_SKIN_END ) ;
	}
	else
	{
		//---	Load Skin
		BOOL done = FALSE ;
		while (!done)
		{
			a.ReadTag() ;
			switch (a.m_rTag)
			{
			case IE_TAG_SKIN_NAME:
				a.ReadString( m_Name ) ;
				break ;

			case IE_TAG_SKIN_PATHNAME:
				a.ReadString( m_File ) ;
				break ;

			case IE_TAG_SKIN_END:
				done = TRUE ;
				CreateFromFile( m_File ) ;
				break ;

			default:
				ASSERT(0) ;
				a.ReadSkip() ;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////

CSkin& CSkin::operator=( CSkin& rSkin )
{
	return *this;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//	Protected implementation
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

xbool CSkin::CreateFromMTF( const char* FileName )
{
	CTextFile		InputFile;
	xbool			success;

	//---	connect to the source file
	success = InputFile.LoadTextFile( FileName );
	if( !success )
		return FALSE;

	//---	delete the old skin and allocate new space
	if( m_pSkin ) delete m_pSkin;
	m_pSkin = new mesh::object;
	if( m_pSkin == NULL )
		return FALSE;

	//---	load the source file
	success = m_pSkin->Load( InputFile );
	if( !success )
		return FALSE;

	//---	set the name from the file name
	int i = strlen(FileName)-1;
	const char* pCh = (char*)&FileName[i];
	while( (i>0) && (*pCh!='\\') && (*pCh!='/') )
		pCh = &FileName[--i];
	if( (*pCh=='\\') || (*pCh=='/') )
		pCh++;
	m_Name.Format( "%s", pCh );
	i = m_Name.Find( '.' );
	if( i != -1 )
		m_Name = m_Name.Left( i );

	return TRUE;
}

//==========================================================================

static
s32 CountASFBoneChildren( CSkel& rSkel, s32 GeomBoneID )
{
    s32 i;
    s32 count = 0;

    for ( i = 0; i < rSkel.m_NBones; i++ )
    {
        if ( (i != GeomBoneID) &&
             (rSkel.m_pBone[i].ParentID == GeomBoneID) )
        {
            count++;
        }
    }

    return count;
}

//==========================================================================

static
void RecurseBuildParentList( mesh::object* pGeom, s32& rBone, s32* ParentList, s32 Parent )
{
    s32 i;
    s32 NewParent;

    if ( rBone >= pGeom->m_nBones )
        return;

    ParentList[rBone] = Parent;

    NewParent = rBone;
    for ( i = 0; i < pGeom->m_pBone[NewParent].m_nChildren; i++ )
    {
        rBone++;
        RecurseBuildParentList( pGeom, rBone, ParentList, NewParent );
    }
}

//==========================================================================

static
s32 FindParentBone( mesh::object* pGeom, s32 MtfBoneID )
{
    s32 CurrBone = 0;
    s32 Parents[100];

    RecurseBuildParentList( pGeom, CurrBone, Parents, 0 );

    return Parents[MtfBoneID];
}

//==========================================================================

static
s32 FindASFBoneID( CSkel& rSkel, mesh::object* pGeom, s32 MtfBoneID )
{
    s32     j;
    s32     ASFBoneID;

    //---   Given a bone name find the .ASF bone equivalent
    ASFBoneID = -1;

    while( ASFBoneID == -1 )
    {
        //---   Search for bone in asf bones
        for( j = 0; j < rSkel.m_NBones; j++ )
        {
            if( x_stricmp( pGeom->m_pBone[MtfBoneID].m_Name, rSkel.m_pBone[j].BoneName ) == 0 )
            {
                ASFBoneID = j;
                break;
            }
        }

        //---   if no bone found try the parent bone
        if( j == rSkel.m_NBones )
        {
            //---   find the parent...the mtf bone layout isn't very nice, but we can
            //      still find the parent if we try...
            MtfBoneID = FindParentBone( pGeom, MtfBoneID );
        }
    }

    return ASFBoneID;
}

//==========================================================================
void CSkin::RemapVertWeights( CSkel& rSkel, mesh::object* pGeom )
{
    s32 ChunkID;
    s32 VertID;
    s32 WeightID;
    
    s32 GeomBoneID;
    s32 ASFBoneID;

    mesh::vertex*   pVert;
    xbool           done;
    s32             i, j, k;

    ASSERT( pGeom );

    //---   re-map the vert weights
    for ( ChunkID = 0; ChunkID < pGeom->m_nChunks; ChunkID++ )
    {
        for ( VertID = 0; VertID < pGeom->m_pChunk[ChunkID].m_nVerts; VertID++ )
        {
            if ( pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_nWeights == 0 )
            {
                //---   no weights assigned, assume its 100% to the root
                pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_nWeights = 1;
                pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_Weight[0].m_iMatrix = 0;
                pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_Weight[0].m_Weight = 1.0f;
            }
            else
            {
                //---   re-map all of the matrix indices
                for ( WeightID = 0; WeightID < pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_nWeights; WeightID++ )
                {
                    GeomBoneID = pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_Weight[WeightID].m_iMatrix;
                    ASFBoneID = FindASFBoneID( rSkel, pGeom, GeomBoneID );

					//---	bone name not found, map it to the root
                    if ( ASFBoneID == -1 )
					{
                        ASFBoneID = 0;
					}

                    pGeom->m_pChunk[ChunkID].m_pVertex[VertID].m_Weight[WeightID].m_iMatrix = ASFBoneID;
                }
            }
        }
    }

    //---   because we've removed dummy bones, there is a chance that verts
    //      are weighted to the same bone more than once, remove that
    
    for ( ChunkID = 0; ChunkID < pGeom->m_nChunks; ChunkID++ )
    {
        for ( VertID = 0; VertID < pGeom->m_pChunk[ChunkID].m_nVerts; VertID++ )
        {
            pVert = &pGeom->m_pChunk[ChunkID].m_pVertex[VertID];

            done = FALSE;
            while ( !done )
            {
                done = TRUE;
                for ( i = 0; i < pVert->m_nWeights; i++ )
                {
                    for ( j = i + 1; j < pVert->m_nWeights; j++ )
                    {
                        if ( pVert->m_Weight[i].m_iMatrix == pVert->m_Weight[j].m_iMatrix )
                        {
                            done = FALSE;

                            //---   the i-th weight and the j-th weight should be added together
                            pVert->m_Weight[i].m_Weight += pVert->m_Weight[j].m_Weight;

                            //---   shift the list of weights down
                            for ( k = j; k < pVert->m_nWeights-1; k++ )
                                pVert->m_Weight[k] = pVert->m_Weight[k+1];

                            //---   we have removed an un-necessary weight
                            pVert->m_nWeights--;

                            break;
                        }
                    }

                    if ( !done )
                        break;
                }
            }
        }
    }

    //---   the geom's skeleton now matches the asf skeleton, so make a new version of the skeleton for the geom
    mesh::bone* pBone;

    pBone = new mesh::bone[rSkel.m_NBones];
    for ( GeomBoneID = 0; GeomBoneID < rSkel.m_NBones; GeomBoneID++ )
    {
        pBone[GeomBoneID].m_Id = GeomBoneID;
        x_strcpy( pBone[GeomBoneID].m_Name, rSkel.GetBoneName(GeomBoneID) );

        pBone[GeomBoneID].m_nChildren = CountASFBoneChildren( rSkel, GeomBoneID );
    }

    pGeom->m_nBones = rSkel.m_NBones;
    delete []pGeom->m_pBone;
    pGeom->m_pBone = pBone;
}

//==========================================================================
static
inline void DeleteIndexFromFacesToProcessList( s32 Index, s32* pArray, s32 Len )
{
	x_memmove( &pArray[Index], &pArray[Index+1], (Len-Index-1)*sizeof(s32) );
}

//==========================================================================
static
inline xbool ValueIsInList( s32 Value, s32* List, s32 Len )
{
	s32* End = &List[Len];
	while( List != End )
		if( Value == *List++ )
			return TRUE;

	return FALSE;
}

//==========================================================================
static
inline s32 FindValueInList( s32 Value, s32* List, s32 Len )
{
	s32 ret=-1;
	s32* End = &List[Len];
	while( List != End )
	{
		if( Value == *List++ )
		{
			ret = Len - (s32)(((u32)End - (u32)List) / sizeof(u32)) - 1;
			break;
		}
	}

	ASSERT(ret>=-1);
	return ret;
}

//==========================================================================
static
inline xbool ListIsSubsetOfList( s32* SubList, s32 SubLen, s32* List, s32 Len )
{
	s32* End = &SubList[SubLen];
	while( SubList != End )
		if( !ValueIsInList( *SubList++, List, Len ) )
			return FALSE;

	return TRUE;
}

//==========================================================================
static
inline xbool CheckAddToList( s32 Value, s32* List, s32& ListLen, s32 MaxListLen )
{
	//---	if the value is in the list, the task is done
	if( ValueIsInList( Value, List, ListLen ) )
		return FALSE;

	//---	if there is no more room in the list, return now
	if( ListLen == MaxListLen )
		return FALSE;

	//---	add the value to the lsit
	List[ListLen++] = Value;

	return TRUE;
}

//==========================================================================
static
inline xbool FindAddToList( s32 Value, s32* List, s32& ListLen, s32 MaxListLen )
{
	//---	if the value is in the list, the task is done
	s32 i = FindValueInList( Value, List, ListLen );
	ASSERT(i>=-1);
	if( i == -1 )
	{
		//---	if there is no more room in the list, return now
		if( ListLen == MaxListLen )
			return FALSE;

		i = ListLen++;
		List[i] = Value;
	}

	return i;
}

//==========================================================================
static
inline xbool CheckAddListToList( s32* SubList, s32 SubListLen, s32* List, s32& ListLen, s32 MaxListLen )
{
	s32* End;

	//---	add the values in the sub-list to the list if they done exist there already
	End = &SubList[SubListLen];
	while( SubList != End )
		CheckAddToList( *SubList++, List, ListLen, MaxListLen );

	return TRUE;
}

//==========================================================================
static
void CheckWeights( mesh::object* pSkin )
{
	mesh::vertex*	pVert;
	mesh::face*		pFace;
	mesh::chunk*	pChunk;
	s32				i, j, k, l;
	s32				iVert;
	f32				TotalWeight;

	//---	loop through each chunk
	for( i=0; i<pSkin->m_nChunks; i++ )
	{
		pChunk = &pSkin->m_pChunk[i];

		//---	loop through all of the faces
		for( j=0; j<pChunk->m_nFaces; j++ )
		{
			pFace = &pChunk->m_pFace[j];

			//---	loop through each vertex
			for( k=pFace->m_bQuad?3:2; k>=0; k-- )
			{
				iVert = pFace->m_Index[k];
				pVert = &pChunk->m_pVertex[iVert];

				TotalWeight = 0.0f;

				//---	sum the vertex weights to determine if the total changed to less than 1.0f
				for( l=0; l<pVert->m_nWeights; l++ )
					TotalWeight += pVert->m_Weight[l].m_Weight;

				ASSERT(ABS(TotalWeight-1.0f) < 0.001f);
			}
		}
	}
}

//==========================================================================
static
void RemoveWeightedMatrixFromVertex( mesh::object* pSkin, mesh::vertex* pVertex, s32 iMat )
{
	mesh::vertex*	pVert;
	mesh::face*		pFace;
	mesh::chunk*	pChunk;
	s32				i, j, k, l;
	s32				iVert;
	f32				RemovedWeight;
	f32				TotalRemainingWeight;
	f32				NewTotalWeight;

	//---	loop through each chunk
	for( i=0; i<pSkin->m_nChunks; i++ )
	{
		pChunk = &pSkin->m_pChunk[i];

		//---	loop through all of the faces
		for( j=0; j<pChunk->m_nFaces; j++ )
		{
			pFace = &pChunk->m_pFace[j];

			//---	loop through each vertex
			for( k=pFace->m_bQuad?3:2; k>=0; k-- )
			{
				iVert = pFace->m_Index[k];
				pVert = &pChunk->m_pVertex[iVert];

				//---	if these vertices are at the same position, assume
				//		that they are the same vertex and must be weighted identically
				if( pVert->m_vPos == pVertex->m_vPos )
				{
					//---	look for the weight which uses this matrix
					for( l=0; l<pVert->m_nWeights; l++ )
					{
						if( pVert->m_Weight[l].m_iMatrix == iMat )
							break;
					}

					//---	if the weighted matrix was found, remove it
					if(( l != pVert->m_nWeights ) && (pVert->m_nWeights > 1))
					{
						//---	save the removed weight
						RemovedWeight			= pVert->m_Weight[l].m_Weight;
						TotalRemainingWeight	= 1.0f - RemovedWeight;
						NewTotalWeight			= 0.0f;

						//---	remove the matrix
						pVert->m_nWeights--;
						for( ; l<pVert->m_nWeights; l++ )
							pVert->m_Weight[l] = pVert->m_Weight[l+1];

						//---	now redistribute the removed weight to the remaining weights.
						for( l=0; l<pVert->m_nWeights; l++ )
						{
							pVert->m_Weight[l].m_Weight += RemovedWeight * (pVert->m_Weight[l].m_Weight / TotalRemainingWeight);
							NewTotalWeight += pVert->m_Weight[l].m_Weight;
						}

						//---	keep total as close to 1.0 as possible
						pVert->m_Weight[pVert->m_nWeights-1].m_Weight += 1.0f - NewTotalWeight;

//						NewTotalWeight = 0.0f;
//						for(l=0; l<pVert->m_nWeights; l++ )
//							NewTotalWeight += pVert->m_Weight[l].m_Weight;
//						ASSERT(ABS(NewTotalWeight-1.0f) < 0.001f);
					}
				}
			}
		}
	}
}

//==========================================================================
//
//	returns TRUE if there were any changes made to the skin data at all.
//
xbool CSkin::LimitMatricesPerFace( s32 MaxMats, s32& MaxFaceWeights, s32& MaxVertexWeights )
{
	struct SFaceMat
	{
		s32 iMatrix;
		f32 TotalWeight;
		s32 NumVerts;
		mesh::vertex* pVertexList[4];
	};
	SFaceMat		FaceMatrices[MAX_FACE_MATRICES];		// list of matrix uses
	xbool			bChangedVertMatrix;						// return value
	s32				FaceMatricesCount;
	SFaceMat*		pFaceMat;
	s32				i, j, k, l, m;
	s32				iVert, iMatrix;
	s32				RepeatCount=0;
	mesh::vertex*	pVert;
	mesh::face*		pFace;
	mesh::chunk*	pChunk;
	xbool			bModified = FALSE;

	MaxFaceWeights		= 0;
	MaxVertexWeights	= 0;

	//---	if the number of weights to allow is zero, do it quickly
	ASSERT(MaxMats>=0);
	if( MaxMats <= 0 )
	{
		//---	loop through each chunk
		for( i=0; i<m_pSkin->m_nChunks; i++ )
		{
			pChunk = &m_pSkin->m_pChunk[i];
			for( j=0; j<pChunk->m_nVerts; j++ )
			{
				pVert = &pChunk->m_pVertex[j];
				pVert->m_nWeights = 0;
			}
		}

		return FALSE;
	}

	//---	cap the MaxMats field to our actual limitation
	if( MaxMats > MAX_FACE_MATRICES )
		MaxMats = MAX_FACE_MATRICES;

//CheckWeights( m_pSkin );

	//---	loop through each chunk
	for( i=0; i<m_pSkin->m_nChunks; i++ )
	{
		pChunk = &m_pSkin->m_pChunk[i];

		//---	flag that no verts have had new matrices added to them (which they may not have been weighted to
		//		prior.  Recuding matrix counts on a vert will not set this true, but adding a new matrix will.
		bChangedVertMatrix = FALSE;

		//---	loop through all of the faces
		for( j=0; j<pChunk->m_nFaces; j++ )
		{
			pFace = &pChunk->m_pFace[j];

			//
			//---	build a list of matrices for this face
			//
			FaceMatricesCount = 0;
			for( k=pFace->m_bQuad?3:2; k>=0; k-- )
			{
				iVert = pFace->m_Index[k];
				pVert = &pChunk->m_pVertex[iVert];

				//---	store the face with the most weights
				if( pVert->m_nWeights > MaxVertexWeights )
					MaxVertexWeights = pVert->m_nWeights;

				//---	add this vert's matrices to the faces list
				for( l=0; l<pVert->m_nWeights; l++ )
				{
					iMatrix = pVert->m_Weight[l].m_iMatrix;
					for( m=0; m<FaceMatricesCount; m++ )
						if( iMatrix == FaceMatrices[m].iMatrix )
							break;

					pFaceMat = &FaceMatrices[m];
					if( m == FaceMatricesCount )
					{
						pFaceMat->iMatrix		= iMatrix;
						pFaceMat->TotalWeight	= pVert->m_Weight[l].m_Weight;
						pFaceMat->NumVerts		= 0;
						pFaceMat->pVertexList[pFaceMat->NumVerts++] = pVert;
						FaceMatricesCount++;
					}
					else
					{
						pFaceMat->TotalWeight	+= pVert->m_Weight[l].m_Weight;
						pFaceMat->pVertexList[pFaceMat->NumVerts++] = pVert;
					}
				}
			}

			//---	if this face has more weights than our total current count, 
			//		set it to the max
			if( FaceMatricesCount > MaxFaceWeights )
				MaxFaceWeights = FaceMatricesCount;
			
			//
			//---	if this face has more matrices than it should, reduce the count
			//
			if( FaceMatricesCount > MaxMats )
			{
				//---	modified is true
				bModified = TRUE;

				//---	find the least common matrix (removing it will affect the fewest number of verts)
				s32 LowestWeightLen	= 0;
				s32 LowestWeight[MAX_FACE_MATRICES];
				for( k=0; k<FaceMatricesCount; k++ )
				{
					//---	look through the list of least used matrices to look for one with the lowest total weight
					for( l=0; l<LowestWeightLen; l++ )
					{
						if( FaceMatrices[k].TotalWeight < FaceMatrices[LowestWeight[l]].TotalWeight )
							break;
					}

					if( l!=MAX_FACE_MATRICES )
					{
						//---	shift down the array to make room.
						x_memmove( &LowestWeight[l+1], &LowestWeight[l], (LowestWeightLen-l)*sizeof(s32) );

						//---	insert the face matricies index into the list
						LowestWeight[l] = k;
						LowestWeightLen++;
					}
				}

				//---	process the list of matrices from beginning to end and remove matrix weights until the total number is down.
				for( l=0; l<LowestWeightLen; l++ )
				{
					pFaceMat = &FaceMatrices[LowestWeight[l]];

					//---	we are done if the number of total face matricies is low enough
					if( FaceMatricesCount <= MaxMats )
						break;

					//---	make sure that all vertices weigh to more than just this one matrix
					for( m=0; m<pFaceMat->NumVerts; m++ )
					{
						pVert = pFaceMat->pVertexList[m];
						if( pVert->m_nWeights == 1 )
							break;
					}

					//---	if there was a vertex which only weights to this matrix, move on to the next matrix possibility
					if( m!=pFaceMat->NumVerts )
						continue;

					//---	get the next matrix to remove
					iMatrix = pFaceMat->iMatrix;

					//---	for each vertex which uses this matrix, find and remove it
					for( m=0; m<pFaceMat->NumVerts; m++ )
					{
						pVert = pFaceMat->pVertexList[m];
						s32 NumWeights = pVert->m_nWeights;

						//---	remove the matrix from all vertices at this position
						RemoveWeightedMatrixFromVertex( m_pSkin, pVert, iMatrix );
					}

					//---	we have removed one weight from the face
					FaceMatricesCount--;
				}

				//---	it is possible that the number of matricies for the face has been reduced to a value heigher than
				//		the requested MaxMats if the value of MaxMats is 3, 2 or 1 (less than the number of vertices in a face)
				//		if this is the case, assign vertices from face to face.

				//---	if there are too many matrix weights, make the first matrix and the second matrix share a vertex
				if( FaceMatricesCount > MaxMats )
				{
					pChunk->m_pVertex[pFace->m_Index[1]].m_Weight[0].m_iMatrix =
						pChunk->m_pVertex[pFace->m_Index[0]].m_Weight[0].m_iMatrix;
					FaceMatricesCount--;
					bChangedVertMatrix = TRUE;
				}

				//---	if there are still too many matrix weights, make the first matrix and the third matrix share a vertex
				if( FaceMatricesCount > MaxMats )
				{
					pChunk->m_pVertex[pFace->m_Index[2]].m_Weight[0].m_iMatrix =
						pChunk->m_pVertex[pFace->m_Index[0]].m_Weight[0].m_iMatrix;
					FaceMatricesCount--;
				}

				//---	if there are still too many matrix weights, make the first matrix and the fourth matrix share a vertex
				if( FaceMatricesCount > MaxMats )
				{
					ASSERT(pFace->m_bQuad);
					pChunk->m_pVertex[pFace->m_Index[3]].m_Weight[0].m_iMatrix =
						pChunk->m_pVertex[pFace->m_Index[0]].m_Weight[0].m_iMatrix;
					FaceMatricesCount--;
				}

				ASSERT(FaceMatricesCount <= MaxMats);
			}
		}

		//---	reducing the number of matrices that a vertex weighs itself to doesn't create the chance that other
		//		faces will increase again to more weighting than they are allowed, BUT there is a possability
		//		that a vertex had to have its last matrix changed to another one.  If this is the case,
		//		repeat the check on the last chunk (until it doesn't happen any more).  Do not repeat more than
		//		a few times to prevent infinite loops (though I don't think that any can be had anyway).
		if( bChangedVertMatrix )
		{
			if( RepeatCount++ < 10 )
				i--;
			else
				RepeatCount = 0;
		}
	}

//CheckWeights( m_pSkin );
/*
	//
	//===	vertices which share a location must weight to the same matrices to prevent tearing
	//
	s32 c1, c2;
	s32 v1, v2;
	mesh::chunk*	pChunk1;
	mesh::chunk*	pChunk2;
	mesh::vertex*	pVert1;
	mesh::vertex*	pVert2;
	s32 CommonMats[MAX_FACE_MATRICES];
	s32	CommonMatsCount;
	f32	RemovedWeight;
	f32	TotalRemainingWeight;
	f32	NewTotalWeight;

	//---	for all vertices in the skin
	for( c1=0; c1<m_pSkin->m_nChunks; c1++ )
	{
		pChunk1 = &m_pSkin->m_pChunk[c1];
		for( v1=0; v1<pChunk1->m_nVerts; v1++ )
		{
			//---	loop through all of the remaining vertices in the skin
			for( c2=c1; c2<m_pSkin->m_nChunks; c2++ )
			{
				pChunk2 = &m_pSkin->m_pChunk[c2];
				for( v2=v1+1; v2<pChunk2->m_nVerts; v2++ )
				{
					pVert1 = &pChunk1->m_pVertex[v1];
					pVert2 = &pChunk2->m_pVertex[v2];

					//---	if the vertices are at the same location, make sure that they have the same weighting.
					//		The weighting must follow a LCD (least common denominator) aproach.  We cannot add any
					//		new matrices to one vertex to make it like another, we must find the like vertices between
					//		them and keep those.
					if( pVert1->m_vPos == pVert2->m_vPos )
					{
						//
						//---	for each matrix of the first vert, check the matrices of the second vert.  If a match is found
						//		add it to our common list of matrices.
						//
						CommonMatsCount = 0;
						for( i=0; i<pVert1->m_nWeights; i++ )
						{
							for( j=0; j<pVert2->m_nWeights; j++ )
							{
								if( pVert1->m_Weight[i].m_iMatrix == pVert2->m_Weight[j].m_iMatrix )
									CommonMats[CommonMatsCount++] = pVert1->m_Weight[i].m_iMatrix;
							}
						}

						//---	if there are no commong matrices, there is nothing to do except let the tearing happen.
						if( CommonMatsCount )
						{
							//
							//---	go back through the first vertex and remove any weights which are for mats not in the common list.
							//
							for( i=0; i<pVert1->m_nWeights; i++ )
							{
								for( j=0; j<CommonMatsCount; j++ )
								{
									if( pVert1->m_Weight[i].m_iMatrix == CommonMats[j] )
										break;
								}

								//---	remove this matrix weight if it wasn't found
								if( j==CommonMatsCount )
								{
									RemovedWeight			= pVert1->m_Weight[i].m_Weight;
									TotalRemainingWeight	= 1.0f - RemovedWeight;
									NewTotalWeight			= 0.0f;

									//---	flag that something has been modified
									bModified = TRUE;

									//---	remove the matrix
									pVert1->m_nWeights--;
									for( j=i; j<pVert1->m_nWeights; j++ )
										pVert1->m_Weight[j] = pVert1->m_Weight[j+1];

									//---	redistribute the weight into the other matrices
									for( j=0; j<pVert1->m_nWeights; j++ )
									{
										pVert1->m_Weight[j].m_Weight += RemovedWeight * (pVert1->m_Weight[j].m_Weight / TotalRemainingWeight);
										NewTotalWeight += pVert1->m_Weight[j].m_Weight;
									}
									pVert1->m_Weight[pVert1->m_nWeights-1].m_Weight += 1.0f - NewTotalWeight;

									//---	we removed one of the weights, we must compensate in our outer loop
									i--;
								}
							}

//	f32 totalweight = 0.0f;
//	for( i=0; i<pVert1->m_nWeights; i++ )
//		totalweight += pVert1->m_Weight[i].m_Weight;
//	ASSERT(ABS(totalweight-1.0f) < 0.001f);


							//
							//---	go back through the second vertex and remove any weights which are for mats not in the common list.
							//
							for( i=0; i<pVert2->m_nWeights; i++ )
							{
								for( j=0; j<CommonMatsCount; j++ )
								{
									if( pVert2->m_Weight[i].m_iMatrix == CommonMats[j] )
										break;
								}

								//---	remove this matrix weight if it wasn't found
								if( j==CommonMatsCount )
								{
									RemovedWeight			= pVert2->m_Weight[i].m_Weight;
									TotalRemainingWeight	= 1.0f - RemovedWeight;
									NewTotalWeight			= 0.0f;

									//---	flag that something has been modified
									bModified = TRUE;

									//---	remove the matrix
									pVert2->m_nWeights--;
									for( j=i; j<pVert2->m_nWeights; j++ )
										pVert2->m_Weight[j] = pVert2->m_Weight[j+1];

									//---	redistribute the weight into the other matrices
									for( j=0; j<pVert2->m_nWeights; j++ )
									{
										pVert2->m_Weight[j].m_Weight += RemovedWeight * (pVert2->m_Weight[j].m_Weight / TotalRemainingWeight);
										NewTotalWeight += pVert2->m_Weight[j].m_Weight;
									}
									pVert2->m_Weight[pVert2->m_nWeights-1].m_Weight += 1.0f - NewTotalWeight;

									//---	we removed one of the weights, we must compensate in our outer loop
									i--;
								}
							}
//	totalweight = 0.0f;
//	for( i=0; i<pVert2->m_nWeights; i++ )
//		totalweight += pVert2->m_Weight[i].m_Weight;
//	ASSERT(ABS(totalweight-1.0f) < 0.001f);

						}
					}
				}
			}
		}
	}

//CheckWeights( m_pSkin );
*/
	return bModified;
}

//==========================================================================
xbool CSkin::LimitMatricesPerChunk( s32 MaxMats, s32& MaxFaceWeights, s32& MaxVertexWeights )
{
	s32				FacesToProcessCount;
	s32*			FacesToProcessList = NULL;
	s32				FacesProcessedCount;
	s32*			FacesProcessedList = NULL;
	s32				i, j, k, l;
	mesh::chunk*	pChunk;
	mesh::vertex*	pVert;
	mesh::face*		pFace;
	mesh::chunk*	NewChunks[MAX_NUM_CHUNKS];
	s32				NewChunksCount=0;
	s32				NewVertexList[MAX_NUM_VERTS];
	s32				NewVertexCount;
	mesh::chunk*	pNewChunk;
	s32				iFace, iVert;
	s32				WeightMatricesCount;
	s32				WeightMatrices[MAX_FACE_MATRICES];
	s32				NumChunkDivisions;
	xbool			bModified	= FALSE;
	xbool			bError		= FALSE;

//#define	COLOR_CHUNKS  // great for debugging these things
#ifdef COLOR_CHUNKS
	f32				RValue	= 1.0f;
	f32				GValue	= 1.0f;
	f32				BValue	= 1.0f;
#endif

	//---	cap the MaxMats field to our actual limitation
	if( MaxMats > MAX_FACE_MATRICES )
		MaxMats = MAX_FACE_MATRICES;

	//---	the process.  Here is a break down of the steps
	//
	//		step 1: remove nasty quads so that we don't artificially remove face verts when the faces will have to be broken up anyway.
	//		step 2:	Look for any faces which have too many among its verts.  Do this for the entire skin.
	//		step 3: Make sure that all verts which are at the same position are weighted the same to prevent tearing.
	//		step 4: for each chunk, break it into chunks of faces which all share the same group of matrix weights
	//		step 4a: Search for the face with the largest number of weights in its vertices.  Select that set of weights to process
	//		step 4b: Collect all faces which weight to matrices in the current list.
	//		step 4c: Build the new chunk out of the selected faces duplicating any required vertices.
	//		step 4d: If there ara any unprocessed faces, go back to step 3a
	//		step 4e: combine any combineable sets generated
	//

	try
	{
		//
		//===	STEP 1: Convert Quads to Triangles
		//
		for( i=0; i<m_pSkin->m_nChunks; i++ )
		{
			pChunk	= &m_pSkin->m_pChunk[i];

			mesh::face	*pNewFace;
			mesh::face	*NewFaceList = new mesh::face[pChunk->m_nFaces]; // could potentially double the number of faces, get that many extra
			s32			NewFaceListCount = 0;
			if( !NewFaceList )
				throw (s32)0;

			for( j=0; j<pChunk->m_nFaces; j++ )
			{
				pFace = &pChunk->m_pFace[j];

				if( pFace->m_bQuad )
				{
					pFace->m_bQuad = FALSE;
					NewFaceList[NewFaceListCount++] = *pFace;

					pNewFace = &NewFaceList[NewFaceListCount++];
					pNewFace->m_bQuad		= FALSE;
					pNewFace->m_iMaterial	= pFace->m_iMaterial;
					pNewFace->m_Index[0]	= pFace->m_Index[0];
					pNewFace->m_Index[1]	= pFace->m_Index[2];
					pNewFace->m_Index[2]	= pFace->m_Index[3];
				}
			}

			//---	if new faces were added, create the joined list and use it
			if( NewFaceListCount > 0 )
			{
				mesh::face	*JoinedFaceList = new mesh::face[pChunk->m_nFaces + NewFaceListCount];
				if( !JoinedFaceList )
					throw (s32)0;

				x_memcpy( JoinedFaceList, pChunk->m_pFace, pChunk->m_nFaces*sizeof(mesh::face) );
				x_memcpy( &JoinedFaceList[pChunk->m_nFaces], NewFaceList, NewFaceListCount*sizeof(mesh::face) );
				delete[] pChunk->m_pFace;
				pChunk->m_pFace = JoinedFaceList;
				pChunk->m_nFaces = pChunk->m_nFaces + NewFaceListCount;
			}

			delete[] NewFaceList;
		}

		//
		//===	STEP 2: Look for any faces which have too many weights among its verts.  If so, reduce the number of weights used by the
		//				verticies of the face.  First remove matricies used by fiewer verts, tie breaker is the matrix with the lowest total weight.
		//===	STEP 3: make sure all vertices at the same position are weighted the same to prevent tearing.
		//
		bModified = LimitMatricesPerFace( MaxMats, MaxFaceWeights, MaxVertexWeights );

		//---	loop through each chunk
		for( i=0; i<m_pSkin->m_nChunks; i++ )
		{
			//---	alias the chunk
			pChunk	= &m_pSkin->m_pChunk[i];
			NumChunkDivisions = 0;

			//---	get storage space for the faces lists
			FacesToProcessList = new s32[pChunk->m_nFaces];
			FacesProcessedList = new s32[pChunk->m_nFaces];
			if( !FacesToProcessList || !FacesProcessedList )
				throw (s32)0;

			//---	setup the FacesToProcessList
			FacesToProcessCount = pChunk->m_nFaces;
			for( j=0; j<FacesToProcessCount; j++ )
				FacesToProcessList[j] = j;

			//---	process the faces until they have all been processed
			while( FacesToProcessCount )
			{
				//---	setup the FacesProcessedList
				FacesProcessedCount	= 0;

				//
				//===	STEP 4a: Search for the face with the largest number of weights in its vertices.  Select that set of weights to process
				//
				s32 FaceMatrices[MAX_FACE_MATRICES];
				s32 FaceMatricesCount;
				WeightMatricesCount	= 0;
				for( j=0; j<FacesToProcessCount; j++ )
				{
					iFace = FacesToProcessList[j];
					pFace = &pChunk->m_pFace[iFace];

					//---	build a list of matrices for this face
					FaceMatricesCount = 0;
					for( k=pFace->m_bQuad?3:2; k>=0; k-- )
					{
						iVert = pFace->m_Index[k];
						pVert = &pChunk->m_pVertex[iVert];

						//---	add this vert's matrices to the faces list
						for( l=0; l<pVert->m_nWeights; l++ )
							CheckAddToList( pVert->m_Weight[l].m_iMatrix, FaceMatrices, FaceMatricesCount, MAX_FACE_MATRICES );
					}

					//---	if this face had more than the last one, sets its matrices as the ones to look out for
					if( FaceMatricesCount > WeightMatricesCount )
					{
						//---	add the matrix indices to the list.
						WeightMatricesCount	= FaceMatricesCount;
						x_memcpy( WeightMatrices, FaceMatrices, FaceMatricesCount*sizeof(s32) );
					}
				}

				//
				//===	STEP 4b: Collect all faces whos weights are a sub-set of the current weight set.
				//
				xbool	bVertWeightsInList;
				for( j=0; j<FacesToProcessCount; j++ )
				{
					iFace = FacesToProcessList[j];
					pFace = &pChunk->m_pFace[iFace];
					for( k=pFace->m_bQuad?3:2; k>=0; k-- )
					{
						iVert = pFace->m_Index[k];
						pVert = &pChunk->m_pVertex[iVert];

						//---	look through the weights of this face, if any of them aren't contained within our list it won't work.
						//		break the loop.
						bVertWeightsInList = TRUE;
						for( l=0; l<pVert->m_nWeights; l++ )
						{
							if( !ValueIsInList( pVert->m_Weight[l].m_iMatrix, WeightMatrices, WeightMatricesCount ) )
							{
								bVertWeightsInList = FALSE;
								break;
							}
						}

						//---	if all the search weight matrices were found, break the loop
						if( !bVertWeightsInList )
							break;
					}

					//---	if all vertices of the face contain weights which are in our current matrix array,
					//		add this face to our processed faces.
					if( k==-1 )
					{
						FacesProcessedList[FacesProcessedCount++] = iFace;
						DeleteIndexFromFacesToProcessList( j--, FacesToProcessList, FacesToProcessCount-- );
					}
				}

				//
				//===	STEP 4c: Build the new chunk out of the selected faces duplicating any required vertices.
				//

				//---	get storage for the new chunk
				pNewChunk = new mesh::chunk;
				if( !pNewChunk )
					throw (s32)0;
				
				//---	build the list of used vertices (remap the faces to the new vertex list)
				//		because these faces will not be referenced again after being processed, we
				//		can change the data in the original skin.  This will save time and space.
				NewVertexCount = 0;
				for( j=0; j<FacesProcessedCount; j++ )
				{
					iFace = FacesProcessedList[j];
					pFace = &pChunk->m_pFace[iFace];
					for( k=pFace->m_bQuad?3:2; k>=0; k-- )
					{
						iVert = pFace->m_Index[k];

						//---	find this vertex in the list of current vertices
						l = FindAddToList( iVert, NewVertexList, NewVertexCount, MAX_NUM_VERTS );
						ASSERT((l>=0)&&(l<NewVertexCount));

						//---	remap this face vertex to the new list
						pFace->m_Index[k] = l;
					}
				}

				//---	build the chunk data
				ASSERT(FacesProcessedCount);
				x_sprintf( pNewChunk->m_Name, "%32s(%d)", pChunk->m_Name, NumChunkDivisions );
				pNewChunk->m_nFaces		= FacesProcessedCount;
				pNewChunk->m_pFace		= new mesh::face[FacesProcessedCount];
				pNewChunk->m_nVerts		= NewVertexCount;
				pNewChunk->m_pVertex	= new mesh::vertex[NewVertexCount];
				if( !pNewChunk->m_pFace || !pNewChunk->m_pVertex )
					throw (s32)0;

				static s=0;
				//---	copy over vertex data
				for( j=0; j<NewVertexCount; j++ )
				{
					pVert = &pNewChunk->m_pVertex[j];

					*pVert = pChunk->m_pVertex[NewVertexList[j]];

					f32 tw= 0.0f;
					for( k=0; k<pVert->m_nWeights; k++ )
						tw += pVert->m_Weight[k].m_Weight;
					ASSERT(ABS(tw-1.0f)<0.001f);

					//---	build a new set of weights for this vertex
					mesh::weight NewWeights[MAX_FACE_MATRICES];
					for( k=0; k<WeightMatricesCount; k++ )
					{
						NewWeights[k].m_iMatrix = WeightMatrices[k];
						NewWeights[k].m_Weight	= 0.0f;
					}

					//---	remap this vertices weighted matrices into the new list
					xbool found;
					for( k=0; k<pVert->m_nWeights; k++ )
					{
						found = FALSE;
						for( l=0; l<WeightMatricesCount; l++ )
							if( pVert->m_Weight[k].m_iMatrix == WeightMatrices[l] )
							{
								NewWeights[l].m_Weight = pVert->m_Weight[k].m_Weight;
								found = TRUE;
								break;
							}
						ASSERT(found);
					}

					f32 TotalWeight = 0.0f;
					//---	set the new weighting into the vertex
					pVert->m_nWeights = WeightMatricesCount;
					for( k=0; k<WeightMatricesCount; k++ )
					{
						pVert->m_Weight[k] = NewWeights[k];
						TotalWeight += NewWeights[k].m_Weight;
					}
					ASSERT(ABS(TotalWeight-1.0f)<0.001f);


#ifdef COLOR_CHUNKS // Color's the chunks, good for debugging
					pVert->m_Color.X = (s&1) ? RValue : 0.0f;
					pVert->m_Color.Y = (s&2) ? GValue : 0.0f;
					pVert->m_Color.Z = (s&4) ? BValue : 0.0f;
#else
					pVert->m_Color.X = 1.0f;
					pVert->m_Color.Y = 1.0f;
					pVert->m_Color.Z = 1.0f;
#endif
				}
				s++;

				//---	copy over face data (note: faces have already been remapped to the new vertex array)
				for( j=0; j<FacesProcessedCount; j++ )
					pNewChunk->m_pFace[j] = pChunk->m_pFace[FacesProcessedList[j]];

				//---	store this chunk in our list of chunks
				NewChunks[NewChunksCount++] = pNewChunk;
				NumChunkDivisions++;

				//---	if we have reached the maximum number of chunks, stop looking for more
				if( NewChunksCount == MAX_NUM_CHUNKS )
					break;
			}

			//
			//===	Search for any chunks which can be combined and combine them
			//
//	use the NumChunkDivisions variable to determine how many chunks the last chunk was broken into.
//	only combine chunks which came from the same chunk in case their materials require them to be separate already.

			delete[] FacesToProcessList;
			delete[] FacesProcessedList;
			FacesToProcessList = NULL;
			FacesProcessedList = NULL;


			//---	if we have reached the maximum number of chunks, stop looking for more
			if( NewChunksCount == MAX_NUM_CHUNKS )
				break;
		}
	}

	//
	//===	catch any exceptions from above
	//
	catch( s32 )
	{
		MessageBox( NULL, "Out of Memory trying to convert skin", "CSkin", MB_ICONERROR );
		bError = TRUE;
	}

	//---	if we have had success
	if( !bError )
	{
		//
		//===	replace the skin's chunks
		//
		pChunk = m_pSkin->m_pChunk;
		m_pSkin->m_pChunk = new mesh::chunk[NewChunksCount];
		if( !m_pSkin->m_pChunk )
			m_pSkin->m_pChunk = pChunk;
		else
		{
			delete[] pChunk;
			m_pSkin->m_nChunks = NewChunksCount;
			for( i=0; i<NewChunksCount; i++ )
			{
				m_pSkin->m_pChunk[i] = *NewChunks[i];
			}
		}
	}

	//
	//===	cleanup
	//
	if( FacesToProcessList ) x_free( FacesToProcessList );
	if( FacesProcessedList ) x_free( FacesProcessedList );
	for( i=0; i<NewChunksCount; i++ )
	{
		delete NewChunks[i];
	}

	return bModified;
}
