////////////////////////////////////////////////////////////////////////////
//
// PS2_Debug.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"
#include "x_stdio.hpp"

#include "Q_Engine.hpp"
#include "Q_Input.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#ifdef X_DEBUG
    #define	DEBUGGER_MESSAGE(str)	x_printf(str);
#else
	#define DEBUGGER_MESSAGE(str)   ((void)0)
#endif

#define	CHARS_PER_LINE	32

#define	VERIFY_STRING	"*** VERIFICATION FAILURE ***\n"
#define	ASSERT_STRING	"*** PC LOAD LETTER ***\n"
#define	UNKNOWN_STRING	"*** UNKNOWN FAILURE ***\n"


extern void INPUT_CheckDevices( void );

// my own version of Microsofts _assert
// returns TRUE if chose to debug (retry)
xbool PS2AssertHandler( s32   Type, 
						char* pFile, 
						s32   Line, 
						char* pExpStr, 
						char* pMsgStr )
{
	f64			T;
	s32			i;
	s32			StrLen;
	xbool		l2, r2, t, c, x, s;
	xbool		InRenderMode = FALSE;
	xbool		RValue       = TRUE;
	xbool		ExitLoop     = FALSE;
	char*		pFilePtr;
	char		CharSave;
	input_event	E;

    static xbool RecursiveAssert = FALSE;

    if ( RecursiveAssert )
    {
        x_printf( "Recursive ASSERT has happened. (Graphics Timeout probably?)" );
        return FALSE;
    }
    RecursiveAssert = TRUE;

	// --- For the PageFlip to succeed, we must not be in a render mode.
	if (ENG_GetRenderMode())
	{
		InRenderMode = TRUE;
		ENG_EndRenderMode();
	}

	DEBUGGER_MESSAGE( "\n" );
	DEBUGGER_MESSAGE( "Press (L2)+(R2)+(X) to Debug\n" );
	DEBUGGER_MESSAGE( "Press (L2)+(R2)+(O) to Skip\n" );
	DEBUGGER_MESSAGE( "Press (L2)+(R2)+(T) to Suppress\n" );

	//---	only attempt to draw the last few characters of the file on the screen
	for( StrLen = 0; pFile[StrLen]; StrLen++);
	if( StrLen > CHARS_PER_LINE )
		pFilePtr = &pFile[StrLen-CHARS_PER_LINE];
	else
		pFilePtr = pFile;

	//---	place debug information on both pages
	for( i=0; i<2; i++ ) 
	{
		//---	display the title
		switch( Type )
		{
			case XDB_VERIFY:
			case XDB_VERIFYS:	x_printfxy( 0, 2, VERIFY_STRING );		break;

			case XDB_ASSERT:
			case XDB_ASSERTS:	x_printfxy( 0, 2, VERIFY_STRING );		break;

			default:			x_printfxy( 0, 2, UNKNOWN_STRING );		break;
		}

		//---	display the location of the debug check
		x_printfxy( 0, 3, "File: %s\n", pFilePtr );
		x_printfxy( 0, 4, "Line: %d\n", Line );

		//---	display the expression (if there is one)
		if( pExpStr )
			x_printfxy( 0, 5, pExpStr );

		//---	display the message (if there is one)
		if( pMsgStr )
		{
			//---	get the length of the message string
			for( StrLen = 0; pMsgStr[StrLen]; StrLen++);

			//---	if it is two long for one line, cut it in half
			if( StrLen > CHARS_PER_LINE )
			{
				x_printfxy( 0, 7, &pMsgStr[CHARS_PER_LINE] );
				CharSave = pMsgStr[CHARS_PER_LINE];
				pMsgStr[CHARS_PER_LINE] = '\0';
				x_printfxy( 0, 6, pMsgStr );
				pMsgStr[CHARS_PER_LINE] = CharSave;
			}
			else
				x_printfxy( 0, 6, pMsgStr );
		}

		x_printfxy( 0,  8, "Press (L2)+(R2)+(X) to Debug\n" );
		x_printfxy( 0,  9, "Press (L2)+(R2)+(O) to Skip\n" );
		x_printfxy( 0, 10, "Press (L2)+(R2)+(T) to Suppress\n" );

		ENG_PageFlip(FALSE);
	}

    //
    // Wait until the user has noticed the message and reacted.
    //

    T = ENG_GetRunTime() + 0.5f;
    while( !ExitLoop )
    {
        // Flip between screens every 1/2 second.
        if( ENG_GetRunTime() > T)
        {
            ENG_PageFlip(FALSE);
            T = ENG_GetRunTime() + 0.5f;
        }

		// empty the input event buffer
		while( INPUT_GetNextEvent( E ) );

		// check the pads for input
		INPUT_CheckDevices();

        // Set our state variables.
        l2 = r2 = t = c = x = s = FALSE;
        for( i = 0; i < 8; i++ )
        {
            l2 = l2 || PRESSED( i, L2 );
            r2 = r2 || PRESSED( i, R2 );
            t = t || PRESSED( i, TRIANGLE );
            c = c || PRESSED( i, CIRCLE );
            x = x || PRESSED( i, CROSS );
			s = s || PRESSED( i, SQUARE );
        }

		//---	check for request to debug
        if( l2 && r2 && x )
		{
			ExitLoop = TRUE;
			RValue = FALSE;
		}

		//---	check for request to skip
        if( l2 && r2 && c )
		{
			ExitLoop = TRUE;
			RValue = TRUE;
		}

		//---	check for request to suppress
        if( l2 && r2 && t )
		{
			ExitLoop = TRUE;
			x_DebugSuppress( pFile, Line );
			RValue = TRUE;
		}
    }

	// --- If we had to end a RenderMode, then we need to restart it
	if (InRenderMode)
		ENG_BeginRenderMode();

    RecursiveAssert = FALSE;

	return RValue;
}
