///////////////////////////////////////////////////////////////////////////
//
//  X_TARGET.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_TARGET_H
#define X_TARGET_H

///////////////////////////////////////////////////////////////////////////
// See which target the compilation process has defined...
//
// The recognized values are shown below.  Each value is prefixed by a 
// character indicating the state of the target's implementation.
//   "*" = implemented and tested
//   "+" = implemented but not thoroughly tested
//   "-" = proposed and/or not completely implemented
//
// -TARGET_SDC_D3D ........... Sega Dreamcast using Direct3D
//
// *TARGET_PC_D3D ............ Win32 using Direct3D
// +TARGET_PC_D3D_NO_ENTRY ... Win32 using Direct3D with no WinMain
//
// *TARGET_PC_WINCON ......... Win32 console application
// -TARGET_PC_WIN32 .......... Win32 windows application
//
// -TARGET_PC_OGL ............ Win32 using OpenGL
//
// +TARGET_N64_ROM ........... N64 ROM cartridge
// *TARGET_N64_DEV ........... N64 development system
// -TARGET_N64_CLIENT ........ N64 client (with PC server)
//
// -TARGET_PSX_ROM............ PlayStation CD-ROM
// -TARGET_PSX_DEV ........... PlayStation development system
// -TARGET_PSX_CLIENT ........ PlayStation client (with PC server)
//
// -TARGET_NUON_DEV .......... NUON (VM Labs Project X) development system
//
// -TARGET_ANSI .............. Generic expecting to link to ANSI functions
// -TARGET_ANSI_NO_IO ........ Link to ANSI; custom external I/O required
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// HOW TO SPECIFY THE TARGET HARDWARE
//
// In Visual C++ 5 and 6:
//      Place "TARGET_PC_D3D" (as appropriate) in the projects settings.  
//      Settings; C/C++ tab; Category: preprocessor; field 
//      'Preprocessor definitions'.  Be sure to select "All Configurations".
//
// In makefiles:
//      Be sure to set "TARGET = TARGET_N64_CLIENT" or
//      "TARGET = TARGET_N64_DEV" (as appropriate) in the makefile.
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Check for "unqualified" target definitions.
//
// If you got an error below, then you specified the target platform but
// did not qualify the target.  For example, you cannot simply specify 
// TARGET_N64, you must specifically indicate TARGET_N64_ROM or 
// TARGET_N64_DEV as appropriate.

#if defined( TARGET_N64 ) || defined( TARGET_PC )
    #error Unqualified target specified.
#endif

///////////////////////////////////////////////////////////////////////////
//  Dream Cast Targets
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_SDC_D3D
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_SDC
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////
//  PC Targets
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PC_D3D
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PC
        #define TARGET_D3D
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PC_D3D_NO_ENTRY
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PC
        #define TARGET_D3D
        #define TARGET_PC_D3D
        #define TARGET_NO_ENTRY
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PC_WINCON
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PC
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PC_WIN32
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PC
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PC_OGL
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PC
        #define TARGET_OGL
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////
//  N64 Targets
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_N64_ROM
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_N64
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_N64_DEV
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_N64
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_N64_CLIENT
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_N64
        #define TARGET_CLIENT
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////
//  PSX Targets
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PSX_ROM
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PSX
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PSX_DEV
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PSX
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PSX_CLIENT
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PSX
        #define TARGET_CLIENT
        #define VALID_TARGET
    #endif
#endif

///////////////////////////////////////////////////////////////////////////
//  NUON (VM Labs Project X) Targets
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_NUON_DEV
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_NUON
        #define VALID_TARGET
    #endif
#endif

////////////////////////////////////////////////////////////////////////////
// If you got an error below, then somehow you have specified more than
// one valid target platform.  You must correct this.

#ifdef MULTIPLE_TARGETS
    #error Multiple hardware targets specified.
#endif

////////////////////////////////////////////////////////////////////////////
// If you got an error below, then you need to define a valid hardware
// platform in your compile process.  See the comment section labeled "How
// To Specify the Target Hardware Platform" near the top of this file.

#ifndef VALID_TARGET
    #error Target invalid or not specified.
#endif

///////////////////////////////////////////////////////////////////////////
//  Endian Designation
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_SDC
#define LITTLE_ENDIAN
#endif

#ifdef TARGET_PC
#define LITTLE_ENDIAN
#endif

#ifdef TARGET_N64
#define BIG_ENDIAN
#endif

#ifdef TARGET_PSX
#define LITTLE_ENDIAN
#endif

#ifdef TARGET_NUON
#define LITTLE_ENDIAN
#endif

#if( !defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN) )
    #error Endian was not determined or set manually.
#endif

///////////////////////////////////////////////////////////////////////////
// When we are using Visual C to compile for a PC target, the macro _DEBUG
// is automatically defined for debug builds.  Under this circumstance, we 
// will go ahead and define X_DEBUG as well.
///////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PC ) && defined( _DEBUG ) && !defined( X_DEBUG )
    #define X_DEBUG
#endif

////////////////////////////////////////////////////////////////////////////

#endif

