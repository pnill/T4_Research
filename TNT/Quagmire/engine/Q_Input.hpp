////////////////////////////////////////////////////////////////////////////
//
// Q_Iput.hpp
//
////////////////////////////////////////////////////////////////////////////

#ifndef Q_INPUT_HPP
#define Q_INPUT_HPP

////////////////////////////////////////////////////////////////////////////
//
// TERMINOLOGY:
//
//  A "button" is something that the user can "press" and "release".  It is
//  essentially a bit.
//
//  A "locator" is something that provides analog information.  Fields are
//  available for up to three values.
//
//  A "gadget" is either a button or a locator.  It is essentially
//  something which a user can physically manipulate.  (Under certain
//  circumstances, a whole device or even the whole system may be
//  considered a single gadget.)
//
//
// ABBREVIATIONS:
//
//  BTN     - button
//  JBTN    - button on joystick 
//  MBTN    - button on mouse
//  LOC     - locator
//  MLOC    - locator on mouse
//  KBD     - keyboard
//  JOY     - joystick
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////
//
// All locator values which can be meaningfully converted to a standardized 
// range will use the extremes below.

#define LOCATOR_MIN    (-1.0f)
#define LOCATOR_MAX    ( 1.0f)


////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////

// Structure which describes a single "User Input Event".
struct input_event
{
    s16     DeviceType;     // Type of device which concerns the event (Mouse, Joystick, ...).
    s16     DeviceNum;      // Which instance of the device (Joystick1, Joystick2, ...).

    s16     GadgetType;     // Type of gadget that caused event: Locator, Button, Message.
    s16     GadgetID;       // ID Number of gadget that caused event.

    s16     Info;           // (Relative or Absolute) | (Button State) | (Msg Code) | etc.

    u16     Flags;          // ...Flags

    f32     X;              // Rest of the Message Info...
    f32     Y;
    f32     Z;

    s32     Timestamp;      // Time the event got registered in the Event Queue.
                            // Unit is 1/60th of a second.
};


////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////

xbool INPUT_GetNextEvent    ( input_event& Event );

f32   INPUT_GetLocatorX     ( s16 DeviceNum, s16 GadgetID );
f32   INPUT_GetLocatorY     ( s16 DeviceNum, s16 GadgetID );
f32   INPUT_GetLocatorZ     ( s16 DeviceNum, s16 GadgetID );

xbool INPUT_GetButton       ( s16 DeviceNum, s16 GadgetID );
xbool INPUT_GetDebounce     ( s16 DeviceNum, s16 GadgetID );
void  INPUT_ClearDebounce   ( void );

xbool INPUT_IsControllerConnected( s32 ControllerID );
s32   INPUT_ReturnLowestConnectedController( void );

#define   PRESSED( DeviceNum, GadgetID )     // defined later
#define  DEBOUNCE( DeviceNum, GadgetID )     // defined later
#define LOCATOR_X( DeviceNum, GadgetID )     // defined later
#define LOCATOR_Y( DeviceNum, GadgetID )     // defined later
#define LOCATOR_Z( DeviceNum, GadgetID )     // defined later


////////////////////////////////////////////////////////////////////////////
//
//  FEEDBACK/RUMBLE Functions
//
//  INPUT_RumblePlay
//
//      Initiate a "rumble".  Intensity is parametric (0.0 thru 1.0).  The
//      duration is specified in seconds.  Any previous rumble values are 
//      overridden.
//
//  INPUT_RumbleStop
//
//      Terminates any current rumbling.
//
//  INPUT_RumblePause
//
//      Halts any current rumbling, but does not clear the duration timer.
//      Useful for pausing a rumble (with the ability to resume) during a
//      "Pause" menu.
//
//  INPUT_RumbleResume
//
//      Resumes any paused rumble.
//
////////////////////////////////////////////////////////////////////////////

void INPUT_RumblePlay       ( s32 ControllerID, f32 Intensity, f32 Duration );
void INPUT_RumbleStop       ( s32 ControllerID );
void INPUT_RumblePause      ( s32 ControllerID );
void INPUT_RumbleResume     ( s32 ControllerID );


////////////////////////////////////////////////////////////////////////////
//
//  DEADZONE/SATURATION Functions
//
//  All input below deadzone reports zero.
//  All input above saturation reports one.
//  All values between deadzone and saturation are scaled to be in the
//  range [0,1]
//
//
//  INPUT_SetDeadZone
//
//      Sets the deadzone width for a controllers.
//      Any controller movement inside of the deadzone will fail to 
//      generate any input events.
//
//  INPUT_SetSaturation
//
//      Sets the upper-bound on a axis value.  Any controller above the
//      saturation value will report full deflection.
//
////////////////////////////////////////////////////////////////////////////

void INPUT_SetDeadZone      ( f32 DeadZone   );
f32  INPUT_GetDeadZone      ( void           );
void INPUT_SetSaturation    ( f32 Saturation );
f32  INPUT_GetSaturation    ( void           );


////////////////////////////////////////////////////////////////////////////
// ENUMERATIONS
////////////////////////////////////////////////////////////////////////////

//-- Enumeration of all types of devices for input.
//   (Used for the DeviceType field in input_event.)
enum
{
    DEVICE_SYSTEM,
    DEVICE_PC_KEYBOARD,
    DEVICE_PC_MOUSE,
    DEVICE_PC_JOYSTICK,
    DEVICE_PS2_JOYSTICK,
    DEVICE_PS2_MEMCARD,
	DEVICE_PS2_MULTITAP,
    DEVICE_GAMECUBE_JOYSTICK,
    DEVICE_XBOX_JOYSTICK,

    DEVICE_END_OF_DEVICES
};

//-- Enueration of the various gadget types.
//   (Used for the GadgetType field in input_event.)
enum
{                                // Associated data concerns...
    GADGET_TYPE_MESSAGE,         //  ... a whole device or the system
    GADGET_TYPE_BUTTON,          //  ... a button on a device
    GADGET_TYPE_LOCATOR_X,       //  ... a 1D locator (eg: dial)
    GADGET_TYPE_LOCATOR_XY,      //  ... a 2D locator (eg: mouse)
    GADGET_TYPE_LOCATOR_XYZ,     //  ... a 3D locator (eg: spaceball)

    GADGET_TYPE_END_OF_LIST,
};

//-- Enumeration of how locator information is represented.
//   (Used for the Info field in input_event.)
enum
{
    LOCATOR_ABS,
    LOCATOR_REL,
};

//-- Enumeration of button states.
//   (Used for the Info field in input_event.)
enum
{
    BUTTON_RELEASE,
    BUTTON_PRESS,
};

////////////////////////////////////////////////////////////////////////////
// GADGETS
////////////////////////////////////////////////////////////////////////////

enum
{
    //////////////////////////////////////////////////////////////////////
    /////////////////////////////// PC ///////////////////////////////////
    //////////////////////////////////////////////////////////////////////
//GADGET_PC_SYS_GROUP_BEGIN,      // SYSTEM
//GADGET_PC_SYS_GROUP_END,

GADGET_PC_JLOC_GROUP_BEGIN,
// |------|--|------------
    GADGET_PC_JLOC_STICK0,
    GADGET_PC_JLOC_STICK1,
    GADGET_PC_JLOC_STICK2,
    GADGET_PC_JLOC_STICK3,
    GADGET_PC_JLOC_ROTATION0,
    GADGET_PC_JLOC_ROTATION1,
    GADGET_PC_JLOC_ROTATION2,
    GADGET_PC_JLOC_ROTATION3,
    GADGET_PC_JLOC_SLIDER0,
    GADGET_PC_JLOC_SLIDER1,
    GADGET_PC_JLOC_SLIDER2,
    GADGET_PC_JLOC_SLIDER3,
    GADGET_PC_JLOC_HAT0,
    GADGET_PC_JLOC_HAT1,
    GADGET_PC_JLOC_HAT2,
    GADGET_PC_JLOC_HAT3,
GADGET_PC_JLOC_GROUP_END,

GADGET_PC_JBTN_GROUP_BEGIN,     // JOYSTICK
// |------|--|--------
    GADGET_PC_JBTN_00,
    GADGET_PC_JBTN_01,
    GADGET_PC_JBTN_02,
    GADGET_PC_JBTN_03,
    GADGET_PC_JBTN_04,
    GADGET_PC_JBTN_05,
    GADGET_PC_JBTN_06,
    GADGET_PC_JBTN_07,
    GADGET_PC_JBTN_08,
    GADGET_PC_JBTN_09,
    GADGET_PC_JBTN_10,
    GADGET_PC_JBTN_11,
    GADGET_PC_JBTN_12,
    GADGET_PC_JBTN_13,
    GADGET_PC_JBTN_14,
    GADGET_PC_JBTN_15,
    GADGET_PC_JBTN_16,
    GADGET_PC_JBTN_17,
    GADGET_PC_JBTN_18,
    GADGET_PC_JBTN_19,
    GADGET_PC_JBTN_20,
    GADGET_PC_JBTN_21,
    GADGET_PC_JBTN_22,
    GADGET_PC_JBTN_23,
    GADGET_PC_JBTN_24,
    GADGET_PC_JBTN_25,
    GADGET_PC_JBTN_26,
    GADGET_PC_JBTN_27,
    GADGET_PC_JBTN_28,
    GADGET_PC_JBTN_29,
    GADGET_PC_JBTN_30,
    GADGET_PC_JBTN_31,
GADGET_PC_JBTN_GROUP_END,

GADGET_PC_MLOC_GROUP_BEGIN,
// |------|--|---------
    GADGET_PC_MLOC_XYZ,
GADGET_PC_MLOC_GROUP_END,

GADGET_PC_MBTN_GROUP_BEGIN,     // MOUSE
// |------|--|-------
    GADGET_PC_MBTN_0,
    GADGET_PC_MBTN_1,
    GADGET_PC_MBTN_2,
    GADGET_PC_MBTN_3,
    GADGET_PC_MBTN_4,
    GADGET_PC_MBTN_5,
    GADGET_PC_MBTN_6,
    GADGET_PC_MBTN_7,
GADGET_PC_MBTN_GROUP_END,

GADGET_PC_KBD_BUTTON_GROUP_BEGIN,   // KEYBOARD
// |------|--|-----------
    GADGET_PC_KBD_ESCAPE       =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x01,
    GADGET_PC_KBD_1            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x02,
    GADGET_PC_KBD_2            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x03,
    GADGET_PC_KBD_3            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x04,
    GADGET_PC_KBD_4            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x05,
    GADGET_PC_KBD_5            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x06,
    GADGET_PC_KBD_6            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x07,
    GADGET_PC_KBD_7            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x08,
    GADGET_PC_KBD_8            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x09,
    GADGET_PC_KBD_9            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x0A,
    GADGET_PC_KBD_0            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x0B,
    GADGET_PC_KBD_HYPHEN       =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x0C,   // - on main keyboard
    GADGET_PC_KBD_EQUALS       =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x0D,
    GADGET_PC_KBD_BACKSPACE    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x0E,   // backspace
    GADGET_PC_KBD_TAB          =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x0F,
    GADGET_PC_KBD_Q            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x10,
    GADGET_PC_KBD_W            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x11,
    GADGET_PC_KBD_E            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x12,
    GADGET_PC_KBD_R            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x13,
    GADGET_PC_KBD_T            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x14,
    GADGET_PC_KBD_Y            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x15,
    GADGET_PC_KBD_U            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x16,
    GADGET_PC_KBD_I            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x17,
    GADGET_PC_KBD_O            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x18,
    GADGET_PC_KBD_P            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x19,
    GADGET_PC_KBD_L_BRACKET    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x1A,
    GADGET_PC_KBD_R_BRACKET    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x1B,
    GADGET_PC_KBD_ENTER        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x1C,   // Enter on main keyboard
    GADGET_PC_KBD_L_CONTROL    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x1D,
    GADGET_PC_KBD_A            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x1E,
    GADGET_PC_KBD_S            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x1F,
    GADGET_PC_KBD_D            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x20,
    GADGET_PC_KBD_F            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x21,
    GADGET_PC_KBD_G            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x22,
    GADGET_PC_KBD_H            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x23,
    GADGET_PC_KBD_J            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x24,
    GADGET_PC_KBD_K            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x25,
    GADGET_PC_KBD_L            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x26,
    GADGET_PC_KBD_SEMICOLON    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x27,
    GADGET_PC_KBD_APOSTROPHE   =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x28,
    GADGET_PC_KBD_GRAVE        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x29,   // accent grave
    GADGET_PC_KBD_L_SHIFT      =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x2A,
    GADGET_PC_KBD_BACKSLASH    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x2B,
    GADGET_PC_KBD_Z            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x2C,
    GADGET_PC_KBD_X            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x2D,
    GADGET_PC_KBD_C            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x2E,
    GADGET_PC_KBD_V            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x2F,
    GADGET_PC_KBD_B            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x30,
    GADGET_PC_KBD_N            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x31,
    GADGET_PC_KBD_M            =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x32,
    GADGET_PC_KBD_COMMA        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x33,
    GADGET_PC_KBD_PERIOD       =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x34,   // . on main keyboard
    GADGET_PC_KBD_SLASH        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x35,   // / on main keyboard
    GADGET_PC_KBD_R_SHIFT      =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x36,
    GADGET_PC_KBD_NUM_ASTERISK =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x37,   // * on numeric keypad
    GADGET_PC_KBD_L_ALT        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x38,   // left Alt
    GADGET_PC_KBD_SPACE        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x39,
    GADGET_PC_KBD_CAPSLOCK     =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x3A,
    GADGET_PC_KBD_F1           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x3B,
    GADGET_PC_KBD_F2           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x3C,
    GADGET_PC_KBD_F3           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x3D,
    GADGET_PC_KBD_F4           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x3E,
    GADGET_PC_KBD_F5           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x3F,
    GADGET_PC_KBD_F6           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x40,
    GADGET_PC_KBD_F7           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x41,
    GADGET_PC_KBD_F8           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x42,
    GADGET_PC_KBD_F9           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x43,
    GADGET_PC_KBD_F10          =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x44,
    GADGET_PC_KBD_NUMLOCK      =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x45,
    GADGET_PC_KBD_SCROLL       =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x46,    // Scroll Lock
    GADGET_PC_KBD_NUM_7        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x47,
    GADGET_PC_KBD_NUM_8        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x48,
    GADGET_PC_KBD_NUM_9        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x49,
    GADGET_PC_KBD_NUM_HYPHEN   =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x4A,    // - on numeric keypad
    GADGET_PC_KBD_NUM_4        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x4B,
    GADGET_PC_KBD_NUM_5        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x4C,
    GADGET_PC_KBD_NUM_6        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x4D,
    GADGET_PC_KBD_NUM_PLUS     =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x4E,    // + on numeric keypad
    GADGET_PC_KBD_NUM_1        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x4F,
    GADGET_PC_KBD_NUM_2        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x50,
    GADGET_PC_KBD_NUM_3        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x51,
    GADGET_PC_KBD_NUM_0        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x52,
    GADGET_PC_KBD_NUM_PERIOD   =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x53,    // . on numeric keypad
    GADGET_PC_KBD_F11          =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x57,
    GADGET_PC_KBD_F12          =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x58,

    GADGET_PC_KBD_F13          =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x64,    //                     (NEC PC98)
    GADGET_PC_KBD_F14          =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x65,    //                     (NEC PC98)
    GADGET_PC_KBD_F15          =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x66,    //                     (NEC PC98)
    GADGET_PC_KBD_KANA         =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x70,    // (Japanese keyboard)
    GADGET_PC_KBD_CONVERT      =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x79,    // (Japanese keyboard)
    GADGET_PC_KBD_NOCONVERT    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x7B,    // (Japanese keyboard)
    GADGET_PC_KBD_YEN          =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x7D,    // (Japanese keyboard)
    GADGET_PC_KBD_NUM_EQUALS   =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x8D,    // = on numeric keypad (NEC PC98)
    GADGET_PC_KBD_CIRCUMFLEX   =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x90,    // (Japanese keyboard)
    GADGET_PC_KBD_AT           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x91,    //                     (NEC PC98)
    GADGET_PC_KBD_COLON        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x92,    //                     (NEC PC98)
    GADGET_PC_KBD_UNDERLINE    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x93,    //                     (NEC PC98)
    GADGET_PC_KBD_KANJI        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x94,    // (Japanese keyboard)
    GADGET_PC_KBD_STOP         =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x95,    //                     (NEC PC98)
    GADGET_PC_KBD_AX           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x96,    //                     (Japan AX)
    GADGET_PC_KBD_UNLABELED    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x97,    //                        (J3100)

    GADGET_PC_KBD_NUM_ENTER    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x9C,    // Enter on numeric keypad
    GADGET_PC_KBD_R_CONTROL    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0x9D,
    GADGET_PC_KBD_NUM_COMMA    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xB3,    // , on numeric keypad (NEC PC98)
    GADGET_PC_KBD_NUM_SLASH    =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xB5,    // / on numeric keypad
    GADGET_PC_KBD_SYSRQ        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xB7,
    GADGET_PC_KBD_R_ALT        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xB8,    // right Alt
    GADGET_PC_KBD_HOME         =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xC7,    // Home on arrow keypad
    GADGET_PC_KBD_UP           =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xC8,    // UpArrow on arrow keypad
    GADGET_PC_KBD_PG_UP        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xC9,    // PgUp on arrow keypad
    GADGET_PC_KBD_LEFT         =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xCB,    // LeftArrow on arrow keypad
    GADGET_PC_KBD_RIGHT        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xCD,    // RightArrow on arrow keypad
    GADGET_PC_KBD_END          =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xCF,    // End on arrow keypad
    GADGET_PC_KBD_DOWN         =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xD0,    // DownArrow on arrow keypad
    GADGET_PC_KBD_PG_DN        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xD1,    // PgDn on arrow keypad
    GADGET_PC_KBD_INSERT       =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xD2,    // Insert on arrow keypad
    GADGET_PC_KBD_DELETE       =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xD3,    // Delete on arrow keypad
    GADGET_PC_KBD_L_WIN        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xDB,    // Left Windows key
    GADGET_PC_KBD_R_WIN        =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xDC,    // Right Windows key
    GADGET_PC_KBD_APPS         =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN +   0xDD,    // AppMenu key

GADGET_PC_KBD_BUTTON_GROUP_END =   GADGET_PC_KBD_BUTTON_GROUP_BEGIN + 0x0100,    // Terminal sentinal

GADGET_PC_DONE,

    //////////////////////////////////////////////////////////////////////
    /////////////////////////////// PS2 //////////////////////////////////
    //////////////////////////////////////////////////////////////////////

//GADGET_PS2_SYS_GROUP_BEGIN,

//GADGET_PS2_SYS_GROUP_END,

GADGET_PS2_JOY_MESSAGE_GROUP_BEGIN,
// |------|---|----------
    GADGET_PS2_UNPLUG,                                                  // Joystick was unplugged
    GADGET_PS2_PLUGIN,                                                  // Joystick was plugged in

GADGET_PS2_JOY_MESSAGE_GROUP_END,

GADGET_PS2_JOY_TYPE_GROUP_BEGIN,
// |------|---|----------
    GADGET_PS2_UNKNOWN,                                                 // Something other than below
    GADGET_PS2_DUALSHOCK,                                               // Dualshock controller
    GADGET_PS2_DIGITAL,                                                 // Old digital controller
    GADGET_PS2_DUALSHOCK2,                                              // Dualshock2 controller
    GADGET_PS2_ANALOG,                                                  // Analog controller without actuators
    GADGET_PS2_DIGITAL_ACT,                                             // Digital with actuators

GADGET_PS2_JOY_TYPE_GROUP_END,

GADGET_PS2_JOY_BUTTON_GROUP_BEGIN,
// |------|---|-------

    GADGET_PS2_L2           = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN +  0,   // L2 Shoulder
    GADGET_PS2_R2           = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN +  1,   // R2 Shoulder
    GADGET_PS2_L1           = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN +  2,   // L1 Shoulder
    GADGET_PS2_R1           = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN +  3,   // R1 Shoulder

    GADGET_PS2_TRIANGLE     = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN +  4,   // Green  Triangle
    GADGET_PS2_CIRCLE       = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN +  5,   // Orange Circle
    GADGET_PS2_CROSS        = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN +  6,   // Blue   X
    GADGET_PS2_SQUARE       = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN +  7,   // Pink   Square

    GADGET_PS2_SELECT       = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN +  8,   // Select Button
    GADGET_PS2_L_BTN        = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN +  9,   // Left  Analog Button
    GADGET_PS2_R_BTN        = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN + 10,   // Right Analog Button
    GADGET_PS2_START        = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN + 11,   // Start Button

    GADGET_PS2_DPAD_U       = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN + 12,   // Left Pad Up
    GADGET_PS2_DPAD_R       = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN + 13,   // Left Pad Right
    GADGET_PS2_DPAD_D       = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN + 14,   // Left Pad Down
    GADGET_PS2_DPAD_L       = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN + 15,   // Left Pad Left

GADGET_PS2_JOY_BUTTON_GROUP_END,

GADGET_PS2_JOY_LOCATOR_GROUP_BEGIN,
// |------|---|------
    GADGET_PS2_L_STICK,
    GADGET_PS2_R_STICK,

    GADGET_PS2_ANALOG_DPAD_R,       // Begin - Pressure sensitive buttons
    GADGET_PS2_ANALOG_DPAD_L,
    GADGET_PS2_ANALOG_DPAD_U,
    GADGET_PS2_ANALOG_DPAD_D,

    GADGET_PS2_ANALOG_TRIANGLE,
    GADGET_PS2_ANALOG_CIRCLE,
    GADGET_PS2_ANALOG_CROSS,
    GADGET_PS2_ANALOG_SQUARE,

    GADGET_PS2_ANALOG_L1,
    GADGET_PS2_ANALOG_R1,
    GADGET_PS2_ANALOG_L2,
    GADGET_PS2_ANALOG_R2,           // End - Pressure sensitive buttons

GADGET_PS2_JOY_LOCATOR_GROUP_END,

GADGET_PS2_DONE,

    //////////////////////////////////////////////////////////////////////
    /////////////////////////////// GAMECUBE//////////////////////////////
    //////////////////////////////////////////////////////////////////////

GADGET_GAMECUBE_JOY_MESSAGE_GROUP_BEGIN,
// |------|---|----------
    GADGET_GAMECUBE_UNPLUG,                                                     // Joystick was unplugged
    GADGET_GAMECUBE_PLUGIN,                                                     // Joystick was plugged in

GADGET_GAMECUBE_JOY_MESSAGE_GROUP_END,

GADGET_GAMECUBE_JOY_TYPE_GROUP_BEGIN,
// |------|---|----------
    GADGET_GAMECUBE_UNKNOWN,                                                    // Something other than below
    GADGET_GAMECUBE_CONTROLLER,                                                 // regular controller

GADGET_GAMECUBE_JOY_TYPE_GROUP_END,

GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN,
// |------|---|-------

    // these MUST be in the same order as the bits in dolphin\pad.h
    GADGET_GAMECUBE_DPAD_L      = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN +  0,
    GADGET_GAMECUBE_DPAD_R      = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN +  1,
    GADGET_GAMECUBE_DPAD_D      = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN +  2,
    GADGET_GAMECUBE_DPAD_U      = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN +  3,

    GADGET_GAMECUBE_Z_TRIG      = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN +  4,
    GADGET_GAMECUBE_R_TRIG      = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN +  5,
    GADGET_GAMECUBE_L_TRIG      = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN +  6,

    GADGET_GAMECUBE_A           = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN +  8,
    GADGET_GAMECUBE_B           = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN +  9,
    GADGET_GAMECUBE_X           = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN + 10,
    GADGET_GAMECUBE_Y           = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN + 11,

    GADGET_GAMECUBE_MENU        = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN + 12,  // same as START
    GADGET_GAMECUBE_START       = GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN + 12,

GADGET_GAMECUBE_JOY_BUTTON_GROUP_END,

GADGET_GAMECUBE_JOY_LOCATOR_GROUP_BEGIN,
// |------|---|------
    GADGET_GAMECUBE_L_STICK,
    GADGET_GAMECUBE_R_STICK,

    GADGET_GAMECUBE_ANALOG_L_TRIG,
    GADGET_GAMECUBE_ANALOG_R_TRIG,

GADGET_GAMECUBE_JOY_LOCATOR_GROUP_END,

GADGET_GAMECUBE_DONE,

    //////////////////////////////////////////////////////////////////////
    ////////////////////////////// XBOX //////////////////////////////////
    //////////////////////////////////////////////////////////////////////

GADGET_XBOX_JOY_MESSAGE_GROUP_BEGIN,
// |------|---|----------
    GADGET_XBOX_UNPLUG,                                                     // Joystick was unplugged
    GADGET_XBOX_PLUGIN,                                                     // Joystick was plugged in

GADGET_XBOX_JOY_MESSAGE_GROUP_END,

GADGET_XBOX_JOY_TYPE_GROUP_BEGIN,
// |------|---|----------
    GADGET_XBOX_UNKNOWN,                                                    // Something other than below
    GADGET_XBOX_CONTROLLER,                                                 // regular controller

GADGET_XBOX_JOY_TYPE_GROUP_END,

GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN,
// |------|---|-------

    GADGET_XBOX_DPAD_U       = GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN +  0,     // Left Pad Up
    GADGET_XBOX_DPAD_D       = GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN +  1,     // Left Pad Down
    GADGET_XBOX_DPAD_L       = GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN +  2,     // Left Pad Left
    GADGET_XBOX_DPAD_R       = GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN +  3,     // Left Pad Right

    GADGET_XBOX_START        = GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN +  4,     // Start Button
    GADGET_XBOX_SELECT       = GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN +  5,     // Select Button

    GADGET_XBOX_L_STICK_BTN  = GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN +  6,     // Left stick button
    GADGET_XBOX_R_STICK_BTN  = GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN +  7,     // Right stick button

GADGET_XBOX_JOY_BUTTON_GROUP_END,

GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN,
// |------|---|-------
    GADGET_XBOX_ANALOG_A     = GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN +  0,     //
    GADGET_XBOX_ANALOG_B     = GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN +  1,     //
    GADGET_XBOX_ANALOG_X     = GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN +  2,     //
    GADGET_XBOX_ANALOG_Y     = GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN +  3,     //
    GADGET_XBOX_ANALOG_BLACK = GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN +  4,     //
    GADGET_XBOX_ANALOG_WHITE = GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN +  5,     //

    GADGET_XBOX_ANALOG_L     = GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN +  6,     // Left analog shoulder trigger
    GADGET_XBOX_ANALOG_R     = GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN +  7,     // Right analog shoulder trigger

GADGET_XBOX_JOY_ANALOG_GROUP_END,

GADGET_XBOX_JOY_LOCATOR_GROUP_BEGIN,
// |------|---|------
    GADGET_XBOX_L_STICK,
    GADGET_XBOX_R_STICK,

GADGET_XBOX_JOY_LOCATOR_GROUP_END,

GADGET_XBOX_DONE,

};


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// !!!! PRIVATE !!!! !!!! PRIVATE !!!! !!!! PRIVATE !!!! !!!! PRIVATE !!!!
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#undef PRESSED
#undef DEBOUNCE
#undef LOCATOR_X
#undef LOCATOR_Y
#undef LOCATOR_Z

#ifdef TARGET_PC
    #define   PRESSED( DeviceNum, GadgetID )    INPUT_GetButton  ( (s16)(DeviceNum), GADGET_PC_ ## GadgetID )
    #define  DEBOUNCE( DeviceNum, GadgetID )    INPUT_GetDebounce( (s16)(DeviceNum), GADGET_PC_ ## GadgetID )
    #define LOCATOR_X( DeviceNum, GadgetID )    INPUT_GetLocatorX( (s16)(DeviceNum), GADGET_PC_ ## GadgetID )
    #define LOCATOR_Y( DeviceNum, GadgetID )    INPUT_GetLocatorY( (s16)(DeviceNum), GADGET_PC_ ## GadgetID )
    #define LOCATOR_Z( DeviceNum, GadgetID )    INPUT_GetLocatorZ( (s16)(DeviceNum), GADGET_PC_ ## GadgetID )
#endif


#ifdef TARGET_PS2
    #define   PRESSED( DeviceNum, GadgetID )    INPUT_GetButton  ( (s16)(DeviceNum), GADGET_PS2_ ## GadgetID )
    #define  DEBOUNCE( DeviceNum, GadgetID )    INPUT_GetDebounce( (s16)(DeviceNum), GADGET_PS2_ ## GadgetID )
    #define LOCATOR_X( DeviceNum, GadgetID )    INPUT_GetLocatorX( (s16)(DeviceNum), GADGET_PS2_ ## GadgetID )
    #define LOCATOR_Y( DeviceNum, GadgetID )    INPUT_GetLocatorY( (s16)(DeviceNum), GADGET_PS2_ ## GadgetID )
    #define LOCATOR_Z( DeviceNum, GadgetID )    INPUT_GetLocatorZ( (s16)(DeviceNum), GADGET_PS2_ ## GadgetID )
#endif


#ifdef TARGET_DOLPHIN
    #define   PRESSED( DeviceNum, GadgetID )    INPUT_GetButton  ( (s16)(DeviceNum), GADGET_GAMECUBE_ ## GadgetID )
    #define  DEBOUNCE( DeviceNum, GadgetID )    INPUT_GetDebounce( (s16)(DeviceNum), GADGET_GAMECUBE_ ## GadgetID )
    #define LOCATOR_X( DeviceNum, GadgetID )    INPUT_GetLocatorX( (s16)(DeviceNum), GADGET_GAMECUBE_ ## GadgetID )
    #define LOCATOR_Y( DeviceNum, GadgetID )    INPUT_GetLocatorY( (s16)(DeviceNum), GADGET_GAMECUBE_ ## GadgetID )
    #define LOCATOR_Z( DeviceNum, GadgetID )    INPUT_GetLocatorZ( (s16)(DeviceNum), GADGET_GAMECUBE_ ## GadgetID )
#endif


#ifdef TARGET_XBOX
    #define   PRESSED( DeviceNum, GadgetID )    INPUT_GetButton  ( (s16)(DeviceNum), GADGET_XBOX_ ## GadgetID )
    #define  DEBOUNCE( DeviceNum, GadgetID )    INPUT_GetDebounce( (s16)(DeviceNum), GADGET_XBOX_ ## GadgetID )
    #define LOCATOR_X( DeviceNum, GadgetID )    INPUT_GetLocatorX( (s16)(DeviceNum), GADGET_XBOX_ ## GadgetID )
    #define LOCATOR_Y( DeviceNum, GadgetID )    INPUT_GetLocatorY( (s16)(DeviceNum), GADGET_XBOX_ ## GadgetID )
    #define LOCATOR_Z( DeviceNum, GadgetID )    INPUT_GetLocatorZ( (s16)(DeviceNum), GADGET_XBOX_ ## GadgetID )
#endif

////////////////////////////////////////////////////////////////////////////
// END
////////////////////////////////////////////////////////////////////////////

#endif // Q_INPUT_HPP
