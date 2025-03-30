////////////////////////////////////////////////////////////////////////////
//
// XBOX_Input.cpp
//
////////////////////////////////////////////////////////////////////////////

#include "x_files.hpp"

#include "Q_XBOX.hpp"
#include "Q_Input.hpp"
#include "E_InputQ.hpp"

#include "XBOX_Input.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define MAX_CONTROLLERS     4

#define ANALOG_BTN_COUNT    8

#define DEFAULT_DEADZONE    0.40f
#define DEFAULT_SATURATION  0.95f

#define ANALOG_BTN_THRESH   0.08f


////////////////////////////////////////////////////////////////////////////
// CONVERSION MACROS
////////////////////////////////////////////////////////////////////////////

#define STICK2FLOAT( x )        ( ((f32)(x)) / 32767.0f)
#define PRESSURE2FLOAT( x )     ( ((f32)(x)) / 255.0f)


////////////////////////////////////////////////////////////////////////////
// STRUCTS
////////////////////////////////////////////////////////////////////////////

struct XBGAMEPAD
{
    HANDLE              hDevice;    // Device handle for the gamepad
    XINPUT_STATE        xiState;    // State of the gamepad
    XINPUT_FEEDBACK     xiFeedback; // feedback parameters

    // Members that store the current and last frame input values
    u16     Buttons,  OldButtons;
    f32     LStickX,  OldLStickX;
    f32     LStickY,  OldLStickY;
    f32     RStickX,  OldRStickX;
    f32     RStickY,  OldRStickY;
    f32     Pressure[ANALOG_BTN_COUNT];
    f32     OldPressure[ANALOG_BTN_COUNT];

    // Members that store event status(i.e. from INPUT_GetNextEvent)
    f32     E_LStickX;
    f32     E_LStickY;
    f32     E_RStickX;
    f32     E_RStickY;
    f32     E_Pressure[ANALOG_BTN_COUNT];
    u16     E_AnalogBtnPressed;         // Non-analog button presses on analog buttons
    u16     E_AnalogBtnDebounce;        // debounce flags for analog buttons
    u16     E_BtnPressed;               // pressed flags for normal buttons
    u16     E_BtnDebounce;              // debounce flags for normal buttons

    // Rumble settings
    f64     RumbleStartTime;
    f64     RumbleDuration;
    f32     RumbleIntensity;
    xbool   RumblePaused;
};

////////////////////////////////////////////////////////////////////////////
// STATIC VARIABLES
////////////////////////////////////////////////////////////////////////////

static XBGAMEPAD    s_GamePads[MAX_CONTROLLERS];

static xbool        s_bInitialized = FALSE;
static s32          s_InputCycle   = 0;

static f32          s_InputDeadZone   = DEFAULT_DEADZONE;
static f32          s_InputSaturation = DEFAULT_SATURATION;


////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////

void Event_Button    ( s16 ControllerID, s32 ButtonID, xbool Pressed );
void Event_Stick     ( s16 ControllerID, s16 StickID, f32 X, f32 Y );
void Event_Pressure  ( s16 ControllerID, s16 StickID, f32 X );
void Event_Connect   ( s16 ControllerID );
void Event_Disconnect( s16 ControllerID );


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

static void XBOXINPUT_ResetController( s32 GamePadID )
{
    ASSERT( (GamePadID >= 0) && (GamePadID < MAX_CONTROLLERS) );

    x_memset( &s_GamePads[GamePadID], 0, sizeof(XBGAMEPAD) );
}

//==========================================================================

err INPUT_InitModule( void )
{
    ASSERT( s_bInitialized == FALSE );
    s_bInitialized = TRUE;

    // Initialize the input queue.
    INPUTQ_InitModule();

    s32   i;
    DWORD dwDeviceMask;

	 // Initialize XBOX DX Input
    XDEVICE_PREALLOC_TYPE DeviceTypes[] =
    {
       {XDEVICE_TYPE_GAMEPAD, 4},       // Game should handle up to 4 controllers plugged in.
#ifdef ENABLE_ASB_NETWORKING
	   {XDEVICE_TYPE_VOICE_MICROPHONE, 4},//for voice communicator (JeffP)
	   {XDEVICE_TYPE_VOICE_HEADPHONE, 4},//for voice communicator (JeffP)
#endif
    };

    // No need for DXWARN, it returns void
    XInitDevices( sizeof(DeviceTypes) / sizeof(XDEVICE_PREALLOC_TYPE), DeviceTypes );

    // Get a mask of all currently available devices
    dwDeviceMask = XGetDevices( XDEVICE_TYPE_GAMEPAD );

    // Open the devices
    for( i = 0; i < MAX_CONTROLLERS; i++ )
    {
        XBOXINPUT_ResetController( i );

        if( dwDeviceMask & (1 << i) ) 
        {
            s_GamePads[i].hDevice = XInputOpen( XDEVICE_TYPE_GAMEPAD, i, XDEVICE_NO_SLOT, NULL );

            x_printf( "PAD CONNECT: %d\n", i );

            Event_Connect( i );
        }
    }

    return ERR_SUCCESS;
}

//==========================================================================

void INPUT_KillModule( void )
{
    ASSERT( s_bInitialized );

    s_bInitialized = FALSE;

    // Shut down the input queue.
    INPUTQ_KillModule();
}

//==========================================================================

static f32 ClampRange( f32 X )
{
    if( (X <=  s_InputDeadZone) &&
        (X >= -s_InputDeadZone) )
    {
        return 0.0f;
    }

    if( X >= s_InputSaturation )
        return 1.0f;

    if( X <= -s_InputSaturation )
        return -1.0f;

    f32 Sign = (X >= 0.0f) ? 1.0f : -1.0f;

    X = ABS(X);

    return Sign * (X - s_InputDeadZone) / (s_InputSaturation - s_InputDeadZone);
}

//==========================================================================

void INPUT_SetDeadZone( f32 DeadZone )
{
    if( (DeadZone >= 0.0f) && (DeadZone <= 1.0f) )
        s_InputDeadZone = DeadZone;
}

//==========================================================================

f32 INPUT_GetDeadZone( void )
{
    return s_InputDeadZone;
}

//==========================================================================

void INPUT_SetSaturation( f32 Saturation )
{
    if( (Saturation >= 0.0f) && (Saturation <= 1.0f) )
        s_InputSaturation = Saturation;
}

//==========================================================================

f32 INPUT_GetSaturation( void )
{
    return s_InputSaturation;
}

//==========================================================================

static void XBOXINPUT_UpdateDevices( void )
{
    // Get status about gamepad insertions and removals. Note that, in order to
    // not miss devices, we will check for removed device BEFORE checking for
    // insertions
    DWORD dwInsertions;
    DWORD dwRemovals;

    XGetDeviceChanges( XDEVICE_TYPE_GAMEPAD, &dwInsertions, &dwRemovals );

    s32 i;

    // Loop through all gamepads
    for( i = 0; i < MAX_CONTROLLERS; i++ )
    {
        // Handle removed devices.
        if( dwRemovals & (1 << i) ) 
        {
            XInputClose( s_GamePads[i].hDevice );

            XBOXINPUT_ResetController( i );

            x_printf( "PAD DISCONNECT: %d\n", i );

            Event_Disconnect( i );
        }

        // Handle inserted devices
        if( dwInsertions & (1 << i) ) 
        {
            s_GamePads[i].hDevice = XInputOpen( XDEVICE_TYPE_GAMEPAD, i, XDEVICE_NO_SLOT, NULL );

            x_printf( "PAD CONNECT: %d\n", i );

            Event_Connect( i );
        }

        // If pad exists, update it's state
        if( s_GamePads[i].hDevice != NULL )
        {
            XInputGetState( s_GamePads[i].hDevice, (XINPUT_STATE*)&s_GamePads[i].xiState );
        }
    }
}

//==========================================================================

static void XBOXINPUT_UpdateControllerInfo( s32 GamePadID )
{
    s32 i;

    XBGAMEPAD*      pGamePad  = &s_GamePads[GamePadID];
    XINPUT_GAMEPAD* pXPadInfo = &s_GamePads[GamePadID].xiState.Gamepad;

    if( pGamePad->hDevice == NULL )
        return;

    // copy button information
    pGamePad->Buttons = pXPadInfo->wButtons;

    // Process the buttons for any changes
    if( pGamePad->Buttons != pGamePad->OldButtons )
    {
        u16 BtnChange = pGamePad->OldButtons ^ pGamePad->Buttons;
        s32 CurBtnBit = 0;

        // While there are delta bits still set, make an event.
        while( BtnChange != 0 )
        {
            if( BtnChange & 0x01 )
            {
                //--- button changed state, make an event.
                Event_Button( GamePadID, CurBtnBit + GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN, ((pGamePad->Buttons >> CurBtnBit) & 0x01) );
            }

            //--- move to next bit, clearing out bit that was just processed
            CurBtnBit++;
            BtnChange >>= 1;
        }
    }

    // convert stick/analog information, clamping them for deadzone and saturation
    pGamePad->LStickX = ClampRange( STICK2FLOAT( pXPadInfo->sThumbLX ) );
    pGamePad->LStickY = ClampRange( STICK2FLOAT( pXPadInfo->sThumbLY ) );
    pGamePad->RStickX = ClampRange( STICK2FLOAT( pXPadInfo->sThumbRX ) );
    pGamePad->RStickY = ClampRange( STICK2FLOAT( pXPadInfo->sThumbRY ) );

    // Look for deltas, if there's a change generate input events
    if( (pGamePad->OldLStickX != pGamePad->LStickX) ||
        (pGamePad->OldLStickY != pGamePad->LStickY) )
    {
        Event_Stick( GamePadID, GADGET_XBOX_L_STICK, pGamePad->LStickX, pGamePad->LStickY );
    }

    if( (pGamePad->OldRStickX != pGamePad->RStickX) ||
        (pGamePad->OldRStickY != pGamePad->RStickY) )
    {
        Event_Stick( GamePadID, GADGET_XBOX_R_STICK, pGamePad->RStickX, pGamePad->RStickY );
    }

    // Process Pressure buttons
    for( i = 0; i < ANALOG_BTN_COUNT; i++ )
    {
        // Clamp analog button values
        // (NOTE: May want to use different clamp function for analog btns, to get more input range than sticks)
        pGamePad->Pressure[i] = ClampRange( PRESSURE2FLOAT( pXPadInfo->bAnalogButtons[i] ) );

        // Test for delta, and generate button press as well as pressure info
        if( pGamePad->OldPressure[i] != pGamePad->Pressure[i] )
        {
            Event_Pressure( GamePadID, GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN + i, pGamePad->Pressure[i] );

            // If analog value crosses threshold, generate a button press or release event
            if( (pGamePad->OldPressure[i] < ANALOG_BTN_THRESH) && (pGamePad->Pressure[i] >= ANALOG_BTN_THRESH) )
            {
                Event_Button( GamePadID, GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN + i, TRUE );
            }
            else if( (pGamePad->OldPressure[i] >= ANALOG_BTN_THRESH) && (pGamePad->Pressure[i] < ANALOG_BTN_THRESH) )
            {
                Event_Button( GamePadID, GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN + i, FALSE );
            }
        }

        // save pressure info for next frame delta test
        pGamePad->OldPressure[i] = pGamePad->Pressure[i];
    }

    // save the input for next frame
    pGamePad->OldLStickX = pGamePad->LStickX;
    pGamePad->OldLStickY = pGamePad->LStickY;
    pGamePad->OldRStickX = pGamePad->RStickX;
    pGamePad->OldRStickY = pGamePad->RStickY;
    pGamePad->OldButtons = pGamePad->Buttons;
}

//==========================================================================

static void XBOXINPUT_UpdateRumble( s32 GamePadID )
{
    if( s_GamePads[GamePadID].RumblePaused )
        return;

    if( s_GamePads[GamePadID].RumbleDuration > 0.0f )
    {
        if( x_GetElapsedTime( s_GamePads[GamePadID].RumbleStartTime ) >= s_GamePads[GamePadID].RumbleDuration )
            INPUT_RumbleStop( GamePadID );
    }
}

//==========================================================================

void INPUT_CheckDevices( void )
{
    XBOXINPUT_UpdateDevices();

    s32 i;

    for( i = 0; i < MAX_CONTROLLERS; i++ )
    {
        XBOXINPUT_UpdateControllerInfo( i );
        XBOXINPUT_UpdateRumble( i );
    }
}

//==========================================================================

static void XBOXINPUT_SetEPadState( input_event& Event )
{
    ASSERT( (Event.DeviceNum >= 0) && (Event.DeviceNum < MAX_CONTROLLERS) );
    ASSERT( Event.DeviceType == DEVICE_XBOX_JOYSTICK );

    XBGAMEPAD* pGamePad = &s_GamePads[Event.DeviceNum];

    if( Event.GadgetType == GADGET_TYPE_BUTTON )
    {
        // Check if button press is from digital or analog button set
        if( Event.GadgetID >= GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN &&
            Event.GadgetID <  GADGET_XBOX_JOY_BUTTON_GROUP_END )
        {
            // Button press is from digital set
            u16 BtnBit = 1 << (Event.GadgetID - GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN);

            if( Event.Info == BUTTON_PRESS )
            {
                if( pGamePad->E_BtnPressed & BtnBit )
                    pGamePad->E_BtnDebounce &= ~BtnBit;         // Button pressed before, clear debounce
                else
                    pGamePad->E_BtnDebounce |= BtnBit;          // Button is going from non-pressed to pressed, set debounce

                pGamePad->E_BtnPressed |= BtnBit;               // Set button pressed
            }
            else
            {
                pGamePad->E_BtnDebounce &= ~BtnBit;             // clear debounce
                pGamePad->E_BtnPressed  &= ~BtnBit;             // clear pressed
            }
        }
        else if( Event.GadgetID >= GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN &&
                 Event.GadgetID <  GADGET_XBOX_JOY_ANALOG_GROUP_END )
        {
            // Button press is from analog set
            u16 BtnBit = 1 << (Event.GadgetID - GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN);

            if( Event.Info == BUTTON_PRESS )
            {
                if( pGamePad->E_AnalogBtnPressed & BtnBit )
                    pGamePad->E_AnalogBtnDebounce &= ~BtnBit;   // Button pressed before, clear debounce
                else
                    pGamePad->E_AnalogBtnDebounce |= BtnBit;    // Button is going from non-pressed to pressed, set debounce

                pGamePad->E_AnalogBtnPressed |= BtnBit;         // Set button pressed
            }
            else
            {
                pGamePad->E_AnalogBtnDebounce &= ~BtnBit;       // clear debounce
                pGamePad->E_AnalogBtnPressed  &= ~BtnBit;       // clear pressed
            }
        }
    }
    else if( Event.GadgetType == GADGET_TYPE_LOCATOR_XY )
    {
        ASSERT( Event.Info == LOCATOR_ABS );

        if( Event.GadgetID == GADGET_XBOX_L_STICK )
        {
            pGamePad->E_LStickX = Event.X;  
            pGamePad->E_LStickY = Event.Y;
        }
        else if( Event.GadgetID == GADGET_XBOX_R_STICK )
        {
            pGamePad->E_RStickX = Event.X;  
            pGamePad->E_RStickY = Event.Y;
        }
    }
    else if( Event.GadgetType == GADGET_TYPE_LOCATOR_X )
    {
        if( Event.GadgetID >= GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN &&
            Event.GadgetID <  GADGET_XBOX_JOY_ANALOG_GROUP_END )
        {
            pGamePad->E_Pressure[ Event.GadgetID - GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN ] = Event.X;
        }
    }
}

//==========================================================================

xbool INPUT_GetNextEvent( input_event& Event )  
{
    xbool bMessagesAvailable;

    // check if any events are in the input queue
    if( INPUTQ_GetNEvents() == 0 )
        return FALSE;

    // get the event
    bMessagesAvailable = INPUTQ_DeQueue( Event );

    // setup the pad status based on event input values
    XBOXINPUT_SetEPadState( Event );

    return bMessagesAvailable;
}

//==========================================================================

void INPUT_ClearDebounce( void )
{
    s32 i;

    for( i = 0; i < MAX_CONTROLLERS; i++ )
    {
        s_GamePads[i].E_BtnDebounce       = 0;
        s_GamePads[i].E_AnalogBtnDebounce = 0;
    }
}

//==========================================================================

xbool INPUT_GetButton( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( (DeviceNum >= 0) && (DeviceNum < MAX_CONTROLLERS) );
    ASSERT( GadgetID >= GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN );
    ASSERT( GadgetID < GADGET_XBOX_JOY_ANALOG_GROUP_END );

    s32        Button;
    XBGAMEPAD* pGamePad = &s_GamePads[ DeviceNum ];

    // Test if button requested is from analog set
    if( GadgetID >= GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN &&
        GadgetID <  GADGET_XBOX_JOY_ANALOG_GROUP_END )
    {
        Button = GadgetID - GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN;

        return ((pGamePad->E_AnalogBtnPressed >> Button) & 0x1) ? TRUE : FALSE;
    }

    // Test if button requested is from digital set
    if( GadgetID >= GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN &&
        GadgetID <  GADGET_XBOX_JOY_BUTTON_GROUP_END )
    {
        Button = GadgetID - GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN;

        return ((pGamePad->E_BtnPressed >> Button) & 0x1) ? TRUE : FALSE;
    }

    return FALSE;
}

//==========================================================================

xbool INPUT_GetDebounce( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( (DeviceNum >= 0) && (DeviceNum < MAX_CONTROLLERS) );
    ASSERT( GadgetID >= GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN );
    ASSERT( GadgetID < GADGET_XBOX_JOY_ANALOG_GROUP_END );

    s32        Button;
    XBGAMEPAD* pGamePad = &s_GamePads[ DeviceNum ];

    // Test if button requested is from analog set
    if( GadgetID >= GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN &&
        GadgetID <  GADGET_XBOX_JOY_ANALOG_GROUP_END )
    {
        Button = GadgetID - GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN;

        return ((pGamePad->E_AnalogBtnDebounce >> Button) & 0x1) ? TRUE : FALSE;
    }

    // Test if button requested is from digital set
    if( GadgetID >= GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN &&
        GadgetID <  GADGET_XBOX_JOY_BUTTON_GROUP_END )
    {
        Button = GadgetID - GADGET_XBOX_JOY_BUTTON_GROUP_BEGIN;

        return ((pGamePad->E_BtnDebounce >> Button) & 0x1) ? TRUE : FALSE;
    }

    return FALSE;
}

//==========================================================================

f32 INPUT_GetLocatorX( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( (DeviceNum >= 0) && (DeviceNum < MAX_CONTROLLERS) );

    XBGAMEPAD* pGamePad = &s_GamePads[ DeviceNum ];

    switch( GadgetID )
    {
        case GADGET_XBOX_L_STICK:  return pGamePad->E_LStickX;
        case GADGET_XBOX_R_STICK:  return pGamePad->E_RStickX;

        case GADGET_XBOX_ANALOG_L:
        case GADGET_XBOX_ANALOG_R:
        case GADGET_XBOX_ANALOG_A:
        case GADGET_XBOX_ANALOG_B:
        case GADGET_XBOX_ANALOG_X:
        case GADGET_XBOX_ANALOG_Y:
        case GADGET_XBOX_ANALOG_BLACK:
        case GADGET_XBOX_ANALOG_WHITE:
        {
            s32 Button = GadgetID - GADGET_XBOX_JOY_ANALOG_GROUP_BEGIN;
            return pGamePad->E_Pressure[ Button ];
        }

        default:
            //ASSERTS( FALSE, "Invalid GadgetID" );
            return 0.0f;
    }
}

//==========================================================================

f32 INPUT_GetLocatorY( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( (DeviceNum >= 0) && (DeviceNum < MAX_CONTROLLERS) );

    XBGAMEPAD* pGamePad = &s_GamePads[ DeviceNum ];

    switch( GadgetID )
    {
        case GADGET_XBOX_L_STICK:  return pGamePad->E_LStickY;
        case GADGET_XBOX_R_STICK:  return pGamePad->E_RStickY;

        default:
            //ASSERTS( FALSE, "Invalid GadgetID" );
            return 0.0f;
    }
}

//==========================================================================

f32 INPUT_GetLocatorZ( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( (DeviceNum >= 0) && (DeviceNum < MAX_CONTROLLERS) );

    return 0.0f;
}

//==========================================================================

static void XBOXINPUT_SetRumbleIntensity( XBGAMEPAD* pGamePad, f32 Intensity )
{
    #define L_MOTOR_START       0.35f   // Intensity value to start left(big) motor
    #define MOTOR_MIN_SPEED_L   0.25f   // Starting value for left motor(to get it turning)
    #define MOTOR_MIN_SPEED_R   0.18f   // Starting value for right motor(to get it turning)
    #define MOTOR_MAX_SPEED     0.85f   // Max motor rumble speed

    f32 LMotorIntensity;
    f32 LMotorSpeed;
    f32 RMotorSpeed;

    if( Intensity > 0.0f )
    {
        // Only use the left motor when the intensity is strong enough to spin it
        if( Intensity > L_MOTOR_START )
        {
            LMotorIntensity = (Intensity - L_MOTOR_START) / (1.0f - L_MOTOR_START);

            LMotorSpeed = MOTOR_MIN_SPEED_L + LMotorIntensity * (MOTOR_MAX_SPEED - MOTOR_MIN_SPEED_L);
        }
        else
        {
            LMotorSpeed = 0.0f;
        }

        RMotorSpeed = MOTOR_MIN_SPEED_R + Intensity * (MOTOR_MAX_SPEED - MOTOR_MIN_SPEED_R);
    }
    else
    {
        LMotorSpeed = 0.0f;
        RMotorSpeed = 0.0f;
    }

    pGamePad->xiFeedback.Rumble.wLeftMotorSpeed  = (WORD)( LMotorSpeed * 65535.0f );
    pGamePad->xiFeedback.Rumble.wRightMotorSpeed = (WORD)( RMotorSpeed * 65535.0f );

    XInputSetState( pGamePad->hDevice, &pGamePad->xiFeedback );
}

//==========================================================================

void INPUT_RumblePlay( s32 ControllerID, f32 Intensity, f32 Duration )
{
    ASSERT( ControllerID >= 0 );
    ASSERT( ControllerID < MAX_CONTROLLERS );

    XBGAMEPAD* pGamePad = &s_GamePads[ ControllerID ];

    if( pGamePad->hDevice == NULL )
        return;

    // Clamp the intensity and duration values
    if( Intensity < 0.0f )
        Intensity = 0.0f;
    else if( Intensity > 1.0f )
        Intensity = 1.0f;

    if( Duration < 0.0f )
        Duration = 0.0f;

    // If the intensity or duration time is zero, no rumble to execute
    if( (Intensity == 0.0f) || (Duration == 0.0f) )
    {
        INPUT_RumbleStop( ControllerID );
        return;
    }

    // Check to see if we are still transferring current motor value
    if( pGamePad->xiFeedback.Header.dwStatus != ERROR_IO_PENDING )
    {
        XBOXINPUT_SetRumbleIntensity( pGamePad, Intensity );

        pGamePad->RumbleStartTime = x_GetTime();
        pGamePad->RumbleDuration  = Duration;
        pGamePad->RumbleIntensity = Intensity;
        pGamePad->RumblePaused    = FALSE;
    }
}

//==========================================================================

void INPUT_RumbleStop( s32 ControllerID )
{
    ASSERT( ControllerID >= 0 );
    ASSERT( ControllerID < MAX_CONTROLLERS );

    XBGAMEPAD* pGamePad = &s_GamePads[ ControllerID ];

    if( pGamePad->hDevice == NULL )
        return;

    if( pGamePad->xiFeedback.Header.dwStatus != ERROR_IO_PENDING )
    {
        XBOXINPUT_SetRumbleIntensity( pGamePad, 0.0f );

        pGamePad->RumbleStartTime = 0;
        pGamePad->RumbleDuration  = 0;
        pGamePad->RumbleIntensity = 0.0f;
        pGamePad->RumblePaused    = FALSE;
    }
}

//==========================================================================

void INPUT_RumblePause( s32 ControllerID )
{
    ASSERT( ControllerID >= 0 );
    ASSERT( ControllerID < MAX_CONTROLLERS );

    XBGAMEPAD* pGamePad = &s_GamePads[ ControllerID ];

    if( pGamePad->RumbleIntensity <= 0.0f )
        return;

    if( pGamePad->RumblePaused )
        return;

    if( pGamePad->hDevice == NULL )
        return;

    if( pGamePad->xiFeedback.Header.dwStatus != ERROR_IO_PENDING )
    {
        XBOXINPUT_SetRumbleIntensity( pGamePad, 0.0f );

        pGamePad->RumblePaused    = TRUE;
        pGamePad->RumbleDuration -= x_GetElapsedTime( pGamePad->RumbleStartTime );
    }
}

//==========================================================================

void INPUT_RumbleResume( s32 ControllerID )
{
    ASSERT( ControllerID >= 0 );
    ASSERT( ControllerID < MAX_CONTROLLERS );

    XBGAMEPAD* pGamePad = &s_GamePads[ ControllerID ];

    if( pGamePad->RumbleIntensity <= 0.0f )
        return;

    if( !pGamePad->RumblePaused )
        return;

    if( pGamePad->hDevice == NULL )
        return;

    if( pGamePad->xiFeedback.Header.dwStatus != ERROR_IO_PENDING )
    {
        XBOXINPUT_SetRumbleIntensity( pGamePad, pGamePad->RumbleIntensity );

        pGamePad->RumblePaused    = FALSE;
        pGamePad->RumbleStartTime = x_GetTime();
    }
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//                                                                        //
//                         EVENT BUILDERS                                 //
//                                                                        //
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static void Event_Button( s16 ControllerID, s32 ButtonID, xbool Pressed )
{
    ASSERT( ControllerID >= 0 );
    ASSERT( ControllerID < MAX_CONTROLLERS );

    input_event Event;

    Event.DeviceType = DEVICE_XBOX_JOYSTICK;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_BUTTON;
    Event.GadgetID   = (s16)ButtonID;
    Event.Timestamp  = s_InputCycle;
    Event.Info       = (Pressed ? BUTTON_PRESS : BUTTON_RELEASE);
    Event.X          = 0;
    Event.Y          = 0;
    Event.Z          = 0;
    Event.Flags      = 0;

    INPUTQ_EnQueue( Event );
    s_InputCycle++;
}

////////////////////////////////////////////////////////////////////////////

static void Event_Stick( s16 ControllerID, s16 StickID, f32 X, f32 Y )
{
    ASSERT( ControllerID >= 0 );
    ASSERT( ControllerID < MAX_CONTROLLERS );

    input_event Event;

    Event.DeviceType = DEVICE_XBOX_JOYSTICK;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_LOCATOR_XY;
    Event.GadgetID   = StickID;
    Event.Timestamp  = s_InputCycle;
    Event.Info       = LOCATOR_ABS;
    Event.X          = X;
    Event.Y          = Y;
    Event.Z          = 0;
    Event.Flags      = 0;

    INPUTQ_EnQueue( Event );
    s_InputCycle++;
}

////////////////////////////////////////////////////////////////////////////

static void Event_Pressure( s16 ControllerID, s16 StickID, f32 X )
{
    ASSERT( ControllerID >= 0 );
    ASSERT( ControllerID < MAX_CONTROLLERS );

    input_event Event;

    Event.DeviceType = DEVICE_XBOX_JOYSTICK;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_LOCATOR_X;
    Event.GadgetID   = StickID;
    Event.Timestamp  = s_InputCycle;
    Event.Info       = LOCATOR_ABS;
    Event.X          = X;
    Event.Y          = 0;
    Event.Z          = 0;
    Event.Flags      = 0;

    INPUTQ_EnQueue( Event );
    s_InputCycle++;
}

////////////////////////////////////////////////////////////////////////////

static void Event_Connect( s16 ControllerID )
{
    ASSERT( ControllerID >= 0 );
    ASSERT( ControllerID < MAX_CONTROLLERS );

    input_event Event;

    Event.DeviceType = DEVICE_XBOX_JOYSTICK;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_MESSAGE;
    Event.GadgetID   = GADGET_XBOX_PLUGIN;
    Event.Timestamp  = s_InputCycle;
    Event.Info       = GADGET_XBOX_CONTROLLER;
    Event.X          = 0;
    Event.Y          = 0;
    Event.Z          = 0;
    Event.Flags      = 0;

    INPUTQ_EnQueue( Event );
    s_InputCycle++;
}

////////////////////////////////////////////////////////////////////////////

static void Event_Disconnect( s16 ControllerID )
{
    ASSERT( ControllerID >= 0 );
    ASSERT( ControllerID < MAX_CONTROLLERS );

    input_event Event;

    Event.DeviceType = DEVICE_XBOX_JOYSTICK;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_MESSAGE;
    Event.GadgetID   = GADGET_XBOX_UNPLUG;
    Event.Timestamp  = s_InputCycle;
    Event.Info       = 0;
    Event.X          = 0;
    Event.Y          = 0;
    Event.Z          = 0;
    Event.Flags      = 0;

    INPUTQ_EnQueue( Event );
    s_InputCycle++;
}

////////////////////////////////////////////////////////////////////////////
s32 INPUT_ReturnLowestConnectedController( void )
{
    s32 ControllerID;

    for( ControllerID = 0; ControllerID < MAX_CONTROLLERS; ControllerID++ )
    {
        if( s_GamePads[ControllerID].hDevice != NULL )
            return ControllerID;
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////
xbool INPUT_IsControllerConnected( s32 ControllerID )
{
    ASSERT( ControllerID >= 0 && ControllerID < MAX_CONTROLLERS );

    if( s_GamePads[ControllerID].hDevice != NULL )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
