///////////////////////////////////////////////////////////////////////////
//
//  Q_ENGINE.HPP
//
///////////////////////////////////////////////////////////////////////////

#ifndef Q_ENGINE_HPP
#define Q_ENGINE_HPP

///////////////////////////////////////////////////////////////////////////
//
//                            Y - up
//                            |
//                            |
//                            |
//                            |
//                            +---------X - left
//                           /
//                          /
//                         /
//                        Z - forward
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "x_color.hpp"
#include "x_stats.hpp"

#include "Q_View.hpp"
#include "Q_Light.hpp"


///////////////////////////////////////////////////////////////////////////
//
//  Area definitions
//
///////////////////////////////////////////////////////////////////////////

#define ENG_AREA_SCREEN       0
#define ENG_AREA_2D_CLIP      1
#define ENG_AREA_3D_VIEWPORT  2


///////////////////////////////////////////////////////////////////////////
//
//  Platform specific engine functions
//
//  All functions listed in this header file will work on any platform
//  supported by the Quagmire engine.  In addition to these "generic" 
//  functions are several which are very platform specific.  Such functions
//  are listed in the following files according to platform.
//
//      PS2.........Q_PS2.h
//      GameCube....Q_GC.h
//      XBOX........Q_XBOX.h
//      PC..........Q_PC.h
//
///////////////////////////////////////////////////////////////////////////
#if defined( TARGET_PS2 )

const s32 kDisplayWidth  = (640);
const s32 kDisplayHeight = (448);

#define MAX_VRAM_TEXTURES       (608)
#define MAX_MIP_LEVELS          8

//---   define some sizes
#define VRAM_BLOCK_SIZE         256
#define VRAM_PAGE_SIZE          (8 * 1024)  // Block size * 32

#define VRAM_SIZE_IN_BLOCKS     16384       // (4 meg / block size)
#define VRAM_SIZE_IN_PAGES      512         // (4 meg / page size)

//---   define various memory locations in page offsets
#define VRAM_FRAME1_PAGE_ADDR            0
#define VRAM_FRAME2_PAGE_ADDR            ((640*480)/2048)           // prepared for max size
#define VRAM_ZBUFF_PAGE_ADDR             (VRAM_FRAME2_PAGE_ADDR*2)
// The permanent texture page, is a 64K block of memory used
// to store a single image without fear that it will be overwritten
// by another texture.
// All-Star baseball used this space to store the image that was 
// projected onto the JumboTron in the stadiums.
#define VRAM_PERMANENT_TEXTURE_PAGE_ADDR (452)
#define VRAM_BASE_PAGE_ADDR              (VRAM_PERMANENT_TEXTURE_PAGE_ADDR+8)
                                                    
                                                    
                                                    

//---   define various memory locations in block offsets
#define VRAM_FRAME1_BLOCK_ADDR              0       
#define VRAM_FRAME2_BLOCK_ADDR              ((640*480)/64)
#define VRAM_ZBUFF_BLOCK_ADDR               (VRAM_FRAME2_BLOCK_ADDR*2)
#define VRAM_PERMANENT_TEXTURE_BLOCK_ADDR   (VRAM_PERMANENT_TEXTURE_PAGE_ADDR*32)
#define VRAM_BASE_BLOCK_ADDR                (VRAM_PERMANENT_TEXTURE_BLOCK_ADDR+256)

#define VRAM_NUM_BLOCKS_FREE    (16384 - VRAM_BASE_BLOCK_ADDR) // 2688 for 640x448 : 1728 for 640x480

//---   how many slots does each bank have?
#define VRAM_BANK_SIZE_256      3
#define VRAM_BANK_SIZE_128      6
#define VRAM_BANK_SIZE_64       10
#define VRAM_BANK_SIZE_32       12
#define VRAM_NSLOTS             31
#define VRAM_NCLUTS             31

//---   define the bank offsets
#define BANK_256_OFFSET         0
#define BANK_128_OFFSET         (VRAM_BANK_SIZE_256)
#define BANK_64_OFFSET          (BANK_128_OFFSET + VRAM_BANK_SIZE_128)
#define BANK_32_OFFSET          (BANK_64_OFFSET  + VRAM_BANK_SIZE_64)

//---   with that knowledge, what are the bank memory addresses?
#define VRAM_BANK_256_ADDR      (VRAM_BASE_BLOCK_ADDR)
#define VRAM_BANK_128_ADDR      (VRAM_BANK_256_ADDR + (256 * VRAM_BANK_SIZE_256))
#define VRAM_BANK_64_ADDR       (VRAM_BANK_128_ADDR + (128 * VRAM_BANK_SIZE_128))
#define VRAM_BANK_32_ADDR       (VRAM_BANK_64_ADDR  + (64  * VRAM_BANK_SIZE_64))
#define VRAM_CLUT_ADDR          (VRAM_BANK_32_ADDR  + (32  * VRAM_BANK_SIZE_32))
#define VRAM_END_ADDR           (VRAM_CLUT_ADDR     + (4   * VRAM_NCLUTS))

#else

const s32 kDisplayWidth  = 640;
const s32 kDisplayHeight = 480;

#endif


///////////////////////////////////////////////////////////////////////////
//
//  Primary Functionality
//
///////////////////////////////////////////////////////////////////////////
//
//  ENG_Activate            - Activate the engine.
//                              + XRes, YRes : Display resolution.
//                              + ZBuffer    : Activates z-buffer support.
//                              + SMemSize   : Size of the scratch memory
//                                buffer.  Note that TWO such buffers are
//                                allocated.
//                              + 32 byte string that has the title of a game.  Example "All-Star Baseball 2003"
//
//                            Note that each platform may have 'prep' 
//                            functions which affect the engine activation
//                            behavior.  Use of some prep functions is 
//                            optional, while use of others is required.
//                            See the platform specific engine header file
//                            for details.
//
//  ENG_Deactivate          - Shut down the engine.  Release all buffers 
//                            allocated by the engine.
//
//  ENG_PageFlip            - Wrap up current render frame and page flip.
//
//  ENG_SetActiveView       - Copy given view into the engine's "active"
//                            view.
//
//  ENG_GetActiveView       - Get a POINTER to the engine's "active" view.
//                            Note that the active view is locked and 
//                            cannot be modified.
//
//  ENG_SetActiveLighting   - Copy given lighting into the engine's 
//                            "active" lighting.
//
//  ENG_GetActiveLighting   - COPY engine's active lighting into given
//                            lighting structure.
//
//  ENG_SetFogParamaters    - Sets parameters for hardware fogging
//  ENG_ActivateFog         - Turns fog on/off
//  ENG_SetFogColor         - Sets an RGB for the fog
//
///////////////////////////////////////////////////////////////////////////

xbool   ENG_Activate            ( s32   XRes,
                                  s32   YRes,
                                  xbool ZBuffer,
                                  s32   SMemSize,
                                  char* ApplicationName = NULL );

void    ENG_Deactivate          ( void );

void    ENG_PageFlip            ( xbool ClearNewScreen = TRUE );

void    ENG_SetActiveView       ( const view& View );
view*   ENG_GetActiveView       ( void );

void    ENG_Set2DClipArea       ( s32  L, s32  T, s32  R, s32  B );
void    ENG_Get2DClipArea       ( s32& L, s32& T, s32& R, s32& B );

void    ENG_SetActiveLighting   ( const lighting& Lighting, s32 iSet = 0 );
void    ENG_GetActiveLighting   ( lighting& Lighting, s32 iSet = 0 );

void    ENG_SetLight            ( s32 LightID, vector3& Dir, color& Color );
void    ENG_SetAmbientLight     ( color& Color );

void    ENG_SpecialCaseFog      (  xbool bActivateSpecial );
void    ENG_SetFogParameters    ( xbool bEnable, f32 fStart = 0.0f, f32 fAccumDepth = 0.0f, f32 fMaxFog = 0.0f, f32 fDensity=0.0f );
void    ENG_ActivateFog         ( xbool bEnable );
void    ENG_SetFogColor         ( u8 Red, u8 Green, u8 Blue );

struct eng_fog
{
    xbool   bEnabled;
    f32     fStart;
    f32     fDepth;
    f32     fMaxFog;
    f32     fDensity;
};

extern color eng_fog_color;
extern eng_fog g_EngineFog;

///////////////////////////////////////////////////////////////////////////
//
//  Render Functionality
//
///////////////////////////////////////////////////////////////////////////
//
//  ENG_BeginRenderMode     - Activate Quagmire's "Render Mode".  All
//                            rendering must take place during render mode.
//                            State changes (such as changing the view) are
//                            prohibited during render mode.
//
//  ENG_SetRenderFlags      - Must be called after ENG_BeginRenderMode()
//                            Determines how the Engine performs its
//                            rendering. These flags must be OR'd together.
//                            One value is explicitly required from each row
//                            The flags are:
//
//              ENG_ZBUFFER_TEST_ON ....... ENG_ZBUFFER_TEST_OFF
//              ENG_ZBUFFER_FILL_ON ....... ENG_ZBUFFER_FILL_OFF
//              ENG_ALPHA_TEST_ON   ....... ENG_ALPHA_TEST_OFF
//              ENG_ALPHA_BLEND_ON  ....... ENG_ALPHA_BLEND_OFF
//
//
//  ENG_SetBlendMode        - Sets the engine's blending mode during rendering.
//                            This must be called after ENG_BeginRenderMode() 
//                            The blending mode flags are:
//
//              ENG_BLEND_NORMAL............Preforms the typical blend 
//                                          (SC - DC) * SA  + DC
//              ENG_BLEND_ADDITIVE..........Preforms an additive blend
//              ENG_BLEND_SUBTRACTIVE.......Preforms a subtractive blend
//              ENG_BLEND_MULTIPLY..........Preforms a multiplicative blend (PS2 UNSUPPORTED)
//              ENG_BLEND_FIXED_ALPHA.......Uses a specified fixed value for 
//                                          the alpha component 
//
//  ENG_EndRenderMode       - Deactivate render mode.
//
//  ENG_GetRenderMode       - Returns TRUE if the engine is currently in
//                            a render mode.  This function should 
//                            generally be used for error checking.
//
///////////////////////////////////////////////////////////////////////////

//--Z Buffer Mode Flags
#define ENG_ZBUFFER_TEST_ON     0x01
#define ENG_ZBUFFER_TEST_OFF    0x02
#define ENG_ZBUFFER_FILL_ON     0x04
#define ENG_ZBUFFER_FILL_OFF    0x08

//-- Alpha Mode Flags
#define ENG_ALPHA_TEST_ON       0x10
#define ENG_ALPHA_TEST_OFF      0x20
#define ENG_ALPHA_BLEND_ON      0x40
#define ENG_ALPHA_BLEND_OFF     0x80

//-- Blending Mode Flags
#define ENG_BLEND_NORMAL        0x01
#define ENG_BLEND_ADDITIVE      0x02
#define ENG_BLEND_SUBTRACTIVE   0x03
#define ENG_BLEND_DEST          0x04
#define ENG_BLEND_MULTIPLY      0x05
#define ENG_BLEND_MULTI_1       0x06
#define ENG_BLEND_MULTI_2       0x07
#define ENG_BLEND_STENCIL       0x08
#define ENG_BLEND_FIXED_ALPHA   0x10


enum
{
    ENG_LIGHTING_SET_DEFAULT    = (1<<0),
    ENG_LIGHTING_SET_DARK       = (1<<1),
    ENG_LIGHTING_SET_ALTERNATE  = (1<<2),
};

#define ENG_LIGHTING_MAX_LEVELS 3



void    ENG_BeginRenderMode     ( void );
void    ENG_GetRenderFlags      ( u32& RenderFlags );
void    ENG_GetBlendMode        ( u32& BlendMode, u8& FixedAlpha );
void    ENG_SetRenderFlags      ( u32 RenderFlags );
void    ENG_SetBlendMode        ( u32 BlendFlags, u8 FixedAlpha = 0x00 );
void    ENG_EndRenderMode       ( void );
xbool   ENG_GetRenderMode       ( void );





///////////////////////////////////////////////////////////////////////////
//
//  View Manipulation...........Q_View.h
//  Lighting Manipulation.......Q_Light.h
//  Device Input................Q_Input.h
//  Scratch Memory..............Q_SMem.h
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//  Auxiliary Functionality
//
///////////////////////////////////////////////////////////////////////////
//
//  ENG_SetBackgroundColor  - Sets the background color fill during a page flip
//
//  ENG_FillScreenArea      - Fill specified portion of the screen with 
//                            specified color.  Note that the color 
//                            includes alpha.
//
//  ENG_FillZBufferArea     - Fill specified portion of the z-buffer with
//                            a value specified parametrically from 0.0
//                            (NearZ) to 1.0 (FarZ).  Currently we only
//                            support values of (0.0) and (1.0).
//
//  ENG_EnableRGBAChannels  - Sets the mask for the R,G,B, and A channels
//                            in the frame buffer.
//
//  ENG_GetResolution       - Get the current resolution.
//
//  ENG_WaitForGraphicsSynch
//                          - Wait until all graphic pipelines have been
//                            completed.
//
//  ENG_GetFPS              - Returns the FPS averaged over the last
//                            eight frames.
//
//  ENG_GetRunTime          - Returns the amount of time, in seconds,
//                            elapsed since the program started.
//
//  ENG_GetPageFlipCount    - Returns the number of times the function
//                            ENG_PageFlip has been called.
//
//  ENG_GetPerfMetrics      - Returns the timer values for total frame time, CPU time,
//                            and GFX processor time.  Units are in seconds.
//
//  ENG_SetBilinear         - Sets bilinear filter state.
//
//  ENG_GetBilinear         - Retrieves bilinear filter state.
//
//  ENG_PrintStats          - Print some basic stats
//
///////////////////////////////////////////////////////////////////////////

void    ENG_SetBackgroundColor      ( color C );

void    ENG_FillScreenArea          ( s32 Area, color Color );
void    ENG_FillZBufferArea         ( s32 Area, f32   Depth );

void    ENG_EnableRGBAChannels      ( xbool R = TRUE,
                                      xbool G = TRUE,
                                      xbool B = TRUE,
                                      xbool A = TRUE );

void    ENG_GetResolution           ( s32& Width, s32& Height );
extern s32     ENG_GetScreenWidth   ( void );
extern s32     ENG_GetScreenHeight  ( void );

void    ENG_WaitForGraphicsSynch    ( void );

f32     ENG_GetFPS                  ( void );
f64     ENG_GetRunTime              ( void );
s32     ENG_GetPageFlipCount        ( void );

void    ENG_GetPerfMetrics          ( s32& nCulled, s32& nClipped, s32& nTotal, f32& fXFOV, f32& fYFOV, f32& FPSTimer, f32& CPUTimer, f32& GFXTimer );

void    ENG_SetBilinear             ( xbool OnOff );
xbool   ENG_GetBilinear             ( void );

void    ENG_PrintStats              ( void );


///////////////////////////////////////////////////////////////////////////
//
//  CPU Time spent between pageflips
//
///////////////////////////////////////////////////////////////////////////

PSTAT_EXTERN( StatEngineCPUTime );


///////////////////////////////////////////////////////////////////////////
//
//  Experts Only Functionality
//
///////////////////////////////////////////////////////////////////////////

void    ENG_GetW2S  ( matrix4& M );
void    ENG_GetW2C  ( matrix4& M );
void    ENG_GetC2S  ( matrix4& M );

///////////////////////////////////////////////////////////////////////////
//
//  NIFTY HELPER MACROS (COPIED FROM PC)
//
///////////////////////////////////////////////////////////////////////////
#if !defined( SAFE_DELETE ) && !defined( SAFE_DELETE_ARRAY )
#define SAFE_DELETE( p )        { if( p ) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY( p )  { if( p ) { delete[] (p);   (p)=NULL; } }
#endif

///////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////

#endif // Q_ENGINE_HPP
