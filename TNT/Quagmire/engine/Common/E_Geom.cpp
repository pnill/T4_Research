#include "x_memory.hpp"

#include "Q_Geom.hpp"
#include "Q_VRAM.hpp"

////////////////////////////////////////////////////////////////////////////
// Statics
////////////////////////////////////////////////////////////////////////////

s32* QGeom::s_pStatNVerts = NULL;
s32* QGeom::s_pStatNTris = NULL;
s32* QGeom::s_pStatNBytes = NULL;

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

//==========================================================================
// Texture functions
//==========================================================================

s32 QGeom::GetNTextures( void )
{
    return m_NTextures;
}

//==========================================================================

s32 QGeom::GetTextureIndex( const char* TextureName )
{
    s32 i;

    for ( i = 0; i < m_NTextures; i++ )
    {
        if ( !x_strcmp( TextureName,
                        &m_pTextureNames[i * TEXTURE_NAME_LENGTH] ) )
        {
            return i;
        }
    }
    return -1;
}

//==========================================================================

const char* QGeom::GetTextureName( s32 TextureIndex )
{
    ASSERT( (TextureIndex >= 0) && (TextureIndex < m_NTextures) );
    return &m_pTextureNames[TextureIndex * TEXTURE_NAME_LENGTH];
}

//==========================================================================

void QGeom::SetTextureArray( x_bitmap* TextureArray )
{
    KillOwnedTextures();

    m_pTextures = TextureArray;
    m_pTexturePtrs = NULL;
}

//==========================================================================

x_bitmap* QGeom::GetTextureArray( void )
{
    return m_pTextures;
}

//==========================================================================

void QGeom::SetTexturePtrArray( x_bitmap** TexturePtrArray )
{
    KillOwnedTextures();

    m_pTextures = NULL;
    m_pTexturePtrs = TexturePtrArray;
}

//==========================================================================

x_bitmap** QGeom::GetTexturePtrArray( void )
{
    return m_pTexturePtrs;
}

//==========================================================================

void QGeom::KillOwnedTextures( void )
{
    s32 i;
    if( m_Flags & GEOM_FLAG_OWNS_TEXTURES )
    {
        if( m_pTextures != NULL )
        {
            for( i = 0; i < m_NTexturesOwned; i++ )
            {
                VRAM_UnRegister( m_pTextures[i] );
            }
            delete[] m_pTextures;
            m_pTextures = NULL;
        }

        m_NTexturesOwned = 0;

        m_Flags &= ~GEOM_FLAG_OWNS_TEXTURES;
    }
}

//==========================================================================

static
radian V3_AngleBetween ( vector3* Va, vector3* Vb )
{
    f32 D;
    f32 Dot;

    ASSERT(Va);
    ASSERT(Vb);

    D = Va->Length() * Vb->Length();

    if ( D == 0.0f )
        return R_0;

    Dot = Va->Dot( *Vb ) / D;

    if ( Dot > 1.0f )       Dot = 1.0f;
    else if ( Dot < -1.0f ) Dot = -1.0f;

    return x_acos( Dot );
}

//==========================================================================

matrix4 QGeom::CalcShadowMatrix( vector3 LightDir, vector4 Plane )
{
    matrix4 Result;
    
    matrix4 LightSquashMatrix;
    matrix4 Rot;
    radian  AngleBetweenNormalAndYUp;
    vector3 YUp = vector3( 0.0f, 1.0f, 0.0f );
    vector3 Normal;
    vector3 Axis = vector3( 1.0f, 0.0f, 0.0f );
    vector3 T;
    vector3 LocalLightDir;

    //---   Check if Plane is a backface to the light
    if ( (LightDir.X * Plane.X) +
         (LightDir.Y * Plane.Y) +
         (LightDir.Z * Plane.Z) > -0.0001f )
    {
        ASSERT( FALSE );
        Result.Identity();
        return Result;
    }

    //---   Get rotation and axis to align plane in Y=0 plane
    Normal.X = Plane.X;
    Normal.Y = Plane.Y;
    Normal.Z = Plane.Z;
    AngleBetweenNormalAndYUp = V3_AngleBetween( &Normal, &YUp );
    if ( ABS(AngleBetweenNormalAndYUp) > 0.001f )
    {
        Axis = Normal.Cross( YUp );
        Axis.Normalize();
    }

    //---   Tranform world to align plane with Y=0
    Rot.SetupAxisRotate( AngleBetweenNormalAndYUp, Axis );
    Result = Rot;
    T.X = 0.0f;
    T.Y = -Plane.W;
    T.Z = 0.0f;
    Result.Translate( T );

    //---   Compute transformed light dir and squash
    LocalLightDir = Rot.Transform( LightDir );
    LightSquashMatrix.Identity();
    LightSquashMatrix.M[1][0] = -LocalLightDir.X / LocalLightDir.Y; // X Trans per Y
    LightSquashMatrix.M[1][1] = 0.0f;                               // Y Trans per Y
    LightSquashMatrix.M[1][2] = -LocalLightDir.Z / LocalLightDir.Y; // Z Trans per Y
    Result = LightSquashMatrix * Result;

    //---   Transform world back to normal orientation
    T.Y = -T.Y;
    Result.Translate( T );
    Rot.SetupAxisRotate( -AngleBetweenNormalAndYUp, Axis );
    Result = Rot * Result;
    return Result;
}

//==========================================================================


//==========================================================================
// Stat-tracking functions
//==========================================================================

void QGeom::SetStatTrackers( s32* pNVerts, s32* pNTris, s32* pNBytes )
{
    s_pStatNVerts = pNVerts;
    s_pStatNTris = pNTris;
    s_pStatNBytes = pNBytes;
}
