///////////////////////////////////////////////////////////////////////////
//
//  PS2INPUT.C
//
//  IMPORTANT NOTES
//  ---------------
//  Multitap and memcard initialization have been commented out in the
//  INPUT_InitModule function.  Something in the 2.0 sony libs is causing
//  the multitap initialization function to never return.
//
//
///////////////////////////////////////////////////////////////////////////
//
//  Nomenclature:
//
//  Port = A physical connector on the PS2.  There are 2 ports on the PS2
//         The ports are numbered 0 and 1.
//
//  Slot = A 'position' where a controller can be connected.  There are 8
//         available slots.  Each port has 4 available slots.  Slot 0 & 4
//         are the physical connections on the PS2, -OR- the 'A' position 
//         on a multitap.  Slots 1-3 and 5-7 are the 'B','C' and 'D'
//         positions on a multitap.
//
//  Numbering = Port/Slot combos will be referred to from here on as 
//              0A,0B,0C,0D,1A,1B,1C,1D, where 0A is port 0 slot 0,
//              and 1A is port1 slot 0.
//
//  Rules of operation:
//
//  A controller will not be validated and initialized if a lower 'root'
//  slot is not in use.  Ex: If a controller is plugged into 0B, and there
//  is nothing connected in 0A, the connection will not follow through. 
//  The controller will remain unusable until something is plugged into 0A.
//  The same goes for the 1BCD slots.  
//
//  Anything connected to port 1 will be ignored if no controller is 
//  connected to 0A.
//
//  No gaps are allowed in multi-tap connections.  0D cannot be used if
//  even 1 of 0A-thru-0C are not in use.
//
//
///////////////////////////////////////////////////////////////////////////
//
//  INPUT SYSTEM CONTROLS
//
///////////////////////////////////////////////////////////////////////////

//#define LET_LOOSE_THE_MONKEYS

#ifdef LET_LOOSE_THE_MONKEYS
#define MONKEY_DELAY_TIME               (0.33f)
#define MONKEY_START_BUTTON_DELAY_TIME  (15.0f)
#define MONKEY_MAX_BUTTONS_PER_TURN     (2)
#define IS_MONKEY_BUTTON_DOWN( GadgetButtonID ) (Cur->Buttons &  (  1 << ((GadgetButtonID) - GADGET_PS2_JOY_BUTTON_GROUP_BEGIN)))
#define SET_MONKEY_BUTTON( GadgetButtonID )     (Cur->Buttons |= (  1 << ((GadgetButtonID) - GADGET_PS2_JOY_BUTTON_GROUP_BEGIN)))
#define CLEAR_MONKEY_BUTTON( GadgetButtonID )   (Cur->Buttons &= (~(1 << ((GadgetButtonID) - GADGET_PS2_JOY_BUTTON_GROUP_BEGIN))))

#endif

///////////////////////////////////////////////////////////////////////////
//  INCLUDES
///////////////////////////////////////////////////////////////////////////

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sifdev.h>
#include <sifrpc.h>
#include <libvu0.h>
#include <libpad.h>
#include <libcdvd.h>
#include <libmtap.h>
#include <libmc.h>

#include "x_plus.hpp"
#include "x_stdio.hpp"
#include "x_time.hpp"
#include "x_debug.hpp"
#include "Q_Input.hpp"
#include "PS2_Input.hpp"
#include "E_InputQ.hpp"
#include "Q_PS2.hpp"

#include "x_memory.hpp"
 

//=====================================
// Include PS2CON_USEDEBUGTEXT to 
// display basic controller info
// to the screen using the x_printfxy
// and x_printf functions
//=====================================

//#define PS2CON_USEDEBUGTEXT

#ifdef PS2CON_USEDEBUGTEXT
#include "x_stdio.hpp"
#define PS2CON_PRINT(a)              x_printf(a);
#else
#define PS2CON_PRINT(a)
#endif


///////////////////////////////////////////////////////////////////////////
//
//  INTERNAL CONTROLLER NAMING  
//
///////////////////////////////////////////////////////////////////////////
#define PS2CON_TYPE_NONE        0
#define PS2CON_TYPE_DIGITAL     1
#define PS2CON_TYPE_ANALOG      2
#define PS2CON_TYPE_DUALSHOCK   3
#define PS2CON_TYPE_DUALSHOCK2  4
#define PS2CON_TYPE_DIGITAL_ACT 5


const char* PS2CON_NAME[] = { "--NA--","DIGITAL","ANALOG","DSHOCK","DSHCK-2", "DIG-AC" };
   
enum
{
    PS2CON_PHASE_CONNECTING = 1,
    PS2CON_PHASE_INIT_DIGITAL,
    PS2CON_PHASE_INIT_ANALOG,
    PS2CON_PHASE_INIT_ANALOG_WAIT_ANALOG_STABLE,
    PS2CON_PHASE_INIT_DIGITAL_TEST_ACTUATORS,
    PS2CON_PHASE_INIT_DIGITAL_WAIT_ACTUATORS_STABLE,
    PS2CON_PHASE_INIT_ANALOG_TEST_ACTUATORS,
    PS2CON_PHASE_INIT_ANALOG_WAIT_ACTUATORS_STABLE,
    PS2CON_PHASE_INIT_ANALOG_TEST_PRESSMODE,
    PS2CON_PHASE_INIT_ANALOG_WAIT_PRESSMODE_STABLE,
    PS2CON_PHASE_WAIT_FOR_STABLE,
    PS2CON_PHASE_ACTIVE,
};

///////////////////////////////////////////////////////////////////////////
//
//  MAPPING OF BUTTON ID TO INPUT SUBSYSTEM GADGET ID
//
///////////////////////////////////////////////////////////////////////////
static const s32 PressureIDMap[] = { 
                                        GADGET_PS2_ANALOG_DPAD_R,
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
                                        GADGET_PS2_ANALOG_R2,
                                    };


///////////////////////////////////////////////////////////////////////////
//
//  CONVERSION MACROS
//
///////////////////////////////////////////////////////////////////////////
#define PS2STICKINT2FLOAT(x)    ((((f32)(x)) - 128.0f) / 128.0f)
#define PS2PRESSURE2FLOAT(x)    (((f32)(x)) / 255.0f)



///////////////////////////////////////////////////////////////////////////
//
//  STRUCTURES
//
///////////////////////////////////////////////////////////////////////////

//==============================================
//  Info required for each pad's input status
//==============================================
typedef struct
{
    u16         Buttons;
    u8          ActDirect[6];
    u8          ActAlign[6];
    f32         LStickX;
    f32         LStickY;
    f32         RStickX;
    f32         RStickY;
    f32         Pressure[12];       // Pressure sensors
}ps2pad_info;

#define PS2PAD_HAS_ACTUATORS        ( 1 <<  0 )
#define PS2PAD_HAS_ANALOG           ( 1 <<  1 )
#define PS2PAD_HAS_PRESSMODE        ( 1 <<  2 )
#define PS2PAD_PRESSMODE_ON         ( 1 <<  3 )     // Press mode activated
#define PS2PAD_PRESSMODE_CHANGE_REQ ( 1 <<  4 )     // Pressmode change request issued
#define PS2PAD_PRESSMODE_REQ_ON     ( 1 <<  5 )     // Request pressmode change to on
#define PS2PAD_ANALOG_CHANGE_REQ    ( 1 <<  6 )     // Analog mode change request issued
#define PS2PAD_ANALOG_REQ_ON        ( 1 <<  7 )     // Request analog change to on
#define PS2PAD_STABLE               ( 1 <<  8 )     // Controller is useable
#define PS2PAD_DISCONNECTING        ( 1 <<  9 )     // Unplug event is in the queue


//==============================================
//  PS2 Pad structure.  Holds connectivity
//  information, as well as the current status
//  of the controller gadgets (updated as the
//  user polls GetNextEvent)
//==============================================
typedef struct
{
    u_long128   DMABuffer[scePadDmaBufferMax] __attribute__ ((aligned(64)));
    s32         Port;                               // Port [0,1]
    s32         Slot;                               // Slot in port [0,3]
    s32         Type;                               // PS2CON_TYPE_xxxxxx
    s32         TermID;
    s32         RTermID;
    s32         Phase;                              // Used by state machine
    u16         Flags;                              // Bitwise flags
    ps2pad_info CurrentInfo;                        // Info this frame
    ps2pad_info LastInfo;                           // Info from last frame
    u8          ActDirect[6];
    u8          ActAlign[6];

    u8          RumbleIntensity;                    // Rumble Intensity [0..255]
	u8			ConnectedViaMultitap;				// Connected through a multitap
    f64         RumbleStartTime;                    // 
    f64         RumbleDuration;                     // 
    xbool       RumblePaused;
    xbool       RumbleRunning;                      // Rumble triggered and running
    // The following vars are updated when the user queries the input
    // system via GetNextEvent...
    u16         EPressed;
    u16         EDebounced;
    f32         ELStickX;
    f32         ELStickY;
    f32         ERStickX;
    f32         ERStickY;
    f32         EPressure[12];
    s32         LogicalID;                  // Numeric ID [0,7] that this device
                                            // is mapped to.
    xbool       OfficiallyConnected;        // TRUE if a CONNECT event has
                                            // been processed for this device
    xbool       Attached;                   // TRUE if a CONNECT event is in the queue.
    xbool       IsBeingScanned;             // TRUE if the controller is being scanned.
}ps2pad_slot;

///////////////////////////////////////////////////////////////////////////
// STORAGE
///////////////////////////////////////////////////////////////////////////

static  xbool           Initialized = FALSE;
static  s32             Cycle       = 0;
static  ps2pad_slot     PS2Pad[8];
static  s32             IDToSlotMap[8];         // Maps ID's to slots
static  s32             NConnected;
static  f32             Deadzone;
static  f32             Saturation;

///////////////////////////////////////////////////////////////////////////
// REQUIRED PROTOTYPES
///////////////////////////////////////////////////////////////////////////

static void Event_Disconnect( s16 DeviceType, s16 ControllerID );

void  PS2_SetAnalogMode		( ps2pad_slot* pPad, xbool On	);
s32	  PS2_GetAnalogMode		( ps2pad_slot* pPad				);
void  PS2_LockAnalogToggle	( ps2pad_slot* pPad, xbool Lock );
void  PS2_SetPressMode		( ps2pad_slot* pPad, xbool On	);


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void ResetPad( ps2pad_slot* Pad )
{
    //if (Pad->LogicalID >= 0)
    //    IDToSlotMap[ Pad->LogicalID ] = -1;

    Pad->EDebounced = 0;
    Pad->ELStickX = 0;
    Pad->ELStickY = 0;
    Pad->EPressed = 0;
    Pad->ERStickX = 0;
    Pad->ERStickY = 0;
    Pad->LogicalID= -1;
    Pad->Type     = PS2CON_TYPE_NONE;
    Pad->Phase    = PS2CON_PHASE_CONNECTING;
    Pad->Flags    = 0;
    Pad->RumbleIntensity    = 0;
    Pad->RumblePaused       = FALSE;
    Pad->RumbleRunning      = FALSE;
    Pad->OfficiallyConnected = FALSE;
    Pad->Attached = FALSE;
    Pad->IsBeingScanned = FALSE;

    s32 i;
    for (i=0;i<12;i++)
        Pad->EPressure[i] = 0;
}

///////////////////////////////////////////////////////////////////////////

xbool INPUT_GetNextEvent( input_event& Event )
{
    xbool           Result;
    s32             ID;
    ps2pad_slot*    Pad;

    ASSERT( Initialized );

    if (INPUTQ_GetNEvents() == 0)
        return FALSE;

    Result = INPUTQ_DeQueue( Event );

    if( Result )
    {
        //
        // Update the device tracking variables.
        //
        ID = Event.DeviceNum;

        ASSERT( ID>=0 );
        ASSERT( ID< 8 );

        Pad = &PS2Pad[ ID ];

        Event.DeviceNum = Pad->LogicalID;
            
        // Button activity?
        if( Event.GadgetType == GADGET_TYPE_BUTTON )
        {
            s32  BitPos;
            u32  BitMask;

            BitPos  = Event.GadgetID - GADGET_PS2_JOY_BUTTON_GROUP_BEGIN;
            BitMask = 1 << BitPos;

            if( Event.Info == BUTTON_PRESS )
            {
                Pad->EPressed   |= BitMask;
                Pad->EDebounced |= BitMask;
            }
            else
            {
                Pad->EPressed &= ~BitMask;
            }
        }

        else

        // Stick movement?
        if( Event.GadgetType == GADGET_TYPE_LOCATOR_XY )
        {
            ASSERT( Event.Info == LOCATOR_ABS );
            if ( Event.GadgetID == GADGET_PS2_L_STICK )
            {
                Pad->ELStickX = Event.X;
                Pad->ELStickY = Event.Y;
            }
            else if ( Event.GadgetID == GADGET_PS2_R_STICK )
            {
                Pad->ERStickX = Event.X;
                Pad->ERStickY = Event.Y;
            }
        }

        else

        // Pressure movement?
        if( Event.GadgetType == GADGET_TYPE_LOCATOR_X )
        {
            ASSERT( Event.Info == LOCATOR_ABS );
            s32     Map;

            Map = Event.GadgetID - GADGET_PS2_ANALOG_DPAD_R;

            ASSERT( (Map >= 0) && (Map < 12));
            
            Pad->EPressure[ Map ] = Event.X;
        }

        else

        // Joystick departure?
        if( Event.GadgetType == GADGET_TYPE_MESSAGE )
        {
            if( Event.GadgetID == GADGET_PS2_UNPLUG )
            {
                // Clear the tracking on the joystick.
                x_printf("__DISCONNECT      %ld [%ld]\n",ID,Pad->LogicalID);
                Event.DeviceNum = Pad->LogicalID;
                
                NConnected--;

                if (Pad->LogicalID >= 0)
                    IDToSlotMap[ Pad->LogicalID ] = -1;

                ResetPad( Pad );
            }
            else
            if( Event.GadgetID == GADGET_PS2_PLUGIN )
            {
                // Depending on whether or not a multitap is being used, should determine 
                // the logical ID of the controller that was plugged in.
                if( PS2IN_IsMultitapConnected( 0 ) )
                {
                    Pad->LogicalID = ID;
                }
                // If there was no multitap in the first slot, then assume that the ID will either be
                // 0 for controller 1, and 4 for controller 2.
                else
                {
                    if( ID == 4 )
                    {
                        Pad->LogicalID = 1;
                    }
                    else if( ID == 0 )
                    {
                        Pad->LogicalID = 0;
                    }
                    else if( ID > 4 )
                    {
                        Pad->LogicalID = ID;
                    }
                    else
                    {
                        // This assert should never happen since it would only if there was a multitap connected after all.
                        //ASSERT( 0 );
						Pad->LogicalID = ID;
                    }
                }

                // Fill out secondary array and adjust tracking info
                IDToSlotMap[ Pad->LogicalID ] = ID;
                NConnected++;
                x_printf("__CONNECT         %ld [%ld]\n",ID,Pad->LogicalID);


                // Replace the DeviceNum with the newer 'fixed-up' ID
                Event.DeviceNum = Pad->LogicalID;
                Pad->OfficiallyConnected = TRUE;
            }
        }
        
        //Event.DeviceNum = Pad->LogicalID;
    }



    return( Result );
}


///////////////////////////////////////////////////////////////////////////
//
//  Binds a pad structure to a specific Port/Slot combination.
//  NOTE: Slots 1-3 are only available when using a multi-tap
//
xbool BindPad( s32 Port, s32 Slot, ps2pad_slot* Pad )
{
    s32     Ret;
    
    x_memset(Pad,0,sizeof(ps2pad_slot));
    
   
    Ret = scePadPortOpen( Port, Slot, Pad->DMABuffer );
    if (Ret == 0)
    {
        PS2CON_PRINT(fs("PS2INPUT: Error binding Pad: Port %ld Slot %ld\n",Port,Slot));
        return FALSE;
    }
    
    Pad->Port    = Port;
    Pad->Slot    = Slot;
    Pad->Phase   = PS2CON_PHASE_CONNECTING;
    
    s32     i;
    for (i=0; i<6; i++)
    {
        Pad->ActDirect[i] = 0;
        Pad->ActAlign[i]  = 0xff;
    }
    
    Pad->ActAlign[0] = 0; // Offset 0 for motor0
    Pad->ActAlign[1] = 1; // Offset 1 for motor1
    
    return TRUE;
    
}

static  xbool sbMultiTapLastFrame0 = FALSE;
static  xbool sbMultiTapLastFrame1 = FALSE;

///////////////////////////////////////////////////////////////////////////

xbool INPUT_InitModule( void )
{
    s32     i;

    ASSERT( !Initialized );
    Initialized = TRUE;

    Deadzone = 0.40f;
    Saturation = 0.95f;

    // Initialize the input queue.
    INPUTQ_InitModule();

    // Clear the pad structures
    for (i=0;i<8;i++)
    {
        x_memset(&PS2Pad[i],0,sizeof(ps2pad_slot));
    }

    #ifdef PS2CON_USEDEBUGTEXT
        x_printf("PS2Input: Initializing...\n");
    #endif

    
    
    // Start up the PS2 controller functions.
    PS2CON_PRINT("MTapInit\n");
    if (1 != sceMtapInit())
    {
        PS2CON_PRINT("PS2INPUT: Error initializeing multi-tap\n");
        return FALSE;
    }
    PS2CON_PRINT("MTapPortOpen(0)\n");
    if (1 != sceMtapPortOpen(0))
    {
        PS2CON_PRINT("PS2INPUT: Error opening port 0 for multi-tap access\n");
        return FALSE;
    }
    PS2CON_PRINT("MTapPortopen(1)\n");
    if (1 != sceMtapPortOpen(1))
    {
        PS2CON_PRINT("PS2INPUT: Error opening port 1 for multi-tap access\n");
        return FALSE;
    }    

    PS2CON_PRINT("PadInit\n");
    // Init the pad library.  NOTE: the '0' does not refer to a port or slot.
    //                              The docs state the param MUST be zero.
    scePadInit(0);

    //x_printf("PORT 0: #Slots = %ld\n",scePadGetSlotMax(0));
    //x_printf("PORT 1: #Slots = %ld\n",scePadGetSlotMax(1));

    // Bind the pad structures to the physical slots on the PS2
    for (i=0;i<8;i++)
    {
        PS2Pad[i].Type = PS2CON_TYPE_NONE;
        PS2Pad[i].Phase = PS2CON_PHASE_CONNECTING; 
        if (!BindPad( i/4, i%4, &PS2Pad[i] ))
        {
            PS2CON_PRINT(fs("PS2INPUT: Error during init of port %ld slot %ld\n",i/4,i%4));
            return FALSE;
        }
        IDToSlotMap[ i ] = -1;
        PS2Pad[i].LogicalID = -1;
    }
    
	// Init multitap status
	sbMultiTapLastFrame0 = PS2IN_IsMultitapConnected( 0 );
	sbMultiTapLastFrame1 = PS2IN_IsMultitapConnected( 1 );

    // Initialized
    PS2CON_PRINT("PS2Input: Init successful\n");


    return TRUE;
}

///////////////////////////////////////////////////////////////////////////

void INPUT_KillModule( void )
{
    ASSERT( Initialized );
    Initialized = FALSE;
    
    #ifdef PS2CON_USEDEBUGTEXT
        x_printf("PS2Input: Shutting down...\n");
    #endif

    // Shut down the PS2 controller functions.
    //PadStopCom();

    // Shut down the input queue.
    INPUTQ_KillModule();
}

///////////////////////////////////////////////////////////////////////////

f32 INPUT_GetLocatorX( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( DeviceNum >= 0 );
    ASSERT( DeviceNum <  8 );

    DeviceNum = IDToSlotMap[ DeviceNum ];
    if (DeviceNum==-1)
        return 0;

    switch( GadgetID )
    {
        case GADGET_PS2_L_STICK:  return PS2Pad[ DeviceNum ].ELStickX;  break; 
        case GADGET_PS2_R_STICK:  return PS2Pad[ DeviceNum ].ERStickX;  break;
             
            // This batch can be grouped into one case
        case GADGET_PS2_ANALOG_L1       : 
        case GADGET_PS2_ANALOG_L2       :
        case GADGET_PS2_ANALOG_R1       :
        case GADGET_PS2_ANALOG_R2       :
        case GADGET_PS2_ANALOG_TRIANGLE :
        case GADGET_PS2_ANALOG_CIRCLE   :
        case GADGET_PS2_ANALOG_CROSS    :
        case GADGET_PS2_ANALOG_SQUARE   :
        case GADGET_PS2_ANALOG_DPAD_U   :
        case GADGET_PS2_ANALOG_DPAD_R   :
        case GADGET_PS2_ANALOG_DPAD_D   :
        case GADGET_PS2_ANALOG_DPAD_L   :
            {
                s32     Map;
                Map = GadgetID - GADGET_PS2_ANALOG_DPAD_R;
                return PS2Pad[ DeviceNum ].EPressure[ Map ];
            }
        default:                  ASSERT( FALSE );              break;
    }
    return( 0 );
}

///////////////////////////////////////////////////////////////////////////

f32 INPUT_GetLocatorY( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( DeviceNum >= 0 );
    ASSERT( DeviceNum <  8 );

    DeviceNum = IDToSlotMap[ DeviceNum ];
    if (DeviceNum==-1)
        return 0;
    
    switch( GadgetID )
    {
        case GADGET_PS2_L_STICK:  return PS2Pad[ DeviceNum ].ELStickY;  break;
        case GADGET_PS2_R_STICK:  return PS2Pad[ DeviceNum ].ERStickY;  break;
        default:                  ASSERT( FALSE );              break;
    }
    return( 0 );
}

///////////////////////////////////////////////////////////////////////////

f32 INPUT_GetLocatorZ( s16 DeviceNum, s16 GadgetID )
{
    ASSERT( DeviceNum >= 0 );
    ASSERT( DeviceNum <  8 );
    ASSERT( GadgetID  >  GADGET_PS2_JOY_LOCATOR_GROUP_BEGIN );
    ASSERT( GadgetID  <  GADGET_PS2_JOY_LOCATOR_GROUP_END );

    DeviceNum = IDToSlotMap[ DeviceNum ];
    if (DeviceNum==-1)
        return 0;

    return( 0 );
}

///////////////////////////////////////////////////////////////////////////

xbool INPUT_GetButton( s16 DeviceNum, s16 GadgetID )
{
    u32  BitMask;

#ifdef X_DEBUG
    // For the love of god, I am copying these fricken enourmous enum values into a temp variable
    // so we can at least look at certain numbers.
    ASSERT( DeviceNum >= 0 );
    ASSERT( DeviceNum <  8 );

    s16 StartGroup = GADGET_PS2_JOY_BUTTON_GROUP_BEGIN;
    s16 EndGroup   = GADGET_PS2_JOY_BUTTON_GROUP_END;

    ASSERT( GadgetID >= StartGroup);
    ASSERT( GadgetID <  EndGroup );
#endif

    DeviceNum = IDToSlotMap[ DeviceNum ];
    if (DeviceNum==-1)
        return 0;

    BitMask = (1 << (GadgetID - GADGET_PS2_JOY_BUTTON_GROUP_BEGIN));
    if( PS2Pad[ DeviceNum ].EPressed & BitMask )
        return( TRUE );
    else
        return( FALSE );
}

///////////////////////////////////////////////////////////////////////////

xbool INPUT_GetDebounce( s16 DeviceNum, s16 GadgetID )
{
    u32  BitMask;

    ASSERT( DeviceNum >= 0 );
    ASSERT( DeviceNum <  8 );
    ASSERT( GadgetID  >=  GADGET_PS2_JOY_BUTTON_GROUP_BEGIN );
    ASSERT( GadgetID  <  GADGET_PS2_JOY_BUTTON_GROUP_END   );

    DeviceNum = IDToSlotMap[ DeviceNum ];
    if (DeviceNum==-1)
        return 0;

    BitMask = (1 << (GadgetID - GADGET_PS2_JOY_BUTTON_GROUP_BEGIN));
    if( PS2Pad[ DeviceNum ].EDebounced & BitMask )
        return( TRUE );
    else
        return( FALSE );
}

///////////////////////////////////////////////////////////////////////////

void INPUT_ClearDebounce( void )
{
    PS2Pad[0].EDebounced = 0;
    PS2Pad[1].EDebounced = 0;
    PS2Pad[2].EDebounced = 0;
    PS2Pad[3].EDebounced = 0;
    PS2Pad[4].EDebounced = 0;
    PS2Pad[5].EDebounced = 0;
    PS2Pad[6].EDebounced = 0;
    PS2Pad[7].EDebounced = 0;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                         EVENT BUILDERS                                //
//                                                                       //
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static
void Event_Button( s16 ControllerID, s32 ButtonBit, xbool Pressed )
{
    input_event Event;
    s16         ButtonID;

    // If the pad is currently disconnecting, stop the event from being queued up.
    if (PS2Pad[ControllerID].Flags & PS2PAD_DISCONNECTING)
        return;

    ButtonID = ButtonBit + GADGET_PS2_JOY_BUTTON_GROUP_BEGIN;

    Event.DeviceType = DEVICE_PS2_JOYSTICK;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_BUTTON;
    Event.GadgetID   = ButtonID;
    Event.Timestamp  = Cycle;
    Event.Info       = (Pressed ? BUTTON_PRESS : BUTTON_RELEASE);
    Event.X          = 0;
    Event.Y          = 0;
    Event.Z          = 0;
    Event.Flags      = 0;

    #ifdef PS2CON_USEDEBUGTEXT
        x_printf("PS2INPUT: ButtonEvent %03ld B:  Ctrl:%ld Bit:%ld Pressed:%ld\n",Cycle,ControllerID,ButtonBit,Pressed);
    #endif
    INPUTQ_EnQueue( Event );

    Cycle++;
}

///////////////////////////////////////////////////////////////////////////

static
void Event_Stick( s16 ControllerID, s16 StickID, f32 X, f32 Y )
{
    input_event Event;

    ASSERT(ControllerID>=0);
    ASSERT(ControllerID<8);

    // If the pad is currently disconnecting, stop the event from being queued up.
    if (PS2Pad[ControllerID].Flags & PS2PAD_DISCONNECTING)
        return;

    Event.DeviceType = DEVICE_PS2_JOYSTICK;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_LOCATOR_XY;
    Event.GadgetID   = StickID;
    Event.Timestamp  = Cycle;
    Event.Info       = LOCATOR_ABS;
    Event.X          = X;
    Event.Y          = -Y;
    Event.Z          = 0;
    Event.Flags      = 0;
    
    #ifdef PS2CON_USEDEBUGTEXT
        x_printf("PS2INPUT: StickEvent %03ld S: Ctr:%ld ID:%ld X:%5.3f Y:%5.3f\n",Cycle,ControllerID,StickID,X,Y);
    #endif
    INPUTQ_EnQueue( Event );
    Cycle++;
}

///////////////////////////////////////////////////////////////////////////

static
void Event_Pressure( s16 ControllerID, s16 StickID, f32 X )
{
    input_event Event;

    // If the pad is currently disconnecting, stop the event from being queued up.
    if (PS2Pad[ControllerID].Flags & PS2PAD_DISCONNECTING)
        return;

    Event.DeviceType = DEVICE_PS2_JOYSTICK;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_LOCATOR_X;
    Event.GadgetID   = StickID;
    Event.Timestamp  = Cycle;
    Event.Info       = LOCATOR_ABS;
    Event.X          = X;
    Event.Y          = 0;
    Event.Z          = 0;
    Event.Flags      = 0;

    #ifdef PS2CON_USEDEBUGTEXT
        x_printf("PS2INPUT: PressureEvent %03ld S: Ctr:%ld ID:%ld X:%5.3f \n",Cycle,ControllerID,StickID,X);
    #endif
    INPUTQ_EnQueue( Event );
    Cycle++;
}

///////////////////////////////////////////////////////////////////////////

static
void Event_Connect( s16 DeviceType, s16 ControllerID, s32 Type )
{
    input_event Event;
    s32         Info;

    // Convert the type
    switch(Type)
    {
        case    PS2CON_TYPE_DIGITAL:
            Info = GADGET_PS2_DIGITAL;
            break;
        case    PS2CON_TYPE_DUALSHOCK:
            Info = GADGET_PS2_DUALSHOCK;
            break;
        case    PS2CON_TYPE_ANALOG:
            Info = GADGET_PS2_ANALOG;
            break;
        case    PS2CON_TYPE_DUALSHOCK2:
            Info = GADGET_PS2_DUALSHOCK2;
            break;
        case    PS2CON_TYPE_DIGITAL_ACT:
            Info = GADGET_PS2_DIGITAL_ACT;
            break;
        default:
            Info = GADGET_PS2_UNKNOWN;
            break;
    }

    #ifdef PS2CON_USEDEBUGTEXT
    x_printf("%ld connect\n",ControllerID);
    #endif

    Event.DeviceType = DeviceType;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_MESSAGE;
    Event.GadgetID   = GADGET_PS2_PLUGIN;
    Event.Timestamp  = Cycle;
    Event.Info       = Info;
    Event.X          = 0;
    Event.Y          = 0;
    Event.Z          = 0;
    Event.Flags      = 0;

    #ifdef PS2CON_USEDEBUGTEXT
        x_printf("PS2INPUT: ConnectEvent %03ld S: Ctr:%ld Type %ld\n",Cycle,ControllerID,Type);
    #endif
    
    INPUTQ_EnQueue( Event );
    PS2Pad[ControllerID].Attached = TRUE;

    Cycle++;
}

///////////////////////////////////////////////////////////////////////////

static
void Event_Disconnect( s16 DeviceType, s16 ControllerID )
{
    // No need to queue up another disconnect message if one is already
    // in the pipe.
    if (PS2Pad[ControllerID].Flags & PS2PAD_DISCONNECTING)
        return;

    if( !PS2Pad[ControllerID].Attached )
        return;

    // If the controller hasn't yet been totally initialized, then no disconnect.
    //  -a similar check could be: (PS2Pad[ControllerID].Phase != PS2CON_PHASE_ACTIVE)
	if (!(PS2Pad[ControllerID].Flags & PS2PAD_STABLE))
        return;

    input_event Event;

    #ifdef PS2CON_USEDEBUGTEXT
    x_printf("%ld disconnect\n",ControllerID);
    #endif

    PS2Pad[ControllerID].Flags |= PS2PAD_DISCONNECTING;

    Event.DeviceType = DeviceType;
    Event.DeviceNum  = ControllerID;
    Event.GadgetType = GADGET_TYPE_MESSAGE;
    Event.GadgetID   = GADGET_PS2_UNPLUG;
    Event.Timestamp  = Cycle;
    Event.Info       = 0;
    Event.X          = 0;
    Event.Y          = 0;
    Event.Z          = 0;
    Event.Flags      = 0;

    #ifdef PS2CON_USEDEBUGTEXT
        x_printf("PS2INPUT: DisconnectEvent %03ld S: Ctr:%ld\n",Cycle,ControllerID);
    #endif
    
    INPUTQ_EnQueue( Event );
    PS2Pad[ControllerID].Attached = FALSE;
    Cycle++;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static 
f32 ClampRange( f32 X )
{
    if ( (X >= -Deadzone) &&
         (X <=  Deadzone))
    {
        return 0;
    }

    if (X >=  Saturation)
        return 1;

    if (X <= -Saturation)
        return -1;

    f32 Sign = X>=0?1:-1;

    X = ABS(X);

    return Sign * (X-Deadzone)/(Saturation-Deadzone);
}

///////////////////////////////////////////////////////////////////////////
static
void ProcessDeltas( ps2pad_slot* pPad )
{
    ps2pad_info*    Cur = &pPad->CurrentInfo;
    ps2pad_info*    Old = &pPad->LastInfo;
    s32             ID = pPad->Port * 4 + pPad->Slot;

#ifdef LET_LOOSE_THE_MONKEYS
    static  f32     MonkeyButtonCountDown = 0.0f;
    static  f32     MonkeyStartButtonCountDownDelay = 0.0f;
    s32             MonkeyButton;
    s32             TurnButtonCount;

    if( MonkeyButtonCountDown <= 0.0f )
    {
        MonkeyButtonCountDown = (NConnected * 60.0f) * MONKEY_DELAY_TIME;  // Take the number of joysticks, * 60frames * time to get your delay.

        // Possible button events, let the monkey randomize
        for( TurnButtonCount = 0; TurnButtonCount < MONKEY_MAX_BUTTONS_PER_TURN; TurnButtonCount++ )
        {
            MonkeyButton = x_irand( GADGET_PS2_JOY_BUTTON_GROUP_BEGIN, GADGET_PS2_JOY_BUTTON_GROUP_END );

            // Either set or clear the button.
            if( x_rand( ) & 0x1 )
                SET_MONKEY_BUTTON( MonkeyButton );
            else
                CLEAR_MONKEY_BUTTON( MonkeyButton );
        }

        // If you have counted down the entire start button delay, reset the timer and force the button to be hit.
        if( MonkeyStartButtonCountDownDelay <= 0.0f )
        {
            MonkeyStartButtonCountDownDelay = (NConnected * 60.0f) * MONKEY_START_BUTTON_DELAY_TIME;
            SET_MONKEY_BUTTON( GADGET_PS2_START );
        }
        else
        {
            CLEAR_MONKEY_BUTTON( GADGET_PS2_START );
        }

        // Filter out some combos that will cause the game to supress asserts or go into the ball cannon.
        CLEAR_MONKEY_BUTTON( GADGET_PS2_SELECT );               // All cheats and debug modes are accessed by the select button.
                                                                // so it should never be pressed.
        if( IS_MONKEY_BUTTON_DOWN( GADGET_PS2_L2 ) )
        {
            // Test for R2 as well to avoid leaving asserts prematurely.
            if( IS_MONKEY_BUTTON_DOWN( GADGET_PS2_R2 ) )
            {
                CLEAR_MONKEY_BUTTON( GADGET_PS2_CROSS );        // To avoid assert debug.
                CLEAR_MONKEY_BUTTON( GADGET_PS2_CIRCLE );       // To avoid assert Skip
                CLEAR_MONKEY_BUTTON( GADGET_PS2_TRIANGLE );     // To avoid assert Supress
            }
        }

        // Possible values for analog sticks
        // From -1,0f to 0.0 to 1.0
        // Let the monkey radomize
        Cur->LStickX = x_frand( -1.0f, 1.0f );
        Cur->LStickY = x_frand( -1.0f, 1.0f );
        Cur->RStickX = x_frand( -1.0f, 1.0f );
        Cur->RStickY = x_frand( -1.0f, 1.0f );
    }
    else
    {
        MonkeyButtonCountDown -= 1.0f;
        MonkeyStartButtonCountDownDelay -= 1.0f;
    }

#endif

    // Process the buttons
    //-- Any button changes?
    if( Cur->Buttons != Old->Buttons )
    {
        u16 Bits;   // All deltas in a bit field.
        s32 Bit;    // Which bit from the bit field are we considering?

        // Get a pattern of deltas.
        Bits = Old->Buttons ^ Cur->Buttons;

        // While there are delta bits still set, make an event.
        for( Bit = 0; Bits; Bit++, Bits >>= 1 )
        {
            if( Bits & 0x01 )
            {
                // We got a delta, make an event.
                Event_Button( ID, Bit, ((Cur->Buttons >> Bit) & 0x01) );
            }
        }    
    }

    // Process the analog sticks
    if (pPad->Flags & PS2PAD_HAS_ANALOG)
    {
        // First thing, rescale the stick values to compensate for the
        // deadzone/threshold
        Cur->LStickX = ClampRange( Cur->LStickX );
        Cur->LStickY = ClampRange( Cur->LStickY );
        Cur->RStickX = ClampRange( Cur->RStickX );
        Cur->RStickY = ClampRange( Cur->RStickY );

        // Look for deltas
        if ( (Old->LStickX != Cur->LStickX) ||
             (Old->LStickY != Cur->LStickY) )
        {
            Event_Stick( ID, GADGET_PS2_L_STICK, 
                         Cur->LStickX,
                         Cur->LStickY );
        }

        if ( (Old->RStickX != Cur->RStickX) ||
             (Old->RStickY != Cur->RStickY) )
        {
            Event_Stick( ID, GADGET_PS2_R_STICK, 
                         Cur->RStickX,
                         Cur->RStickY );
        }
    }


    // Process Pressmode
    if (pPad->Flags & PS2PAD_HAS_PRESSMODE)
    {
        s32     i;

        for ( i=0; i<12; i++)
        {
            Cur->Pressure[i] = ClampRange(Cur->Pressure[i]);
            
            if (Cur->Pressure[i] != Old->Pressure[i])
            {
                Event_Pressure( ID, PressureIDMap[i], Cur->Pressure[i] );
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////
//
//  How to interpret the data received from scePadRead
//
//  (taken from libpad.txt)
//
//  Offset 1 data varies depending on type of controller.  Refer
//  to libpad.txt for further info
//
//        +------------------+------------------------------------+
//        |Offset (bytes)    Contents                             |
//        +------------------+------------------------------------+
//        |0                 Successful communication: 0,         |
//        |                  otherwise: 0xff                      |
//        |1                 High-order 4 bits: 0x7               |
//        |                  Low-order 4 bits: Data length/2      |
//        |2,3               Digital button state                 |
//        |                  (1: released, 0: pushed)             |
//        |4                 Analog stick right (X direction)     |
//        |5                 Analog stick right (Y direction)     |
//        |6                 Analog stick left (X direction)      |
//        |7                 Analog stick left (Y direction)      |
//        |8                 Pressure sensitivity information(>)  |
//        |9                 Pressure sensitivity information(<)  |
//        |10                Pressure sensitivity information(^)  |
//        |11                Pressure sensitivity information(V)  |
//        |12                Pressure sensitivity information(Tr) |
//        |13                Pressure sensitivity information(O)  |
//        |14                Pressure sensitivity information(X)  |
//        |15                Pressure sensitivity information(Sq) |
//        |16                Pressure sensitivity information(L1) |
//        |17                Pressure sensitivity information(R1) |
//        |18                Pressure sensitivity information(L2) |
//        |19                Pressure sensitivity information(R2) |
//        +------------------+------------------------------------+
//
///////////////////////////////////////////////////////////////////////////
static s32 LastRead[8];
static s32 ReadFrame = 0;

s32 ReadPad( ps2pad_slot* pPad )
{
	s32     Len;
	s32     TermID = 0;
	s32     Port;
    s32     Slot;
	u8      Data[32];
	u16     PadData = 0;

    Port  = pPad->Port;
	Slot  = pPad->Slot;

    ASSERT(LastRead[Port*4+Slot] != ReadFrame);
    LastRead[Port*4+Slot] = ReadFrame;

	Len = scePadRead( Port, Slot, Data );

	if (Len==0)
	{
		// Failed to get information
        #ifdef PS2CON_USEDEBUGTEXT
            x_printf("INPUT: Read Failed Port %ld Slot %ld\n",Port,Slot);
        #endif
		return 0;
	}

	if( Data[0] == 0 )
	{
		PadData = 0xffff^((Data[2]<<8)|Data[3]);
        ps2pad_info*    Cur;
        Cur = &pPad->CurrentInfo;

        Cur->Buttons = PadData;
        Cur->RStickX = PS2STICKINT2FLOAT( Data[4] );
        Cur->RStickY = PS2STICKINT2FLOAT( Data[5] );
        Cur->LStickX = PS2STICKINT2FLOAT( Data[6] );
        Cur->LStickY = PS2STICKINT2FLOAT( Data[7] );

        Cur->Pressure[0]  = PS2PRESSURE2FLOAT( Data[8] );
        Cur->Pressure[1]  = PS2PRESSURE2FLOAT( Data[9] );
        Cur->Pressure[2]  = PS2PRESSURE2FLOAT( Data[10] );
        Cur->Pressure[3]  = PS2PRESSURE2FLOAT( Data[11] );
        Cur->Pressure[4]  = PS2PRESSURE2FLOAT( Data[12] );
        Cur->Pressure[5]  = PS2PRESSURE2FLOAT( Data[13] );
        Cur->Pressure[6]  = PS2PRESSURE2FLOAT( Data[14] );
        Cur->Pressure[7]  = PS2PRESSURE2FLOAT( Data[15] );
        Cur->Pressure[8]  = PS2PRESSURE2FLOAT( Data[16] );
        Cur->Pressure[9]  = PS2PRESSURE2FLOAT( Data[17] );
        Cur->Pressure[10] = PS2PRESSURE2FLOAT( Data[18] );
        Cur->Pressure[11] = PS2PRESSURE2FLOAT( Data[19] );
        
		TermID = pPad->TermID = Data[1];
/*
        x_printf("ID = %ld   EXID = %ld\n",
            scePadInfoMode( Port, Slot, InfoModeCurID, -1 ),
            scePadInfoMode( Port, Slot, InfoModeCurExID, 0 ));
*/
        // Check to see if the stick buttons should be clamped to
        // zero (analog stick in digital mode reports non-zero
        // values for the sticks)
        if (pPad->Flags & PS2PAD_HAS_ANALOG)
        {
            // This is indicitive of the user having toggled the
            // controller to digial mode.
            s32 ID;

            ID = ((TermID >> 4) & 0xF);
            
            // ID 0x7 is reported by dualshock 1 & 2 when they are in
            // analog mode, and 0x5 is supposed to be reported by the
            // old analog controller (the one without actuators).
            if ( (ID!=0x7) && (ID!=0x5) )
            {
                Cur->LStickX = Cur->LStickY = 0;
                Cur->RStickX = Cur->RStickY = 0;
            }
        }
	}
	return (TermID);
}


///////////////////////////////////////////////////////////////////////////



static
void ScanPS2Pad( ps2pad_slot* pPad )
{
    s32     Phase;
    s32     Port;
    s32     Slot;
    s32     State;

    Port  = pPad->Port;
    Slot  = pPad->Slot;
    Phase = pPad->Phase;


    // Get the current state of the pad
    State = scePadGetState(Port, Slot);
    if ( State == scePadStateDiscon )
    {
        // Check to see if the controller was previously connected
        if (pPad->Attached || pPad->OfficiallyConnected)
        {
            Event_Disconnect( DEVICE_PS2_JOYSTICK, Port*4 + Slot );
            #ifdef PS2CON_USEDEBUGTEXT
                x_printf("DISCONNECT Port %ld Slot %ld\n",Port,Slot);
            #endif
            // Pad CANNOT be reset here.
            // Until the UNPLUG event is removed from the queue by the 
            // user, they can still query for controller info.
        }
        else
        {
            if( pPad->IsBeingScanned )
            {
                ResetPad( pPad );
            }
        }

        pPad->OfficiallyConnected = FALSE;
        pPad->Type = PS2CON_TYPE_NONE;
        pPad->Flags = 0;
        Phase = PS2CON_PHASE_CONNECTING;
    }

    switch( Phase )
    {
        //=================================================================
        case PS2CON_PHASE_CONNECTING:
        //
        //  Get the basic info and extended info about the controller
        //  to decide where to start
        //
        //=================================================================

		if ( (State != scePadStateStable) && 
             (State != scePadStateFindCTP1) &&
             (State != scePadStateExecCmd) )
             break;

        pPad->Type = PS2CON_TYPE_NONE;

        PS2CON_PRINT("PS2CON_PHASE_CONNECTING\n");
        pPad->IsBeingScanned = TRUE;
		pPad->ConnectedViaMultitap = PS2IN_IsMultitapConnected( pPad->Port );

        // Send CONNECT Event to notify the user, and reserve this slot
        // As time goes on, the device will be enumerated through this
        // switch statement, and as capabilities are detected, they will
        // be added to the controller structure.  
        //
        //  NOTE: Only send the connect once.
        //        This case may be entered multiple times, if the 
        //        controller has been detected, but is not yet ready
        //        to communicate with the system.
        //        Case in point is the DualShock2, which appears to 
        //        take slightly more time than normal to become stable.
        //
        if (!pPad->Attached)
            Event_Connect( DEVICE_PS2_JOYSTICK, Port*4 + Slot, GADGET_PS2_UNKNOWN );
        

        // Take a pass through the mode table and see if the controller
        // supports analog mode.  Need to do this for freak controllers
        // that have physical analog switches instead of digital soft
        // switches.  Ex: DUAL IMPACT Game Pad by performance.
        {
            s32     i;
            s32     NModes;
            s32     Mode;

            NModes = scePadInfoMode(Port, Slot, InfoModeIdTable, -1);
	    	NModes = MIN(16,NModes);

            for (i=0;i<NModes;i++)
            {
                Mode = scePadInfoMode( Port, Slot, InfoModeIdTable, i );
                //
                //  Check for 7, which == ANALOG mode supported
                //
                if (Mode == 7)
                {
                    pPad->Flags |= PS2PAD_HAS_ANALOG;
                }
            }
        }
        
        s32 ID,EXID;

        ID = scePadInfoMode( Port, Slot, InfoModeCurID, 0 );
        if ( ID == 0 ) break;

        EXID = scePadInfoMode( Port, Slot, InfoModeCurExID, 0 );
        if ( EXID > 0 )
        {
            #ifdef PS2CON_USEDEBUGTEXT
                if ( (EXID < 100) && (ID != EXID) )
                    x_printf ("ID:%d ExID:%d [%d][%d]\n", ID, EXID, Port, Slot);
            #endif
            ID = EXID;
        }

        switch(ID)
        {
            // STANDARD 
            case 4:
            Phase = PS2CON_PHASE_INIT_DIGITAL;
            break;

            // ANALOG 
            case 7:
            Phase = PS2CON_PHASE_INIT_ANALOG;
            break;

            default:
            Phase = PS2CON_PHASE_ACTIVE;
            break;
        }
        break;

        //=================================================================
        case PS2CON_PHASE_INIT_DIGITAL:
        //
        //  Test to see if the controller has any actuators.  If it does,
        //  then we should (to be sure) check to see if it can be 
        //  switched to analog mode.
        //
        //=================================================================
        // Test to see if controller has vibration capability
        #ifdef PS2CON_USEDEBUGTEXT
            x_printf("PS2CON_PHASE_INIT_DIGITAL\n");
        #endif
        if (scePadInfoMode( Port, Slot, InfoModeCurExID, 0 ) == 0)
        {
            pPad->Type = PS2CON_TYPE_DIGITAL;
            Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
            break;
        }
        // The controller has actuators
        pPad->Flags |= PS2PAD_HAS_ACTUATORS;
        pPad->Type = PS2CON_TYPE_DIGITAL_ACT;
        // We still need to check if it might be an analog stick that is
        // in digital mode
        Phase = PS2CON_PHASE_INIT_ANALOG;
        break;

        //=================================================================
        case PS2CON_PHASE_INIT_ANALOG:
        //
        //  Try to enable analog mode, to determine the specis of the
        //  controller
        //
        //=================================================================
        PS2CON_PRINT("PS2CON_PHASE_INIT_ANALOG\n");
        if (scePadSetMainMode( Port, Slot, 1, 0 ) == 1)
        {
            Phase = PS2CON_PHASE_INIT_ANALOG_WAIT_ANALOG_STABLE;
        }
        else
        {
             PS2CON_PRINT("Mainmode failed\n");
             // Analog mode failed, flag the controller as a Digital one,
            // unless it has already been marked as a "digital with
            // actuators"
            if (pPad->Type == PS2CON_TYPE_NONE)
                pPad->Type = PS2CON_TYPE_DIGITAL;
            Phase = PS2CON_PHASE_WAIT_FOR_STABLE;

            Phase = PS2CON_PHASE_INIT_DIGITAL_TEST_ACTUATORS;
            break;
        }
        break;

        //=================================================================
        case PS2CON_PHASE_INIT_ANALOG_WAIT_ANALOG_STABLE:
        //
        //  Wait for the analog switch-over to complete, or fail
        //
        //=================================================================
        #ifdef PS2CON_USEDEBUGTEXT
            x_printf("PS2CON_PHASE_INIT_ANALOG_WAIT_ANALOG_STABLE\n");
        #endif
        if (scePadGetReqState( Port, Slot ) == scePadReqStateComplete)
        {
            // Analog mode has been verified
            Phase = PS2CON_PHASE_INIT_ANALOG_TEST_ACTUATORS;
            pPad->Flags |= PS2PAD_HAS_ANALOG;
            break;
        }
        if (scePadGetReqState( Port, Slot ) == scePadReqStateFailed)
        {
            // Analog mode failed, flag the controller as a Digital one,
            // unless it has already been marked as a "digital with
            // actuators"
            if (pPad->Type == PS2CON_TYPE_NONE)
                pPad->Type = PS2CON_TYPE_DIGITAL;
            Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
            break;
        }
        break;



        //=================================================================
        case PS2CON_PHASE_INIT_DIGITAL_TEST_ACTUATORS:
        //
        //  Test to see if the controller supports actuators
        //  If not, it is just ANALOG, otherwise, it may be a DUALSHOCK
        //  or a DUALSHOCK-2, depending on support for pressure functions
        //
        //  This case is here to allow correct activation and setup of
        //  actuators on a digital pad
        //
        //=================================================================
        {
            if (scePadGetState( Port, Slot ) != scePadStateStable)
                break;
            s32 NAct = scePadInfoAct( Port, Slot, -1, 0 );

            PS2CON_PRINT("PS2CON_PHASE_INIT_DIGITAL_TEST_ACTUATORS\n");
            PS2CON_PRINT(fs("# Actuators : %ld\n",NAct));
            
            if (NAct == 0 )
            {
                // No actuators
                pPad->Type = PS2CON_TYPE_DIGITAL;
                Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
                break; 
            }
            // Asynchronous function
            if (scePadSetActAlign(pPad->Port, pPad->Slot, pPad->ActAlign)==0)
	        {
                // Align failed.  Assume there are no actuators
	            pPad->Type = PS2CON_TYPE_DIGITAL;
                Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
	            break;
	        }
            Phase = PS2CON_PHASE_INIT_DIGITAL_WAIT_ACTUATORS_STABLE; 
        }
        break;

        //=================================================================
        case PS2CON_PHASE_INIT_DIGITAL_WAIT_ACTUATORS_STABLE:
        //
        //  Wait for the actuator command to complete, or fail
        //
        //  This case is here to allow correct activation and setup of
        //  actuators on a digital pad       
        //
        //=================================================================
        #ifdef PS2CON_USEDEBUGTEXT
            x_printf("PS2CON_PHASE_INIT_DIGITAL_WAIT_ACTUATORS_STABLE\n");
        #endif
        if (scePadGetReqState( Port, Slot ) == scePadReqStateComplete)
        {
            // Analog mode has been verified
            Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
            pPad->Flags |= PS2PAD_HAS_ACTUATORS;
            break;
        }
        if (scePadGetReqState( Port, Slot ) == scePadReqStateFailed)
        {
            // Actuators failed, flag the controller as an analog one.
            pPad->Type = PS2CON_TYPE_DIGITAL;
            Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
            break;
        }
        break;





        //=================================================================
        case PS2CON_PHASE_INIT_ANALOG_TEST_ACTUATORS:
        //
        //  Test to see if the controller supports actuators
        //  If not, it is just ANALOG, otherwise, it may be a DUALSHOCK
        //  or a DUALSHOCK-2, depending on support for pressure functions
        //
        //=================================================================
        {
            if (scePadGetState( Port, Slot ) != scePadStateStable)
                break;
            s32 NAct = scePadInfoAct( Port, Slot, -1, 0 );
            #ifdef PS2CON_USEDEBUGTEXT
                x_printf("PS2CON_PHASE_INIT_ANALOG_TEST_ACTUATORS\n");
                x_printf("# Actuators : %ld\n",NAct);
            #endif
                        
            if (NAct == 0 )
            {
                // No actuators
                pPad->Type = PS2CON_TYPE_ANALOG;
                Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
                break;
            }

           // Asynchronous function
            if (scePadSetActAlign(pPad->Port, pPad->Slot, pPad->ActAlign)==0)
	        {
                // Align failed.  Assume there are no actuators
	            pPad->Type = PS2CON_TYPE_ANALOG;
                Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
	            break;
	        }
            Phase = PS2CON_PHASE_INIT_ANALOG_WAIT_ACTUATORS_STABLE;
        }
        break;

        //=================================================================
        case PS2CON_PHASE_INIT_ANALOG_WAIT_ACTUATORS_STABLE:
        //
        //  Wait for the actuator command to complete, or fail
        //
        //=================================================================
        #ifdef PS2CON_USEDEBUGTEXT
            x_printf("PS2CON_PHASE_INIT_ANALOG_WAIT_ACTUATORS_STABLE\n");
        #endif
        if (scePadGetReqState( Port, Slot ) == scePadReqStateComplete)
        {
            // Analog mode has been verified
            Phase = PS2CON_PHASE_INIT_ANALOG_TEST_PRESSMODE;
            pPad->Flags |= PS2PAD_HAS_ACTUATORS;
            break;
        }
        if (scePadGetReqState( Port, Slot ) == scePadReqStateFailed)
        {
            // Actuators failed, flag the controller as an analog one.
            pPad->Type = PS2CON_TYPE_ANALOG;
            Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
            break;
        }
        break;

        //=================================================================
        case PS2CON_PHASE_INIT_ANALOG_TEST_PRESSMODE:
        //
        //  Test to see if the controller supports pressure sensitive mode.
        //  If it does, then it is a Dualshock 2, otherwise, it is a
        //  normal Dualshock
        //
        //=================================================================
        if (scePadGetState( Port, Slot ) != scePadStateStable)
            break;

        #ifdef PS2CON_USEDEBUGTEXT
            x_printf("PS2CON_PHASE_INIT_ANALOG_TEST_PRESSMODE\n");
        #endif
        if (scePadInfoPressMode( Port, Slot ) == 1)
        {
            Phase = PS2CON_PHASE_INIT_ANALOG_WAIT_PRESSMODE_STABLE;                
        }
        else
        {     
            pPad->Type = PS2CON_TYPE_DUALSHOCK;
            Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
        }


        break;

        //=================================================================
        case PS2CON_PHASE_INIT_ANALOG_WAIT_PRESSMODE_STABLE:
        //
        //  Wait for the pressmode switchover to succeed or fail
        //
        //=================================================================
        #ifdef PS2CON_USEDEBUGTEXT
            x_printf("PS2CON_PHASE_INIT_ANALOG_WAIT_PRESSMODE_STABLE\n");
        #endif
        if (scePadGetReqState( Port, Slot ) == scePadReqStateComplete)
        {
            // Pressmode has been verified
            pPad->Type = PS2CON_TYPE_DUALSHOCK2;
            pPad->Flags |= PS2PAD_HAS_PRESSMODE;
            Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
            break;
        }
        if (scePadGetReqState( Port, Slot ) == scePadReqStateFailed)
        {
            // Activating pressmode failed, flag the controller as a normal Dualshock
            pPad->Type = PS2CON_TYPE_DUALSHOCK;
            Phase = PS2CON_PHASE_WAIT_FOR_STABLE;
            break;
        }
        break;

        //=================================================================
        case PS2CON_PHASE_WAIT_FOR_STABLE:
        //
        //  At this point, the controller has been identified.
        //  Once the pad becomes stable, a connection event will be 
        //  enqueued, and we can begin reading the pad
        //
        //=================================================================
        #ifdef PS2CON_USEDEBUGTEXT
            x_printf("PS2CON_PHASE_WAIT_FOR_STABLE\n");
        #endif
        s32 SubState;
        SubState = scePadGetState( Port, Slot );

        if ((SubState == scePadStateStable) || (SubState == scePadStateFindCTP1))
        {
			//Lock the controller
			PS2_SetAnalogMode( pPad, TRUE );
			PS2_LockAnalogToggle( pPad, TRUE );			

            // Controller is ready
            Phase = PS2CON_PHASE_ACTIVE;
            #ifdef PS2CON_USEDEBUGTEXT
                x_printf("Controller Ready: Type = %s\n",PS2CON_NAME[pPad->Type]);
            #endif

            // Scanning is complete.
            pPad->IsBeingScanned = FALSE;

            // CONNECT is no longer sent from here.  It is sent from the 
            // first switch Case.
            // Leaving the call here just as a reference.
            //
            //Event_Connect( Port*4 + Slot, pPad->Type );
            break;
        }

        //=================================================================
        //=================================================================
        //=================================================================
        case PS2CON_PHASE_ACTIVE:
        //=================================================================
        //=================================================================
        //=================================================================
        pPad->Flags |= PS2PAD_STABLE;

        if ( (State != scePadStateStable  ) && 
             (State != scePadStateFindCTP1) )
             break;

        s32 TermID;

        TermID = ReadPad( pPad );

        if ( TermID > 0 ) 
        {
            pPad->RTermID = TermID;
            ProcessDeltas( pPad );
        }
        break;
    }

    pPad->Phase = Phase;
}



void PS2INPUT_SetActuators(s32 ID, s32 motor1, s32 motor2)
{
    if (!(PS2Pad[ID].Flags & PS2PAD_HAS_ACTUATORS))
        return;

    PS2Pad[ID].ActDirect[0] = (motor1>0);
    PS2Pad[ID].ActDirect[1] = motor2;
    scePadSetActDirect(PS2Pad[ID].Port, PS2Pad[ID].Slot, PS2Pad[ID].ActDirect);
}

///////////////////////////////////////////////////////////////////////////
//
// INPUT_CheckDevices
//
///////////////////////////////////////////////////////////////////////////
void INPUT_CheckDevices( void )
{
    s16     i;
    xbool   Port0,Port1;
    s64     Start;
    Start = x_GetTimerTicks();
    static s32  Frame = 0;
    Frame++;
    ReadFrame++;
    xbool         bMultiTapThisFrame0 = PS2IN_IsMultitapConnected( 0 );
	xbool         bMultiTapThisFrame1 = PS2IN_IsMultitapConnected( 1 );

	// If the multitap in slot0 changes states, just cycle through all connections of the first 5 controllers.
    // This is used right now for ASB because the game only supports max 4 players.  If their the multitap in the
    // first controller slot is not present.  Joystick 0 will be player 1, joystick 5 will be player 2.  If the multitap
    // is in place, then the players will correspond to the joystick connections to the multitap in slot 0.
    if( bMultiTapThisFrame0 != sbMultiTapLastFrame0 )
    {
		if (bMultiTapThisFrame0)
			Event_Connect( DEVICE_PS2_MULTITAP, 0, PS2CON_TYPE_NONE );
		else
			Event_Disconnect( DEVICE_PS2_MULTITAP, 0 );
		s16 i;
        for( i = 0; i <= 4; i++)
            Event_Disconnect( DEVICE_PS2_JOYSTICK, i );

		sbMultiTapLastFrame0 = bMultiTapThisFrame0;
    }

	if (bMultiTapThisFrame1 != sbMultiTapLastFrame1 )
	{
		if (bMultiTapThisFrame1)
			Event_Connect( DEVICE_PS2_MULTITAP, 1, PS2CON_TYPE_NONE );
		else
			Event_Disconnect( DEVICE_PS2_MULTITAP, 1 );

		sbMultiTapLastFrame1 = bMultiTapThisFrame1;
	}

    Port0 = Port1 = FALSE;

    if (PS2Pad[0].Flags & PS2PAD_STABLE)
        Port0=TRUE;
    if (PS2Pad[4].Flags & PS2PAD_STABLE)
        Port1=TRUE;

    // Only allow a new controller to connect if the root port is
    // filled.  ie: only allow 1-3 connect if 0 is connected
    //                         5-7            4
    // Also, 4 is only allowed to connect if 0 is already connected
    //
    // Performing this test here saves us from scanning a port
    // that will never be allowed to connect, causing a waste of 
    // resources in the process.
    //
    for (i=0;i<8;i++)
    {
        ScanPS2Pad( &PS2Pad[i] );        
    }
  
    // Need a tick to handle actuator triggers
    f64 CurrentTime = x_GetTime();

    
    for (i=0;i<8;i++)
    {
        ps2pad_slot*    Pad = &PS2Pad[i];
        PS2Pad[i].LastInfo = PS2Pad[i].CurrentInfo;
            
        if (Pad->Type != PS2CON_TYPE_NONE)
        {

			// BEGIN CONTROLLER RUMBLE SETUP ----------------------------------
            xbool   StopRumble = FALSE;

			if( Pad->RumbleIntensity > 0.0f )
			{
				if( Pad->RumblePaused )
				{
					if( Pad->RumbleRunning )
						StopRumble = TRUE;
				}
				else
				{
					if( Pad->RumbleRunning == FALSE )
					{
						PS2INPUT_SetActuators( i, 0, Pad->RumbleIntensity );
						Pad->RumbleRunning = TRUE;
					}

					if( x_GetTimeDiff( Pad->RumbleStartTime, CurrentTime ) > Pad->RumbleDuration )
					{
						StopRumble           = TRUE;
						Pad->RumblePaused    = FALSE;
						Pad->RumbleIntensity = 0;
					}
				}
			}
			else
			{
				// Stop actuators if Intensity is zero and they are still flagged as running
				//if( Pad->RumbleRunning )
					StopRumble = TRUE;
			}

			if( StopRumble )
			{
				PS2INPUT_SetActuators( i, 0, 0 );
				Pad->RumbleRunning = FALSE;
			}

			// END CONTROLLER RUMBLE SETUP ------------------------------------

            //
            // Look for analog mode change requests
            //
            if (Pad->Flags & PS2PAD_ANALOG_CHANGE_REQ)
            {
                s32 On = Pad->Flags & PS2PAD_ANALOG_REQ_ON;

                if (1 == scePadSetMainMode( Pad->Port, Pad->Slot, On?1:0, On?0:2 ))
                {
                    // mode change was sent successfully
                    Pad->Flags &= (~(PS2PAD_ANALOG_CHANGE_REQ | PS2PAD_ANALOG_REQ_ON));
                }
            }
            //
            //  Look for press mode change requests
            //  ( This is attached via an else statement because both 
            //    mode changes cannot be issued in the same frame.  Each
            //    operation takes a couple of frames to complete, and nothing
            //    else can be done until the controller becomes stable )
            //
            else if (Pad->Flags & PS2PAD_PRESSMODE_CHANGE_REQ)
            {
                s32 On = Pad->Flags & PS2PAD_PRESSMODE_REQ_ON;

                if (On)
                {
                    if (1 == scePadEnterPressMode( Pad->Port, Pad->Slot ))
                    {
                        // mode change was sent successfully
                        Pad->Flags &= (~(PS2PAD_PRESSMODE_CHANGE_REQ | PS2PAD_PRESSMODE_REQ_ON));
                        Pad->Flags |= PS2PAD_PRESSMODE_ON;
                    }
                }
                else
                {
                    if (1 == scePadExitPressMode( Pad->Port, Pad->Slot ))
                    {
                        // mode change was sent successfully
                        Pad->Flags &= (~(PS2PAD_PRESSMODE_CHANGE_REQ | PS2PAD_PRESSMODE_REQ_ON));
                        Pad->Flags &= (~PS2PAD_PRESSMODE_ON);
                    }
                }
            }
        }
    }


    //  Used for timing input subsystem cpu time
//    Start = x_GetTimerTicks()-Start;
//    if (Frame % 10 == 0)
//        x_printf("INPUT: %5.3f\n",x_TicksToMs(Start));

	//Added per Stevan Hird 1/12.


}

///////////////////////////////////////////////////////////////////////////

void PS2_SetAnalogMode( s32 Controller, xbool On )
{
    ASSERT( Controller>=0 && Controller<8 );

    Controller = IDToSlotMap[ Controller ];
    if (Controller == -1)
        return;
    ps2pad_slot*    P = &PS2Pad[Controller];

    if (P->Type == PS2CON_TYPE_NONE)
        return;
    if (!(P->Flags & PS2PAD_HAS_ANALOG))
        return;

    P->Flags |= PS2PAD_ANALOG_CHANGE_REQ;
    if (On)
        P->Flags |= PS2PAD_ANALOG_REQ_ON;
}

///////////////////////////////////////////////////////////////////////////
void PS2_SetAnalogMode( ps2pad_slot* pPad, xbool On )
{
    ASSERT( pPad );

    if (pPad->Type == PS2CON_TYPE_NONE)
        return;
    if (!(pPad->Flags & PS2PAD_HAS_ANALOG))
        return;

    pPad->Flags |= PS2PAD_ANALOG_CHANGE_REQ;
    if (On)
        pPad->Flags |= PS2PAD_ANALOG_REQ_ON;
}

///////////////////////////////////////////////////////////////////////////

xbool PS2_GetAnalogMode( s32 Controller )
{
    ASSERT( Controller>=0 && Controller<8 );

    Controller = IDToSlotMap[ Controller ];
    if (Controller == -1)
        return FALSE;

    ps2pad_slot*    P = &PS2Pad[Controller];

    if (P->Type == PS2CON_TYPE_NONE)
        return FALSE;
    if (!(P->Flags & PS2PAD_HAS_ANALOG))
        return FALSE;

    s32 Ret = scePadInfoMode( P->Port, P->Slot, InfoModeCurID, -1 );

    if (Ret == 7 || Ret == 5)
        return TRUE;
    return FALSE;
}
///////////////////////////////////////////////////////////////////////////

s32 PS2_GetAnalogMode( ps2pad_slot* pPad )
{
    ASSERT( pPad );

    if (pPad->Type == PS2CON_TYPE_NONE)
        return FALSE;
    if (!(pPad->Flags & PS2PAD_HAS_ANALOG))
        return FALSE;

    s32 Ret = scePadInfoMode( pPad->Port, pPad->Slot, InfoModeCurID, -1 );

    if (Ret == 7 || Ret == 5)
        return TRUE;
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////

void PS2_LockAnalogToggle( s32 Controller, xbool Lock )
{
    ASSERT( Controller>=0 && Controller<8 );

    Controller = IDToSlotMap[ Controller ];
    if (Controller == -1)
        return;
    ps2pad_slot*    P = &PS2Pad[Controller];

    if (P->Type == PS2CON_TYPE_NONE)
        return;
    if (!(P->Flags & PS2PAD_HAS_ANALOG))
        return;

    xbool On = PS2_GetAnalogMode(Controller);

	if (Lock)
		scePadSetMainMode( P->Port, P->Slot, On?1:0, 3 );
	else
		scePadSetMainMode( P->Port, P->Slot, On?1:0, 2 );
       
    // Need to reset the actuator settings
    scePadSetActAlign(P->Port, P->Slot, P->ActAlign);

    // Need to reset the pressure mode settings
    if (P->Flags & PS2PAD_PRESSMODE_ON)
    {
        PS2_SetPressMode( Controller, TRUE );
    }
}

///////////////////////////////////////////////////////////////////////////

void PS2_LockAnalogToggle( ps2pad_slot* pPad, xbool Lock )
{
    ASSERT( pPad );

    if (pPad->Type == PS2CON_TYPE_NONE)
        return;
    if (!(pPad->Flags & PS2PAD_HAS_ANALOG))
        return;

    xbool On = PS2_GetAnalogMode(pPad);
    
	//The offset in the guide says that we're suppose to put 1 (from the table) in the offs param
	//it doesn't work right. so, we're sending 0
    if (Lock)
        scePadSetMainMode( pPad->Port, pPad->Slot, 0, 3 );
    else
        scePadSetMainMode( pPad->Port, pPad->Slot, On?1:0, 2 );
       
    // Need to reset the actuator settings
    scePadSetActAlign(pPad->Port, pPad->Slot, pPad->ActAlign);

    // Need to reset the pressure mode settings
    if (pPad->Flags & PS2PAD_PRESSMODE_ON)
    {
        PS2_SetPressMode( pPad, TRUE );
    }
}

///////////////////////////////////////////////////////////////////////////

void PS2_SetPressMode( s32 Controller, xbool On )
{
    ASSERT( Controller>=0 && Controller<8 );

    Controller = IDToSlotMap[ Controller ];
    if (Controller == -1)
        return;
    ps2pad_slot*    P = &PS2Pad[Controller];

    if (P->Type == PS2CON_TYPE_NONE)
        return;
    if (!(P->Flags & PS2PAD_HAS_PRESSMODE))
        return;

    // Flag the mode change request
    P->Flags |= PS2PAD_PRESSMODE_CHANGE_REQ;
    if (On)
        P->Flags |= PS2PAD_PRESSMODE_REQ_ON;
}   

///////////////////////////////////////////////////////////////////////////

void PS2_SetPressMode( ps2pad_slot* pPad, xbool On )
{
    if (pPad->Type == PS2CON_TYPE_NONE)
        return;
    if (!(pPad->Flags & PS2PAD_HAS_PRESSMODE))
        return;

    // Flag the mode change request
    pPad->Flags |= PS2PAD_PRESSMODE_CHANGE_REQ;
    if (On)
        pPad->Flags |= PS2PAD_PRESSMODE_REQ_ON;
}   

///////////////////////////////////////////////////////////////////////////

void INPUT_RumblePlay( s32 Controller, f32 Intensity, f32 Duration )
{
    ASSERT( Controller>=0 && Controller<8 );

    Controller = IDToSlotMap[ Controller ];

    if( Controller == -1 )
        return;

    ps2pad_slot* Pad = &PS2Pad[ Controller ];

    if( Intensity < 0.0f )
        Intensity = 0.0f;
    else if( Intensity > 1.0f )
        Intensity = 1.0f;

	if( Duration < 0.0f )
		Duration = 0.0f;

    Pad->RumbleIntensity = (u8)(Intensity * 255);
    Pad->RumblePaused    = FALSE;
    Pad->RumbleStartTime = x_GetTime();
	Pad->RumbleDuration  = Duration;
    Pad->RumbleRunning   = FALSE;
}

///////////////////////////////////////////////////////////////////////////

void INPUT_RumbleStop( s32 Controller )
{
    ASSERT( Controller>=0 && Controller<8 );

    Controller = IDToSlotMap[ Controller ];

    if( Controller == -1 )
        return;

    ps2pad_slot* Pad = &PS2Pad[ Controller ];

    Pad->RumbleIntensity = 0;
    Pad->RumblePaused    = FALSE;
    Pad->RumbleStartTime = 0;
	Pad->RumbleDuration  = 0;
}

///////////////////////////////////////////////////////////////////////////

void INPUT_RumblePause( s32 Controller )
{
    ASSERT( Controller>=0 && Controller<8 );

    Controller = IDToSlotMap[ Controller ];
    if( Controller == -1 )
        return;

    ps2pad_slot* Pad = &PS2Pad[ Controller ];

    if( Pad->RumbleIntensity > 0 )
    {
		if( Pad->RumblePaused == FALSE )
		{
			Pad->RumblePaused    = TRUE;
			Pad->RumbleDuration -= x_GetElapsedTime( Pad->RumbleStartTime );
//			Pad->RumbleRunning   = FALSE;
		}
    }
}

///////////////////////////////////////////////////////////////////////////

void INPUT_RumbleResume( s32 Controller )
{
    ASSERT( Controller>=0 && Controller<8 );

    Controller = IDToSlotMap[ Controller ];

    if( Controller == -1 )
        return;

    ps2pad_slot* Pad = &PS2Pad[ Controller ];

    if( Pad->RumbleIntensity > 0 )
    {
		if( Pad->RumblePaused )
		{
			Pad->RumblePaused    = FALSE;
			Pad->RumbleRunning   = FALSE;
			Pad->RumbleStartTime = x_GetTime();
		}
    }
}

///////////////////////////////////////////////////////////////////////////

void INPUT_SetDeadZone      ( f32 aDeadzone   )
{
    Deadzone = aDeadzone;
}

///////////////////////////////////////////////////////////////////////////

f32  INPUT_GetDeadZone      ( void           )
{
    return Deadzone;
}

///////////////////////////////////////////////////////////////////////////

void INPUT_SetSaturation    ( f32 aSaturation )
{
    Saturation = aSaturation;
}

///////////////////////////////////////////////////////////////////////////

f32  INPUT_GetSaturation    ( void           )
{
    return Saturation;
}

///////////////////////////////////////////////////////////////////////////

void PS2IN_GetControllerInfo( s32 Controller, u32& Flags )
{
    ASSERT( Initialized );
    ASSERT( Controller>=0 && Controller<8 );

    Flags = 0;

    Controller = IDToSlotMap[ Controller ];
    if (Controller == -1)      
        return;
    
    ps2pad_slot*    Pad = &PS2Pad[Controller];

    if (Pad->Type != PS2CON_TYPE_NONE)
    {
        Flags |= PS2IN_CONNECTED;

        if (Pad->Flags & PS2PAD_HAS_ACTUATORS)
            Flags |= PS2IN_HAS_ACTUATORS;
        if (Pad->Flags & PS2PAD_HAS_ANALOG)
            Flags |= PS2IN_HAS_ANALOG;
        if (Pad->Flags & PS2PAD_HAS_PRESSMODE)
            Flags |= PS2IN_HAS_PRESSMODE;
        if (Pad->Flags & PS2PAD_PRESSMODE_ON)
            Flags |= PS2IN_PRESSMODE_ON;
		if (Pad->ConnectedViaMultitap)
			Flags |= PS2IN_CONNECTEDVIAMULTITAP;
    }    
}

xbool PS2IN_IsMultitapConnected( s32 Port )
{
    s32 Res = sceMtapGetConnection(Port);
    if (Res == 1)
        return TRUE;
    return FALSE;
}

s32   PS2IN_GetControllerPort( s32 Controller )
{
	ASSERT( Initialized );
    ASSERT( Controller>=0 && Controller<8 );

    Controller = IDToSlotMap[ Controller ];
    if (Controller == -1)      
        return -1;
    
    ps2pad_slot*    Pad = &PS2Pad[Controller];

	return Pad->Port;
}

///////////////////////////////////////////////////////////////////////////
xbool INPUT_IsControllerConnected( s32 ControllerID )
{
    ASSERT( ControllerID >= 0 && ControllerID < 8 );

    u32 ControllerFlag = 0;

    PS2IN_GetControllerInfo( ControllerID, ControllerFlag );
    if( ControllerFlag & PS2IN_CONNECTED )
        return TRUE;
    else
        return FALSE;
}

xbool PS2IN_IsControllerConnectedViaMultitap( s32 ControllerID )
{
	ASSERT( ControllerID >= 0 && ControllerID < 8 );

    u32 ControllerFlag = 0;

    PS2IN_GetControllerInfo( ControllerID, ControllerFlag );
    if( ControllerFlag & PS2IN_CONNECTEDVIAMULTITAP )
        return TRUE;
    else
        return FALSE;
}

///////////////////////////////////////////////////////////////////////////
s32 INPUT_ReturnLowestConnectedController( void )
{
    u8 i;

    for( i = 0; i < 8; i++ )
    {
        if( IDToSlotMap[i] != -1 )
		{
			if (PS2Pad[IDToSlotMap[i]].Type != PS2CON_TYPE_NONE) //Rusty: Only return for controller
				return i;
		}
    }

    return -1;
}