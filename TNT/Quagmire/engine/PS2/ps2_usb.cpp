	  
#include "x_stdio.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"
#include "x_memory.hpp"

#include "E_Text.hpp"

#include "PS2_usb.hpp"

#include "sifdev.h"
#include <sifrpc.h>

#include <eekernel.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////

#ifdef X_DEBUG
//==========================================================================
//
//  DEBUG HELPERS:::
//
//  define PS2USB_VERBOSE_INIT to have the PS2USB system print boot info to
//  the tty.  

#define PS2USB_VERBOSE_INIT

//==========================================================================
//
//  define PS2USB_VERBOSE to have all PS2USB operation print operational
//  information to the tty
//

#define PS2USB_VERBOSE

///////////////////////////////////////////////////////////////////////////
#endif

//==========================================================================
// Some defines to make working with the sony libs a bit easier to read
//

#ifdef PS2USB_VERBOSE
#define PS2USB_PRINT(a)              x_printf(a);
#else
#define PS2USB_PRINT(a)
#endif

//==========================================================================
#define PS2USB_TYPE_ATFILE		(0)		// File is being loaded through atmon
#define PS2USB_TYPE_HOST		(1)		// File is loaded using the 'host0' prefix
#define PS2USB_TYPE_PORT		(2)		// File is a network port
//==========================================================================
typedef struct
{
	char			Filename[128];
	s32				type;						// Connection type (see defines above)
	s32				handle;						// Host file handle, -1 if it's a CD based file
	X_FILE			*cd_filehandle;				// Handle for a CD based file
	s32				position;					// Current file position on target
	s32				HostPosition;				// Current file position on host
	s32				length;						// File length
}ps2usb_file;

char HostPrefix[64];

//==========================================================================

s32 PS2USBInit      = 0;

////////////////////////////////////////////////////////////////////////////
//
//  Storage for sizes of async reads. (Used by x_freadastatus)
//
static s32 AsyncReadSizes[50];


//==========================================================================
void PS2USB_Init( s32 argc, char *argv[] )
{
    /* This function does all the necessary initialisation.        */
    /* it returns zero on success, or, non zero on failure.        */

    /* Local Variables */
    /* --------------- */

	x_memset( AsyncReadSizes, -1, sizeof(AsyncReadSizes) );

	PS2USBInit = TRUE;
	if ( (argc==1) && (x_strncmp(argv[0],"atfile:",7)==0) )
	{
		char *src,*dst;

		src=argv[0];
		dst=HostPrefix;

		while ((*src!=',') && (*src) )
		{
			*dst=*src;
			dst++;src++;
		}
		*dst=',';dst++;
		*dst=0x0;
		x_printf("Atfile I/O enabled, prefix is %s\n",HostPrefix);
	}
	else
	{
		x_strcpy(HostPrefix,"atfile0:");
		x_printf("No atfile prefix found, using host0:\n");
	}

}

//==========================================================================

void PS2USB_Kill( void )
{

}

//==========================================================================


//==========================================================================

X_FILE* PS2USB_Open  ( const char* Filename, const char* Mode )
{
    char*       Ptr;
	s32         Read = 0, Write = 0, Append = 0;
	ps2usb_file *fp;
	s32			mode = SCE_RDONLY;
	
	ASSERT( PS2USBInit );

	ASSERT( Filename );
	ASSERT( Mode );
	ASSERT( x_strlen(Filename) > 0 );

	fp = (ps2usb_file *)x_malloc(sizeof(ps2usb_file));
	//-- verify that we're trying to read.
	for ( Ptr = (char*)Mode ; *Ptr != 0 ; Ptr++ )
	{
		switch ( *Ptr )
		{
			case 'r': case 'R':
				Read = 1;
				break;
			case 'w': case 'W':
				Write = 1;
				break;
			case 'a': case 'A':
				Append = 1;
				break;
		}
	}
	if (Read) mode=SCE_RDONLY;
	if (Write)
	{
		if (Append) 
			mode = SCE_WRONLY;
		else
			mode = SCE_CREAT;
	}

    if ( (Read == 0) && (Write == 0) && (Append == 0) )
        return NULL;

	if (Filename[0]=='@')		// Are we going to open a net connection?
	{
		x_sprintf(fp->Filename,"atstream:%s",&Filename[1]);
	}
	else
	{
	
		x_sprintf(fp->Filename,"%s%s",HostPrefix,Filename);
	}

	x_printf("Attempting to open %s\n",fp->Filename);
	fp->handle = sceOpen(fp->Filename,mode);
	if (fp->handle < 0)
	{
		x_printf("handle = <failed>\n",fp->handle);
		x_free(fp);
		return NULL;
	}

	fp->length = sceLseek(fp->handle,0,SCE_SEEK_END);
//	x_printf("handle = %d  (file size = %d)\n",fp->handle, fp->length);
	x_printf("handle = %d\n",fp->handle);
	if (!Append)
		sceLseek(fp->handle,0,SCE_SEEK_SET);

	fp->type = PS2USB_TYPE_ATFILE;

	fp->position=0;
 
    return (X_FILE*)fp;
}

//==========================================================================

void    PS2USB_Close ( X_FILE* Stream                         )
{
    ps2usb_file* fp = (ps2usb_file*) Stream;

	sceClose(fp->handle);
	ASSERT( PS2USBInit );
	x_free(fp);
}

//==========================================================================
s32     PS2USB_Read  ( X_FILE* Stream, byte* Base, s32 Amount )
{
    ps2usb_file*fp = (ps2usb_file*) Stream;

	ASSERT( PS2USBInit );
	fp->position+=Amount;
	return sceRead(fp->handle,Base,Amount);
}

//==========================================================================
s32     PS2USB_ReadA ( void*   pBuffer,
					   s32     Bytes,
					   X_FILE* pFile,
					   s32     FileOffset,
					   s32     Priority,
					   s32&    TaskID )
{
	for (TaskID=0; TaskID < 50; TaskID++)
	{
		if (AsyncReadSizes[TaskID] == -1)
			break;
	}
	AsyncReadSizes[TaskID] = Bytes;

    ps2usb_file*fp = (ps2usb_file*) pFile;

	ASSERT( PS2USBInit );
	fp->position += Bytes;
	return sceRead(fp->handle,pBuffer,Bytes);
}

//==========================================================================
s32     PS2USB_ReadAStatus( s32 TaskID, s32& BytesRead )
{
	if (AsyncReadSizes[TaskID] >= 0)
	{
		BytesRead = AsyncReadSizes[TaskID];
		AsyncReadSizes[TaskID] = -1;
		return X_STATUS_COMPLETE;
	}

	else
	{
		BytesRead = 0;
		return X_STATUS_NOTFOUND;
	}
}

//==========================================================================

s32     PS2USB_Write ( X_FILE* Stream, byte* Base, s32 Amount )
{
	ps2usb_file *fp=(ps2usb_file *)Stream;
	ASSERT( PS2USBInit );
	fp->position += Amount;
	return sceWrite(fp->handle,(void *)Base,Amount);

}

//==========================================================================
s32     PS2USB_Seek  ( X_FILE* Stream, const s32 Offset, s32 Origin )
{
    ps2usb_file* fp = (ps2usb_file*) Stream;

	ASSERT( PS2USBInit );

	switch (Origin)
	{
	case X_SEEK_SET:
		fp->position = Offset;
		break;
	case X_SEEK_CUR:
		fp->position = Offset + fp->position;
		break;
	case X_SEEK_END:
		fp->position = fp->length + Offset;
		break;
	default:
		ASSERT(0);
		break;
	}
	sceLseek(fp->handle,fp->position,SCE_SEEK_SET);
	return 0;
}

//==========================================================================

s32     PS2USB_Tell  ( X_FILE* Stream                         )
{
    ps2usb_file* fp = (ps2usb_file*) Stream;

	ASSERT( PS2USBInit );

	return (fp->position);
}

//==========================================================================

xbool   PS2USB_EOF   ( X_FILE* Stream                         )
{
    ps2usb_file* fp = (ps2usb_file*) Stream;

	ASSERT( PS2USBInit );
	return (fp->position == fp->length);
}

//==========================================================================

s32     PS2USB_GetFileSize( X_FILE* Stream )
{
    ps2usb_file* fp = (ps2usb_file*) Stream;

	ASSERT( PS2USBInit );
	return (fp->length);
}

//==========================================================================

void PS2USB_Printf ( const char* Text )
{
	ps2usb_file *fp;
	s32			mode;
	
	ASSERT( PS2USBInit );

	ASSERT( Text );

	fp = (ps2usb_file *)x_malloc(sizeof(ps2usb_file));

	//-- verify that we're trying to read.
	mode=SCE_RDONLY;
	x_sprintf(fp->Filename,"%s#:%s", HostPrefix, Text);

	fp->handle = sceOpen(fp->Filename,mode);
	fp->type = PS2USB_TYPE_ATFILE;
//	fp->type = PS2USB_TYPE_PORT;

	fp->position=0;
 
//	sceClose(fp->handle);
	x_free(fp);
}

//==========================================================================
//static const char* SystemImage = "host0:C:\\TNT\\3rdParty\\PS2\\sce\\iop\\modules\\ioprp21.img";

void    PS2USB_Interface     ( void )
{
    x_SetIOHandlers( PS2USB_Open, 
                     PS2USB_Close, 
                     PS2USB_Read, 
					 PS2USB_ReadA,
					 PS2USB_ReadAStatus,
                     PS2USB_Write, 
                     PS2USB_Seek, 
                     PS2USB_Tell, 
                     PS2USB_EOF );

	x_SetPrintHandlers(PS2USB_Printf,
					   TEXT_PrintStringAt);




#if 0
	// This is what we do to open a passive connection. i.e. waiting for
	// a client to connect to us.
	{
		X_FILE *fp;
		fp = NULL;
		while (!fp)
		{
			fp = x_fopen("@!-8192","rb");
			if (!fp) x_printf("Stream open failed\n");
		}
	}
#endif
}


