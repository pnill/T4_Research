////////////////////////////////////////////////////////////////////////////
//
//  X_BITMAP.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_BITMAP_HPP
#define X_BITMAP_HPP

///////////////////////////////////////////////////////////////////////////////

#ifndef X_COLOR_HPP
#include "x_color.hpp"
#endif
#include "x_stdio.hpp"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class x_bitmap
{
///////////////////////////////////////////////////////////////////////////////
public:


///////////////////////////////////////////////////////////////////////////////
// ENUMS    
///////////////////////////////////////////////////////////////////////////////


    enum format
    {
        FMT_NULL,           //                  0

        FMT_P4_RGB_565,     //                  1
        FMT_P4_RGB_888,    
        FMT_P4_URGB_8888,  
        FMT_P4_RGBU_8888,  
        FMT_P4_ARGB_8888,   //                  5 
        FMT_P4_RGBA_8888,  
        FMT_P4_ABGR_8888,   // NEW
        FMT_P4_UBGR_8888,   // NEW
        FMT_P4_RGBA_5551,  
        FMT_P4_ARGB_1555,   //                  10
        FMT_P4_RGBU_5551,  
        FMT_P4_URGB_1555, 
        FMT_P4_ABGR_1555,   // NEW
        FMT_P4_UBGR_1555,   // NEW              14

        FMT_P8_RGB_565,     //                  15
        FMT_P8_RGB_888,    
        FMT_P8_URGB_8888,  
        FMT_P8_RGBU_8888,  
        FMT_P8_ARGB_8888,  
        FMT_P8_RGBA_8888,   //                  20
        FMT_P8_ABGR_8888,   // NEW
        FMT_P8_UBGR_8888,   // NEW
        FMT_P8_RGBA_5551,  
        FMT_P8_ARGB_1555,  
        FMT_P8_RGBU_5551,   //                  25
        FMT_P8_URGB_1555, 
        FMT_P8_ABGR_1555,   // NEW
        FMT_P8_UBGR_1555,   // NEW              28

        FMT_16_RGB_565,     //                  29
        FMT_16_ARGB_4444,   //                  30
        FMT_16_RGBA_4444,   
        FMT_16_ARGB_1555,  
        FMT_16_RGBA_5551,  
        FMT_16_URGB_1555,  
        FMT_16_RGBU_5551,   //                  35
        FMT_16_ABGR_1555,   // NEW
        FMT_16_UBGR_1555,   // NEW              37

        FMT_24_RGB_888,     //                  38
        FMT_24_ARGB_8565,   //                  39

        FMT_32_RGBU_8888,   //                  40
        FMT_32_URGB_8888,  
        FMT_32_ARGB_8888,  
        FMT_32_RGBA_8888,  
        FMT_32_ABGR_8888,   // NEW
        FMT_32_BGRA_8888,   // NEW              45
    
        FMT_I4,            
        FMT_I8,            

        FMT_END
    };

    ///////////////////////////////////////////////////////////////////////////

    struct fmt_desc
    {
        format   Format;        // Format define
        xbool    IsClutUsed;    // Is a palette used?
        s32      BPP;           // Bits per pixel
        s32      BPC;           // Bits per color
        s32      BU;            // Bits used
        u32      RMask;         // Red   mask
        u32      GMask;         // Green mask
        u32      BMask;         // Blue  mask
        u32      AMask;         // Alpha mask
        s32      RShift;        // Red   shift
        s32      GShift;        // Green shift
        s32      BShift;        // Blue  shift
        s32      AShift;        // Alpha shift
    };

    ///////////////////////////////////////////////////////////////////////////
public:

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
     x_bitmap();                                // Constructor
     x_bitmap( const x_bitmap& Src );           // Copy constructor
    ~x_bitmap();                                // Destructor

    void operator=(const x_bitmap& Src);        // Assignment
    
    ///////////////////////////////////////////////////////////////////////////
    // SETUP FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    void SetupBitmap          ( format    Format,
                                s32       Width,
                                s32       Height,
                                xbool     IsDataOwned,
                                void*     pPixelData,
                                s32       NMipsToBuild = 0);

    void SetupBitmap          ( format    Format,
                                s32       Width,
                                s32       Height,
                                xbool     IsDataOwned,
                                void*     pPixelData,
                                void*     pClutData,
                                s32       NClutColors,
                                s32       NMipsToBuild = 0);

    void SetupBitmap          ( format    Format,
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
                                s32       NMipsToBuild = 0);

    void SetupBitmap          ( format    Format,
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
                                xbool     GenerateMips );

    void BuildMips            ( s32 NMipsToBuild = 100 );

    void SetAllAlpha          ( s32 Alpha );

    ///////////////////////////////////////////////////////////////////////////
    // SAVE AND LOAD FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    xbool Save      ( X_FILE* fp        );
    xbool Save      ( byte*   pStream   );
    xbool Save      ( char*   pFileName );

    xbool Load      ( X_FILE* fp        );
    xbool Load      ( byte*   pStream   );
    xbool Load      ( byte*   pStream, s32& rNBytesRead );
    xbool Load      ( char*   pFileName );

    s32   GetSerializedSize ( void );

    ////////////////////////////////////////////////////////////////////////
    //  COPY FUNCTIONS
    ////////////////////////////////////////////////////////////////////////

    void CopyFrom       ( const x_bitmap& Src );    // Copys source
    void CopyHighFrom   ( const x_bitmap& Src );    // Copys source to hi memory

    ///////////////////////////////////////////////////////////////////////////
    //
    // KILL FUNCTION -     * If the pointers are owned, release them.
    //                     * Cause a reset of the data memebers 
    //
    ///////////////////////////////////////////////////////////////////////////

    void KillBitmap ( void );


    ///////////////////////////////////////////////////////////////////////////
    // GETTERS
    //
    //  Most are self explanatory.  
    //
    //  GetTotalDataSize  - Retrieves the size of the data that m_pData points
    //                      to, regardless of having mips/no mips
    //
    ///////////////////////////////////////////////////////////////////////////

    format      GetFormat           ( void          ) const;
    s32         GetBPP              ( void          ) const; 
    s32         GetBPC              ( void          ) const; 
    s32         GetNClutColors      ( void          ) const; 
    s32         GetNCluts           ( void          ) const;
    s32         GetWidth            ( s32 MipID = 0 ) const; 
    s32         GetHeight           ( s32 MipID = 0 ) const; 
    s32         GetPWidth           ( s32 MipID = 0 ) const; 
    s32         GetPHeight          ( s32 MipID = 0 ) const;
    s32         GetNMips            ( void          ) const;

    s32         GetClutSize         ( void          ) const; 
    s32         GetDataSize         ( s32 MipID = 0 ) const; 
    s32         GetTotalDataSize    ( void          ) const;

    xbool       IsClutBased         ( void          ) const; 
    xbool       IsAlpha             ( void          ) const; 
    xbool       IsClutOwned         ( void          ) const; 
    xbool       IsDataOwned         ( void          ) const; 

    ///////////////////////////////////////////////////////////////////////////
    // USED BY VRAM MANAGER
    ///////////////////////////////////////////////////////////////////////////
    s32         GetVRAMID           ( void ) const;
    void        SetVRAMID           ( s32 ID );

    ///////////////////////////////////////////////////////////////////////////
    // DEBUG FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    xbool       SanityCheck         ( void ) const;

    ///////////////////////////////////////////////////////////////////////////
    // DIRECT ACCESS FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////
    //
    // GetDataPtr    - Retrieves the pixel data pointer.
    // GetClutPtr    - Retrieves a pointer to the specified clut.
    // GetRawDataPtr - Retrieves the m_pData member regardless of mip/no mip
    //
    ///////////////////////////////////////////////////////////////////////////

    byte*       GetDataPtr          ( s32 MipID = 0  ) const;
    byte*       GetRawDataPtr       ( void           ) const;
    byte*       GetClutPtr          ( s32 ClutID = 0 ) const; 

    ///////////////////////////////////////////////////////////////////////////
    // MAGIC PUT AND GET PIXEL
    ///////////////////////////////////////////////////////////////////////////
    //
    // Using the Get and Set routines below allows you to read and write color
    // and index data while not worrying about format specifics.  You must 
    // call SetPixelFormat which will setup a number of values allowing the
    // set and get routines to operate as quickly as possible.  If you are only
    // going to read or only going to write then you may pass in NULL for the
    // DstBMP or SrcBMP in SetPixelFormat.  If you do not call SetPixelFormat
    // before the set and get calls the behavior is undefined.
    //
    ///////////////////////////////////////////////////////////////////////////

    static void  SetPixelFormat     ( x_bitmap& DstBMP,      
                                      x_bitmap& SrcBMP, 
                                      s8        DstClutID = 0, 
                                      s8        SrcClutID = 0,
									  s8		nMipID = 0);
    static void  SetPixelFormat     ( x_bitmap& DstBMP, 
                                      s8        DstClutID = 0,
									  s8		nMipID = 0);

    static color GetPixelColor      ( s32 X, s32 Y, s32 nMipID = 0 );
    static s32   GetPixelIndex      ( s32 X, s32 Y, s32 nMipID = 0 );
    static color GetClutColor       ( s32 Index    );
                       
    static void  SetPixelColor      ( s32 X, s32 Y, color C, s32 nMipID = 0 );
    static void  SetPixelIndex      ( s32 X, s32 Y, s32 Index, s32 nMipID = 0 );
    static void  SetClutColor       ( s32 Index, color C      );
                       
    ///////////////////////////////////////////////////////////////////////////
    // FORMAT INFO
    ///////////////////////////////////////////////////////////////////////////
    //
    // GetFormatDesc will fill out the FormatInfo structure with the correct
    // information for that particular FormatID.  FindFormat will search 
    // through it's internal tables and look for the format that matches the
    // description in FormatInfo.  The search only involves comparing IsClutUsed,
    // BPP, and the four masks.  If the format was not found FindFormat will
    // return -1.
    //
    ///////////////////////////////////////////////////////////////////////////

    static void    GetFormatDesc    ( fmt_desc& FormatDesc, format Format );
    static format  FindFormat       ( fmt_desc& FormatDesc                );

    ///////////////////////////////////////////////////////////////////////////
    // FORMAT CONVERSIONS
    ///////////////////////////////////////////////////////////////////////////
    //
    // These three routines are used strictly to convert between color formats.
    // To be honest, they are quite slow, so don't use them too heavily.
    //
    ///////////////////////////////////////////////////////////////////////////

    static u32     ColorToFormat    ( color SrcColor, format DstFormat );
    static color   FormatToColor    ( u32   SrcColor, format SrcFormat );
    static u32     FormatToFormat   ( u32   SrcColor, format SrcFormat, format DstFormat );

    ///////////////////////////////////////////////////////////////////////////
    // RARE USAGE FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////
    //
    // These functions are used in limited locations such as the bitmap loaders
    // etc... Do not use unless you know what you are getting into.
    //
    ///////////////////////////////////////////////////////////////////////////

    void	SetCompressed		( void );
    void	ClearCompressed		( void );
    xbool	IsCompressed		( void ) const;

    void	SetClutSwizzled		( void );
    void	ClearClutSwizzled	( void );
    xbool	IsClutSwizzled		( void ) const;

    void	SetDataSwizzled		( void );
    void	ClearDataSwizzled	( void );
    xbool	IsDataSwizzled		( void ) const;

    void	SetReducedIntensity		( void );
    void	ClearReducedIntensity	( void );
    xbool	IsReducedIntensity		( void ) const;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// PROTECTED !!! PROTECTED !!! PROTECTED !!! PROTECTED !!! PROTECTED !!! 
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
protected:

    void Reset          ( void );                   // Sets all members to null state
    void Build32BitMipData( u32* Src, s32 SrcW, s32 SrcH, 
                            u32* Dst, s32 DstW, s32 DstH );
    
///////////////////////////////////////////////////////////////////////////
protected:

    enum flags
    {
		FLAG_DATA_OWNED         = ( 1 << 0 ),
		FLAG_CLUT_OWNED         = ( 1 << 1 ),
		FLAG_CLUT_SWIZZLED      = ( 1 << 2 ),
		FLAG_COMPRESSED_S3TC    = ( 1 << 3 ),
		FLAG_XBITMAP_IS_VALID   = ( 1 << 4 ),
		FLAG_REDUCED_INTENSITY  = ( 1 << 5 ),
		FLAG_DATA_SWIZZLED      = ( 1 << 6 ),
    };

///////////////////////////////////////////////////////////////////////////
protected:

    static const fmt_desc s_Format[];

///////////////////////////////////////////////////////////////////////////
protected:

    byte*   m_pClut;            // Pointer for the clut memory
    byte*   m_pData;            // Pointer for the pixel memory
    s32     m_DataSize;         // Size of the data ( In bytes )
    s32     m_ClutSize;         // Size of the clut ( In bytes )

    s16     m_Width;            // Width ( In Pixels )
    s16     m_Height;           // Height ( In Pixels )
    s16     m_PW;               // Physical Width ( In Pixels )
    s16     m_PH;               // Physical Height ( In Pixels )
    s16     m_VramId;           // ID used internally by vram manager
    u16     m_Flags;            // Flags 

    u8      m_MaxClutID;        // Number of colors in the clut - 1
    s8      m_Format;           // Data format from define list
    s8      m_NCluts;           // Number of Cluts
    s8      m_NMips;            // Number of mipmaps (always at least 1)
};

////////////////////////////////////////////////////////////////////////////
//
//  Explanation of m_pData memory layout:
//
//  The begining of the block of memory is a mip-table.  Each table entry
//  is setup as : [ Offset from m_pData(32bit) ][ MipW(16) ][ MipH(16) ]
//
//  The size of the table is ALIGN_16( ((m_NMips+1) * 8 ) )
//
//  There will always be at least 1 entry (the original image)
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#ifndef X_BITMAP_INLINE_HPP
#include "x_bitmap_inline.hpp"
#endif

////////////////////////////////////////////////////////////////////////////

#endif
