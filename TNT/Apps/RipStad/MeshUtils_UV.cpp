////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////
#define MAX_UV 16.0f
#define MIN_UV -16.0f
#define EXTENT_UV ((MAX_UV)-(MIN_UV))

#define UV_MAX 8.0f                         //-- Max UV val
#define UV_MIN -4.0f                        //-- Max UV val
#define UV_EXT ((UV_MAX)-(UV_MIN))          //-- Ext that tiggers translation
#define UV_HARD_LIMIT 12.0f                 //-- Ext that tiggers drastic measures

////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////

#include "x_plus.hpp"
#include "MeshUtils_UV.h"
#include "AsciiMesh.h"

xbool IsUVsNormalized( mesh::chunk& Chunk )
{
    vector2 Min( MAX_UV, MAX_UV );
    vector2 Max( MIN_UV, MIN_UV );

    for ( s32 iVert = 0; iVert < Chunk.m_nVerts; ++iVert )
    {
        mesh::vertex& Vert( Chunk.m_pVerts[iVert] );

        for( s32 iUV = 0; iUV < Vert.m_nStages; ++iUV )
        {
            Min.X = MIN( Min.X, Vert.m_UVs[iUV].X );
            Min.Y = MIN( Min.Y, Vert.m_UVs[iUV].Y );
            Max.X = MAX( Max.X, Vert.m_UVs[iUV].X );
            Max.Y = MAX( Max.Y, Vert.m_UVs[iUV].Y );
        }
    }

    if ( Min.X < MIN_UV || Min.Y < MIN_UV ) return FALSE;
    if ( Max.X > MAX_UV || Max.Y > MAX_UV ) return FALSE;
    return TRUE;
}

void AdjustUV( mesh::chunk& Chunk, xbool* pbTag, s32 iVert, f32 TransX, f32 TransY )
{
    mesh::vertex& Vert( Chunk.m_pVerts[iVert] );
    xbool& bTagged( pbTag[iVert] );
    if ( bTagged ) return;

    //-- Affect vert's uvs
    for( s32 iUV = 0; iUV < Vert.m_nStages; ++iUV )
    {
        Vert.m_UVs[iUV].X += TransX;
        Vert.m_UVs[iUV].Y += TransY;
        bTagged = TRUE;
    }

    //-- Find faces that use affected vert and affect the face's other verts also
    for( s32 iFace = 0; iFace < Chunk.m_nFaces; ++iFace )
    {
        bool bFoundVertInFace = false;
        mesh::face& Face( Chunk.m_pFaces[iFace] );
        s32 NVerts = (Face.m_bQuad) ? 4 : 3;
        for ( s32 iV = 0; iV < NVerts; ++iV )
        {
            if ( Face.m_Index[iV] == iVert ) 
            {
                bFoundVertInFace = TRUE;
                break;
            }
        }

        //-- Vert is found in face affect other verts
        if ( bFoundVertInFace )
        {
            s32 NVerts = (Face.m_bQuad) ? 4 : 3;
            for ( s32 iV2 = 0; iV2 < NVerts; ++iV2 )
            {
                AdjustUV( Chunk, pbTag, Face.m_Index[iV2], TransX, TransY );
            }
        }
    }
}

void NormalizeUVs( mesh::chunk& Chunk )
{
    
    //-- If uvs are ok, we are done
    if ( IsUVsNormalized(Chunk) ) return;

    //-- normalize each face
    for( s32 iFace = 0; iFace < Chunk.m_nFaces; ++iFace )
    {
        mesh::face& Face( Chunk.m_pFaces[iFace] );

        //-- find the uv range of the face
        s32 NVerts = (Face.m_bQuad) ? 4 : 3;
        for ( s32 iVert = 0; iVert < NVerts; ++iVert )
        {
            vector2 Min( MAX_UV, MAX_UV );
            vector2 Max( MIN_UV, MIN_UV );
            mesh::vertex& Vert( Chunk.m_pVerts[ Face.m_Index[iVert] ] );

            for( s32 iUV = 0; iUV < Vert.m_nStages; ++iUV )
            {
                Min.X = MIN( Min.X, Vert.m_UVs[iUV].X );
                Min.Y = MIN( Min.Y, Vert.m_UVs[iUV].Y );
                Max.X = MAX( Max.X, Vert.m_UVs[iUV].X );
                Max.Y = MAX( Max.Y, Vert.m_UVs[iUV].Y );
            }

            if ( Min.X < MIN_UV || Min.Y < MIN_UV || Max.X > MAX_UV || Max.Y > MAX_UV )
            {
                f32 ExtentX = Max.X - Min.X;
                f32 ExtentY = Max.Y - Min.Y;
                f32 TransX = MIN_UV-Min.X;
                f32 TransY = MIN_UV-Min.Y;

                //-- Allocate a buffer to store info on verts
                xbool* pbTag = new xbool[Chunk.m_nVerts];
                x_memset( pbTag, 0, sizeof( xbool )*Chunk.m_nVerts ); 
                ASSERT( pbTag );

                AdjustUV( Chunk, pbTag, iVert, TransX, TransY );

                delete []pbTag;

            }
        }
    }
}

xbool IsUVsNormalized( vector2* pUV, s32 nUVs )
{
    f32 MinU = UV_MAX;
    f32 MinV = UV_MAX;
    f32 MaxU = UV_MIN;
    f32 MaxV = UV_MIN;

    for ( s32 j = 0; j < nUVs; j++ )
    {
        MinU = MIN( pUV[j].X, MinU );
        MinV = MIN( pUV[j].Y, MinV );
        MaxU = MAX( pUV[j].X, MaxU );
        MaxV = MAX( pUV[j].Y, MaxV );
    }

    if ( MinU < UV_MIN || MaxU > UV_MAX ) return FALSE;
    if ( MinV < UV_MIN || MaxV > UV_MAX ) return FALSE;
    return TRUE;

}
void AdjustUV( vector2* pUV, s32 nUVs )
{
    s32 j;
    f32 MinU = UV_MAX;
    f32 MinV = UV_MAX;
    f32 MaxU = UV_MIN;
    f32 MaxV = UV_MIN;

    for ( j = 0; j < nUVs; j++ )
    {
        MinU = MIN( pUV[j].X, MinU );
        MinV = MIN( pUV[j].Y, MinV );
        MaxU = MAX( pUV[j].X, MaxU );
        MaxV = MAX( pUV[j].Y, MaxV );
    }

    f32 ExtU = MaxU-MinU;
    f32 ExtV = MaxV-MinV;

    f32 BaseU = UV_MIN;
    f32 BaseV = UV_MIN;

    if ( ExtU > UV_EXT ) 
    {
        BaseU -= (f32)((s32)(ExtU-UV_EXT)+1);
    }
    if ( ExtV > UV_EXT ) 
    {
        BaseV -= (f32)((s32)(ExtV-UV_EXT)+1);
    }

    f32 TransX = 0.0f;
    if ( MinU < UV_MIN || MaxU > UV_MAX ) TransX = BaseU-MinU;
    f32 TransY = 0.0f;
    if ( MinV < UV_MIN || MaxV > UV_MAX ) TransY = BaseV-MinV;
    
    
    f32 LastU = 0.0f;
    f32 LastV = 0.0f;

    for ( j = 0; j < nUVs; j++ )
    {
        pUV[j] += vector2( TransX, TransY );

        if ( pUV[j].X < -UV_HARD_LIMIT || pUV[j].X > UV_HARD_LIMIT )
        {
            pUV[j].X = ((2*LastU) - pUV[j].X);
        }
        if ( pUV[j].Y < -UV_HARD_LIMIT || pUV[j].Y > UV_HARD_LIMIT )
        {
            pUV[j].Y = ((2*LastV) - pUV[j].Y);
        }
        LastU = pUV[j].X;
        LastV = pUV[j].Y;
    }
}

