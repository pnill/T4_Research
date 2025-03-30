#include "x_files.hpp"
#include "Q_Input.hpp"
#include "E_InputQ.hpp"
#include "PC_Video.hpp"
#include "PC_Input.hpp"

//=====================================================================================================================================
// DEFINES
//=====================================================================================================================================
#define	MAX_INPUT_DEVICES	(8)
#define ISPRESS(A) (((A) & (1<<7)) == (1<<7))



#define MAX_LOCATOR_RANGE	(LOCATOR_MAX)
#define MAX_MOUSE_MOVEMENT_SPEED  (10.0f)
#define	BUFFER_SIZE			(256)
#define	MIN_RANGE			(-32767)
#define	MAX_RANGE			(32767)
#define	MAX_DEADZONE		(2000)	// value between 0-10000
#define	MAX_SATURATION		(9500)	// value between 0-10000

#define MAX_JOYSTICK_STICKS     4
#define MAX_JOYSTICK_ROTATIONS  4
#define MAX_JOYSTICK_SLIDERS    4
#define MAX_JOYSTICK_HATS       4
#define MAX_JOYSTICK_BUTTONS    32

#define PC_INPUT_DEBOUNCE       0x01
#define PC_INPUT_PRESSED        0x02

//=====================================================================================================================================
// LOCAL DATA TYPES
//=====================================================================================================================================
typedef struct 
{
    f32 X, Y, Z;    // Group (X,Y,Z) for devices
} InputVector3;


//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct                           
{
    InputVector3   Stick[MAX_JOYSTICK_STICKS];          // State of all the Sticks
    InputVector3   Rotation[MAX_JOYSTICK_ROTATIONS];    // State of all the Rotation 
    f32            Slider[MAX_JOYSTICK_SLIDERS];        // State of all the Sliders
    InputVector3   Hat[MAX_JOYSTICK_HATS];              // State of all the Hats
    u8             Button[MAX_JOYSTICK_BUTTONS];        // State of all the Buttons 
} JoystickInputState;


//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
    u8 Key[256];                        // State of all the Buttons

} KeyboardInputState;


//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
    f32  X, Y, Z;                       // Curent position of the Mouse
    u8   Button[8];                     // State of all the Buttons

} MouseInputState;


//-------------------------------------------------------------------------------------------------------------------------------------
typedef union
{
    JoystickInputState  Joystick;     // State of the joystick
    KeyboardInputState  Keyboard;     // State of the keyboard
    MouseInputState     Mouse;        // State of the mouse

} InputState;


//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct 
{
    LPDIRECTINPUTDEVICE8	mpDIDevice;         // The DirectX Input Device interface pointer.
	DIDEVCAPS				mDeviceCaps;        // this devices capabilities
	s32						mGroupID;           // DeviceID indicating which of same type.  Ex.  Joystick0, 1, or 2.  Mouse 1 or 2. etc...
    s32						mQuagType;          // Type of the device (Type from Quagmire)
	xbool					mbPolledDevice;     // TRUE if needs to call Poll before getting data
	char					mName[128];	        // name of the device
    InputState				mState;		        // Curent State of the device
	LPDIRECTINPUTEFFECT     mpEffect;           // effect handler

} InputDevice;


//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct s_InputManager
{
    HWND		    mHWND;                          // Main apps window.
    LPDIRECTINPUT8	mpDirectInput;                  // D3D Input device object.

	InputDevice		mDevices[MAX_INPUT_DEVICES];    // Input Devices.
    s32				mNDevices;                      // Number of devices used.
} InputManager;



//=====================================================================================================================================
// STATIC Globals
//=====================================================================================================================================
static xbool				sbInitialized = FALSE;
static DIDEVICEOBJECTDATA	sDeviceData[BUFFER_SIZE];
static InputManager         sInputManager;


//=====================================================================================================================================
// LOCAL FUNCTIONS 
//=====================================================================================================================================
static err INPUT_BufferInit( InputDevice* pThis, s32 BufferSize );


//=================================================================
// SetInRange
//=================================================================
static f32 SetInRange( s32 Value, f32 Range )
{
	f32	newValue;

	newValue = (f32)Value / (f32)MAX_RANGE;

    if (newValue >  Range)
		return Range;
    if (newValue < (-Range))
		return -Range;

    return newValue;
}



//=================================================================
// INPUT_AddDevice
//=================================================================
HRESULT INPUT_AddDevice( const DIDEVICEINSTANCE* pAvailableInputDevice )
{
	InputDevice			    *pManagedDevice = NULL;
    DWORD					dwDeviceType = pAvailableInputDevice->dwDevType;
	LPDIRECTINPUTDEVICE8	pNewInputDevice = NULL;

    // Create a device handler based on the available input device.
    DXWARN( sInputManager.mpDirectInput->CreateDevice( pAvailableInputDevice->guidInstance, &pNewInputDevice, NULL ));
	ASSERT( pNewInputDevice );
	DXWARN( pNewInputDevice->Unacquire() );

    // Get a copy of the next free input device from the input manager.
	pManagedDevice = &sInputManager.mDevices[sInputManager.mNDevices];
	ASSERT( pManagedDevice );

    // MOUSE
	// ===============================================
    if( GET_DIDEVICE_TYPE(dwDeviceType) == DI8DEVTYPE_MOUSE )
	{
        DXWARN(pNewInputDevice->SetCooperativeLevel( sInputManager.mHWND, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND));
		DXWARN(pNewInputDevice->SetDataFormat(&c_dfDIMouse));
        pManagedDevice->mbPolledDevice = TRUE;
		pManagedDevice->mQuagType      = DEVICE_PC_MOUSE;
	}
	// ===============================================
    // KEYBOARD
	// ===============================================
    else if( GET_DIDEVICE_TYPE(dwDeviceType) == DI8DEVTYPE_KEYBOARD )
	{
        DXWARN(pNewInputDevice->SetCooperativeLevel( sInputManager.mHWND, DISCL_NONEXCLUSIVE|DISCL_BACKGROUND));
		DXWARN(pNewInputDevice->SetDataFormat(&c_dfDIKeyboard));
        pManagedDevice->mbPolledDevice = FALSE;
		pManagedDevice->mQuagType = DEVICE_PC_KEYBOARD;
	}
	// ===============================================
    // JOYSTICK
	// ===============================================
    else if(( GET_DIDEVICE_TYPE(dwDeviceType) == DI8DEVTYPE_JOYSTICK ) ||
			( GET_DIDEVICE_TYPE(dwDeviceType) == DI8DEVTYPE_GAMEPAD ))
	{
//        DXWARN(pNewInputDevice->SetCooperativeLevel( sInputManager.mHWND, DISCL_EXCLUSIVE|DISCL_FOREGROUND));
        DXWARN(pNewInputDevice->SetCooperativeLevel( sInputManager.mHWND, DISCL_NONEXCLUSIVE|DISCL_BACKGROUND));
		DXWARN(pNewInputDevice->SetDataFormat(&c_dfDIJoystick2));
        pManagedDevice->mbPolledDevice = FALSE;
		pManagedDevice->mQuagType = DEVICE_PC_JOYSTICK;

		// ===============================================
		// Setup some properties for the joystick
		// ===============================================
		DIPROPRANGE dipr;
		DIPROPDWORD	dipdw;

		// ===============================================
		// set the range of motion for the axis
		// ===============================================
		dipr.diph.dwSize       = sizeof(DIPROPRANGE);
		dipr.diph.dwHeaderSize = sizeof(dipr.diph);
		dipr.diph.dwHow        = DIPH_DEVICE;//BYOFFSET;
		dipr.lMin              = MIN_RANGE;
		dipr.lMax              = MAX_RANGE;
	    dipr.diph.dwObj        = 0;
		DXWARN(pNewInputDevice->SetProperty( DIPROP_RANGE, &dipr.diph));

		// ===============================================
		// Set the deadzone for the device
		// ===============================================
	    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	    dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	    dipdw.diph.dwHow        = DIPH_DEVICE;
	    dipdw.dwData            = MAX_DEADZONE;
	    dipdw.diph.dwObj        = 0;
		DXWARN(pNewInputDevice->SetProperty( DIPROP_DEADZONE, &dipdw.diph));

		// ===============================================
		// Set the saturation for the device
		// ===============================================
	    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	    dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	    dipdw.diph.dwHow        = DIPH_DEVICE;
	    dipdw.dwData            = MAX_SATURATION;
	    dipdw.diph.dwObj        = 0;
		DXWARN(pNewInputDevice->SetProperty( DIPROP_SATURATION, &dipdw.diph));

	}
	// ===============================================
    // A device was found that we don't care about
	// ===============================================
	else
	{
		x_printf("INPUT: don't recognize \"%s\" as a valid device!\n", pAvailableInputDevice->tszProductName);

	    // Continue enumerating suitable devices
		SAFE_RELEASE(pNewInputDevice);
	    return S_OK;
	}

	// keep a copy of the name
	x_strncpy(pManagedDevice->mName, pAvailableInputDevice->tszProductName, 128-2);


	// Keep pointer to device
    pManagedDevice->mpDIDevice = pNewInputDevice;

	// Get the capabilities of this input device.
	pManagedDevice->mDeviceCaps.dwSize = sizeof(DIDEVCAPS);
	DXWARN(pManagedDevice->mpDIDevice->GetCapabilities(&pManagedDevice->mDeviceCaps));

	// increase the number of devices we have, and check for too many
    sInputManager.mNDevices++;
	ASSERT(sInputManager.mNDevices < MAX_INPUT_DEVICES);

	// Create the rumble effect
	if( pManagedDevice->mDeviceCaps.dwFlags & DIDC_FORCEFEEDBACK )
	{
	    DWORD           rgdwAxes[2]     = { DIJOFS_X, DIJOFS_Y };
	    LONG            rglDirection[2] = { 0, 0 };
		DICONSTANTFORCE cf              = { 0 };

		DIEFFECT eff;
		ZeroMemory( &eff, sizeof(eff) );
		eff.dwSize                  = sizeof(DIEFFECT);
		eff.dwFlags                 = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
		eff.dwDuration              = INFINITE;
		eff.dwSamplePeriod          = 0;
		eff.dwGain                  = DI_FFNOMINALMAX;
		eff.dwTriggerButton         = DIEB_NOTRIGGER;
		eff.dwTriggerRepeatInterval = 0;
		eff.cAxes                   = 1;
		eff.rgdwAxes                = rgdwAxes;
		eff.rglDirection            = rglDirection;
		eff.lpEnvelope              = 0;
		eff.cbTypeSpecificParams    = sizeof(DICONSTANTFORCE);
		eff.lpvTypeSpecificParams   = &cf;
		eff.dwStartDelay            = 0;

		// Create the prepared effect
		DXWARN( pManagedDevice->mpDIDevice->CreateEffect( GUID_ConstantForce, &eff, &pManagedDevice->mpEffect, NULL ) );
	}

    // Continue enumerating suitable devices
    return S_OK;
}


//=================================================================
// callback function for enumerating devices
//=================================================================
xbool CALLBACK EnumDevicesCB( LPCDIDEVICEINSTANCE pAvailableInputDevice, VOID* pContext )
{
x_printf("[Input] adding device: %s\n", pAvailableInputDevice->tszProductName);

    INPUT_AddDevice( pAvailableInputDevice);

    // Continue enumerating suitable devices
    return DIENUM_CONTINUE;
}


//=================================================================
// INPUT_ConfigureDevices
//=================================================================
HRESULT INPUT_ConfigureDevices( HWND hWnd )
{
	s32			i;
    HRESULT		Result;

    // Unacquire the devices so that mouse doesn't control the game while in control panel
    for( i = 0; i < sInputManager.mNDevices; i++ )
        DXWARN(sInputManager.mDevices[i].mpDIDevice->Unacquire());

	Result = sInputManager.mpDirectInput->EnumDevices(DI8DEVCLASS_ALL, EnumDevicesCB, NULL, DIEDFL_ALLDEVICES);
	DXWARN(Result);

	// setup buffers
    for( i = 0; i < sInputManager.mNDevices; i++ )
	{
		InputDevice	*pManagedDevice;

		pManagedDevice = &sInputManager.mDevices[i];
		ASSERT(pManagedDevice);

        // Build the data buffer for this device.
        VERIFY(INPUT_BufferInit(pManagedDevice, BUFFER_SIZE));
  
		Result = pManagedDevice->mpDIDevice->Acquire();
		DXWARN(Result);
	}

    return Result;
}


//============================================================================
// INPUT_GetData
//----------------------------------------------------------------------------
// This function is used to read the devices data. The poll devices we need to
// extract the data by first calliong the Polling function in DInput.
// NOTE: When the device is disconected this function will not return an error,
//       The user need to call "IsDeviceConnected" to detect that.
//============================================================================
err PCINPUT_GetData(InputDevice *pIDevice, s32 *nEvents)
{
    HRESULT     Result;

    //-----------------------------------------------------------------------
    // Check States
    //-----------------------------------------------------------------------
    ASSERT( sInputManager.mpDirectInput );

	// start with no events incase there is an error
    *nEvents = 0;

    //------------------------------------------------------------------------
    // Handle devices that need polling
    //------------------------------------------------------------------------
	Result = DI_OK;
	if( pIDevice->mbPolledDevice )
	{
		pIDevice->mpDIDevice->Acquire();
		Result = pIDevice->mpDIDevice->Poll();

		// dodgy hack
		if (Result == DIERR_UNPLUGGED)
		{
			x_printfxy(2,20, "POLL - error");
			return ERR_SUCCESS;
		}
//		{
//			DXWARN(pIDevice->mpDIDevice->Unacquire());
//			DXWARN(pIDevice->mpDIDevice->Acquire());
//		}
	}

    //------------------------------------------------------------------------
    // Try to obtain the device data.
    //------------------------------------------------------------------------
    if( !FAILED(Result) )
	{
		*nEvents = BUFFER_SIZE;

        Result = pIDevice->mpDIDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),
												    (DIDEVICEOBJECTDATA*)sDeviceData,
												    (unsigned long*)nEvents,
												     0);

//#ifdef USEDEBUGTEXT
		if (Result == DI_BUFFEROVERFLOW)
			x_printf("INPUT: BUFFER OVERFLOWED!\n");
//#endif

	    //------------------------------------------------------------------------
	    // If we have an error make sure that is not because it is not acquired
	    //------------------------------------------------------------------------
	    if( FAILED(Result) )
	    {
			// why did it fail?
			DXWARN(Result);

			// it did fail, so don't use any of the data
			*nEvents = 0;

	        Result = pIDevice->mpDIDevice->Acquire();
	        while( Result == DIERR_INPUTLOST ) 
	            Result = pIDevice->mpDIDevice->Acquire();

	        if ( FAILED(Result) )
	        {
				DXWARN(Result);
	            return ERR_FAILURE;
	        }
	    }
	}

    return ERR_SUCCESS;
}

void PCINPUT_ResetMouseState( InputDevice* pIDevice )
{
    MouseInputState*  State = &pIDevice->mState.Mouse; 
	//-------------------------------------------------------------
	// reset locator values to the current state of the mouse
	//-------------------------------------------------------------
	DIMOUSESTATE     MouseState;
	pIDevice->mpDIDevice->GetDeviceState( sizeof(DIMOUSESTATE), &MouseState );
	State->X =  ((f32)((s32)MouseState.lX)*1.3f) / MAX_MOUSE_MOVEMENT_SPEED;
	State->Y = -(f32)(s32)MouseState.lY / MAX_MOUSE_MOVEMENT_SPEED;
	State->Z =  (f32)(s32)MouseState.lZ / MAX_MOUSE_MOVEMENT_SPEED;

	if( State->X >  1.0f ) State->X =  1.0f;
	if( State->Y >  1.0f ) State->Y =  1.0f;
	if( State->Z >  1.0f ) State->Z =  1.0f;

	if( State->X < -1.0f ) State->X = -1.0f;
	if( State->Y < -1.0f ) State->Y = -1.0f;
	if( State->Z < -1.0f ) State->Z = -1.0f;
}

//=================================================================
// MouseToInputEvent
//=================================================================
static void PCINPUT_MouseToInputEvent( input_event& InputEvent, InputDevice* pIDevice, DIDEVICEOBJECTDATA* Data )
{
    s32               ButtonN;
    MouseInputState*  State; 

	PCINPUT_ResetMouseState( pIDevice );
    //-------------------------------------------------------------
    // Initialize local variables
    //-------------------------------------------------------------
    State = &pIDevice->mState.Mouse;

    //-------------------------------------------------------------
    // Fill general info about the event
    //-------------------------------------------------------------
    InputEvent.DeviceNum  = (s16)pIDevice->mGroupID;
    InputEvent.Timestamp  = Data->dwTimeStamp;
    InputEvent.DeviceType = DEVICE_PC_MOUSE;

    //-------------------------------------------------------------
    // Update device
    //-------------------------------------------------------------
//    State->X = State->Y = State->Z = 0; // For relative devices they don't have really a previous state
    switch( Data->dwOfs )
    {
        case DIMOFS_BUTTON0: ButtonN = 0; InputEvent.Info = ISPRESS(Data->dwData); break;
        case DIMOFS_BUTTON1: ButtonN = 1; InputEvent.Info = ISPRESS(Data->dwData); break;
        case DIMOFS_BUTTON2: ButtonN = 2; InputEvent.Info = ISPRESS(Data->dwData); break;
        case DIMOFS_BUTTON3: ButtonN = 3; InputEvent.Info = ISPRESS(Data->dwData); break;
        case DIMOFS_BUTTON4: ButtonN = 4; InputEvent.Info = ISPRESS(Data->dwData); break;
        case DIMOFS_BUTTON5: ButtonN = 5; InputEvent.Info = ISPRESS(Data->dwData); break;
        case DIMOFS_BUTTON6: ButtonN = 6; InputEvent.Info = ISPRESS(Data->dwData); break;
        case DIMOFS_BUTTON7: ButtonN = 7; InputEvent.Info = ISPRESS(Data->dwData); break;
        case DIMOFS_X:       InputEvent.X =  ((f32)((s32)Data->dwData)*1.3f) / MAX_MOUSE_MOVEMENT_SPEED; break;
        case DIMOFS_Y:       InputEvent.Y = -(f32)(s32)Data->dwData / MAX_MOUSE_MOVEMENT_SPEED; break;
        case DIMOFS_Z:       InputEvent.Z =  (f32)(s32)Data->dwData / MAX_MOUSE_MOVEMENT_SPEED; break;

        default: ASSERT(FALSE);
    }

    //-------------------------------------------------------------
    // Populate specifics about the message
    //-------------------------------------------------------------
    switch( Data->dwOfs )
    {
        case DIMOFS_BUTTON0: 
        case DIMOFS_BUTTON1: 
        case DIMOFS_BUTTON2: 
        case DIMOFS_BUTTON3: 
        case DIMOFS_BUTTON4: 
        case DIMOFS_BUTTON5: 
        case DIMOFS_BUTTON6: 
        case DIMOFS_BUTTON7: 
                InputEvent.GadgetType = GADGET_TYPE_BUTTON;
                InputEvent.GadgetID   = (s16)(GADGET_PC_MBTN_0 + ButtonN);
                break;

        case DIMOFS_X:       
        case DIMOFS_Y:       
        case DIMOFS_Z:       
                InputEvent.Info       = LOCATOR_REL;
                InputEvent.GadgetType = GADGET_TYPE_LOCATOR_XYZ; 
                InputEvent.GadgetID   = GADGET_PC_MLOC_XYZ;
                break;

        default: ASSERT(FALSE);
    }
}

//=================================================================
// KeyboardToInputEvent
//=================================================================
static void PCINPUT_KeyboardToInputEvent( input_event& InputEvent, InputDevice* pIDevice, DIDEVICEOBJECTDATA* Data )
{
    s32                 Key;
    KeyboardInputState* State;

    //-------------------------------------------------------------
    // Initialize local variables
    //-------------------------------------------------------------
    State = &pIDevice->mState.Keyboard;

    //-------------------------------------------------------------
    // Fill general info about the event
    //-------------------------------------------------------------
    InputEvent.DeviceNum  = (s16)pIDevice->mGroupID;
    InputEvent.Timestamp  = Data->dwTimeStamp;
    InputEvent.DeviceType = DEVICE_PC_KEYBOARD;

    //-------------------------------------------------------------
    // Update device
    //-------------------------------------------------------------
    Key = Data->dwOfs - DIK_ESCAPE;
    InputEvent.Info = ISPRESS(Data->dwData);

    //-------------------------------------------------------------
    // Populate specifics about the message
    //-------------------------------------------------------------
    InputEvent.GadgetType = GADGET_TYPE_BUTTON;
    InputEvent.GadgetID   = (s16)(GADGET_PC_KBD_ESCAPE + Key) ;

}


//=================================================================
// JoystickToInputEvent
//=================================================================
static void PCINPUT_JoystickToInputEvent( input_event& InputEvent, InputDevice* pIDevice, DIDEVICEOBJECTDATA* Data )
{
    s32                 Key;
    s32                 Type;
    JoystickInputState* State;

    //-------------------------------------------------------------
    // Initialize local variables
    //-------------------------------------------------------------
    State = &pIDevice->mState.Joystick;

    //-------------------------------------------------------------
    // Fill general info about the event
    //-------------------------------------------------------------
    InputEvent.DeviceNum  = (s16)pIDevice->mGroupID;
    InputEvent.Timestamp  = Data->dwTimeStamp;
    InputEvent.DeviceType = DEVICE_PC_JOYSTICK;

    //-------------------------------------------------------------
    // Update device
    //-------------------------------------------------------------

    //---------------------------------------------------------
    // JOYSTICK BUTTONS
    //---------------------------------------------------------
    if ( ( Data->dwOfs >= DIJOFS_BUTTON0 ) && ( Data->dwOfs <= DIJOFS_BUTTON31 ))
    {
        Type = 0;

        Key = (Data->dwOfs - DIJOFS_BUTTON0) / (DIJOFS_BUTTON1 - DIJOFS_BUTTON0);
        InputEvent.Info = ISPRESS(Data->dwData);
    }
    //---------------------------------------------------------
    // JOYSTICK HATS
    //---------------------------------------------------------
    else
    if ( ( Data->dwOfs >= DIJOFS_POV(0) ) && ( Data->dwOfs <= DIJOFS_POV(3) ) )
    {
        Type = 1;

        Key = (Data->dwOfs - DIJOFS_POV(0)) / (DIJOFS_POV(1) - DIJOFS_POV(0));

        if ( Data->dwData == -1 )
        {
            InputEvent.X = 0;
            InputEvent.Y = 0;
        }
        else
        {
			f32 Radians = DEG_TO_RAD(Data->dwData / DI_DEGREES);

            InputEvent.X = x_sin( Radians ) * MAX_LOCATOR_RANGE;
            InputEvent.Y = x_cos( Radians ) * MAX_LOCATOR_RANGE;

//            f32 Radians = Data->dwData * ((-1 / 100.0f) * (PI / 180.0f));
//            State->Hat[ Key ].X = x_cos( Radians ) * MAX_LOCATOR_RANGE;
//            State->Hat[ Key ].Y = x_sin( Radians ) * MAX_LOCATOR_RANGE;
        }
    }
    //---------------------------------------------------------
    // JOYSTICK SLIDER
    //---------------------------------------------------------
    else
    if ( ( Data->dwOfs >= DIJOFS_SLIDER(0) ) && ( Data->dwOfs <= DIJOFS_SLIDER(2) ) )
    {
        Type = 2;

        Key = (Data->dwOfs - DIJOFS_SLIDER(0)) / (DIJOFS_SLIDER(1) - DIJOFS_SLIDER(0));

        InputEvent.X = SetInRange( Data->dwData, MAX_LOCATOR_RANGE );
    }
    //---------------------------------------------------------
    // JOYSTICK ROTATION AND STICK
    //---------------------------------------------------------
    else
    {
        Key = 0;

		//-------------------------------------------------------------
		// reset locator values to the current state of the joystick
		//-------------------------------------------------------------
		DIJOYSTATE2     JoyState;
		pIDevice->mpDIDevice->GetDeviceState( sizeof(DIJOYSTATE2), &JoyState );

        switch( Data->dwOfs )
        {
        case DIJOFS_RX:
        case DIJOFS_RY:
        case DIJOFS_RZ:
			Type = 3;
			InputEvent.GadgetID   = (s16)(GADGET_PC_JLOC_ROTATION0 + Key);
			InputEvent.X    =   SetInRange(  JoyState.lRx, MAX_LOCATOR_RANGE );
	        InputEvent.Y    =  -SetInRange(  JoyState.lRy, MAX_LOCATOR_RANGE );
	        InputEvent.Z    =   SetInRange(  JoyState.lRz, MAX_LOCATOR_RANGE );
			break;
        case DIJOFS_X:
        case DIJOFS_Y:
        case DIJOFS_Z:
			Type = 4;
			InputEvent.GadgetID   = (s16)(GADGET_PC_JLOC_STICK0 + Key);
			InputEvent.X    =   SetInRange(  JoyState.lX, MAX_LOCATOR_RANGE );
	        InputEvent.Y    =  -SetInRange(  JoyState.lY, MAX_LOCATOR_RANGE );
	        InputEvent.Z    =   SetInRange(  JoyState.lZ, MAX_LOCATOR_RANGE );
			break;

        default: ASSERT( FALSE ) ;
        }
    }

    //-------------------------------------------------------------
    // Populate specifics about the message
    //-------------------------------------------------------------
    switch( Type )
    {
        case 0:     InputEvent.GadgetType = GADGET_TYPE_BUTTON;
                    InputEvent.GadgetID   = (s16)(GADGET_PC_JBTN_00 + Key);
                    break;

        case 1:     InputEvent.GadgetID   = (s16)(GADGET_PC_JLOC_HAT0 + Key) ;
                    InputEvent.GadgetType = GADGET_TYPE_LOCATOR_XY;
                    InputEvent.Info       = LOCATOR_ABS;
                    break;

        case 2:     InputEvent.GadgetID   = (s16)(GADGET_PC_JLOC_SLIDER0 + Key) ;
                    InputEvent.GadgetType = GADGET_TYPE_LOCATOR_X;
                    InputEvent.Info       = LOCATOR_ABS;
                    break;

        case 3:     InputEvent.GadgetID   = (s16)(GADGET_PC_JLOC_ROTATION0 + Key);
                    InputEvent.GadgetType = GADGET_TYPE_LOCATOR_XYZ;
                    InputEvent.Info       = LOCATOR_ABS;
                    break;

        case 4:     InputEvent.GadgetID   = (s16)(GADGET_PC_JLOC_STICK0 + Key);
                    InputEvent.GadgetType = GADGET_TYPE_LOCATOR_XYZ;
                    InputEvent.Info       = LOCATOR_ABS;
                    break;

        default: ASSERT( FALSE );
    }
}

//=================================================================
// INPUT_PollEvents
//=================================================================
static err PCINPUT_PollEvents( void )
{
    s32           i;
    s32           NEvents;
    s32           e;
    input_event   Event;

	for (i=0; i<sInputManager.mNDevices; i++)
	{
	    InputDevice	*pIDevice = &sInputManager.mDevices[i];
		ASSERT(pIDevice);

        //-------------------------------------------------------------
        // Query for curent events
        //-------------------------------------------------------------
        NEvents = 0;
        if( PCINPUT_GetData( pIDevice, &NEvents ))
		{
			if( NEvents == 0 && pIDevice->mQuagType == DEVICE_PC_MOUSE ) PCINPUT_ResetMouseState( pIDevice );

			for( e = 0; e < NEvents; e++ )
			{
				x_memset(&Event, 0, sizeof(input_event));

				switch (pIDevice->mQuagType)
				{
				    case DEVICE_PC_MOUSE:		PCINPUT_MouseToInputEvent(Event,    pIDevice, &sDeviceData[e]); break;
				    case DEVICE_PC_KEYBOARD:	PCINPUT_KeyboardToInputEvent(Event, pIDevice, &sDeviceData[e]); break;
				    case DEVICE_PC_JOYSTICK:	PCINPUT_JoystickToInputEvent(Event, pIDevice, &sDeviceData[e]); break;

				    default:
    					x_printf("don't know what this device is\n");
					    ASSERT(FALSE);
				}

				// Make sure all locator values are within range
				if( Event.X >  1.0f ) Event.X =  1.0f;
				if( Event.Y >  1.0f ) Event.Y =  1.0f;
				if( Event.Z >  1.0f ) Event.Z =  1.0f;

				if( Event.X < -1.0f ) Event.X = -1.0f;
				if( Event.Y < -1.0f ) Event.Y = -1.0f;
				if( Event.Z < -1.0f ) Event.Z = -1.0f;

				INPUTQ_EnQueue( Event );
			}
		}
	}

    return ERR_SUCCESS;
}

//=================================================================
// INPUT_InitModule
//=================================================================
err INPUT_InitModule( HWND hWnd )
{
	// can only initialize once
    ASSERT( !sbInitialized );
    sbInitialized = TRUE;

	// clear out our structure
	x_memset(&sInputManager, 0, sizeof(InputManager));

    // Initialize the input queue.
    INPUTQ_InitModule();

    // Store data
    sInputManager.mHWND = hWnd;

     // Create the base DirectInput object
    DXCHECK( DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&sInputManager.mpDirectInput, NULL) );

	INPUT_ConfigureDevices( hWnd );

    return ERR_SUCCESS;
}

//=================================================================
// INPUT_KillModule
//=================================================================
void INPUT_KillModule( void )
{
    ASSERT( sbInitialized );
    sbInitialized = FALSE;
    int i;

	// release all the devices
    for( i = 0; i < sInputManager.mNDevices; i++ )
	{
		InputDevice	*pManagedDevice = &sInputManager.mDevices[i];
		ASSERT(pManagedDevice);

        DXWARN(pManagedDevice->mpDIDevice->Unacquire( ));
		SAFE_RELEASE(pManagedDevice->mpDIDevice);
	}

	// release the input object
	SAFE_RELEASE(sInputManager.mpDirectInput);

    // Shut down the input queue.
    INPUTQ_KillModule();
}



//=================================================================
// INPUT_CheckDevices
//=================================================================
void INPUT_CheckDevices( void )
{
    //---------------------------------------------------------
    // Read Direct Input Messages
    //---------------------------------------------------------
    PCINPUT_PollEvents();

}

//============================================================================
// BufferInit
//----------------------------------------------------------------------------
// Note: Certain devices such the Joysticks are consider "DI_POLLEDDEVICE"
//       That means that they can't have a buffer. Result will return 
//       DI_POLLEDDEVICE and the buffer will not be created. 
//============================================================================
static err INPUT_BufferInit( InputDevice*   pIDevice, s32 BufferSize )
{
    DIPROPDWORD   PropDWord;
    HRESULT       Result;

    //-----------------------------------------------------------------------
    // Check States
    //-----------------------------------------------------------------------
    ASSERT( pIDevice );
	ASSERT( sInputManager.mpDirectInput);

    
    //-----------------------------------------------------------------------
    // Create The Buffer 
    //-----------------------------------------------------------------------
    PropDWord.diph.dwSize       = sizeof(DIPROPDWORD);
    PropDWord.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    PropDWord.diph.dwObj        = 0;
    PropDWord.diph.dwHow        = DIPH_DEVICE;
    PropDWord.dwData            = BufferSize;

    Result = pIDevice->mpDIDevice->SetProperty(DIPROP_BUFFERSIZE, &PropDWord.diph );

	// Is it a polled device?
	if (Result == DI_POLLEDDEVICE)
	{
		pIDevice->mbPolledDevice = TRUE;
	}
	else
		DXWARN(Result);


	if( FAILED(Result) )
	{
		ASSERTS(FALSE, "Unable to create a Buffer to Store Input States");
		return ERR_FAILURE;
	}

    return ERR_SUCCESS;
}


//=================================================================
// Gadget2Device
//=================================================================
s32 Gadget2Device( s32 Gadget )
{
    if ((Gadget >= GADGET_PC_JLOC_GROUP_BEGIN) && 
        (Gadget <= GADGET_PC_JLOC_GROUP_END  ))
	{
		return DEVICE_PC_JOYSTICK;
	}

    if ((Gadget >= GADGET_PC_JBTN_GROUP_BEGIN) && 
        (Gadget <= GADGET_PC_JBTN_GROUP_END  ))
	{
		return DEVICE_PC_JOYSTICK;
	}

    if ((Gadget >= GADGET_PC_MLOC_GROUP_BEGIN) && 
        (Gadget <= GADGET_PC_MLOC_GROUP_END  ))
	{
		return DEVICE_PC_MOUSE;
	}

    if ((Gadget >= GADGET_PC_MBTN_GROUP_BEGIN) && 
        (Gadget <= GADGET_PC_MBTN_GROUP_END  ))
	{
		return DEVICE_PC_MOUSE;
	}

    if ((Gadget >= GADGET_PC_KBD_BUTTON_GROUP_BEGIN) && 
        (Gadget <= GADGET_PC_KBD_BUTTON_GROUP_END  ))
	{
		return DEVICE_PC_KEYBOARD;
	}

    return -1;
}




//=================================================================
// INPUT_GetState
//=================================================================
static
void INPUT_GetState( input_event& Event )
{
	s32				i;
	InputDevice	*pIDevice;
	xbool			bFound;
	s32				device;

    //-------------------------------------------------------------
    // get the input module instance
    //-------------------------------------------------------------
	ASSERT ( sInputManager.mpDirectInput );
//    if ( sInputManager.mpDirectInput == NULL ) 
//    {
//        x_memset( &Event, 0, sizeof(input_event) );
//        return;
//    }

	// device what the device is from the GadgetID
    device = Gadget2Device( Event.GadgetID );

	// find the device in the device list
	bFound = FALSE;
	for (i=0; i<sInputManager.mNDevices; i++)
	{
		pIDevice = &sInputManager.mDevices[i];
		ASSERT(pIDevice);
		if (pIDevice->mQuagType == device)
		{
			bFound = TRUE;
			break;
		}
	}

	// not found the device that generated this message
	if (bFound == FALSE)
		return;
	

    switch( device )
    {
	///////////////////////////////////////////////////////////////////////////
	// ---- KEYBOARD ----
    case DEVICE_PC_KEYBOARD:
        ASSERT ( pIDevice->mpDIDevice );
//        if ( pIDevice->mpDIDevice == NULL ) 
//        {
//            x_memset( &Event, 0, sizeof(input_event) );
//            return;
//        }
        
        //-----------------------------------------------------
        // Return whether the buttom was press or not
        //-----------------------------------------------------
        Event.Info = pIDevice->mState.Keyboard.Key[ Event.GadgetID - GADGET_PC_KBD_ESCAPE ];
        break;

	///////////////////////////////////////////////////////////////////////////
	// ---- MOUSE ----
    case DEVICE_PC_MOUSE:
        ASSERT ( pIDevice->mpDIDevice );
//        if ( pIDevice->mpDIDevice == NULL ) 
//        {
//            x_memset( &Event, 0, sizeof(input_event) );
//            return;
//        }
        
        //-----------------------------------------------------
        // If it is not a button return false
        //-----------------------------------------------------
        if ( Event.GadgetID <= GADGET_PC_MBTN_GROUP_BEGIN )
        {
            //-----------------------------------------------------
            // Set the axis for the mouse
            //-----------------------------------------------------
            Event.X = pIDevice->mState.Mouse.X;
            Event.Y = pIDevice->mState.Mouse.Y;
            Event.Z = pIDevice->mState.Mouse.Z;
        }
        else
        {
            //-----------------------------------------------------
            // Return whether the buttom was press or not
            //-----------------------------------------------------
            Event.Info = pIDevice->mState.Mouse.Button[ Event.GadgetID - GADGET_PC_MBTN_0 ];
        }
        break;

	///////////////////////////////////////////////////////////////////////////
   	// ---- JOYSTICK ----
	case DEVICE_PC_JOYSTICK:
        ASSERT ( pIDevice->mpDIDevice );
//        if ( pIDevice->mpDIDevice == NULL ) 
//        {
//            x_memset( &Event, 0, sizeof(input_event) );
//            return;
//        }
        
        //-----------------------------------------------------
        // If it is not a button return false
        //-----------------------------------------------------
        if ( Event.GadgetID <= GADGET_PC_JBTN_GROUP_BEGIN )
        {
            //-----------------------------------------------------
            // Set the axis for the joystick
            //-----------------------------------------------------
            if ( Event.GadgetID <= GADGET_PC_JLOC_STICK3 )
            {
                Event.X = pIDevice->mState.Joystick.Stick[ Event.GadgetID - GADGET_PC_JLOC_STICK0 ].X;
                Event.Y = pIDevice->mState.Joystick.Stick[ Event.GadgetID - GADGET_PC_JLOC_STICK0 ].Y;
                Event.Z = pIDevice->mState.Joystick.Stick[ Event.GadgetID - GADGET_PC_JLOC_STICK0 ].Z;
            }
            else
            if ( Event.GadgetID <= GADGET_PC_JLOC_ROTATION3 )
            {
                Event.X = pIDevice->mState.Joystick.Rotation[ Event.GadgetID - GADGET_PC_JLOC_ROTATION0 ].X;
                Event.Y = pIDevice->mState.Joystick.Rotation[ Event.GadgetID - GADGET_PC_JLOC_ROTATION0 ].Y;
                Event.Z = pIDevice->mState.Joystick.Rotation[ Event.GadgetID - GADGET_PC_JLOC_ROTATION0 ].Z;
            }
            else
            if ( Event.GadgetID <= GADGET_PC_JLOC_SLIDER3 )
            {
                Event.X = pIDevice->mState.Joystick.Slider[ Event.GadgetID - GADGET_PC_JLOC_SLIDER0 ];
            }
            else
            if ( Event.GadgetID <= GADGET_PC_JLOC_HAT3 )
            {
                Event.X = pIDevice->mState.Joystick.Hat[ Event.GadgetID - GADGET_PC_JLOC_HAT0 ].X;
                Event.Y = pIDevice->mState.Joystick.Hat[ Event.GadgetID - GADGET_PC_JLOC_HAT0 ].Y;
                Event.Z = pIDevice->mState.Joystick.Hat[ Event.GadgetID - GADGET_PC_JLOC_HAT0 ].Z;
            }
            else
            {
                ASSERTS( FALSE, "UNKNOWN JOYSTICK GADGET" );
            }
        }
        else
        {
            //-----------------------------------------------------
            // Return whether the buttom was press or not
            //-----------------------------------------------------
            Event.Info = pIDevice->mState.Joystick.Button[ Event.GadgetID - GADGET_PC_JBTN_00 ];
        }

        break;
    }
}

//=================================================================
// INPUT_SetState
//=================================================================
static
void INPUT_SetState( input_event& Event )
{
	s32				i;
	InputDevice	*pIDevice;
	xbool			bFound;
	s32				device;

    //-------------------------------------------------------------
    // get the input module instance
    //-------------------------------------------------------------
    ASSERT( sInputManager.mpDirectInput );

	// device what the device is from the GadgetID
    device = Gadget2Device( Event.GadgetID );

	// find the device in the device list
	bFound = FALSE;
	for (i=0; i<sInputManager.mNDevices; i++)
	{
		pIDevice = &sInputManager.mDevices[i];
		ASSERT(pIDevice);
		if (pIDevice->mQuagType == device)
		{
			bFound = TRUE;
			break;
		}
	}

	// not found the device that generated this message
	if ((bFound == FALSE) || (pIDevice->mpDIDevice == NULL))
	{
		ASSERT(FALSE);
		return;
	}
	

    switch( device )
    {
	///////////////////////////////////////////////////////////////////////////
   	// ---- KEYBOARD ----
    case DEVICE_PC_KEYBOARD:
        //-----------------------------------------------------
        // Return whether the buttom was press or not
        //-----------------------------------------------------
        if ( Event.Info )
        {
            pIDevice->mState.Keyboard.Key[ Event.GadgetID - GADGET_PC_KBD_ESCAPE ] = PC_INPUT_DEBOUNCE | PC_INPUT_PRESSED;
        }
        else
        {
            pIDevice->mState.Keyboard.Key[ Event.GadgetID - GADGET_PC_KBD_ESCAPE ] &= ~PC_INPUT_DEBOUNCE;
        }
        break;

	///////////////////////////////////////////////////////////////////////////
   	// ---- MOUSE ----
    case DEVICE_PC_MOUSE:
        //-----------------------------------------------------
        // If it is not a button return false
        //-----------------------------------------------------
        if ( Event.GadgetID <= GADGET_PC_MBTN_GROUP_BEGIN )
        {
            //-----------------------------------------------------
            // Set the axis for the mouse
            //-----------------------------------------------------
            pIDevice->mState.Mouse.X = Event.X;
            pIDevice->mState.Mouse.Y = Event.Y;
            pIDevice->mState.Mouse.Z = Event.Z;
        }
        else
        {
            //-----------------------------------------------------
            // Return whether the buttom was press or not
            //-----------------------------------------------------
            if ( Event.Info )
            {
                pIDevice->mState.Mouse.Button[ Event.GadgetID - GADGET_PC_MBTN_0 ] = PC_INPUT_DEBOUNCE | PC_INPUT_PRESSED;
            }
            else
            {
                pIDevice->mState.Mouse.Button[ Event.GadgetID - GADGET_PC_MBTN_0 ] &= ~PC_INPUT_DEBOUNCE;
            }
        }
        break;

	///////////////////////////////////////////////////////////////////////////
   	// ---- JOYSTICK ----
    case DEVICE_PC_JOYSTICK:
        //-----------------------------------------------------
        // If it is not a button return false
        //-----------------------------------------------------
        if ( Event.GadgetID <= GADGET_PC_JBTN_GROUP_BEGIN )
        {
            //-----------------------------------------------------
            // Set the axis for the joystick
            //-----------------------------------------------------
            if ( Event.GadgetID <= GADGET_PC_JLOC_STICK3 )
            {
                pIDevice->mState.Joystick.Stick[ Event.GadgetID - GADGET_PC_JLOC_STICK0 ].X = Event.X;
                pIDevice->mState.Joystick.Stick[ Event.GadgetID - GADGET_PC_JLOC_STICK0 ].Y = Event.Y;
                pIDevice->mState.Joystick.Stick[ Event.GadgetID - GADGET_PC_JLOC_STICK0 ].Z = Event.Z;
            }
            else
            if ( Event.GadgetID <= GADGET_PC_JLOC_ROTATION3 )
            {
                pIDevice->mState.Joystick.Rotation[ Event.GadgetID - GADGET_PC_JLOC_ROTATION0 ].X = Event.X;
                pIDevice->mState.Joystick.Rotation[ Event.GadgetID - GADGET_PC_JLOC_ROTATION0 ].Y = Event.Y;
                pIDevice->mState.Joystick.Rotation[ Event.GadgetID - GADGET_PC_JLOC_ROTATION0 ].Z = Event.Z;
            }
            else
            if ( Event.GadgetID <= GADGET_PC_JLOC_SLIDER3 )
            {
                pIDevice->mState.Joystick.Slider[ Event.GadgetID - GADGET_PC_JLOC_SLIDER0 ] = Event.X;
            }
            else
            if ( Event.GadgetID <= GADGET_PC_JLOC_HAT3 )
            {
                pIDevice->mState.Joystick.Hat[ Event.GadgetID - GADGET_PC_JLOC_HAT0 ].X = Event.X;
                pIDevice->mState.Joystick.Hat[ Event.GadgetID - GADGET_PC_JLOC_HAT0 ].Y = Event.Y;
                pIDevice->mState.Joystick.Hat[ Event.GadgetID - GADGET_PC_JLOC_HAT0 ].Z = Event.Z;
            }
            else
            {
                ASSERTS( FALSE, "UNKNOWN JOYSTICK GADGET" );
            }
        }
        else
        {
            //-----------------------------------------------------
            // Return whether the buttom was press or not
            //-----------------------------------------------------
            if ( Event.Info )
            {
                pIDevice->mState.Joystick.Button[ Event.GadgetID - GADGET_PC_JBTN_00 ] = PC_INPUT_DEBOUNCE | PC_INPUT_PRESSED;;
            }
            else
            {
                pIDevice->mState.Joystick.Button[ Event.GadgetID - GADGET_PC_JBTN_00 ] &= ~PC_INPUT_DEBOUNCE;
            }
        }

        break;
    }
}


//=================================================================
// INPUT_GetNextEvent
//=================================================================

xbool INPUT_GetNextEvent( input_event& Event )	
{
    s32                 NEvents;
    xbool               MoreMessages;

    //-------------------------------------------------------------
    // get the number of events
    //-------------------------------------------------------------
    NEvents = INPUTQ_GetNEvents();

    //---------------------------------------------------------
    // Events
    //---------------------------------------------------------
    if ( NEvents == 0 )
    {
        MSG Msg;

        //---------------------------------------------------------
        // Read windows Messages
        //---------------------------------------------------------
        while( PeekMessage( &Msg, NULL, 0, 0, PM_NOREMOVE) )
        {
            if ( !GetMessage( &Msg, NULL, 0, 0 ) )
            {
                return TRUE;
            }

            TranslateMessage( &Msg );
            DispatchMessage( &Msg );
        }

        //---------------------------------------------------------
        // Next set of messages will be handle next time around
        // Because we always have messages!!
        //---------------------------------------------------------
        return FALSE;
    }

    //---------------------------------------------------------
    // get the event
    //---------------------------------------------------------    
    MoreMessages = INPUTQ_DeQueue( Event );

    //---------------------------------------------------------
    // Update the device state
    //---------------------------------------------------------    
    INPUT_SetState( Event );

    //---------------------------------------------------------
    // return
    //---------------------------------------------------------    
    return MoreMessages;
}

//=================================================================
// INPUT_ClearDebounce
//=================================================================
void INPUT_ClearDebounce( void )
{
    s32				i, j;
    InputDevice*	pIDevice;

    //-------------------------------------------------------------
    // get the input module instance
    //-------------------------------------------------------------
    if ( sInputManager.mpDirectInput == NULL )
		return;

	for (i=0; i<sInputManager.mNDevices; i++)
	{
		pIDevice = &sInputManager.mDevices[i];
		ASSERT(pIDevice);

		switch (pIDevice->mQuagType)
		{
			//-------------------------------------------------------------
			// Clear every mouse button
			//-------------------------------------------------------------
			case DEVICE_PC_MOUSE:
				if ( pIDevice->mpDIDevice )
				{
					for ( j = 0; j < 4; j ++)
					{
						pIDevice->mState.Mouse.Button[ j ] &= ~2;
					}
				}
				break;

		    //-------------------------------------------------------------
		    // Clear every keyboard button
		    //-------------------------------------------------------------
			case DEVICE_PC_KEYBOARD:
				if ( pIDevice->mpDIDevice )
				{
					for ( j = 0; j < GADGET_PC_KBD_BUTTON_GROUP_END - GADGET_PC_KBD_ESCAPE; j ++)
					{
						pIDevice->mState.Keyboard.Key[ j ] &= ~2;
					}
				}
				break;
    
			//-------------------------------------------------------------
			// Clear every Joy button
			//-------------------------------------------------------------
			case DEVICE_PC_JOYSTICK:
				if ( pIDevice->mpDIDevice )
				{
					for ( j = 0; j < 32; j ++)
					{
						pIDevice->mState.Joystick.Button[ j ] &= ~2;
					}
				}
				break;
		}
	}
}

// These are not complete functions by any means, it was just filled to appease the linking of the project.
s32 INPUT_ReturnLowestConnectedController( void )
{
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------
// These are not complete functions by any means, it was just filled to appease the linking of the project
xbool INPUT_IsControllerConnected( s32 ControllerID )
{
    return TRUE;
}

//=================================================================
// INPUT_GetButton
//=================================================================
xbool INPUT_GetButton( s16 DeviceNum, s16 GadgetID )
{
    input_event Event;

    Event.DeviceNum = DeviceNum;
    Event.GadgetID  = GadgetID;

    INPUT_GetState( Event );
    return (Event.Info & 1) != 0;
}

//=================================================================
// INPUT_GetDebounce
//=================================================================
xbool INPUT_GetDebounce( s16 DeviceNum, s16 GadgetID )
{
    input_event Event;

    Event.DeviceNum = DeviceNum;
    Event.GadgetID  = GadgetID;

    INPUT_GetState( Event );

    return (Event.Info & 2) != 0;
}

//=================================================================
// INPUT_GetLocatorX
//=================================================================
f32 INPUT_GetLocatorX( s16 DeviceNum, s16 GadgetID )
{
    input_event Event;

    Event.DeviceNum = DeviceNum;
    Event.GadgetID  = GadgetID;
	Event.X = 0.0f;

    INPUT_GetState( Event );

    return Event.X;
}

//=================================================================
// INPUT_GetLocatorY
//=================================================================
f32 INPUT_GetLocatorY( s16 DeviceNum, s16 GadgetID )
{
    input_event Event;

    Event.DeviceNum  = DeviceNum;
    Event.GadgetID   = GadgetID;
	Event.Y = 0.0f;

    INPUT_GetState( Event );

    return Event.Y;
}

//=================================================================
// INPUT_GetLocatorZ
//=================================================================
f32 INPUT_GetLocatorZ( s16 DeviceNum, s16 GadgetID )
{
    input_event Event;

    Event.DeviceNum   = DeviceNum;
    Event.GadgetID    = GadgetID;
	Event.Z = 0.0f;

    INPUT_GetState( Event );

    return Event.Z;
}


//=================================================================
// INPUT_RumblePlay
//=================================================================
void INPUT_RumblePlay( s32 ControllerID, f32 Intensity, f32 Duration )
{
	InputDevice	*pIDevice = &sInputManager.mDevices[ControllerID];

	if( !(pIDevice->mDeviceCaps.dwFlags & DIDC_FORCEFEEDBACK) ) return;

	LONG rglDirection[2] = { 0, 0 };

    DICONSTANTFORCE cf;

    cf.lMagnitude = (s32)(Intensity*10000);

    DIEFFECT eff;
    ZeroMemory( &eff, sizeof(eff) );
    eff.dwSize                = sizeof(DIEFFECT);
    eff.dwFlags               = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    eff.cAxes                 = 1;
    eff.rglDirection          = rglDirection;
    eff.lpEnvelope            = 0;
    eff.cbTypeSpecificParams  = sizeof(DICONSTANTFORCE);
    eff.lpvTypeSpecificParams = &cf;
    eff.dwStartDelay            = 0;

    // Now set the new parameters and start the effect immediately.
    pIDevice->mpEffect->SetParameters( &eff, DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_START );
	pIDevice->mpEffect->Start( 1, 0 );
	
}


//=================================================================
// INPUT_RumblePlay
//=================================================================
void INPUT_RumbleStop( s32 ControllerID )
{
    return;
	InputDevice	*pIDevice = &sInputManager.mDevices[ControllerID];
	if( !(pIDevice->mDeviceCaps.dwFlags & DIDC_FORCEFEEDBACK) ) return;
	pIDevice->mpEffect->Stop();
}

//=================================================================
// INPUT_RumblePlay
//=================================================================
void INPUT_RumblePause( s32 ControllerID )
{
    return;
	InputDevice	*pIDevice = &sInputManager.mDevices[ControllerID];
	if( !(pIDevice->mDeviceCaps.dwFlags & DIDC_FORCEFEEDBACK) ) return;
	pIDevice->mpEffect->Stop();
}

//=================================================================
// INPUT_RumblePlay
//=================================================================
void INPUT_RumbleResume( s32 ControllerID )
{
    return;
	InputDevice	*pIDevice = &sInputManager.mDevices[ControllerID];
	if( !(pIDevice->mDeviceCaps.dwFlags & DIDC_FORCEFEEDBACK) ) return;
	pIDevice->mpEffect->Start( 1, 0 );
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

