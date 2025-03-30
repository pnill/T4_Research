///////////////////////////////////////////////////////////////////////////
//
//  Q_PS2.HPP
//
///////////////////////////////////////////////////////////////////////////

#ifndef Q_PS2_HPP
#define Q_PS2_HPP

///////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_color.hpp"
#include "x_bitmap.hpp"

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  PS2 Display list
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

extern  byte*        pDList;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  PS2 Engine functions
//
//  PS2_SetDestAlphaTest - Determines whether a pixel passes or fails
//      based on the dest alpha. If TestMode is 0, pixels whose destination
//      alpha is 0 pass, and if TestMode is 1, pixels whose destination
//      alpha is 1 pass.
//
//  PS2_SetFBA - Sets the FBA register on/off
//
//  PS2_SetMipK - Sets the mip-map K value
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void PS2_SetDestAlphaTest   ( xbool OnOff, s32 TestMode );
void PS2_SetFBA             ( xbool OnOff );
void PS2_SetMipK            ( f32 MipK );

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  PS2 Specific PREP Functions
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  These calls prepare various engine internals before initialization.
//  Some preps are required for the engine to function.  Others are 
//  optional.
//
///////////////////////////////////////////////////////////////////////////
//
//  PS2_PrepDListSize - REQUIRED
//      
//      Sets initial PS2 display list size.  Memory for display list is 
//      taken from the Scratch Memory.  (Size of display list can later be
//      changed with function PS2_SetDListSize.)
//
///////////////////////////////////////////////////////////////////////////
//
//  PS2_Prep3rdPartyDirectory - OPTIONAL
//
//      Sets the 3rd party 'sce' directory. This is a way to allow TnT to
//      exist in a directory other than C:\TnT. Set this directory to where
//      "TnT\3rdParty\PS2\sce" will exist on your hard drive.
//
///////////////////////////////////////////////////////////////////////////

void    PS2_PrepDListSize           ( s32 NBytes );
void    PS2_Prep3rdPartyDirectory   ( const char* dir );

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  Miscellaneous PS2 Specific Functions
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  PS2_SetDListSize
//
//      Set the size of the PS2 display list.  The new value does not go 
//      into effect until the next frame.  Display list memory is taken 
//      from the Scratch Memory.
//
///////////////////////////////////////////////////////////////////////////

void    PS2_SetDListSize        ( s32 NBytes );

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  VIF1 MicroCode Management Support
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  PS2_RegisterMicroCode
//      
//      Registers a new microcode with the engine.  The value returned can
//      be used to later activate the microcode.  
//      
//  PS2_ActivateMicroCode
//      
//      Activates the specified microcode.  ID must be that of a microcode 
//      registered at run time.  For Experts Only.
//      
///////////////////////////////////////////////////////////////////////////

s32     PS2_RegisterMicroCode   ( char* Name,
                                  void* CodeDMAAddr,
                                  void* DataDMAAddr );

void    PS2_ActivateMicroCode   ( s32 ID );

s32     PS2_GetActiveMicroCode  ( void );

void    PS2_InvalidateMicroCode ( void );


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//  Display Buffer Functions
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//  PS2_InitializeGraphics
//
//      Calls sceGsResetGraph and SetupFS_AA_buffer to set the graphics
//      buffers for the given graphics settings.
//
//  PS2_ScreenShot
//
//      Grabs framebuffer and stores it to a BMP file
//
//  PS2_PeekZBuffer
//
//      Peeks at the Z value at Position (X,Y) in the Z-Buffer
//
//  PS2_GetFrameBufferAddress
//  
//      Determines which frame buffer we are using and provides the address offset
//      into the GS local memory.
//      The xbool parameter bGetOppositeOffset is used when you want to get the
//      address of the back buffer, and not the front buffer.  The default
//      value of will give you the address of the front buffer.
//
//	PS2_IsProgressiveScanOn
//
//		Will return whether PS2_InitializeGraphics was last called with bIsProgressive
//		set to true.
////////////////////////////////////////////////////////////////////////////
void    PS2_InitializeGraphics              ( xbool bIsGame, xbool bIsProgressive = FALSE, xbool bIsPAL = FALSE );
void	PS2_ScreenShot			            ( const char* pFilename );
s32     PS2_PeekZBuffer                     ( s32 X, s32 Y ); 
void    PS2_ScreenClearImmediate            ( xbool bClearAll, u8 Red, u8 Green, u8 Blue, u8 Alpha = 255 );
void    PS2_GetClipMetrics                  ( s32 &nCulled, s32 &nClipped, s32 &nTotal );

xbool	PS2_IsProgressiveScanOn		        ( void );

void    PS2_InitialLoadScreen_Init          ( void );
void    PS2_InitialLoadScreen_Kill          ( void );
void    PS2_InitialLoadScreen_Pump          ( void );
s32     PS2_GetZBufferAddress               ( void );
s32     PS2_GetZBufferPSM                   ( void );
s16     PS2_GetFrameBufferAddress           ( xbool bGetOppositeOffset = FALSE );
s32     PS2_GetFrameBufferPSM               ( void );
s32     PS2_GetDisplayBufferAddress         ( void );
s32     PS2_GetDisplayBufferPSM             ( void );
f32     PS2_GetScreenXConversionMultiplier  ( void );
f32     PS2_GetScreenYConversionMultiplier  ( void );
void    PS2_SetFrameBufferSizeOverride      ( xbool bSet );
u64     PS2_GetFRAMESetting                 ( void );
void    PS2_ConvertBackCoordToFrontCoord    ( f32 &DstX, f32 &DstY, const f32 SrcX, const f32 SrcY );


void    PS2_InitGraphicsCheat               ( s32 iIndex = 0 );
void    PS2_KillGraphicsCheat               ( s32 iIndex = 0 );
extern xbool PS2_IsGraphicsCheatActive      ( s32 iIndex = 0 );

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  PS2 INPUT Support
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  PS2_SetAnalogMode
//      
//      Forces an analog-capable controller into analog mode (On==TRUE) or
//      digital mode (On==FALSE);
//      
//  PS2_GetAnalogMode
//
//      Returns TRUE if the controller is in analog mode. FALSE if it is
//      not in analog mode, or if the controller doesn't support analog.
//
//  PS2_LockAnalogToggle
//
//      Locks/Unlocks a controllers analog button into its current state.
//      Set the state to whatever is appropriate before locking.
//
//      **NOTE** 
//
//      Toggling the lock mechanism causes the sony libs to reset
//      the actuator settings, stoping any current actuatior activity.
//      The Quagmire input system will reset the actuators and re-enable
//      any ongoing actuator activity, but there will be a noticalbe
//      interruption in the rumble.
//
//      **NOTE-2**
//
//      Toggling the lock mechanism causes pressure-sensitive mode to be
//      exited.  The Quagmire input system will return the pressure
//      sensitive mode to it's pre-locked state, but the hardware requires
//      a pause while controllers adjust.  This will create a noticable
//      interruption in the pressure-sensitive state.
//
//
//  PS2_SetPressMode
//
//      Enables or disables pressure-sensitive mode on Dualshock2 
//      controllers.
//      
///////////////////////////////////////////////////////////////////////////
void  PS2_SetAnalogMode    ( s32 Controller, xbool On   );
xbool PS2_GetAnalogMode    ( s32 Controller             );
void  PS2_LockAnalogToggle ( s32 Controller, xbool Lock );
void  PS2_SetPressMode     ( s32 Controller, xbool On   );

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//  PS2 PERFORMANCE TUNING Support
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//  PS2_SetRenderModeDescriptions
//
//      Sets an array of strings that will be used to describe the performance
//      between each block of ENG_BeginRenderMode() and ENG_EndRenderMode()
//
//  PS2_BeginTiming
//
//      Tells the engine to start recording timing information between
//      blocks of ENG_BeginRenderMode() and ENG_EndRenderMode()
//
//  PS2_EndTiming
//
//      Tells the engine to stop recording the timing information and save
//      the recorded data out to a comma-delimited text file
//
////////////////////////////////////////////////////////////////////////////

void PS2_SetRenderModeDescriptions  ( char** Descriptions, s32 NDescriptions );
void PS2_BeginTiming                ( void );
void PS2_EndTiming                  ( char* FileName );




void    PS2_InitNonBlockingVSync( void );
void    PS2_KillNonBlockingVSync( void );
xbool   PS2_IsNonBlockingVSync  ( void );
void    PS2_WaitVSync           ( void );

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//  PS2 DISPLAY LIST DEBUGGING
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//  PS2_DLIST_DEBUGGING -   Enable this define and litter PS2_DLIST_VERIFY
//                          calls all over your code to track down bugs
//                          related to a corrupted display list. DO NOT
//                          leave this enabled for a final build.
//
//  PS2_DLIST_VERIFY    -   Litter your code with this macro, and when it
//                          finds a trouble spot it will ASSERT or throw
//                          up an exception on a release build
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

//#define PS2_DLIST_DEBUGGING 0x1

#ifndef PS2_DLIST_DEBUGGING
    #define PS2_DLIST_VERIFY()
#else // PS2_DLIST_DEBUGGING
    #define PS2_DLIST_VERIFY()    { PS2_DListVerify(__FILE__, __LINE__); }
    void PS2_DListVerify( const char* Filename, s32 LineNumber );
#endif // PS2_DLIST_DEBUGGING


///////////////////////////////////////////////////////////////////////////
#endif
///////////////////////////////////////////////////////////////////////////
