////////////////////////////////////////////////////////////////////////////
//
// TBlockWriteBMP.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_bitmap.hpp"

#include "TBlockWriteBMP.h"
#include "TBlockIMEXObj.h"
#include "StatusOutput.h"

#include "ZLib.h"


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

static void* zlib_Alloc( voidpf opaque, uInt items, uInt size )
{
    // using standard malloc to avoid any thread issues(since TBlock is multi-thread)
    void* pAlloc = malloc( items * size );

    if( !pAlloc )
        return Z_NULL;

    return pAlloc;
}

//==========================================================================

static void zlib_Free( voidpf opaque, voidpf address )
{
    // using standard free to avoid any thread issues(since TBlock is multi-thread)
    free( address );
}

//==========================================================================

static xbool CompressData( byte* pData, s32 DataSize, byte** ppCompData, s32* pCompDataSize )
{
    s32      TempCompSize;
    s32      RetVal;
    byte*    pCompressedData;
    z_stream zStream;

    ASSERT( pData != NULL );
    ASSERT( DataSize > 0 );
    ASSERT( ppCompData != NULL );
    ASSERT( pCompDataSize != NULL );

    // calc minimum size needed to zip data
    TempCompSize = (s32)(DataSize * 1.00125f) + 12;

    // allocate buffer for zipped data
    pCompressedData = (byte*)x_malloc( TempCompSize );
    ASSERT( pCompressedData != NULL );

    if( pCompressedData == NULL )
    {
        STATUS::Print( STATUS::MSG_WARNING, "ZLib Error: Out of memory, Leaving texture data un-zipped.\n" );
        return FALSE;
    }

    // setup zlib structure
    x_memset( &zStream, 0, sizeof(z_stream) );

    zStream.zalloc = zlib_Alloc;
    zStream.zfree  = zlib_Free;
    zStream.opaque = 0;

    zStream.next_in   = (Bytef*)pData;
    zStream.avail_in  = (uInt)  DataSize;
    zStream.next_out  = (Bytef*)pCompressedData;
    zStream.avail_out = (uInt)  TempCompSize;

    // initialize zlib for compression
    RetVal = deflateInit( &zStream, Z_DEFAULT_COMPRESSION );

    if( RetVal < Z_OK )
    {
        STATUS::Print( STATUS::MSG_WARNING, "ZLib Error: %s, Leaving texture data un-zipped.\n", zStream.msg );
        x_free( pCompressedData );
        return FALSE;
    }

    // compress the data
    RetVal = deflate( &zStream, Z_FINISH );

    if( RetVal < Z_OK )
    {
        STATUS::Print( STATUS::MSG_WARNING, "ZLib Error: %s, Leaving texture data un-zipped.\n", zStream.msg );
        RetVal = deflateEnd( &zStream );
        x_free( pCompressedData );
        return FALSE;
    }

    // clean up
    RetVal = deflateEnd( &zStream );

    if( RetVal < Z_OK )
    {
        STATUS::Print( STATUS::MSG_WARNING, "ZLib Error: %s\n", zStream.msg );
    }

    // check if data was unable to be compressed
    if( (s32)zStream.total_out >= DataSize )
    {
        x_free( pCompressedData );
        return FALSE;
    }

    *ppCompData    = pCompressedData;
    *pCompDataSize = zStream.total_out;

    return TRUE;
}

//==========================================================================

void TBlock_WriteBMP( x_bitmap& xBMP, s32 ExportTarget, X_FILE* pXFile )
{
    struct tb_bitmap : public x_bitmap
    {
        x_bitmap::m_pData;
        x_bitmap::m_pClut;
    };

    byte* pDataSave;
    byte* pClutSave;
    byte* pCompData    = NULL;
    s32   CompDataSize = 0;

    ASSERT( pXFile != NULL );

    // only use ZLib on gamecube export target
    if( ExportTarget != TBLOCK_TGT_GAMECUBE )
    {
        xBMP.Save( pXFile );
        return;
    }

    // compress the texture data
    if( FALSE == CompressData( xBMP.GetRawDataPtr(), xBMP.GetTotalDataSize(), &pCompData, &CompDataSize ) )
    {
        xBMP.Save( pXFile );
        return;
    }

    pDataSave = ((tb_bitmap*)&xBMP)->m_pData;
    pClutSave = ((tb_bitmap*)&xBMP)->m_pClut;

    ((tb_bitmap*)&xBMP)->m_pData = NULL;
    ((tb_bitmap*)&xBMP)->m_pClut = NULL;

    // Save out x_bitmap structure
    x_fwrite( &xBMP, sizeof(x_bitmap), 1, pXFile );

    ((tb_bitmap*)&xBMP)->m_pData = pDataSave;
    ((tb_bitmap*)&xBMP)->m_pClut = pClutSave;

    // Save out compressed texture data
    x_fwrite( pCompData, CompDataSize, 1, pXFile );

    x_free( pCompData );

    // Save out CLUT if present
    if( (xBMP.GetClutSize() > 0) && (xBMP.GetClutPtr() != NULL) )
        x_fwrite( xBMP.GetClutPtr(), xBMP.GetClutSize(), 1, pXFile );
}

//==========================================================================
