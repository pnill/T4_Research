///////////////////////////////////////////////////////////////////////////
//
//  E_TEXT.CPP
//
///////////////////////////////////////////////////////////////////////////

#define CUSTOM_TEXT_HANDLER

#include "x_debug.hpp"
#include "x_stdio.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"

#include "E_Text.hpp"
#include "Q_Text.hpp"
#include "Q_View.hpp"
#include "Q_Engine.hpp"


// POSSIBLE CHANGES TO THIS MODULE 
//  - Set up support for single buffered text if adequate to situation.

///////////////////////////////////////////////////////////////////////////

#define LINES_OF_SCROLL_TEXT    8

///////////////////////////////////////////////////////////////////////////

// System initialized flag.
static  s32     s_Initialized = 0;

// Buffers for pending text.
static  char*   s_SmallText[2];   // Small text is double buffered.
static  char*   s_LargeText[2];   // Large text is double buffered.
static  char*   s_ScrollText;

// Cursor in scroll text.
static  s32     Cursor;

// Pointer to last line in scroll text area.
static  char*   s_ScrollTextLine;

// Size of screen in normal (not large) character cells.
static  s32     s_Width;
static  s32     s_Height;

// Font size for normal (not large) characters.
static  s32     s_FontSizeX;
static  s32     s_FontSizeY;

// Number of pixels the characters are indented
static  s32     s_XPixelsIndented;
static  s32     s_YPixelsIndented;

// Which buffer to use.
static  s32     s_InputBuf;   // Used to catch incoming text.
static  s32     s_OutputBuf;  // Used to put text pixels on screen.
                            
// Print using large text?
static  xbool   s_LargeTextMode;

// Function to render the string at the platform level.
static  render_string_fnptr     s_RenderString;

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void TEXT_InitModule( s32 XRes,           s32 YRes, 
                      s32 FontSizeX,      s32 FontSizeY,
                      s32 XPIndented,     s32 YPIndented,
                      render_string_fnptr RenderStringFunction )
{
    if( s_Initialized++ )
        return;

    // Determine size of text area in character cells.
    s_FontSizeX       = FontSizeX;
    s_FontSizeY       = FontSizeY;
    s_XPixelsIndented = XPIndented;
    s_YPixelsIndented = YPIndented;
    s_Width           = (XRes - (2*s_XPixelsIndented)) / s_FontSizeX;
    s_Height          = (YRes - (2*s_YPixelsIndented)) / s_FontSizeY;

    // Allocate buffers for text.
    // We need an extra character on the end of each buffer line.
    s_SmallText[0] = (char*)x_malloc( (s_Width  +1) *  s_Height    );
    s_SmallText[1] = (char*)x_malloc( (s_Width  +1) *  s_Height    );
    s_LargeText[0] = (char*)x_malloc( (s_Width/2+1) * (s_Height/2) ); 
    s_LargeText[1] = (char*)x_malloc( (s_Width/2+1) * (s_Height/2) ); 
    s_ScrollText   = (char*)x_malloc( (s_Width  +1) * LINES_OF_SCROLL_TEXT );

    ASSERT( s_SmallText[0] );
    ASSERT( s_SmallText[1] );
    ASSERT( s_LargeText[0] );
    ASSERT( s_LargeText[1] );
    ASSERT( s_ScrollText   );

    // Clear the buffers.
    x_memset( s_SmallText[0], 0, (s_Width  +1) *  s_Height    );          
    x_memset( s_SmallText[1], 0, (s_Width  +1) *  s_Height    );          
    x_memset( s_LargeText[0], 0, (s_Width/2+1) * (s_Height/2) );          
    x_memset( s_LargeText[1], 0, (s_Width/2+1) * (s_Height/2) );          
    x_memset( s_ScrollText,   0, (s_Width  +1) * LINES_OF_SCROLL_TEXT );

    // No large text at this time.
    s_LargeTextMode = FALSE;

    // Cursor starts on left.
    Cursor = 0;

    // Scroll text line is last line in scroll text buffer.
    s_ScrollTextLine = s_ScrollText + ((s_Width+1) * (LINES_OF_SCROLL_TEXT-1));

    // Set initial buffer values.
    s_InputBuf  = 0;
    s_OutputBuf = 1;

    // Set all the call backs.
    s_RenderString = RenderStringFunction;
}

///////////////////////////////////////////////////////////////////////////

void TEXT_KillModule( void )
{
    if( --s_Initialized )
        return;

    // Release the buffers.
    x_free( s_SmallText[0] );
    x_free( s_SmallText[1] );
    x_free( s_LargeText[0] );
    x_free( s_LargeText[1] );
    x_free( s_ScrollText   );

    // Clear all the callbacks.
    s_RenderString = NULL;
}                        

///////////////////////////////////////////////////////////////////////////

void TEXT_SwitchBuffers( void )
{
    ASSERT( s_Initialized );

    // Switch the buffer indexes.
    s_OutputBuf = 1 - s_OutputBuf;
    s_InputBuf  = 1 - s_InputBuf;

    // Clear the new write buffers.
    x_memset( s_SmallText[s_InputBuf], 0, (s_Width  +1) *  s_Height    );      
    x_memset( s_LargeText[s_InputBuf], 0, (s_Width/2+1) * (s_Height/2) );

    // Automatically disengage large text mode.
    s_LargeTextMode = FALSE;
}

///////////////////////////////////////////////////////////////////////////

static
void RenderTextBuffer( char* pBuffer, 
                       s32   Columns, 
                       s32   Lines, 
                       xbool Large )
{
    s32     X, Y;
    s32     i, j;
    s32     PSizeX, PSizeY;
    xbool   InString;
    char*   S;        

    PSizeX = Large ? s_FontSizeX*2 : s_FontSizeX;
    PSizeY = Large ? s_FontSizeY*2 : s_FontSizeY;

    Y = s_YPixelsIndented;

    for( i = 0; i < Lines; i++ )
    {
        X        = s_XPixelsIndented;
        InString = FALSE;
        S        = pBuffer + (i * (Columns+1));

        for( j = 0; j < Columns; j++ )
        {
            if( (*S != 0) && (!InString) )
            {
                s_RenderString( X, Y, S, Large );
                InString = TRUE;
            }
            else
            if( (*S == 0) && (InString) )
            {
                InString = FALSE;
            }

            S += 1;
            X += PSizeX;
        }

        Y += PSizeY;
    }          
}

///////////////////////////////////////////////////////////////////////////

void TEXT_RenderAllText( void )
{
    ASSERT( s_Initialized );
    ASSERT( s_RenderString );

    //
    // "Stamp" the scroll text onto the small text buffer.
    //
    {
        s32 DestOffset = (s_Width+1) * (s_Height-LINES_OF_SCROLL_TEXT);
        s32 Characters = (s_Width+1) * LINES_OF_SCROLL_TEXT;

        char*   s = s_ScrollText;  
        char*   d = s_SmallText[s_OutputBuf] + DestOffset;

        while( Characters )
        {
            if( *s )    *d = *s;
            s++;
            d++;
            Characters--;
        }
    }

    //
    // Render small text in the output buffer.
    //

    RenderTextBuffer( s_SmallText[s_OutputBuf], s_Width, s_Height, FALSE );

    //
    // Render large text in the output buffer.
    //

    RenderTextBuffer( s_LargeText[s_OutputBuf], s_Width/2, s_Height/2, TRUE );
}

///////////////////////////////////////////////////////////////////////////

void TEXT_LargeMode( void )
{
    ASSERT( s_Initialized );
    s_LargeTextMode = TRUE;
}

///////////////////////////////////////////////////////////////////////////

void TEXT_SmallMode( void )
{
    ASSERT( s_Initialized );
    s_LargeTextMode = FALSE;
}

///////////////////////////////////////////////////////////////////////////

static 
void s_ScrollTextOneLine( void )
{
    x_memmove( s_ScrollText, 
               s_ScrollText + s_Width + 1,
               (s_Width+1) * (LINES_OF_SCROLL_TEXT-1) );
    x_memset ( s_ScrollTextLine, 0, s_Width );

    Cursor = 0;
}

///////////////////////////////////////////////////////////////////////////

void TEXT_PrintString( const char* pString )
{
    ASSERT( s_Initialized );

    if( pString == NULL )
        return;

    // Add string to scroll text area.

    while( *pString )
    {
        if( *pString == 0x08 )
        {
            if( Cursor == -1 )
                s_ScrollTextOneLine();

            // Expand the tab here.
            // Space to next multiple of 8 column.
            do
            {
                if( Cursor < s_Width )
                    s_ScrollTextLine[Cursor] = ' ';
                Cursor++;
            } while( Cursor & 0x07 );
        }
        else
        if( *pString == 0x0A )
        {
            if( Cursor == -1 )
                s_ScrollTextOneLine();

            // Handle line feed
            Cursor = -1;
        }
        else
        if( *pString == 0x0D )
        {
            // Handle carriage return
            // (Do nothing)
        }
        else
        if( *pString == 0x20 )
        {
            if( Cursor == -1 )
                s_ScrollTextOneLine();

            // Handle space
            if( Cursor < s_Width )
                s_ScrollTextLine[Cursor] = ' ';
            Cursor++;
        }
        else
        {
            // Handle all other characters
            if( (Cursor == -1) || (Cursor >= s_Width) )
                s_ScrollTextOneLine();

            // Stick the character in the data buffer.
            s_ScrollTextLine[Cursor] = *pString;

            Cursor++;
        }           

        pString++;
    }
}

///////////////////////////////////////////////////////////////////////////

void TEXT_PrintStringAt( const char* String, s32 X, s32 Y )
{
    s32     OriginalX = X;
    s32     W, H;     
    char*   Buffer;

    ASSERT( s_Initialized );
    ASSERT( s_InputBuf != -1 );

    // No string?  No work!
    if( String == NULL )
        return;

    // Set locals based on text size.
    if( s_LargeTextMode )
    {
        Buffer = s_LargeText[s_InputBuf];
        W = s_Width  / 2;
        H = s_Height / 2;
    }
    else
    {
        Buffer = s_SmallText[s_InputBuf];
        W = s_Width; 
        H = s_Height;
    }

    // Too far right?  Don't even bother.
    if( X >= W )
        return;

    //
    // While there's still some string left AND we are not off the bottom.
    //

    while( (*String) && (Y < H) )
    {
        if( *String == 0x08 )
        {
            // Expand the tab here.
            // Space to next multiple of 8 column.
            do
            {
                if( (X < W) && (X >= 0) && (Y >= 0) )
                    Buffer[ (Y*(W+1)) + X ] = ' ';
                X++;
            } while( X & 0x07 );
        }
        else
        if( *String == 0x0D )
        {
            // Carriage return.
            // Do nothing.
        }
        else
        if( *String == 0x0A )
        {
            // Line feed.
            Y++;
            X = OriginalX;
        }
        else
        {
            // All other characters.
            if( (X < W) && (X >= 0) && (Y >= 0) )
                Buffer[ (Y*(W+1)) + X ] = *String;
            X++;
        }

        String++;
    }
}

///////////////////////////////////////////////////////////////////////////

void TEXT_GetCharSize( s32& Width, s32& Height )
{
    ASSERT( s_Initialized );
    Width  = s_LargeTextMode ? s_FontSizeX*2 : s_FontSizeX;
    Height = s_LargeTextMode ? s_FontSizeY*2 : s_FontSizeY;
}

///////////////////////////////////////////////////////////////////////////

void TEXT_GetTextArea( s32& Columns, s32& Rows )
{
    ASSERT( s_Initialized );
    if( Columns )   Columns = s_LargeTextMode ? s_Width/2  : s_Width;
    if( Rows )      Rows    = s_LargeTextMode ? s_Height/2 : s_Height;
}

///////////////////////////////////////////////////////////////////////////

void TEXT_3DToCell( const vector3& Point, s32& CX, s32& CY )
{
    vector3     Screen;
    s32         SizeX, SizeY;
    view*       View;

    ASSERT( s_Initialized );

    SizeX = s_LargeTextMode ? s_FontSizeX*2 : s_FontSizeX;
    SizeY = s_LargeTextMode ? s_FontSizeY*2 : s_FontSizeY;

    View = ENG_GetActiveView();
    
    View->PointToScreen( view::V_WORLD, Point, Screen );
    TEXT_2DToCell( (s32)(Screen.X), (s32)(Screen.Y), CX, CY );
    CX = (((s32)(Screen.X)) - s_XPixelsIndented) / SizeX;
    CY = (((s32)(Screen.Y)) - s_YPixelsIndented) / SizeY;
}

///////////////////////////////////////////////////////////////////////////

void TEXT_2DToCell( s32 SX, s32 SY, s32& CX, s32& CY )
{
    s32 SizeX, SizeY;

    ASSERT( s_Initialized );

    SizeX = s_LargeTextMode ? s_FontSizeX*2 : s_FontSizeX;
    SizeY = s_LargeTextMode ? s_FontSizeY*2 : s_FontSizeY;

    CX = (SX - s_XPixelsIndented) / SizeX;
    CY = (SY - s_YPixelsIndented) / SizeY;
}

///////////////////////////////////////////////////////////////////////////

void TEXT_CellTo2D( s32 CX, s32 CY, s32& SX, s32& SY )
{
    s32 SizeX, SizeY;

    ASSERT( s_Initialized );
    
    SizeX = s_LargeTextMode ? s_FontSizeX*2 : s_FontSizeX;
    SizeY = s_LargeTextMode ? s_FontSizeY*2 : s_FontSizeY;

    SX = (CX * SizeX) + s_XPixelsIndented;
    SY = (CY * SizeY) + s_YPixelsIndented;
}

///////////////////////////////////////////////////////////////////////////

void TEXT_Print2D( s32 SX, s32 SY, const char* pFormat, ... )
{
    x_va_list  Args;
    s32        CX, CY;

    ASSERT( s_Initialized );
    ASSERT( pFormat );

    x_va_start( Args, pFormat );
    TEXT_2DToCell( SX, SY, CX, CY );

    TEXT_PrintStringAt( (const char*)vfs( pFormat, Args ), CX, CY );
}

///////////////////////////////////////////////////////////////////////////

void TEXT_Print3D( const vector3& Point, const char* pFormat, ... )
{
    x_va_list  Args;
    s32        CX, CY;

    ASSERT( s_Initialized );
    ASSERT( pFormat );

    x_va_start( Args, pFormat );
    TEXT_3DToCell( Point, CX, CY );

    TEXT_PrintStringAt( (const char*)vfs( pFormat, Args ), CX, CY );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
