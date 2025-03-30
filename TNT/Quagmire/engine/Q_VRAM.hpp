///////////////////////////////////////////////////////////////////////////
//
//  Q_VRAM.HPP
//
///////////////////////////////////////////////////////////////////////////

#ifndef Q_VRAM_HPP
#define Q_VRAM_HPP

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "x_color.hpp"
#include "x_bitmap.hpp"


///////////////////////////////////////////////////////////////////////////
// DEFINITIONS
///////////////////////////////////////////////////////////////////////////

#define WRAP_MODE_CLAMP    0
#define WRAP_MODE_TILE     1
#define WRAP_MODE_MIRROR   2
#define WRAP_MODE_REGION   3
#define WRAP_MODE_R_REPEAT 4


///////////////////////////////////////////////////////////////////////////
// PUBLIC PROTOTYPES
///////////////////////////////////////////////////////////////////////////
                                
///////////////////////////////////////////////////////////////////////////
//
// Basic functionality 
//
///////////////////////////////////////////////////////////////////////////
//
// VRAM_Init            - Initialize the VRAM sub-system
//
// VRAM_Kill            - Kill the VRAM sub-system
//
// VRAM_IsActive        - Reports if a texture is currently in VRAM.
//
// VRAM_Activate        - Moves a texture into VRAM and sets it as the 
//                        working texture. Optional: If you know the range
//                        of Mip levels you need, that can be specified
//                        as well.
//
// VRAM_Deactivate      - Remove a texture from VRAM.
//                      
// VRAM_DeactivateAll   - Removes all textures from VRAM.
//
// VRAM_SetWrapMode     - Indicates what type of wrapping will apply to 
//                        the UV pair.
//                      
//      When the U or V values fall outside of the 0.0 thru 1.0 range, 
//      pixels for texturing are selected as follows:
//
//      WRAP_MODE_CLAMP    - Texture coordinates outside the range [0.0, 1.0] 
//                           are set to the texture color at 0.0 or 1.0, 
//                           respectively.
//
//      WRAP_MODE_TILE     - Tile the texture at every integer junction. For 
//                           example, for u values between 0 and 3, the  
//                           texture will be repeated three times; no  
//                           mirroring is performed.
//
//      WRAP_MODE_MIRROR   - Similar to WRAP_MODE_TILE, except that the 
//                           texture is flipped at every integer junction. 
//                           For u, or v values between 0 and 1, for example, 
//                           the texture is addressed normally, between 1 and 
//                           2 the texture is flipped (mirrored), between 2 
//                           and 3 the texture is normal again, and so on. 
//
//      WRAP_MODE_REGION   - Texture coordinates outside the range
//                           [UMIN, UMAX] and [VMIN, VMAX] are set to the
//                           texture color at UMIN or UMAX (or VMIN or VMAX),
//                           respectively.
//
//      WRAP_MODE_R_REPEAT - The region specified by [UMIN, UMAX] and [VMIN, VMAX]
//                           are repeated.
//
// VRAM_SetStatTrackers - Inserts a set of vars which will be filled with
//                        various VRAM stats. If any parameter is NULL,
//                        stat tracking will be turned off. By default,
//                        all stat tracking is turned off.
//
///////////////////////////////////////////////////////////////////////////

void  VRAM_Init                  ( void );
void  VRAM_Kill                  ( void );

void  VRAM_Register              ( x_bitmap& BMP, s32 iContext = 0 );
void  VRAM_UnRegister            ( x_bitmap& BMP );
xbool VRAM_IsRegistered          ( x_bitmap& BMP );

void  VRAM_Activate              ( x_bitmap& BMP );
void  VRAM_Activate              ( const x_bitmap* BMP, s32 nBMPCount = 1 );
void  VRAM_Activate              ( x_bitmap& BMP, s32 MinMip, s32 MaxMip );
void  VRAM_Deactivate            ( x_bitmap& BMP );
void  VRAM_DeactivateAll         ( void );

xbool VRAM_IsActive              ( x_bitmap& BMP );

void  VRAM_SetWrapMode           ( s32 UMode, s32 VMode,
                                   f32 UMin = 0.0f, f32 UMax = 1.0f,
                                   f32 VMin = 0.0f, f32 VMax = 1.0f );

void  VRAM_SetStatTrackers       ( s32* pNTextureUploads,
                                   s32* pNBytesUploaded );

void  VRAM_UseBilinear           ( xbool bActivate = TRUE );
void  VRAM_UseTrilinear          ( xbool bActivate = TRUE );




///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// PLATFORM-SPECIFIC FUNCTIONS FOR INTERNAL USE ONLY
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


#if defined( TARGET_PS2 )
///////////////////////////////////////////////////////////////////////////
// PS2 SPECIFIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////
//                          
// VRAM_PS2_GetTexAddr  - Get the block address of a recently activated texture
// VRAM_PS2_GetClutAddr - Get the block address of a recently activated clut
//                              
///////////////////////////////////////////////////////////////////////////

s32 VRAM_PS2_GetLog2        ( s32 Dimension );
s32 VRAM_PS2_GetFormat      ( x_bitmap& BMP );
s32 VRAM_PS2_GetTexAddr     ( x_bitmap& BMP, s32 MipID = 0 );
s32 VRAM_PS2_GetClutAddr    ( x_bitmap& BMP );

u32 VRAM_PS2_GetPermanentTexturePageAddress( void );

s32 VRAM_PS2_GetContext     ( x_bitmap& BMP );

#elif defined( TARGET_DOLPHIN )
///////////////////////////////////////////////////////////////////////////
// GAMECUBE SPECIFIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////
//
// VRAM_GC_Activate    - sets the specified bitmap to a specific texture stage
// VRAM_GC_Deactivate  - removes a texture from a specific texture stage
// VRAM_GC_SetWrapMode - sets the texture wrap mode on a specific texture stage
//
///////////////////////////////////////////////////////////////////////////

void  VRAM_GC_Activate      ( x_bitmap& BMP, s32 TevStage );
void  VRAM_GC_Deactivate    ( s32 TevStage );
void  VRAM_GC_SetWrapMode   ( s32 UMode, s32 VMode, s32 TevStage );

#elif defined( TARGET_PC )
///////////////////////////////////////////////////////////////////////////
// PC SPECIFIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////
//
// VRAM_PC_Activate    - Sets the specified bitmap to a specific texture stage
// VRAM_PC_Deactivate  - Removes a texture from a specific texture stage
// VRAM_PC_SetWrapMode - Sets the texture wrap mode on a specific texture stage
//
////////////////////////////////////////////////////////////////////////////
void VRAM_PC_Activate      ( x_bitmap& BMP, s32 TexStage );
void VRAM_PC_Deactivate    ( s32 TevStage );
void VRAM_PC_SetWrapMode   ( s32 UMode, s32 VMode, s32 TexStage );

#elif defined( TARGET_XBOX )
///////////////////////////////////////////////////////////////////////////
// XBOX SPECIFIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////
//
// VRAM_XBOX_Activate    - Sets the specified bitmap to a specific texture stage
// VRAM_XBOX_Deactivate  - Removes a texture from a specific texture stage
// VRAM_XBOX_SetWrapMode - Sets the texture wrap mode on a specific texture stage
//
////////////////////////////////////////////////////////////////////////////
void VRAM_XBOX_Activate      ( x_bitmap& BMP, s32 TexStage );
void VRAM_XBOX_Deactivate    ( s32 TevStage );
void VRAM_XBOX_SetWrapMode   ( s32 UMode, s32 VMode, s32 TexStage );
void *VRAM_XBOX_GetTexture   ( x_bitmap& BMP ); // return is LPDIRECT3DTEXTURE8

///////////////////////////////////////////////////////////////////////////
#endif



////////////////////////////////////////////////////////////////////////////
// VRAM DEBUG FUNCTIONS
////////////////////////////////////////////////////////////////////////////
#if defined( X_DEBUG )

void VRAM_DBG_Register   ( x_bitmap& BMP, char* File, s32 Line, s32 iContext = 0 );
void VRAM_DBG_Dump       ( const char* Filename );
s32  VRAM_DBG_FreeSlots  ( void );
s32  VRAM_DBG_UsedSlots  ( void );

//#define VRAM_Register( BMP, iContext ) VRAM_DBG_Register( BMP, __FILE__, __LINE__, iContext )
//#define VRAM_Register( BMP ) VRAM_DBG_Register( BMP, __FILE__, __LINE__, 0 )

#else

#define VRAM_DBG_Register( BMP, File, Line, iContext )
#define VRAM_DBG_Dump( Filename )
#define VRAM_DBG_FreeSlots()                    (-1)
#define VRAM_DBG_UsedSlots()                    (-1)

#endif

////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////

#endif // Q_VRAM_HPP
