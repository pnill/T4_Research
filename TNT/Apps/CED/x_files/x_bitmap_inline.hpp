////////////////////////////////////////////////////////////////////////////
//
//  X_BITMAP_INLINE.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_BITMAP_INLINE_HPP
#define X_BITMAP_INLINE_HPP

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

typedef struct
{
    s32     Offset;
    s16     W,H;
}miptable;

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

//=========================================================================

inline x_bitmap::x_bitmap()
{
    Reset();
}


//=========================================================================

inline void x_bitmap::SetupBitmap( format    Format,
                                   s32       Width,
                                   s32       Height,
                                   xbool     IsDataOwned,
                                   void*     pPixelData,
                                   s32       NMipsToBuild)
{
    SetupBitmap( Format,
                 Width,
                 Height,
                 IsDataOwned,
                 pPixelData,
                 NULL,
                 0,
                 NMipsToBuild);
}

//=========================================================================

inline void x_bitmap::SetupBitmap( format    Format,
                                   s32       Width,
                                   s32       Height,
                                   xbool     IsDataOwned,
                                   void*     pPixelData,
                                   void*     pClutData,
                                   s32       NClutColors,
                                   s32       NMipsToBuild)
{
    ASSERT( Format > FMT_NULL );
    ASSERT( Format < FMT_END  );

    SetupBitmap( Format,
                 Width,
                 Height,
                 Width,
                 Height,
                 IsDataOwned,
                 ( s_Format[ Format ].BPP * Width * Height ) / 8,
                 pPixelData,
                 IsDataOwned,
                 ( s_Format[ Format ].BPC * NClutColors ) / 8,
                 pClutData,
                 NClutColors,
                 NMipsToBuild);
}

//=========================================================================

inline void x_bitmap::SetupBitmap( format    Format,
                                   s32       Width,
                                   s32       Height,
                                   s32       PWidth,
                                   s32       PHeight,
                                   xbool     IsDataOwned,
                                   s32       DataSizeInBytes,
                                   void*     pPixelData,
                                   xbool     IsClutOwned,
                                   s32       ClutSizeInBytes,
                                   void*     pClutData,
                                   s32       NClutColors,
                                   s32       NMips, 
                                   xbool     GenerateMips )
{
    ASSERT( Format > FMT_NULL );
    ASSERT( Format < FMT_END  );

    SetupBitmap( Format,
                 Width,
                 Height,
                 Width,
                 Height,
                 IsDataOwned,
                 DataSizeInBytes,
                 pPixelData,
                 IsClutOwned,
                 ClutSizeInBytes,
                 pClutData,
                 NClutColors,
                 (GenerateMips) ? NMips : 0);

    m_NMips = NMips;
}

//=========================================================================

inline xbool x_bitmap::IsClutBased( void ) const
{
    return (m_pClut!=NULL)?TRUE:FALSE;
}

//=========================================================================

inline xbool x_bitmap::IsAlpha( void ) const
{
    return s_Format[ m_Format ].AMask != 0;
}

//=========================================================================

inline void x_bitmap::GetFormatDesc( fmt_desc& FormatDesc, format Format )
{
    ASSERT( Format > FMT_NULL );
    ASSERT( Format < FMT_END  );

    FormatDesc = s_Format[ Format ];
}

//=========================================================================

inline u32 x_bitmap::FormatToFormat( u32 SrcColor, format SrcFormat, format DstFormat )
{
    return ColorToFormat( FormatToColor( SrcColor, SrcFormat ), DstFormat );
}

//=========================================================================

inline xbool x_bitmap::IsClutOwned( void ) const
{
    return (m_Flags&FLAG_CLUT_OWNED) == FLAG_CLUT_OWNED;
}

//=========================================================================

inline s32 x_bitmap::IsDataOwned( void ) const
{
    return (m_Flags&FLAG_DATA_OWNED) == FLAG_DATA_OWNED;
}

//=========================================================================

inline x_bitmap::format x_bitmap::GetFormat( void ) const
{
    return (x_bitmap::format)m_Format;
}

//=========================================================================

inline s32 x_bitmap::GetBPP( void ) const
{
    return s_Format[ m_Format ].BPP;        
}

//=========================================================================

inline s32 x_bitmap::GetBPC( void ) const
{
    return s_Format[ m_Format ].BPC;
}

//=========================================================================

inline s32 x_bitmap::GetNClutColors( void ) const
{
    return (m_pClut!=NULL)?m_MaxClutID+1:0;        
}

//=========================================================================

inline s32 x_bitmap::GetWidth( s32 MipID ) const
{
    ASSERT(MipID <= m_NMips);
    if (m_NMips)
    {
        miptable* M = (miptable*)m_pData;
        return M[MipID].W;
    }
    else
        return m_Width;
}

//=========================================================================

inline s32 x_bitmap::GetHeight( s32 MipID ) const
{
    ASSERT(MipID <= m_NMips);
    if (m_NMips)
    {
        miptable* M = (miptable*)m_pData;
        return M[MipID].H;
    }
    else
        return m_Height;
}

//=========================================================================

inline s32 x_bitmap::GetPWidth( s32 MipID ) const
{
    ASSERT(MipID <= m_NMips);

    if (m_NMips)
    {
        miptable* M = (miptable*)m_pData;
        return M[MipID].W;
    }
    else
    {
        return m_PW;
    }
}

//=========================================================================

inline s32 x_bitmap::GetPHeight( s32 MipID ) const
{
    ASSERT(MipID <= m_NMips);
    if (m_NMips)
    {
        miptable* M = (miptable*)m_pData;
        return M[MipID].H;
    }
    else
    {
        return m_PH;
    }
}

//=========================================================================

inline s32 x_bitmap::GetVRAMID( void ) const
{
    return m_VramId;
}

//=========================================================================

inline void x_bitmap::SetVRAMID( s32 ID )
{
    m_VramId = (s16)ID;
}

//=========================================================================

inline s32 x_bitmap::GetDataSize( s32 MipID ) const
{
    ASSERT(MipID <= m_NMips);
    
    if (m_NMips)
    {
        miptable* M = (miptable*)m_pData;
        return (M[MipID].W*M[MipID].H * s_Format[ m_Format ].BPP) / 8;
    }
    else
        return m_DataSize;
} 

//=========================================================================

inline s32 x_bitmap::GetTotalDataSize( void ) const
{
    return m_DataSize;
}
 
//=========================================================================

inline s32 x_bitmap::GetClutSize( void ) const
{
    return m_ClutSize;
}

//=========================================================================

inline byte* x_bitmap::GetDataPtr( s32 MipID ) const
{
    ASSERT(MipID <= m_NMips);
    if (m_NMips)
    {
        miptable* M = (miptable*)m_pData;
        return m_pData + M[MipID].Offset;
    }
    else
        return m_pData;
        
}

//=========================================================================

inline byte* x_bitmap::GetRawDataPtr( void ) const
{
    return m_pData;
}

//=========================================================================

inline byte* x_bitmap::GetClutPtr( s32 ClutID ) const
{
    return m_pClut + ((m_ClutSize * (x_bitmap::s_Format[m_Format].BPC) * ClutID) / 8);
}

//=========================================================================

inline void x_bitmap::SetPixelFormat( x_bitmap& SrcDstBMP, s8 SrcDstClut, s8 nMipID )
{
    SetPixelFormat( SrcDstBMP, SrcDstBMP, SrcDstClut, SrcDstClut, nMipID );
}

//=========================================================================

inline s32 x_bitmap::GetNCluts( void ) const
{
    return m_NCluts;
}

//=========================================================================

inline void x_bitmap::SetCompressed( void )
{
    m_Flags |= FLAG_COMPRESSED_S3TC;
}

inline void x_bitmap::ClearCompressed( void )
{
    m_Flags &= ~FLAG_COMPRESSED_S3TC;
}

inline xbool x_bitmap::IsCompressed( void ) const
{
    return (m_Flags & FLAG_COMPRESSED_S3TC);
}

//=========================================================================

inline void x_bitmap::SetClutSwizzled( void )
{
    m_Flags |= FLAG_CLUT_SWIZZLED;
}

inline void x_bitmap::ClearClutSwizzled( void )
{
    m_Flags &= ~FLAG_CLUT_SWIZZLED;
}

inline xbool x_bitmap::IsClutSwizzled( void ) const
{
    return (m_Flags & FLAG_CLUT_SWIZZLED);
}

//=========================================================================

inline void x_bitmap::SetDataSwizzled( void )
{
    m_Flags |= FLAG_DATA_SWIZZLED;
}

inline void x_bitmap::ClearDataSwizzled( void )
{
    m_Flags &= ~FLAG_DATA_SWIZZLED;
}

inline xbool x_bitmap::IsDataSwizzled( void ) const
{
    return (m_Flags & FLAG_DATA_SWIZZLED);
}

//=========================================================================

inline void x_bitmap::SetReducedIntensity( void )
{
    m_Flags |= FLAG_REDUCED_INTENSITY;
}

inline void x_bitmap::ClearReducedIntensity( void )
{
    m_Flags &= ~FLAG_REDUCED_INTENSITY;
}

inline xbool x_bitmap::IsReducedIntensity( void ) const
{
    return (m_Flags & FLAG_REDUCED_INTENSITY);
}

//=========================================================================

inline s32 x_bitmap::GetNMips( void ) const
{
    return m_NMips;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#endif