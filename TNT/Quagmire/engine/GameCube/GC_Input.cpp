///////////////////////////////////////////////////////////////////////////
//
//  GC_INPUT.CPP
//
//
//
///////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_files.hpp"

#include "Q_Input.hpp"
#include "E_InputQ.hpp"

#include "GC_Video.hpp"
#include "GC_Input.hpp"

#include <dolphin/pad.h>


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

//#define ENABLE_INPUT_MONKEY

#define MONKEY_DELAY_TIME               (0.085f)
#define MONKEY_START_BUTTON_DELAY_TIME  (40.0f)
#define MONKEY_MAX_BUTTONS_PER_TURN     (3)

#define MAX_CONTROLLERS     4

//--------------------------------------------------------------------------

#define MIN_L_STICK_X       (-115)
#define MAX_L_STICK_X       (105)
#define MIN_L_STICK_Y       (-105)
#define MAX_L_STICK_Y       (115)

#define MIN_R_STICK_X       (-105)
#define MAX_R_STICK_X       (105)
#define MIN_R_STICK_Y       (-97)
#define MAX_R_STICK_Y       (105)

#define MAX_L_TRIG_ANALOG   (215)
#define MAX_R_TRIG_ANALOG   (223)


#define DEFAULT_DEADZONE    0.20f
#define DEFAULT_SATURATION  0.90f


////////////////////////////////////////////////////////////////////////////
// CONVERSION MACROS
////////////////////////////////////////////////////////////////////////////

#define L_STICK_X_2FLOAT(x) (((x) >= 0) ? ((f32)(x)/MAX_L_STICK_X) : (-(f32)(x)/MIN_L_STICK_X))
#define L_STICK_Y_2FLOAT(x) (((x) >= 0) ? ((f32)(x)/MAX_L_STICK_Y) : (-(f32)(x)/MIN_L_STICK_Y))

#define R_STICK_X_2FLOAT(x) (((x) >= 0) ? ((f32)(x)/MAX_R_STICK_X) : (-(f32)(x)/MIN_R_STICK_X))
#define R_STICK_Y_2FLOAT(x) (((x) >= 0) ? ((f32)(x)/MAX_R_STICK_Y) : (-(f32)(x)/MIN_R_STICK_Y))

#define L_TRIG_2FLOAT(x)    ((f32)(x) / MAX_L_TRIG_ANALOG)
#define R_TRIG_2FLOAT(x)    ((f32)(x) / MAX_R_TRIG_ANALOG)


////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////

struct GC_GAMEPAD
{
    // Values stored on pad read on current frame and last frame
    f32     LStickX,    LastLStickX;
    f32     LStickY,    LastLStickY;
    f32     RStickX,    LastRStickX;
    f32     RStickY,    LastRStickY;
    f32     LTrigger,   LastLTrigger;
    f32     RTrigger,   LastRTrigger;
    u16     Buttons,    LastButtons;

    xbool   Connected;          // is this controller connected
    s32     LastErr;            // last error code recieved from pad

    // Values set on each INPUT_GetNextEvent()
    f32     E_LStickX;
    f32     E_LStickY;
    f32     E_RStickX;
    f32     E_RStickY;
    f32     E_LTrigger;
    f32     E_RTrigger;
    u16     E_BtnPressed;
    u16     E_BtnDebounce;

    f32     RumbleIntensity;    // Rumble Intensity(0 to 1.0)
    f64     RumbleStartTime;    // Time value rumbling started on
    f64     RumbleDuration;     // Seconds to keep rumbling
    f64     RumbleRunTime;      // length of time rumble has lasted
    xbool   RumbleInProgress;   // Rumble state, on or off
    xbool   RumblePaused;       // is rumble paused
    xbool   RumbleRunning;      // is rumble currently running
};


////////////////////////////////////////////////////////////////////////////
// STATIC VARIABLES
////////////////////////////////////////////////////////////////////////////

static PADStatus    s_GamePads[PAD_MAX_CONTROLLERS];
static GC_GAMEPAD   s_PadStatus[PAD_MAX_CONTROLLERS];

static xbool        s_bInitialized    = FALSE;
static s32          s_InputCycle      = 0;

static f32          s_InputDeadZone   = DEFAULT_DEADZONE;
static f32          s_InputSaturation = DEFAULT_SATURATION;


////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////

static void Event_Button    ( s16 ControllerID, s32 ButtonBit, xbool Pressed );
static void Event_Stick     ( s16 ControllerID, s16 StickID, f32 X, f32 Y );
static void Event_Pressure  ( s16 ControllerID, s16 StickID, f32 X );
static void Event_Connect   ( s16 ControllerID );
static void Event_Disconnect( s16 ControllerID );


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

err INPUT_InitModule( void )
{
    ASSERT( s_bInitialized == FALSE );
    s_bInitialized = TRUE;

    //--- Initialize the input queue.
    INPUTQ_InitModule();

    //--- sets analog mode so there's 8 bits for each analog stick, 8 bits triggers
    PADSetAnalogMode( PAD_MODE_3 );

    PADInit();

    //--- this function must be called in case the reset button was pressed
    PADRecalibrate( PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT );

    //--- clear out data structures
    x_memset( s_PadStatus, 0, PAD_MAX_CONTROLLERS * sizeof(GC_GAMEPAD) );

    return ERR_SUCCESS;
}

//==========================================================================

void INPUT_KillModule( void )
{
    ASSERT( s_bInitialized != FALSE );
    s_bInitialized = FALSE;

    //--- Shut down the input queue.
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

static void GCINPUT_ResetPad( s32 Pad )
{
    ASSERT( (Pad >= 0) && (Pad < PAD_MAX_CONTROLLERS) );

    x_memset( &s_PadStatus[Pad], 0, sizeof(GC_GAMEPAD) );
}

//==========================================================================

static void GCINPUT_CheckPadStatus( void )
{
    u32 PadBit;
    u32 PadsToReset;
    u32 PadsConnected;
    s32 Pad;

    PadsToReset   = 0x00;
    PadsConnected = 0x00;

    //--- check controller connection status
    for( Pad = 0; Pad < PAD_MAX_CONTROLLERS; Pad++ )
    {
        PadBit = PAD_CHAN0_BIT >> Pad;

        if( s_GamePads[Pad].err == PAD_ERR_NONE )
        {
            //--- if this pad is connected, mark it as such
            PadsConnected |= PadBit;

            if( s_PadStatus[Pad].Connected == FALSE )
            {
                //--- controller has just been plugged in
                x_printf( "PAD CONNECT: %d\n", Pad );

                s_PadStatus[Pad].Connected = TRUE;
                Event_Connect( Pad );
            }
        }
        else if( s_GamePads[Pad].err == PAD_ERR_NO_CONTROLLER )
        {
            //--- if this pad is not present, we have to reset it's slot
            PadsToReset |= PadBit;

            if( s_PadStatus[Pad].Connected )
            {
                //--- controller has just been pulled out
                x_printf( "PAD DISCONNECT: %d\n", Pad );

                GCINPUT_ResetPad( Pad );

                s_PadStatus[Pad].Connected = FALSE;
                Event_Disconnect( Pad );
            }
        }
        else if( s_GamePads[Pad].err == PAD_ERR_NOT_READY )
        {
            //PadsToReset |= PadBit;
            //PadsConnected |= PadBit;
        }
        else if( s_GamePads[Pad].err == PAD_ERR_TRANSFER )
        {
            //PadsToReset |= PadBit;
            //PadsConnected |= PadBit;
        }

        s_PadStatus[Pad].LastErr = s_GamePads[Pad].err;
    }

    if( PadsToReset )
        PADReset( PadsToReset );
}

//==========================================================================

#ifdef ENABLE_INPUT_MONKEY

static void GCINPUT_Monkey( PADStatus* pPad )
{
    #define MONKEY_BUTTON_DOWN( BtnID )    (pPad->button &  (  1 << ((BtnID) - GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN)))
    #define SET_MONKEY_BUTTON( BtnID )     (pPad->button |= (  1 << ((BtnID) - GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN)))
    #define CLEAR_MONKEY_BUTTON( BtnID )   (pPad->button &= (~(1 << ((BtnID) - GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN))))

    static f32 MonkeyButtonCountDown           = 0.0f;
    static f32 MonkeyStartButtonCountDownDelay = 0.0f;
    s32        MonkeyButton;
    s32        TurnButtonCount;

    if( MonkeyButtonCountDown <= 0.0f )
    {
        s32 NPadsConnected = 0;
        s32 i;

        for( i = 0; i < PAD_MAX_CONTROLLERS; i++ )
            NPadsConnected += (s_PadStatus[i].Connected ? 1 : 0);

        //--- Take the number of joysticks * 60frames * time to get your delay.
        MonkeyButtonCountDown = (NPadsConnected * 60.0f) * MONKEY_DELAY_TIME;

        //--- Possible button events, let the monkey randomize
        for( TurnButtonCount = 0; TurnButtonCount < MONKEY_MAX_BUTTONS_PER_TURN; TurnButtonCount++ )
        {
            MonkeyButton = x_irand( GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN, GADGET_GAMECUBE_JOY_BUTTON_GROUP_END-1 );

            // Either set or clear the button.
            if( x_rand() & 0x1 )
                SET_MONKEY_BUTTON( MonkeyButton );
            else
                CLEAR_MONKEY_BUTTON( MonkeyButton );
        }

        //--- If you have counted down the entire start button delay, reset the timer and force the button to be hit.
        if( MonkeyStartButtonCountDownDelay <= 0.0f )
        {
            MonkeyStartButtonCountDownDelay = (NPadsConnected * 60.0f) * MONKEY_START_BUTTON_DELAY_TIME;
            SET_MONKEY_BUTTON( GADGET_GAMECUBE_START );
        }
        else
        {
            CLEAR_MONKEY_BUTTON( GADGET_GAMECUBE_START );
        }

        //--- Filter out some combos that will cause the game to supress asserts
        if( MONKEY_BUTTON_DOWN( GADGET_GAMECUBE_L_TRIG ) )
        {
            if( MONKEY_BUTTON_DOWN( GADGET_GAMECUBE_R_TRIG ) )
            {
                CLEAR_MONKEY_BUTTON( GADGET_GAMECUBE_A );   // To avoid assert Debug
                CLEAR_MONKEY_BUTTON( GADGET_GAMECUBE_B );   // To avoid assert Skip
                CLEAR_MONKEY_BUTTON( GADGET_GAMECUBE_Y );   // To avoid assert Supress
            }
        }

        //--- Let the monkey radomize possible values for analog input
        pPad->stickX       = x_irand( MIN_L_STICK_X, MAX_L_STICK_X );
        pPad->stickY       = x_irand( MIN_L_STICK_Y, MAX_L_STICK_Y );
        pPad->substickX    = x_irand( MIN_R_STICK_X, MAX_R_STICK_X );
        pPad->substickY    = x_irand( MIN_R_STICK_Y, MAX_R_STICK_Y );
        pPad->triggerLeft  = x_irand( 0, MAX_L_TRIG_ANALOG );
        pPad->triggerRight = x_irand( 0, MAX_R_TRIG_ANALOG );
    }
    else
    {
        MonkeyButtonCountDown           -= 1.0f;
        MonkeyStartButtonCountDownDelay -= 1.0f;
    }
}

#endif //ENABLE_INPUT_MONKEY

//==========================================================================

static void GCINPUT_UpdatePadInfo( s32 Pad )
{
    GC_GAMEPAD* pStatus;
    PADStatus*  pPad;

    pStatus = &s_PadStatus[Pad];
    pPad    = &s_GamePads[Pad];

    //--- if this controller isn't plugged in, nothing to do
    if( pStatus->Connected == FALSE )
        return;

    #ifdef ENABLE_INPUT_MONKEY
        GCINPUT_Monkey( pPad );
    #endif

    //--- copy button information
    pStatus->Buttons = pPad->button;

    //--- Process the buttons for any changes
    if( pStatus->Buttons != pStatus->LastButtons )
    {
        u16 BtnChange = pStatus->LastButtons ^ pStatus->Buttons;
        s32 CurBtnBit = 0;

        //--- While there are delta bits still set, make an event.
        while( BtnChange != 0 )
        {
            if( BtnChange & 0x01 )
            {
                //--- button changed state, make an event.
                Event_Button( Pad, CurBtnBit, ((pStatus->Buttons >> CurBtnBit) & 0x01) );
            }

            //--- move to next bit, clearing out bit that was just processed
            CurBtnBit++;
            BtnChange >>= 1;
        }
    }

    //--- convert stick/analog information, clamping them for deadzone and saturation
    pStatus->LStickX  = ClampRange( L_STICK_X_2FLOAT( pPad->stickX )       );
    pStatus->LStickY  = ClampRange( L_STICK_Y_2FLOAT( pPad->stickY )       );
    pStatus->RStickX  = ClampRange( R_STICK_X_2FLOAT( pPad->substickX )    );
    pStatus->RStickY  = ClampRange( R_STICK_Y_2FLOAT( pPad->substickY )    );
    pStatus->LTrigger = ClampRange(    L_TRIG_2FLOAT( pPad->triggerLeft )  );
    pStatus->RTrigger = ClampRange(    R_TRIG_2FLOAT( pPad->triggerRight ) );

    //--- Look for deltas, if there's a change generate input events
    if( (pStatus->LastLStickX != pStatus->LStickX) ||
        (pStatus->LastLStickY != pStatus->LStickY) )
    {
        Event_Stick( Pad, GADGET_GAMECUBE_L_STICK, pStatus->LStickX, pStatus->LStickY );
    }

    if( (pStatus->LastRStickX != pStatus->RStickX) ||
        (pStatus->LastRStickY != pStatus->RStickY) )
    {
        Event_Stick( Pad, GADGET_GAMECUBE_R_STICK, pStatus->RStickX, pStatus->RStickY );
    }

    if( pStatus->LastLTrigger != pStatus->LTrigger )
    {
        Event_Pressure( Pad, GADGET_GAMECUBE_ANALOG_L_TRIG, pStatus->LTrigger );
    }

    if( pStatus->LastRTrigger != pStatus->RTrigger )
    {
        Event_Pressure( Pad, GADGET_GAMECUBE_ANALOG_R_TRIG, pStatus->RTrigger );
    }

    //--- save the input for next frame
    pStatus->LastLStickX  = pStatus->LStickX;
    pStatus->LastLStickY  = pStatus->LStickY;
    pStatus->LastRStickX  = pStatus->RStickX;
    pStatus->LastRStickY  = pStatus->RStickY;
    pStatus->LastLTrigger = pStatus->LTrigger;
    pStatus->LastRTrigger = pStatus->RTrigger;
    pStatus->LastButtons  = pStatus->Buttons;
}

//==========================================================================

void GCINPUT_UpdateRumbleState( s32 ControllerID, f64 CurrentTime )
{
    GC_GAMEPAD* pGPad = &s_PadStatus[ ControllerID ];

    if( pGPad->Connected == FALSE )
        return;

    xbool StopRumble = FALSE;

    if( pGPad->RumbleIntensity > 0.0f )
    {
        //--- check if rumble has been paused
        if( pGPad->RumblePaused )
        {
            //--- if paused but currently running, stop the rumble
            if( pGPad->RumbleRunning != FALSE )
                StopRumble = TRUE;
        }
        else
        {
            #define RUMBLE_TIMER    0.04

            //--- normal rumble operation
            if( pGPad->RumbleRunning == FALSE )
            {
                //--- rumble is off, check if it should be turned on
                if( pGPad->RumbleRunTime <= 0 )
                {
                    //--- off timer has run out, turn rumble back on
                    PADControlMotor( ControllerID, PAD_MOTOR_RUMBLE );
                    pGPad->RumbleRunning = TRUE;

                    //--- reset timer
                    pGPad->RumbleRunTime = RUMBLE_TIMER;
                }
                else
                {
                    //--- rumble should stay off, decrement timer
                    pGPad->RumbleRunTime -= pGPad->RumbleIntensity * RUMBLE_TIMER;
                }
            }
            else
            {
                //--- rumble is on, check if it should stop
                if( pGPad->RumbleRunTime <= 0 )
                {
                    //--- stop timer has run out, quit rumble
                    StopRumble = TRUE;
                    pGPad->RumbleRunTime = RUMBLE_TIMER;
                }
                else
                {
                    //--- rumble still has time left, decrement the timer
                    pGPad->RumbleRunTime -= (1.0 - (f64)pGPad->RumbleIntensity) * RUMBLE_TIMER;
                }
            }

            //--- rumble duration check to quit the rumble operation
            if( x_GetTimeDiff( pGPad->RumbleStartTime, CurrentTime ) >= pGPad->RumbleDuration )
            {
                StopRumble              = TRUE;
                pGPad->RumblePaused     = FALSE;
                pGPad->RumbleIntensity  = 0.0f;
                pGPad->RumbleRunTime    = 0;
                pGPad->RumbleInProgress = FALSE;
            }
        }
    }
    else
    {
        //--- Stop rumble if intensity is zero still flagged as running
        if( pGPad->RumbleRunning )
            StopRumble = TRUE;
    }

    //--- check if logic needs the rumble motor to quit
    if( StopRumble )
    {
        //--- if the rumble motor stops for intensity, do a soft stop otherwise a hard stop
        if( pGPad->RumbleInProgress )
            PADControlMotor( ControllerID, PAD_MOTOR_STOP );
        else
            PADControlMotor( ControllerID, PAD_MOTOR_STOP_HARD );

        pGPad->RumbleRunning = FALSE;
    }
}

//==========================================================================

void INPUT_CheckDevices( void )
{
    s32 Pad;

    //--- read the pad data
    PADRead( s_GamePads );

    //--- check for controller plugin/unplug events
    GCINPUT_CheckPadStatus();

    f64 CurTime = x_GetTime();

    //--- process the controller info changes
    for( Pad = 0; Pad < PAD_MAX_CONTROLLERS; Pad++ )
    {
        GCINPUT_UpdatePadInfo( Pad );
        GCINPUT_UpdateRumbleState( Pad, CurTime );
    }
}

//==========================================================================

static void GCINPUT_SetEPadState( input_event& Event )
{
    ASSERT( (Event.DeviceNum >= 0) && (Event.DeviceNum < PAD_MAX_CONTROLLERS) );
    ASSERT( Event.DeviceType == DEVICE_GAMECUBE_JOYSTICK );

    GC_GAMEPAD* pGPad = &s_PadStatus[ Event.DeviceNum ];

    if( Event.GadgetType == GADGET_TYPE_BUTTON )
    {
        //--- Button press or release, mark the button as such
        u16 BtnBit  = 1 << (Event.GadgetID - GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN);

        if( Event.Info == BUTTON_PRESS )
        {
            if( pGPad->E_BtnPressed & BtnBit )
                pGPad->E_BtnDebounce &= ~BtnBit;    // button pressed before, clear debounce
            else
                pGPad->E_BtnDebounce |= BtnBit;     // button is going from non-pressed to pressed, set debounce

            pGPad->E_BtnPressed |= BtnBit;          // set button pressed
        }
        else
        {
            pGPad->E_BtnDebounce &= ~BtnBit;        // clear debounce
            pGPad->E_BtnPressed  &= ~BtnBit;        // clear pressed
        }
    }
    else if( Event.GadgetType == GADGET_TYPE_LOCATOR_XY )
    {
        //--- Stick locator event, set X and Y values
        ASSERT( Event.Info == LOCATOR_ABS );

        if( Event.GadgetID == GADGET_GAMECUBE_L_STICK )
        {
            pGPad->E_LStickX = Event.X;
            pGPad->E_LStickY = Event.Y;
        }
        else if( Event.GadgetID == GADGET_GAMECUBE_R_STICK )
        {
            pGPad->E_RStickX = Event.X;
            pGPad->E_RStickY = Event.Y;
        }
    }
    else if( Event.GadgetType == GADGET_TYPE_LOCATOR_X )
    {
        //--- single axis(analog btn) event
        ASSERT( Event.Info == LOCATOR_ABS );

        if( Event.GadgetID == GADGET_GAMECUBE_ANALOG_L_TRIG )
        {
            pGPad->E_LTrigger = Event.X;
        }
        else if( Event.GadgetID == GADGET_GAMECUBE_ANALOG_R_TRIG )
        {
            pGPad->E_RTrigger = Event.X;
        }
    }
    else if( Event.GadgetType == GADGET_TYPE_MESSAGE )
    {
        if( Event.GadgetID == GADGET_GAMECUBE_UNPLUG )
        {
            // nothing to do for now...
        }
        else if( Event.GadgetID == GADGET_GAMECUBE_PLUGIN )
        {
            // nothing to do for now...
        }
    }
}

//==========================================================================

xbool INPUT_GetNextEvent( input_event& Event )
{
    xbool bMessagesAvailable;

    //--- check if any events are in the input queue
    if( INPUTQ_GetNEvents() == 0 )
        return FALSE;

    //--- get the event
    bMessagesAvailable = INPUTQ_DeQueue( Event );

    //--- setup the pad status based on event input values
    GCINPUT_SetEPadState( Event );

    return bMessagesAvailable;
}

//==========================================================================

void INPUT_ClearDebounce( void )
{
    s32 Pad;

    for( Pad = 0; Pad < PAD_MAX_CONTROLLERS; Pad++ )
        s_PadStatus[Pad].E_BtnDebounce = 0;
}


//==========================================================================
xbool INPUT_IsControllerConnected( s32 ControllerID )
{
    ASSERT( ControllerID >= 0 && ControllerID < MAX_CONTROLLERS );
    return s_PadStatus[ControllerID].Connected;
}


//==========================================================================
s32 INPUT_ReturnLowestConnectedController( void )
{
    s32 ControllerID;

    for( ControllerID = 0; ControllerID < PAD_MAX_CONTROLLERS; ControllerID++ )
    {
        if( s_PadStatus[ControllerID].Connected )
            return ControllerID;
    }

    return -1;
}

//==========================================================================

xbool INPUT_GetButton( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( (DeviceNum >= 0) && (DeviceNum < PAD_MAX_CONTROLLERS) );
    ASSERT( GadgetID >= GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN );
    ASSERT( GadgetID < GADGET_GAMECUBE_JOY_BUTTON_GROUP_END );

    GC_GAMEPAD* pGPad = &s_PadStatus[ DeviceNum ];

    s32 Button = GadgetID - GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN;

    return ((pGPad->E_BtnPressed >> Button) & 0x1) ? TRUE : FALSE;
}

//==========================================================================

xbool INPUT_GetDebounce( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( (DeviceNum >= 0) && (DeviceNum < PAD_MAX_CONTROLLERS) );
    ASSERT( GadgetID >= GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN );
    ASSERT( GadgetID < GADGET_GAMECUBE_JOY_BUTTON_GROUP_END );

    GC_GAMEPAD* pGPad = &s_PadStatus[ DeviceNum ];

    s32 Button = GadgetID - GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN;

    return ((pGPad->E_BtnDebounce >> Button) & 0x1) ? TRUE : FALSE;
}

//==========================================================================

f32 INPUT_GetLocatorX( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( (DeviceNum >= 0) && (DeviceNum < PAD_MAX_CONTROLLERS) );

    GC_GAMEPAD* pGPad = &s_PadStatus[ DeviceNum ];

    switch( GadgetID )
    {
        case GADGET_GAMECUBE_L_STICK:       return pGPad->E_LStickX;
        case GADGET_GAMECUBE_R_STICK:       return pGPad->E_RStickX;
        case GADGET_GAMECUBE_ANALOG_L_TRIG: return pGPad->E_LTrigger;
        case GADGET_GAMECUBE_ANALOG_R_TRIG: return pGPad->E_RTrigger;

        default:
            ASSERTS( FALSE, "Invalid GadgetID" );
            return 0.0f;
    }
}

//==========================================================================

f32 INPUT_GetLocatorY( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( (DeviceNum >= 0) && (DeviceNum < PAD_MAX_CONTROLLERS) );

    GC_GAMEPAD* pGPad = &s_PadStatus[ DeviceNum ];

    switch( GadgetID )
    {
        case GADGET_GAMECUBE_L_STICK:       return pGPad->E_LStickY;
        case GADGET_GAMECUBE_R_STICK:       return pGPad->E_RStickY;

        default:
            ASSERTS( FALSE, "Invalid GadgetID" );
            return 0.0f;
    }
}

//==========================================================================

f32 INPUT_GetLocatorZ( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( (DeviceNum >= 0) && (DeviceNum < PAD_MAX_CONTROLLERS) );

    //--- no Z-axis on GameCube controller
    return 0.0f;
}

//==========================================================================

void INPUT_RumblePlay( s32 ControllerID, f32 Intensity, f32 Duration )
{
    ASSERT( (ControllerID >= 0) && (ControllerID < PAD_MAX_CONTROLLERS) );

    GC_GAMEPAD* pGPad = &s_PadStatus[ControllerID];

    if( Intensity < 0.0f )
        Intensity = 0.0f;
    else if( Intensity > 1.0f )
        Intensity = 1.0f;

    if( Duration < 0.0f )
        Duration = 0.0f;

    if( !pGPad->RumbleInProgress )
        pGPad->RumbleRunTime = 0;

    pGPad->RumbleIntensity  = Intensity;
    pGPad->RumblePaused     = FALSE;
    pGPad->RumbleStartTime  = x_GetTime();
    pGPad->RumbleDuration   = Duration;
    pGPad->RumbleInProgress = TRUE;
}

//==========================================================================

void INPUT_RumbleStop( s32 ControllerID )
{
    ASSERT( (ControllerID >= 0) && (ControllerID < PAD_MAX_CONTROLLERS) );

    GC_GAMEPAD* pGPad = &s_PadStatus[ControllerID];

    pGPad->RumbleIntensity  = 0.0f;
    pGPad->RumblePaused     = FALSE;
    pGPad->RumbleStartTime  = 0;
    pGPad->RumbleDuration   = 0;
    pGPad->RumbleRunTime    = 0;
    pGPad->RumbleInProgress = FALSE;
}

//==========================================================================

void INPUT_RumblePause( s32 ControllerID )
{
    ASSERT( (ControllerID >= 0) && (ControllerID < PAD_MAX_CONTROLLERS) );

    GC_GAMEPAD* pGPad = &s_PadStatus[ControllerID];

    if( pGPad->RumbleIntensity > 0 )
    {
        if( pGPad->RumblePaused == FALSE )
        {
            pGPad->RumblePaused    = TRUE;
            pGPad->RumbleDuration -= x_GetElapsedTime( pGPad->RumbleStartTime );
        }
    }
}

//==========================================================================

void INPUT_RumbleResume( s32 ControllerID )
{
    ASSERT( (ControllerID >= 0) && (ControllerID < PAD_MAX_CONTROLLERS) );

    GC_GAMEPAD* pGPad = &s_PadStatus[ControllerID];

    if( pGPad->RumbleIntensity > 0 )
    {
        if( pGPad->RumblePaused )
        {
            pGPad->RumblePaused    = FALSE;
            pGPad->RumbleRunning   = FALSE;
            pGPad->RumbleStartTime = x_GetTime();
            pGPad->RumbleRunTime   = 0;
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                         EVENT BUILDERS                                //
//                                                                       //
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static void Event_Button( s16 ControllerID, s32 ButtonBit, xbool Pressed )
{
    ASSERT( ControllerID >= 0 );
    ASSERT( ControllerID < PAD_MAX_CONTROLLERS );

    input_event Event;
    s16         ButtonID;

    ButtonID = ButtonBit + GADGET_GAMECUBE_JOY_BUTTON_GROUP_BEGIN;

    Event.DeviceType = DEVICE_GAMECUBE_JOYSTICK;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_BUTTON;
    Event.GadgetID   = ButtonID;
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
    ASSERT( ControllerID < PAD_MAX_CONTROLLERS );

    input_event Event;

    Event.DeviceType = DEVICE_GAMECUBE_JOYSTICK;
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
    ASSERT( ControllerID < PAD_MAX_CONTROLLERS );

    input_event Event;

    Event.DeviceType = DEVICE_GAMECUBE_JOYSTICK;
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
    ASSERT( ControllerID < PAD_MAX_CONTROLLERS );

    input_event Event;

    Event.DeviceType = DEVICE_GAMECUBE_JOYSTICK;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_MESSAGE;
    Event.GadgetID   = GADGET_GAMECUBE_PLUGIN;
    Event.Timestamp  = s_InputCycle;
    Event.Info       = GADGET_GAMECUBE_CONTROLLER;
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
    ASSERT( ControllerID < PAD_MAX_CONTROLLERS );

    input_event Event;

    Event.DeviceType = DEVICE_GAMECUBE_JOYSTICK;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_MESSAGE;
    Event.GadgetID   = GADGET_GAMECUBE_UNPLUG;
    Event.Timestamp  = s_InputCycle;
    Event.Info       = 0;
    Event.X          = 0;
    Event.Y          = 0;
    Event.Z          = 0;
    Event.Flags      = 0;

    INPUTQ_EnQueue( Event );
    s_InputCycle++;
}