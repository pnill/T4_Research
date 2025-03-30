////////////////////////////////////////////////////////////////////////////
//
//  X_TARGET.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_TARGET_HPP
#define X_TARGET_HPP

////////////////////////////////////////////////////////////////////////////
//
//  One target specification must be defined by the "exterior" environment.
//  That is, by the "project" or "makefile" which in turn causes files to be
//  compiled.
//  
//  The recognized values are shown below.  Each value is prefixed by a 
//  character indicating the state of the target's implementation.              
//    "*" = implemented and tested                                              
//    "+" = implemented but not thoroughly tested                               
//    "-" = proposed and/or not completely implemented                          
//  
//      Target macro            Description                 Debug Config     Output Directory
//      -------------------------------------------------------------------------------------
//   +  <none on PC>            Generic PC app; no new      Debug            Debug
//                              or delete; safe for MFC
//      
//   +  TARGET_PC               Generic PC app; redefines   PC Debug         DPC
//                              global new and delete
//      
//   +  TARGET_PS2_DEV          Sony Playstation 2 DevKit   PS2 DevKit       DPS2Dev
//   +  TARGET_PS2_DVD          Sony Playstation 2 DVD      PS2 DVD          DPS2DVD
//  
//   -  TARGET_DOLPHIN_DEV      Nintendo Dolphin DevKit     Dolphin DevKit   DDolphinDev
//   -  TARGET_DOLPHIN_DVD      Nintendo Dolphin DVD        Dolphin DVD      DDolphinDVD
//  
//   -  TARGET_XBOX_DEV         Microsoft XBox DevKit		XBox DevKit		 DXBoxDev
//   -  TARGET_XBOX_DVD			Microsoft XBox DVD			XBox DVD		 DXBoxDVD
//  
//  Each configuration has a release version as well.  "Debug" in the 
//  configuration name becomes "Release", and a leading "D" in the output
//  directory becomes a "R".
//  
//  For the "Debug" (and "Release") configurations, no target macro is 
//  defined.  In this case (and only when on a PC), the system assumes
//  that it is on a PC.  Furthermore, to be safe, it does NOT define
//  overloaded versions of operators new and delete.  This allows usage
//  within MFC programs.
//
//  To enable the overloaded operators new and delete (and to gain the 
//  heap debugging support the x_files provide), use the "PC Debug" (or "PC
//  Release") configuration explicitly.
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  HOW TO SPECIFY THE TARGET HARDWARE
////////////////////////////////////////////////////////////////////////////
//
//  In Visual C++ 6:
//      Place "TARGET_PC" (as appropriate) in the projects settings.  
//      Settings; C/C++ tab; Category: preprocessor; field 
//      'Preprocessor definitions'.  Be sure to select "All Configurations".
//
//  In Code Warrior 5:
//      You have to specify a "prefix file" in the Target Settings : C/C++
//		Language preference panel.  This file should be a simple header file
//      containing "#define TARGET_<whatever>".  There will need to be a
//		separate prefix file for each configuration.
//
//  In makefiles:
//      Be sure to set "TARGET = TARGET_PS2_DEV" or
//      "TARGET = TARGET_PS2_DVD" (as appropriate) in the makefile.
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  Check for "insufficiently qualified" targets definitions.
//
//  If you got an error here, then you specified a target platform but did 
//  sufficiently qualify the target.  For example, you cannot simply specify
//  TARGET_PS2.  You must specifically indicate TARGET_PS2_DEV or
//  TARGET_PS2_DVD as appropriate.
////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PS2 )
    #error Ambiguous target specification.  Consult file x_target.hpp.
#endif
#if defined( TARGET_DOLPHIN )
    #error Ambiguous target specification.  Consult file x_target.hpp.
#endif
#if defined( TARGET_XBOX )
    #error Ambiguous target specification.  Consult file x_target.hpp.
#endif

////////////////////////////////////////////////////////////////////////////
//  Playstation 2 Targets
////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PS2_DEV
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PS2
        #define VALID_TARGET
    #endif
#endif

#ifdef TARGET_PS2_DVD
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PS2
        #define VALID_TARGET
    #endif
#endif

#ifdef TARGET_PS2_USB
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PS2
        #define VALID_TARGET
    #endif
#endif

////////////////////////////////////////////////////////////////////////////
//  Dolphin Targets
////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_DOLPHIN_DEV
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_DOLPHIN
        #define VALID_TARGET
    #endif
#endif

#ifdef TARGET_DOLPHIN_DVD
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_DOLPHIN
        #define VALID_TARGET
    #endif
#endif

////////////////////////////////////////////////////////////////////////////
//  XBox Targets
////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_XBOX_DEV
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_XBOX
        #define VALID_TARGET
    #endif
#endif

#ifdef TARGET_XBOX_DVD
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_XBOX
        #define VALID_TARGET
    #endif
#endif

#ifdef TARGET_XBOX_PC
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_XBOX
        #define VALID_TARGET
    #endif
#endif

////////////////////////////////////////////////////////////////////////////
//  PC Targets
////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PC
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define VALID_TARGET
    #endif
#endif

////////////////////////////////////////////////////////////////////////////
//  If you got an error below, then somehow you have specified more than
//  one valid target platform.  You must correct this.
////////////////////////////////////////////////////////////////////////////

#ifdef MULTIPLE_TARGETS
    #error Multiple hardware targets specified.
#endif

////////////////////////////////////////////////////////////////////////////
//  If you got an error below, then you need to define a valid hardware
//  platform in your compile process.  See the comment section labeled "How
//  To Specify the Target Hardware Platform" near the top of this file.
////////////////////////////////////////////////////////////////////////////

#ifndef VALID_TARGET
    #ifdef _MSC_VER
        #define TARGET_PC
        #define USE_NATIVE_NEW_AND_DELETE
    #else
        #error Target invalid or not specified.
    #endif
#endif

////////////////////////////////////////////////////////////////////////////
//  Endian Designation
////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PC
#define LITTLE_ENDIAN
#endif

#ifdef TARGET_PS2
#define LITTLE_ENDIAN
#endif

#ifdef TARGET_DOLPHIN
#define LITTLE_ENDIAN
#endif

#ifdef TARGET_XBOX
#define LITTLE_ENDIAN
#endif

#if( !defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN) )
    #error Endian was not determined or set manually.
#endif

////////////////////////////////////////////////////////////////////////////
//  Extra checking and stuff per compiler and platform...
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  When we are using Developer Studio to compile for a PC target, the 
//  symbol _DEBUG is, by default, automatically defined for debug builds.  
//  Under this circumstance, we will go ahead and define X_DEBUG as well.
////////////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER ) && defined( _DEBUG ) && !defined( X_DEBUG )
    #define X_DEBUG
#endif

////////////////////////////////////////////////////////////////////////////
//  Under Code Warrior, we have the opportunity to make sure that 
//  enumeration types are always 32 bits.
////////////////////////////////////////////////////////////////////////////

#ifdef __MWERKS__
    #pragma enumsalwaysint on
#endif

////////////////////////////////////////////////////////////////////////////

#endif

