////////////////////////////////////////////////////////////////////////////
//
// XBOX_Dev.cpp
//
////////////////////////////////////////////////////////////////////////////

#ifndef XBOX_DEV_HPP_INCLUDED
#define XBOX_DEV_HPP_INCLUDED

// FILEIO TYPES
typedef void (*QSyncReadCallback)( void );  // read callback function type

////////////////////////////////////////////////////////////////////////////
// FUNCTIONS CALLED INTERNALLY BY QUAGMIRE
////////////////////////////////////////////////////////////////////////////

X_FILE* XBOX_Open( const char* pFilename, const char* pMode );
void    XBOX_Close( X_FILE* pFile );
s32     XBOX_Read( X_FILE* pFile, byte* pBuffer, s32 Bytes );
s32     XBOX_ReadA( void*   pBuffer, s32 Bytes, X_FILE* pFile, s32 FileOffset, s32 Priority, s32& TaskID );
s32     XBOX_ReadAStatus( s32 TaskID, s32& BytesRead );
s32     XBOX_Write( X_FILE* pFile, byte* pBuffer, s32 Bytes );
s32     XBOX_Seek( X_FILE* pFile, s32 Offset, s32 Origin );
s32     XBOX_Tell( X_FILE* pFile );
xbool   XBOX_EOF( X_FILE* pFile );
void    XBOX_DVD_InitModule( void );
void    XBOX_DVD_KillModule( void );
void    XBOX_DVD_DisplayAccessLog( xbool ResetTimes = FALSE );
void    XBOX_DVD_SetReadCallback( QSyncReadCallback pFunction );

////////////////////////////////////////////////////////////////////////////

#endif // XBOX_DEV_HPP_INCLUDED