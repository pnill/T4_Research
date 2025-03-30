////////////////////////////////////////////////////////////////////////////
//
// TBlockIMEXObj.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCKIMEXOBJ_H_INCLUDED
#define TBLOCKIMEXOBJ_H_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_stdio.hpp"

#include "TBlockTexture.h"


////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

// TBlock export targets
enum TBLOCK_TARGET
{
    TBLOCK_TGT_NOTSET,
    TBLOCK_TGT_PS2,
    TBLOCK_TGT_GAMECUBE,
    TBLOCK_TGT_XBOX,
    TBLOCK_TGT_PCDX8,

    TBLOCK_TGT_COUNT
};

// Return codes from import/export
#define TB_RCODE_OK             (1)
#define TB_RCODE_IMEX_STARTED   (2)
#define TB_RCODE_FAIL_ERROR     (0)
#define TB_RCODE_INPROGRESS     (-1)
#define TB_RCODE_NODATA         (-2)
#define TB_RCODE_FILE_ERROR     (-3)
#define TB_RCODE_TERMINATED     (-4)

#define TB_RCODE_FAILED( RC )   (((RC) <= 0) ? TRUE : FALSE)
#define TB_RCODE_SUCCESS( RC )  (((RC) >  0) ? TRUE : FALSE)


////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////

// Return FALSE to indicate the TBlockIMEXObj was destroyed(i.e. pIMEXObj is
//    invalid), TRUE if not(data is still intact)
typedef xbool (*TBExportCB)( u32 cbParam, class TBlockIMEXObj* pIMEXObj, s32 RetCode );


////////////////////////////////////////////////////////////////////////////
// TBlockIMEXObj CLASS
////////////////////////////////////////////////////////////////////////////

class TBlockIMEXObj
{
public:

    ///////////////////////////////////////////////////////
    // IMPORT - EXPORT
    ///////////////////////////////////////////////////////

    s32 Import       ( const char*     pImportFilename,
                       TBlockSettings* pGlobalSettings,
                       TBExportCB      FinishCallback = NULL,
                       u32             cbParam        = 0 );

    s32 ExportCurData( const char*     pExportFilename,
                       TBLOCK_TARGET   Target,
                       TBExportCB      FinishCallback = NULL,
                       u32             cbParam        = 0 );

    s32 Export       ( const char*      pExportFilename,
                       TBLOCK_TARGET    Target,
                       s32              NFiles,
                       TBlockSettings*  pGlobalSettings,
                       TBlockSettings** ppTexSettings,
                       TBlockPath**     ppTexPaths,
                       TBExportCB       FinishCallback = NULL,
                       u32              cbParam        = 0 );

    s32 TestExport   ( const char*      pExportFilename,
                       TBLOCK_TARGET    Target,
                       TBlockSettings*  pTexSettings,
                       TBlockPath&      rTexPath );

    ///////////////////////////////////////////////////////
    // INFORMATION
    ///////////////////////////////////////////////////////

    s32           GetNFiles     ( void );
    TBLOCK_TARGET GetTarget     ( void );
    void          GetTBSetting  ( s32 Index, TBlockSettings& rTBS );
    void          GetTBPath     ( s32 Index, TBlockPath& rTBPath );
    void          GetGlobalTBS  ( TBlockSettings& rGlobalTBS );
    const char*   GetFilename   ( void );

    ///////////////////////////////////////////////////////
    // DEBUG SETTINGS
    ///////////////////////////////////////////////////////

    void  EnableTintMips    ( xbool bEnable );
    xbool IsTintMipsEnabled ( void );

    ///////////////////////////////////////////////////////
    // ASYNC OPERATIONS
    ///////////////////////////////////////////////////////

    xbool IsFinished ( void );
    void  StopProcess( void );

    ///////////////////////////////////////////////////////
    // INITIALISATION - DESTRUCTION
    ///////////////////////////////////////////////////////

    void  KillData( void );

     TBlockIMEXObj( void );
    ~TBlockIMEXObj( void );

protected:

    ///////////////////////////////////////////////////////
    // HELPER FUNCTIONS
    ///////////////////////////////////////////////////////

    static UINT Thread_Import( LPVOID pParam );
    static UINT Thread_Export( LPVOID pParam );

    s32 ExecuteImport( void );
    s32 ExecuteExport( void );

    s32 ImportTBlockFile( void );
    s32 ImportTextFile  ( void );
    //##### TEMP #######
    s32 ImportOLDTEXFILE( void );

    s32 ExportTBlockFile( void );
    s32 ExportTextFile  ( void );

    xbool IsTgtBigEndian( void );

    ///////////////////////////////////////////////////////
    // DATA MEMBERS
    ///////////////////////////////////////////////////////

    struct TBNameOffset
    {
        TBlockShortName m_ShortName;        // short name of texture
        s32             m_Offset;           // offset into export file
    };

    TBlockPath*     m_pPaths;               // paths for each texture
    TBlockSettings* m_pTBS;                 // settings for each texture
    TBNameOffset*   m_pNameOffset;          // short names and offsets for textures
    TBlockSettings  m_GlobalTBS;            // global settings
    TBExportCB      m_pFinishCallback;      // callback function used in async operations
    u32             m_cbParam;              // callback function parameter
    s32             m_NFiles;               // number of textures exported/imported
    TBLOCK_TARGET   m_Target;               // export target
    xbool           m_bTextFile;            // is import/export a text file
    xbool           m_bInProgress;          // is import/export in progress
    xbool           m_bStopProcess;         // flag to terminate async operation
    xbool           m_bKillOnFinish;        // internal flag to destroy data
    void*           m_pThread;              // pointer to async thread
    X_FILE*         m_pXFile;               // file pointer for import/export
    TBlockPath      m_Filename;             // filename used for final export
    TBlockPath      m_strTempFilename;      // filename for intermediate export file
    xbool           m_bDebugTintMips;       // turn mip tinting on/off
};


////////////////////////////////////////////////////////////////////////////

#endif // TBLOCKIMEXOBJ_H_INCLUDED