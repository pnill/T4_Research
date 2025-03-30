////////////////////////////////////////////////////////////////////////////
//
// TBlockToolDlg.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "x_debug.hpp"

#include "TBlockTool.h"
#include "TBlockToolDlg.h"
#include "SwapIndexDlg.h"
#include "AUX_Bitmap.hpp"

#pragma warning( disable : 4311 ) // disable warning(4311): 'type cast' : pointer truncation from 'SOME_TYPE *' to 'SOME_OTHER_TYPE'
#pragma warning( disable : 4312 ) // disable warning(4312): 'type cast' : conversion from 'SOME_TYPE' to 'SOME_OTHER_TYPE *' of greater size


////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define APP_WINDOW_TITLE            "TextureBlock Tool"
#define APP_VERSION_NUMBER          "v1.6"

#define DEFAULT_SAVE_TEXTURE_NAME   "Bitmaps"
#define DEFAULT_SAVE_TEXTFILE_NAME  "TextureList"

#define FILE_ADD_FILTER             "Graphic Files|*.bmp;*.gif;*.tga;*.pcx||"
#define FILE_SAVE_TEX_FILTER        "Texture File|*.tex||"
#define FILE_SAVE_TXT_FILTER        "Text File|*.txt||"
#define FILE_SAVE_TGA_FILTER        "32-bit Targa(TGA) File|*.tga||"
#define FILE_OPEN_FILTER            "TBlock or TextList Files|*.tex;*.txt||"

#define COLOR_VAL_MAX_CHARS         3
#define GAMMA_VAL_MAX_CHARS         5


/////////////////////////////////////////////////
// Combo box string constants
/////////////////////////////////////////////////

const char* COLORSPACE_CS_TEXT[COLORSPACE_COUNT] =
{
    "APPLE_RGB",
    "SMPTE_C",
    "SRGB",
    "PAL_SECAM",
    "COLORMATCH_RGB",
    "ADOBE_RGB",
    "NTSC",
    "CIE_RGB",
    "WIDEGAMUT_RGB",
};

const char* COLORSPACE_WP_TEXT[WHITEPOINT_COUNT] =
{
    "CIE_STD_A",
    "D50",
    "CIE_STD_B",
    "D55",
    "CIE_STD_E",
    "D65",
    "CIE_STD_C",
    "D75",
    "D9300",
};

const char* GAMMA_TEXT[ GAMMA_COUNT + 1 ] =
{
    "APPLE_RGB",
    "SMPTE_C",
    "SRGB",
    "PAL_SECAM",
    "COLORMATCH_RGB",
    "ADOBE_RGB",
    "NTSC",
    "CIE_RGB",
    "WIDEGAMUT_RGB",
    " Custom Gamma",
};


/////////////////////////////////////////////////
// File list box constants
/////////////////////////////////////////////////

#define LIST_NO_SELECTION   (-1)
#define LIST_ERROR          (-1)

// List column IDs
enum LIST_HEADER_COLUMNS
{
    LISTHDR_ID_PATH,
    LISTHDR_ID_SHORTNAME,
    LISTHDR_ID_WIDTH,
    LISTHDR_ID_HEIGHT,
    LISTHDR_ID_BPP,

    LISTHDR_ID_COUNT
};

// Order that columns appear(left to right)
int LIST_HDR_ORDER[LISTHDR_ID_COUNT] =
{
    LISTHDR_ID_SHORTNAME,
    LISTHDR_ID_PATH,
    LISTHDR_ID_WIDTH,
    LISTHDR_ID_HEIGHT,
    LISTHDR_ID_BPP,
};

// Column header text
char* LISTHDR_TEXT[LISTHDR_ID_COUNT] =
{
    "Texture Path",
    "Export Name",
    "W",
    "H",
    "BPP",
};

// Column widths
const int LIST_HDR_PATH_MINWIDTH[LISTHDR_ID_COUNT] =
{
    250,
    75,
    35,
    35,
    35,
};


////////////////////////////////////////////////////////////////////////////
// TBlockToolDlg dialog
////////////////////////////////////////////////////////////////////////////

TBlockToolDlg::TBlockToolDlg( TBlockIMEXObj* pIMEXObj, CWnd* pParent )
    : CDialog( TBlockToolDlg::IDD, pParent )
{
    //{{AFX_DATA_INIT(TBlockToolDlg)
    //}}AFX_DATA_INIT
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon( IDI_ICON_SMALL );

    //----------------------------------------------------------

    AfxInitRichEdit();

    char Drive [X_MAX_DRIVE];
    char Folder[X_MAX_DIR];
    char FName [X_MAX_FNAME];
    char Ext   [X_MAX_EXT];
    const char* pIMEXFName = NULL;

    if( pIMEXObj != NULL )
    {
        pIMEXFName = pIMEXObj->GetFilename();
        if( x_strlen( pIMEXFName ) > 0 )
        {
            x_splitpath( pIMEXFName, Drive, Folder, FName, Ext );
        }
        else
        {
            pIMEXFName = NULL;
        }
    }

    if( pIMEXFName != NULL )
    {
        x_makepath( m_TempStr, Drive, Folder, NULL, NULL );

        // save current working folder for Open/Save dialog
        strcpy( m_AddFolder,     m_TempStr );
        strcpy( m_SaveFolder,    m_TempStr );
        strcpy( m_OpenFolder,    m_TempStr );
        strcpy( m_TxtSaveFolder, m_TempStr );

        // Assign default save names for Open/Save dialogs
        strcpy( m_SaveFilename,    FName );
        strcpy( m_TxtSaveFilename, FName );
    }
    else
    {
        ::GetCurrentDirectory( sizeof(m_TempStr)-1, m_TempStr );

        // save current working folder for Open/Save dialog
        strcpy( m_AddFolder,     m_TempStr );
        strcpy( m_SaveFolder,    m_TempStr );
        strcpy( m_OpenFolder,    m_TempStr );
        strcpy( m_TxtSaveFolder, m_TempStr );

        // Assign default save names for Open/Save dialogs
        strcpy( m_SaveFilename,    DEFAULT_SAVE_TEXTURE_NAME );
        strcpy( m_TxtSaveFilename, DEFAULT_SAVE_TEXTFILE_NAME );
    }


    m_ComboUpdateEditBox = FALSE;
    m_bThisReady         = FALSE;
    m_bCodeAddRemoveItem = FALSE;

    m_pIMEXObj = pIMEXObj;

    // Init default global settings
    m_GlobalSetting.InitDefault();
    m_GlobalSetting.SetApplyAlpha( TRUE, TRUE, FALSE, FALSE );
    m_pDisplayedSetting = &m_GlobalSetting;

    // setup output message handling
    STATUS::InitHandlers( (u32)this, StatusPrintCB, StatusProgStartCB, StatusProgStepCB );

    // Allocate and initialize the item setting data pool
    m_pSettingPool = NULL;

    m_pSettingPool = (SettingArray*)malloc( sizeof(SettingArray) );

    VERIFYS( m_pSettingPool != NULL, "Memory allocation failure!" );

    memset( m_pSettingPool, 0, sizeof(SettingArray) );
}

//==========================================================================

TBlockToolDlg::~TBlockToolDlg( void )
{
    // Free memory allocated for item settings
    SettingArray* pTemp;

    while( m_pSettingPool != NULL )
    {
        pTemp = m_pSettingPool;
        m_pSettingPool = pTemp->pNextArray;

        free( pTemp );
    }
}

//==========================================================================

TBlockSettings* TBlockToolDlg::AllocItemSetting( void )
{
    int i;
    SettingArray* pTemp;

    pTemp = m_pSettingPool;

    // Loop through current available settings in pool, try
    // to find an unused one for a new item
    while( pTemp != NULL )
    {
        for( i = 0; i < TBS_ARRAY_SIZE; i++ )
        {
            if( pTemp->TBSInUse[i] == FALSE )
            {
                pTemp->TBSInUse[i] = TRUE;
                pTemp->TBSArray[i] = m_GlobalSetting;

                return &(pTemp->TBSArray[i]);
            }
        }

        pTemp = pTemp->pNextArray;
    }

    // If control reaches here, no free slots, allocate another block
    pTemp = (SettingArray*)malloc( sizeof(SettingArray) );
    VERIFYS( pTemp != NULL, "Memory allocation failure!" );

    // clear out new data
    memset( pTemp, 0, sizeof(SettingArray) );

    // add to current pool list
    pTemp->pNextArray = m_pSettingPool;

    m_pSettingPool = pTemp;

    // mark first slot in use and return it
    pTemp->TBSInUse[0] = TRUE;

    return &(pTemp->TBSArray[0]);
}

//==========================================================================

void TBlockToolDlg::FreeItemSetting( TBlockSettings* pTBS )
{
    int i;
    SettingArray* pTemp;

    // check for null pointer
    if( pTBS == NULL )
        return;

    pTemp = m_pSettingPool;

    // find this setting in data pool, and mark it as unused
    while( pTemp != NULL )
    {
        for( i = 0; i < TBS_ARRAY_SIZE; i++ )
        {
            if( &(pTemp->TBSArray[i]) == pTBS )
            {
                pTemp->TBSInUse[i] = FALSE;
                return;
            }
        }

        pTemp = pTemp->pNextArray;
    }

    // Couldn't find pointer
    ASSERTS( FALSE, "Free memory error: Setting not found in allocated pool." );
}

//==========================================================================

BOOL TBlockToolDlg::LoadDataFromImport( void )
{
    int      i;
    int      NItems;

    TBlockPath     TBPath;
    TBlockSettings TBS;

    if( m_pIMEXObj == NULL )
        return FALSE;

    NItems = (int)m_pIMEXObj->GetNFiles();

    if( NItems <= 0 )
        return FALSE;

    // set current export target
    this->SetExportTarget( m_pIMEXObj->GetTarget() );

    // Get global setting from import
    m_pIMEXObj->GetGlobalTBS( m_GlobalSetting );

    // Add the path and settings to item list
    for( i = 0; i < NItems; i++ )
    {
        m_pIMEXObj->GetTBPath( i, TBPath );
        m_pIMEXObj->GetTBSetting( i, TBS );

        this->AddItem( i, TBPath, &TBS );
    }

    // Use drive and folder from first item to set new "Add" location
    char Drive [X_MAX_DRIVE];
    char Folder[X_MAX_DIR];

    m_pIMEXObj->GetTBPath( 0, m_TempStr );
    x_splitpath( m_TempStr, Drive, Folder, NULL, NULL );
    x_makepath( m_AddFolder, Drive, Folder, NULL, NULL );

    return TRUE;
}

//==========================================================================

void TBlockToolDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(TBlockToolDlg)
    //}}AFX_DATA_MAP
}


////////////////////////////////////////////////////////////////////////////
// MESSAGE MAP
////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(TBlockToolDlg, CDialog)
    //{{AFX_MSG_MAP(TBlockToolDlg)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_INSERT, OnButtonInsert)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
    ON_BN_CLICKED(IDC_BUTTON_REMOVEALL, OnButtonRemoveall)
    ON_BN_CLICKED(IDC_BUTTON_INSERT_EMPTY, OnButtonInsertEmpty)
    ON_BN_CLICKED(IDC_BUTTON_SORTALPHA, OnButtonSortalpha)
    ON_BN_CLICKED(IDC_BUTTON_MOVEDOWN, OnButtonMovedown)
    ON_BN_CLICKED(IDC_BUTTON_MOVEUP, OnButtonMoveup)
    ON_BN_CLICKED(IDC_BUTTON_MOVEINDEX, OnButtonMoveindex)
    ON_BN_CLICKED(IDC_BUTTON_SWAPINDEX, OnButtonSwapindex)
    ON_BN_CLICKED(IDC_BUTTON_TESTEXPORT, OnButtonTestExport)
    ON_BN_CLICKED(IDC_BUTTON_REFRESH_PROPS, OnButtonRefreshProps)
    ON_BN_CLICKED(IDC_BUTTON_OPEN, OnButtonOpen)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_TXT, OnButtonSaveTxt)
    ON_BN_CLICKED(IDC_BUTTON_EXPORT, OnButtonExport)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_OUTPUT, OnButtonClearOutput)
    ON_BN_CLICKED(IDC_CHECK_ALPHA_BYFILE, OnCheckAlphaByfile)
    ON_BN_CLICKED(IDC_CHECK_APPLYALPHA, OnCheckApplyalpha)
    ON_BN_CLICKED(IDC_CHECK_BUILDMIPS, OnCheckBuildmips)
    ON_BN_CLICKED(IDC_CHECK_CHANGEINTENSITY, OnCheckChangeintensity)
    ON_BN_CLICKED(IDC_CHECK_COLORSPACE, OnCheckColorspace)
    ON_BN_CLICKED(IDC_CHECK_GAMMA, OnCheckGamma)
    ON_CBN_SELCHANGE(IDC_COMBO_PRESET_GAMMA, OnSelchangeComboPresetGamma)
    ON_EN_CHANGE(IDC_EDIT_GAMMA_VALUE, OnChangeEditGammaValue)
    ON_BN_CLICKED(IDC_CHECK_HOTCOLOR, OnCheckHotcolor)
    ON_BN_CLICKED(IDC_CHECK_HOTCOLOR_TEST, OnCheckHotcolorTest)
    ON_BN_CLICKED(IDC_RADIO_VIEWGLOBAL, OnRadioViewglobal)
    ON_BN_CLICKED(IDC_RADIO_VIEWCUSTOM, OnRadioViewcustom)
    ON_BN_CLICKED(IDC_CHECK_USEGLOBAL, OnCheckUseglobal)
    ON_BN_CLICKED(IDC_CHECK_COMPRESS, OnCheckCompress)
    ON_BN_CLICKED(IDC_CHECK_INTENSITY_MAP, OnCheckIntensityMap)
    ON_BN_CLICKED(IDC_CHECK_TINTMIPS, OnCheckTintMips)
    ON_WM_SYSCOMMAND()
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILES, OnDblclkListFiles)
    ON_NOTIFY(NM_RCLICK, IDC_LIST_FILES, OnRclickListFiles)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////
// TBlockToolDlg MESSAGE HANDLERS
////////////////////////////////////////////////////////////////////////////

void TBlockToolDlg::OnPaint( void )
{
    // If you add a minimize button to your dialog, you will need the code below
    //  to draw the icon.  For MFC applications using the document/view model,
    //  this is automatically done for you by the framework.

    if( IsIconic() )
    {
        CPaintDC dc( this ); // device context for painting

        SendMessage( WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0 );

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics( SM_CXICON );
        int cyIcon = GetSystemMetrics( SM_CYICON );
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width()  - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon( x, y, m_hIcon );
    }
    else
    {
        CDialog::OnPaint();
    }
}

//==========================================================================

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR TBlockToolDlg::OnQueryDragIcon( void )
{
    return (HCURSOR)m_hIcon;
}

//==========================================================================

void TBlockToolDlg::OnSysCommand( UINT nID, LPARAM lParam )
{
    // If system message is Close(i.e. mouse click on X button), kill dialog
    if( nID == SC_CLOSE )
        this->EndDialog( IDD );

    CDialog::OnSysCommand( nID, lParam );
}

//==========================================================================

void TBlockToolDlg::OnOK( void )
{
    // overloaded to keep default input from closing window
}

//==========================================================================

void TBlockToolDlg::OnCancel( void )
{
    // overloaded to keep default input from closing window
}

//==========================================================================

BOOL TBlockToolDlg::OnInitDialog( void )
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon( m_hIcon, TRUE );       // Set big icon
    SetIcon( m_hIcon, FALSE );      // Set small icon

    //----------------------------------------------------------

    CButton* pBtn;
    DWORD    Style;
    int      i;

    // Set dialog window text
    sprintf( m_TempStr, "%s %s", APP_WINDOW_TITLE, APP_VERSION_NUMBER );
    this->SetWindowText( m_TempStr );

    // Save pointer to file list control
    m_pFileList = (CListCtrl*)GetDlgItem( IDC_LIST_FILES );

    // Setup initial layout of file list control
    Style = LVS_EX_FULLROWSELECT | m_pFileList->GetExtendedStyle();
    m_pFileList->SetExtendedStyle( Style );

    for( i = 0; i < LISTHDR_ID_COUNT; i++ )
    {
        m_pFileList->InsertColumn( i, LISTHDR_TEXT[i], LVCFMT_LEFT, LIST_HDR_PATH_MINWIDTH[i], i-1 );
    }

    m_pFileList->SetColumnOrderArray( LISTHDR_ID_COUNT, LIST_HDR_ORDER );

    // Set default platform target
    this->SetExportTarget( TBLOCK_TGT_PS2 );

    // Set default state of controls
    this->InitSettingsControlsDefData();
    this->UpdateSettingControls();

    // load import data(if it exists)
    if( this->LoadDataFromImport() )
    {
        this->UpdateSettingControls();
        this->UpdateItemCountDisplay();
        this->UpdateSelectionDisplay();
        this->UpdateListHorizSize();
    }

    // kill off import data
    if( m_pIMEXObj != NULL )
        m_pIMEXObj->KillData();

    // don't keep reference to import data(since dialog doesn't own it)
    // instead set it to own IMEX member
    m_pIMEXObj = &m_IMEXObject;

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEWGLOBAL );
    pBtn->SetCheck( TRUE );
    OnRadioViewglobal();

    //##### TEMP: Disabling this button until it's function works
    pBtn = (CButton*)GetDlgItem( IDC_BUTTON_SORTALPHA );
    pBtn->EnableWindow( FALSE );

    m_bThisReady = TRUE;

    //----------------------------------------------------------

    this->SetFocus();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

//==========================================================================

void TBlockToolDlg::SetExportTarget( TBLOCK_TARGET Target )
{
    int      RadioBtnID;
    CButton* pBtn;

    // get control ID for button that corresponds to target
    switch( Target )
    {
     case TBLOCK_TGT_PS2:       RadioBtnID = IDC_RADIO_TARGET_PS2;      break;
     case TBLOCK_TGT_GAMECUBE:  RadioBtnID = IDC_RADIO_TARGET_GAMECUBE; break;
     case TBLOCK_TGT_XBOX:      RadioBtnID = IDC_RADIO_TARGET_XBOX;     break;
     case TBLOCK_TGT_PCDX8:     RadioBtnID = IDC_RADIO_TARGET_PC;       break;

     default:
        RadioBtnID = -1;
        return; //break;
    }

    // Clear state of export target buttons
    pBtn = (CButton*)GetDlgItem( IDC_RADIO_TARGET_PS2 );
    pBtn->SetCheck( FALSE );
    pBtn = (CButton*)GetDlgItem( IDC_RADIO_TARGET_GAMECUBE );
    pBtn->SetCheck( FALSE );
    pBtn = (CButton*)GetDlgItem( IDC_RADIO_TARGET_XBOX );
    pBtn->SetCheck( FALSE );
    pBtn = (CButton*)GetDlgItem( IDC_RADIO_TARGET_PC );
    pBtn->SetCheck( FALSE );

    // Set state of current target
    if( RadioBtnID != -1 )
    {
        pBtn = (CButton*)GetDlgItem( RadioBtnID );
        pBtn->SetCheck( TRUE );
    }
}

//==========================================================================

TBLOCK_TARGET TBlockToolDlg::GetExportTarget( void )
{
    CButton* pBtn;

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_TARGET_PS2 );
    if( pBtn->GetCheck() )
        return TBLOCK_TGT_PS2;

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_TARGET_GAMECUBE );
    if( pBtn->GetCheck() )
        return TBLOCK_TGT_GAMECUBE;

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_TARGET_XBOX );
    if( pBtn->GetCheck() )
        return TBLOCK_TGT_XBOX;

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_TARGET_PC );
    if( pBtn->GetCheck() )
        return TBLOCK_TGT_PCDX8;

    return TBLOCK_TGT_NOTSET;
}

//==========================================================================

void TBlockToolDlg::InitSettingsControlsDefData( void )
{
    int        i;
    int        ItemIndex;
    CEdit*     pEdit;
    CComboBox* pCombo1;
    CComboBox* pCombo2;

    // Init color scale group controls
    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_R );
    pEdit->SetLimitText( COLOR_VAL_MAX_CHARS );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_G );
    pEdit->SetLimitText( COLOR_VAL_MAX_CHARS );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_B );
    pEdit->SetLimitText( COLOR_VAL_MAX_CHARS );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_A );
    pEdit->SetLimitText( COLOR_VAL_MAX_CHARS );

    // Init color space group controls
    pCombo1 = (CComboBox*)GetDlgItem( IDC_COMBO_CSPACE_SRC );
    pCombo2 = (CComboBox*)GetDlgItem( IDC_COMBO_CSPACE_DST );

    pCombo1->ResetContent();
    pCombo2->ResetContent();
    for( i = 0; i < COLORSPACE_COUNT; i++ )
    {
        // Add string values to combo boxes, and then
        // set item data to correspond with integer value of item.
        // This is done because the list box is sorted alphabetically
        ItemIndex = pCombo1->AddString( COLORSPACE_CS_TEXT[i] );
        pCombo1->SetItemData( ItemIndex, i );

        ItemIndex = pCombo2->AddString( COLORSPACE_CS_TEXT[i] );
        pCombo2->SetItemData( ItemIndex, i );
    }

    pCombo1 = (CComboBox*)GetDlgItem( IDC_COMBO_WHITEPT_SRC );
    pCombo2 = (CComboBox*)GetDlgItem( IDC_COMBO_WHITEPT_DST );

    pCombo1->ResetContent();
    pCombo2->ResetContent();
    for( i = 0; i < WHITEPOINT_COUNT; i++ )
    {
        // Add string values to combo boxes, and then
        // set item data to correspond with integer value of item.
        // This is done because the list box is sorted alphabetically
        ItemIndex = pCombo1->AddString( COLORSPACE_WP_TEXT[i] );
        pCombo1->SetItemData( ItemIndex, i );

        ItemIndex = pCombo2->AddString( COLORSPACE_WP_TEXT[i] );
        pCombo2->SetItemData( ItemIndex, i );
    }

    // Init Gamma Correction group controls
    pCombo1 = (CComboBox*)GetDlgItem( IDC_COMBO_PRESET_GAMMA );
    pCombo1->ResetContent();
    for( i = 0; i < (GAMMA_COUNT + 1); i++ )
    {
        // Add string values to combo boxes, and then
        // set item data to correspond with integer value of item.
        // This is done because the list box is sorted alphabetically
        ItemIndex = pCombo1->AddString( GAMMA_TEXT[i] );
        pCombo1->SetItemData( ItemIndex, i );
    }

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_GAMMA_VALUE );
    pEdit->SetLimitText( GAMMA_VAL_MAX_CHARS );

    // Init Hot color test group controls
    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_R );
    pEdit->SetLimitText( COLOR_VAL_MAX_CHARS );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_G );
    pEdit->SetLimitText( COLOR_VAL_MAX_CHARS );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_B );
    pEdit->SetLimitText( COLOR_VAL_MAX_CHARS );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_A );
    pEdit->SetLimitText( COLOR_VAL_MAX_CHARS );
}

//==========================================================================

void TBlockToolDlg::UpdateSettingControls( void )
{
    CButton*   pBtn;
    CEdit*     pEdit;
    CComboBox* pCombo;
    CSpinButtonCtrl* pSpin;

    xbool bTest0, bTest1, bTest2, bTest3;
    u32   uVal0,  uVal1,  uVal2,  uVal3;
    f32   GammaVal;

    EGamma      PresetGamma;
    EColorSpace SrcCS, DstCS;
    EWhitePoint SrcWP, DstWP;

    // Single item controls
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_SWIZZLE );
    pBtn->SetCheck( m_pDisplayedSetting->GetSwizzle() );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_COMPRESS );
    pBtn->SetCheck( m_pDisplayedSetting->GetCompressFmt() );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_INTENSITY_MAP );
    pBtn->SetCheck( m_pDisplayedSetting->GetIntensityMap() );

    pBtn = (CButton*)GetDlgItem( IDC_4BIT_TEST_CHECK );
    pBtn->SetCheck( m_pDisplayedSetting->GetAllow4BitConversionTest( ) );


    // Apply Alpha control group
    bTest0 = m_pDisplayedSetting->GetApplyAlpha( bTest1, bTest2, bTest3 );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_PUNCHTHROUGH );
    pBtn->SetCheck( bTest2 );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_GENTRANSPARENT );
    pBtn->SetCheck( bTest3 );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_ALPHA_BYFILE );
    pBtn->SetCheck( bTest1 );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_APPLYALPHA );
    pBtn->SetCheck( bTest0 );
    OnCheckApplyalpha();

    // Build Mips control group
    bTest0 = m_pDisplayedSetting->GetBuildMips( bTest1, uVal0 );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_CUSTOMMIPS );
    pBtn->SetCheck( bTest1 );

    pSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_NMIPLEVELS );
    pSpin->SetRange( TBLOCK_BUILD_MIPS_MIN, TBLOCK_BUILD_MIPS_MAX );
    pSpin->SetPos( uVal0 );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_BUILDMIPS );
    pBtn->SetCheck( bTest0 );
    OnCheckBuildmips();

    // Change Color Intensity control group
    bTest0 = m_pDisplayedSetting->GetColorScale( uVal0, uVal1, uVal2, uVal3 );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_R );
    sprintf( m_TempStr, "%ld", uVal0 );
    pEdit->SetWindowText( m_TempStr );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_G );
    sprintf( m_TempStr, "%ld", uVal1 );
    pEdit->SetWindowText( m_TempStr );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_B );
    sprintf( m_TempStr, "%ld", uVal2 );
    pEdit->SetWindowText( m_TempStr );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_A );
    sprintf( m_TempStr, "%ld", uVal3 );
    pEdit->SetWindowText( m_TempStr );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_CHANGEINTENSITY );
    pBtn->SetCheck( bTest0 );
    OnCheckChangeintensity();

    // Convert Color Space control group
    bTest0 = m_pDisplayedSetting->GetColorSpace( SrcCS, SrcWP, DstCS, DstWP );

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_CSPACE_SRC );
    pCombo->SelectString( -1, COLORSPACE_CS_TEXT[SrcCS] );

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_WHITEPT_SRC );
    pCombo->SelectString( -1, COLORSPACE_WP_TEXT[SrcWP] );

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_CSPACE_DST );
    pCombo->SelectString( -1, COLORSPACE_CS_TEXT[DstCS] );

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_WHITEPT_DST );
    pCombo->SelectString( -1, COLORSPACE_WP_TEXT[DstWP] );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_COLORSPACE );
    pBtn->SetCheck( bTest0 );
    OnCheckColorspace();

    // Gamma Correction control group
    bTest0 = m_pDisplayedSetting->GetGamma( bTest1, PresetGamma, GammaVal );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_INV_GAMMA );
    pBtn->SetCheck( bTest1 );

    if( PresetGamma == GAMMA_COUNT )
    {
        sprintf( m_TempStr, "%3.2f", GammaVal );
        pEdit = (CEdit*)GetDlgItem( IDC_EDIT_GAMMA_VALUE );
        pEdit->SetWindowText( m_TempStr );
    }

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_PRESET_GAMMA );
    pCombo->SelectString( -1, GAMMA_TEXT[PresetGamma] );
    OnSelchangeComboPresetGamma();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_GAMMA );
    pBtn->SetCheck( bTest0 );
    OnCheckGamma();

    // Hot Color Fix control group
    bTest0 = m_pDisplayedSetting->GetHotColorFix( bTest1, bTest2 );

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_NTSC );
    pBtn->SetCheck( bTest1 );

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_PAL );
    pBtn->SetCheck( !bTest1 );

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_LUMINANCE );
    pBtn->SetCheck( bTest2 );

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_SATURATION );
    pBtn->SetCheck( !bTest2 );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR );
    pBtn->SetCheck( bTest0 );
    OnCheckHotcolor();

    // Hot Color Test control group
    bTest0 = m_pDisplayedSetting->GetHotColorTest( bTest1, uVal0, uVal1, uVal2, uVal3 );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_R );
    sprintf( m_TempStr, "%ld", uVal0 );
    pEdit->SetWindowText( m_TempStr );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_G );
    sprintf( m_TempStr, "%ld", uVal1 );
    pEdit->SetWindowText( m_TempStr );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_B );
    sprintf( m_TempStr, "%ld", uVal2 );
    pEdit->SetWindowText( m_TempStr );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_A );
    sprintf( m_TempStr, "%ld", uVal3 );
    pEdit->SetWindowText( m_TempStr );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR_TEST );
    pBtn->SetCheck( bTest0 );
    OnCheckHotcolorTest();

    // View settings control group
    bTest0 = m_pDisplayedSetting->GetUseGlobal();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_USEGLOBAL );
    pBtn->SetCheck( bTest0 );

    // Debug controls
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_TINTMIPS );
    pBtn->SetCheck( m_pIMEXObj->IsTintMipsEnabled() );
}

//==========================================================================

void TBlockToolDlg::StoreCurSettingFromControls( void )
{
    CButton*   pBtn;
    CEdit*     pEdit;
    CComboBox* pCombo;
    CSpinButtonCtrl* pSpin;

    xbool bTest0, bTest1, bTest2, bTest3;
    u32   uVal0,  uVal1,  uVal2,  uVal3;
    f32   GammaVal;

    EGamma      PresetGamma;
    EColorSpace SrcCS, DstCS;
    EWhitePoint SrcWP, DstWP;

    // Single item controls
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_SWIZZLE );
    m_pDisplayedSetting->SetSwizzle( pBtn->GetCheck() );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_COMPRESS );
    m_pDisplayedSetting->SetCompressFmt( pBtn->GetCheck() );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_INTENSITY_MAP );
    m_pDisplayedSetting->SetIntensityMap( pBtn->GetCheck() );

    pBtn = (CButton*)GetDlgItem( IDC_4BIT_TEST_CHECK );
    m_pDisplayedSetting->SetAllow4BitConversionTest( pBtn->GetCheck( ) );

    // Apply Alpha control group
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_GENTRANSPARENT );
    bTest3 = pBtn->GetCheck();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_PUNCHTHROUGH );
    bTest2 = pBtn->GetCheck();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_ALPHA_BYFILE );
    bTest1 = pBtn->GetCheck();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_APPLYALPHA );
    bTest0 = pBtn->GetCheck();

    m_pDisplayedSetting->SetApplyAlpha( bTest0, bTest1, bTest2, bTest3 );

    // Build Mips control group
    pSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_NMIPLEVELS );
    pSpin->SetRange( TBLOCK_BUILD_MIPS_MIN, TBLOCK_BUILD_MIPS_MAX );
    uVal0 = pSpin->GetPos();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_CUSTOMMIPS );
    bTest1 = pBtn->GetCheck();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_BUILDMIPS );
    bTest0 = pBtn->GetCheck();

    m_pDisplayedSetting->SetBuildMips( bTest0, bTest1, uVal0 );

    // Change Color Intensity control group
    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_R );
    pEdit->GetWindowText( m_TempStr, sizeof(m_TempStr)-1 );
    sscanf( m_TempStr, "%ld", &uVal0 );
    uVal0 = TBLOCK_CLAMP_CLR_INT( uVal0 );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_G );
    pEdit->GetWindowText( m_TempStr, sizeof(m_TempStr)-1 );
    sscanf( m_TempStr, "%ld", &uVal1 );
    uVal1 = TBLOCK_CLAMP_CLR_INT( uVal1 );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_B );
    pEdit->GetWindowText( m_TempStr, sizeof(m_TempStr)-1 );
    sscanf( m_TempStr, "%ld", &uVal2 );
    uVal2 = TBLOCK_CLAMP_CLR_INT( uVal2 );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_A );
    pEdit->GetWindowText( m_TempStr, sizeof(m_TempStr)-1 );
    sscanf( m_TempStr, "%ld", &uVal3 );
    uVal3 = TBLOCK_CLAMP_CLR_INT( uVal3 );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_CHANGEINTENSITY );
    bTest0 = pBtn->GetCheck();

    m_pDisplayedSetting->SetColorScale( bTest0, uVal0, uVal1, uVal2, uVal3 );

    // Convert Color Space control group
    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_CSPACE_SRC );
    SrcCS = (EColorSpace)pCombo->GetItemData( pCombo->GetCurSel() );

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_WHITEPT_SRC );
    SrcWP = (EWhitePoint)pCombo->GetItemData( pCombo->GetCurSel() );

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_CSPACE_DST );
    DstCS = (EColorSpace)pCombo->GetItemData( pCombo->GetCurSel() );

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_WHITEPT_DST );
    DstWP = (EWhitePoint)pCombo->GetItemData( pCombo->GetCurSel() );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_COLORSPACE );
    bTest0 = pBtn->GetCheck();

    m_pDisplayedSetting->SetColorSpace( bTest0, SrcCS, SrcWP, DstCS, DstWP );

    // Gamma Correction control group
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_INV_GAMMA );
    bTest1 = pBtn->GetCheck();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_GAMMA );
    bTest0 = pBtn->GetCheck();

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_PRESET_GAMMA );
    PresetGamma = (EGamma)pCombo->GetItemData( pCombo->GetCurSel() );

    if( PresetGamma == GAMMA_COUNT )
    {
        pEdit = (CEdit*)GetDlgItem( IDC_EDIT_GAMMA_VALUE );
        pEdit->GetWindowText( m_TempStr, sizeof(m_TempStr)-1 );
        sscanf( m_TempStr, "%f", &GammaVal );

        GammaVal = TBLOCK_CLAMP_GAMMA_VAL( GammaVal );

        m_pDisplayedSetting->SetGamma( bTest0, bTest1, GammaVal );
    }
    else
    {
        m_pDisplayedSetting->SetGamma( bTest0, bTest1, PresetGamma );
    }

    // Hot Color Test control group
    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_R );
    pEdit->GetWindowText( m_TempStr, sizeof(m_TempStr)-1 );
    sscanf( m_TempStr, "%ld", &uVal0 );
    uVal0 = TBLOCK_CLAMP_CLR_INT( uVal0 );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_G );
    pEdit->GetWindowText( m_TempStr, sizeof(m_TempStr)-1 );
    sscanf( m_TempStr, "%ld", &uVal1 );
    uVal1 = TBLOCK_CLAMP_CLR_INT( uVal1 );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_B );
    pEdit->GetWindowText( m_TempStr, sizeof(m_TempStr)-1 );
    sscanf( m_TempStr, "%ld", &uVal2 );
    uVal2 = TBLOCK_CLAMP_CLR_INT( uVal2 );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_A );
    pEdit->GetWindowText( m_TempStr, sizeof(m_TempStr)-1 );
    sscanf( m_TempStr, "%ld", &uVal3 );
    uVal3 = TBLOCK_CLAMP_CLR_INT( uVal3 );

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_NTSC );
    bTest1 = pBtn->GetCheck();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR_TEST );
    bTest0 = pBtn->GetCheck();

    m_pDisplayedSetting->SetHotColorTest( bTest0, bTest1, uVal0, uVal1, uVal2, uVal3 );

    // Hot Color Fix control group
    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_LUMINANCE );
    bTest2 = pBtn->GetCheck();

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_NTSC );
    bTest1 = pBtn->GetCheck();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR );
    bTest0 = pBtn->GetCheck();

    m_pDisplayedSetting->SetHotColorFix( bTest0, bTest1, bTest2 );

    // View settings control group
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_USEGLOBAL );
    m_pDisplayedSetting->SetUseGlobal( pBtn->GetCheck() );
}

//==========================================================================

int TBlockToolDlg::GetCurSelectedItem( void )
{
    int      CurSel;
    POSITION CurPos;

    CurPos = m_pFileList->GetFirstSelectedItemPosition();

    if( CurPos == NULL )
        return LIST_NO_SELECTION;

    CurSel = m_pFileList->GetNextSelectedItem( CurPos );

    return CurSel;
}

//==========================================================================

int TBlockToolDlg::GetListItemCount( void )
{
    return m_pFileList->GetItemCount();
}

//==========================================================================

void TBlockToolDlg::UpdateListHorizSize( void )
{
    int i;

    // Loop through all the columns adjusting their width to match content
    for( i = 0; i < LISTHDR_ID_COUNT; i++ )
    {
        m_pFileList->SetColumnWidth( i, LVSCW_AUTOSIZE );

        // Clamp column width
        if( m_pFileList->GetColumnWidth( i ) < LIST_HDR_PATH_MINWIDTH[i] )
            m_pFileList->SetColumnWidth( i, LIST_HDR_PATH_MINWIDTH[i] );
    }
}

//==========================================================================

void TBlockToolDlg::HiLiteItem( int Index )
{
    int ItemCount;
    int CurSel;

    ItemCount = m_pFileList->GetItemCount();

    // Select no item if index is out of range
    if( (Index < 0) || (Index >= ItemCount) )
    {
        CurSel = this->GetCurSelectedItem();
        m_pFileList->SetItemState( CurSel, 0, LVIS_FOCUSED | LVIS_SELECTED );
    }
    else
    {
        m_pFileList->SetItemState( Index, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED );
        m_pFileList->SetSelectionMark( Index );
        m_pFileList->SetFocus();
    }
}

//==========================================================================

int TBlockToolDlg::AddItem( int Index, const char* ItemText, TBlockSettings* pTBSSrc )
{
    int Err;
    int ItemCount;

    ItemCount = m_pFileList->GetItemCount();

    // Make sure index to add is valid
    if( (Index < 0) || (Index > ItemCount) )
        Index = ItemCount;

    m_bCodeAddRemoveItem = TRUE;

    // Add the item to the list
    Index = m_pFileList->InsertItem( Index, ItemText );
    Err   = Index;

    m_bCodeAddRemoveItem = FALSE;

    if( (Err == LB_ERR) || (Err == LB_ERRSPACE) )
        return LIST_ERROR;

    TBlockSettings* pTBS;
    TBlockShortName ShortName;
    s32 Width = -1;
    s32 Height = -1;
    s32 BPP = -1;

    TBLOCKPATH_GetShortName( *((TBlockPath*)ItemText), ShortName );
    // Allocate a tblock setting
    pTBS = this->AllocItemSetting();

    m_bCodeAddRemoveItem = TRUE;

    // Put short name for item into list control
    m_pFileList->SetItem( Index, LISTHDR_ID_SHORTNAME, LVIF_TEXT, ShortName, 0, 0, 0, 0 );
    char temp_str[20];
    x_sprintf( temp_str, "%d", Width );
    m_pFileList->SetItem( Index, LISTHDR_ID_WIDTH, LVIF_TEXT, temp_str, 0, 0, 0, 0 );
    x_sprintf( temp_str, "%d", Height );
    m_pFileList->SetItem( Index, LISTHDR_ID_HEIGHT, LVIF_TEXT, temp_str, 0, 0, 0, 0 );
    x_sprintf( temp_str, "%d", BPP );
    m_pFileList->SetItem( Index, LISTHDR_ID_BPP, LVIF_TEXT, temp_str, 0, 0, 0, 0 );

    // Set the item's data to the TBlock setting
    m_pFileList->SetItemData( Index, (DWORD)pTBS );

    m_bCodeAddRemoveItem = FALSE;

    // if a setting is being provided, copy it
    if( pTBSSrc != NULL )
    {
        memcpy( pTBS, pTBSSrc, sizeof(TBlockSettings) );
    }

    return Index;
}

//==========================================================================

int TBlockToolDlg::RemoveItem( int Index )
{
    int Err;
    int ItemCount;
    TBlockSettings* pTBS;

    if( Index < 0 )
        return LIST_ERROR;

    ItemCount = this->GetListItemCount();

    if( Index >= ItemCount )
        return LIST_ERROR;

    m_bCodeAddRemoveItem = TRUE;

    // Free the allocated tblock setting from item
    pTBS = (TBlockSettings*)m_pFileList->GetItemData( Index );

    this->FreeItemSetting( pTBS );

    Err = m_pFileList->DeleteItem( Index );

    m_bCodeAddRemoveItem = FALSE;

    return Err;
}

//==========================================================================

void TBlockToolDlg::RemoveAllItems( void )
{
    int i;
    int ItemCount;
    TBlockSettings* pTBS;

    ItemCount = this->GetListItemCount();

    if( ItemCount <= 0 )
        return;

    // release all the item settings
    for( i = 0; i < ItemCount; i++ )
    {
        pTBS = (TBlockSettings*)m_pFileList->GetItemData( i );
        this->FreeItemSetting( pTBS );
    }

    // remove all the items from list
    m_pFileList->DeleteAllItems();
}

//==========================================================================

void TBlockToolDlg::UpdateItemCountDisplay( void )
{
    CStatic* pStatic;
    int      ItemCount;

    ItemCount = this->GetListItemCount();

    sprintf( m_TempStr, "%d", ItemCount );

    pStatic = (CStatic*)GetDlgItem( IDC_STATIC_NTEXTURES );

    pStatic->SetWindowText( m_TempStr );
}

//==========================================================================

void TBlockToolDlg::UpdateSelectionDisplay( void )
{
    CButton* pBtn;
    CStatic* pStatic;
    int      ItemIndex;
    int      ItemCount;

    ItemIndex = this->GetCurSelectedItem();
    ItemCount = this->GetListItemCount();

    // Update the display of item settings
    if( ItemIndex == LIST_NO_SELECTION )
    {
        pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEWGLOBAL );
        pBtn->SetCheck( TRUE );

        OnRadioViewglobal();
    }
    else
    {
        pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEWCUSTOM );
        pBtn->EnableWindow( TRUE );
        pBtn->SetCheck( TRUE );

        OnRadioViewcustom();
    }

    // Show/Hide controls that can't operate unless a selection is made
    pBtn = (CButton*)GetDlgItem( IDC_BUTTON_REMOVE );
    pBtn->EnableWindow( ItemIndex != LIST_NO_SELECTION );
    pBtn = (CButton*)GetDlgItem( IDC_BUTTON_MOVEUP );
    pBtn->EnableWindow( ItemIndex != LIST_NO_SELECTION );
    pBtn = (CButton*)GetDlgItem( IDC_BUTTON_MOVEDOWN );
    pBtn->EnableWindow( ItemIndex != LIST_NO_SELECTION );
    pBtn = (CButton*)GetDlgItem( IDC_BUTTON_SWAPINDEX );
    pBtn->EnableWindow( ItemIndex != LIST_NO_SELECTION );
    pBtn = (CButton*)GetDlgItem( IDC_BUTTON_MOVEINDEX );
    pBtn->EnableWindow( ItemIndex != LIST_NO_SELECTION );
    pBtn = (CButton*)GetDlgItem( IDC_BUTTON_TESTEXPORT );
    pBtn->EnableWindow( ItemIndex != LIST_NO_SELECTION );

    // Show/Hide controls that can't operate unless the list contains items
    pBtn = (CButton*)GetDlgItem( IDC_BUTTON_REMOVEALL );
    pBtn->EnableWindow( ItemCount > 0 );
    pBtn = (CButton*)GetDlgItem( IDC_BUTTON_EXPORT );
    pBtn->EnableWindow( ItemCount > 0 );
    pBtn = (CButton*)GetDlgItem( IDC_BUTTON_SAVE_TXT );
    pBtn->EnableWindow( ItemCount > 0 );


    // Format text to match current selection index, if there is one
    // and set the control's text to it.
    if( ItemIndex == LIST_NO_SELECTION )
        sprintf( m_TempStr, "None" );
    else
        sprintf( m_TempStr, "%d", ItemIndex );

    pStatic = (CStatic*)GetDlgItem( IDC_STATIC_CURSELECTION );
    pStatic->SetWindowText( m_TempStr );
}

//==========================================================================

void TBlockToolDlg::AddFilesViaWindows( BOOL bInsert )
{
    const long FILE_ADD_DLG_FLAGS = OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

    CString  bmFilename( '\0', 300 );
    POSITION CurPos;

    int  OrigSel;
    int  CurSel;
    int  Err;
    char OpenFilenameBuffer[2048];

    if( this->GetListItemCount() >= TBLOCK_MAXFILES )
    {
        //display error message
        return;
    }

    memset( OpenFilenameBuffer, 0, sizeof(OpenFilenameBuffer) );

    // Create file dialog to add files to list
    CFileDialog OpenDlg( TRUE,              //Open file dialog
                         NULL,              //Default extension
                         NULL,              //initial filename
                         FILE_ADD_DLG_FLAGS,//Flags
                         FILE_ADD_FILTER,   //Filter
                         this );            //Parent Wnd

    // Set filename buffer so we can add a lot of files, and
    // setup initial folder to start in as a convenience
    OpenDlg.m_ofn.lpstrFile       = OpenFilenameBuffer;
    OpenDlg.m_ofn.nMaxFile        = sizeof(OpenFilenameBuffer)-1;
    OpenDlg.m_ofn.lpstrInitialDir = m_AddFolder;

    if( IDCANCEL == OpenDlg.DoModal() )
        return;

    // Update the current folder to add files from
    ::GetCurrentDirectory( sizeof(m_AddFolder)-1, m_AddFolder );

    // Get filenames from the Open File dialog box, add them to list
    CurPos = OpenDlg.GetStartPosition();
    if( CurPos == NULL )
        return;

    OrigSel = this->GetCurSelectedItem();

    // Choose current insertion point
    if( LIST_NO_SELECTION == OrigSel )
        CurSel = this->GetListItemCount();
    else
        CurSel = OrigSel + 1;


    while( CurPos != NULL )
    {
        bmFilename = OpenDlg.GetNextPathName( CurPos );

        // Add the filename to the list box
        if( bInsert )
            Err = this->AddItem( CurSel, bmFilename );
        else
            Err = this->AddItem( -1, bmFilename );

        if( Err != LIST_ERROR )
            CurSel++;

        if( this->GetListItemCount() >= TBLOCK_MAXFILES )
        {
            //display error message
            break;
        }
    }

    this->HiLiteItem( OrigSel );

    this->UpdateItemCountDisplay();
    this->UpdateSelectionDisplay();
    this->UpdateListHorizSize();
}

//==========================================================================

void TBlockToolDlg::OnButtonAdd( void )
{
    this->AddFilesViaWindows( FALSE );
}

//==========================================================================

void TBlockToolDlg::OnButtonInsert( void )
{
    this->AddFilesViaWindows( TRUE );
}

//==========================================================================

void TBlockToolDlg::OnButtonRemove( void )
{
    int ItemCount;
    int LastIndex;
    int CurSel;
    
    CurSel = this->GetCurSelectedItem();

    if( CurSel == LIST_NO_SELECTION )
        return;

    ItemCount = this->GetListItemCount();

    // Remove the current selection from list
    this->RemoveItem( CurSel );

    LastIndex = ItemCount - 1;

    // Make sure current selection will be in list
    if( CurSel > LastIndex )
        CurSel = LastIndex;

    this->HiLiteItem( CurSel );

    this->UpdateItemCountDisplay();
    this->UpdateSelectionDisplay();
    this->UpdateListHorizSize();
}

//==========================================================================

void TBlockToolDlg::OnButtonRemoveall( void )
{
    this->RemoveAllItems();

    this->UpdateItemCountDisplay();
    this->UpdateSelectionDisplay();
    this->UpdateListHorizSize();
}

//==========================================================================

void TBlockToolDlg::OnButtonInsertEmpty( void )
{
    int CurSel;
    int ItemCount;

    ItemCount = this->GetListItemCount();

    if( ItemCount >= TBLOCK_MAXFILES )
    {
        //display error message
        return;
    }

    CurSel = this->GetCurSelectedItem();

    // If no selection, add empty to end of list
    if( (CurSel == LIST_NO_SELECTION) && (ItemCount > 0) )
        CurSel = ItemCount;
    else
        CurSel += 1;

    // init default path
    TBLOCKPATH_InitDefault( *((TBlockPath*)m_TempStr2) );

    this->AddItem( CurSel, m_TempStr2 );

    this->HiLiteItem( CurSel );

    this->UpdateItemCountDisplay();
    this->UpdateSelectionDisplay();
    this->UpdateListHorizSize();
}

//==========================================================================
/*
static int CALLBACK SortItemListABC( LPARAM Param1, LPARAM Param2, LPARAM SortParam )
{
    CListCtrl* pList = (CListCtrl*)SortParam;
    CString ItemStr1 = pList->GetItemText( Param1, LISTHDR_ID_PATH );
    CString ItemStr2 = pList->GetItemText( Param2, LISTHDR_ID_PATH );

    int CmpResult = strcmp( ItemStr1, ItemStr2 );

    if( CmpResult < 0 )
        return -1;
    else if( CmpResult > 0 )
        return 1;

    if( Param1 < Param2 )
        return -1;
    else// if( Param1 > Param2 )
        return 1;

    return 0;
}
*/
//--------------------------------------------------------------------------

void TBlockToolDlg::OnButtonSortalpha( void )
{
    if( this->GetListItemCount() <= 1 )
        return;

    //m_pFileList->SortItems( SortItemListABC, (DWORD)(m_pFileList) );

    this->UpdateSelectionDisplay();
}

//==========================================================================

void TBlockToolDlg::OnButtonMovedown( void )
{
    int CurSel;
    int LastIndex;
    TBlockSettings ItemTBS;
    DWORD dwItemData;

    CurSel    = this->GetCurSelectedItem();
    LastIndex = this->GetListItemCount() - 1;

    if( CurSel == LIST_NO_SELECTION )
        return;

    // Can't move item any farther, hilite item and return focus to list
    if( CurSel == LastIndex )
    {
        this->HiLiteItem( CurSel );
        return;
    }

    // Process for moving item is just removing item and re-adding it at the next index
    m_pFileList->GetItemText( CurSel, LISTHDR_ID_PATH, m_TempStr2, sizeof(m_TempStr2)-1 );

    // Save settings for this item before removing it
    dwItemData = (DWORD)m_pFileList->GetItemData( CurSel );
    ItemTBS = *((TBlockSettings*)dwItemData);

    this->RemoveItem( CurSel );

    CurSel += 1;

    this->AddItem( CurSel, m_TempStr2 );

    // restore item settings
    dwItemData = (DWORD)m_pFileList->GetItemData( CurSel );
    *((TBlockSettings*)dwItemData) = ItemTBS;

    this->HiLiteItem( CurSel );

    this->UpdateItemCountDisplay();
    this->UpdateSelectionDisplay();
}

//==========================================================================

void TBlockToolDlg::OnButtonMoveup( void )
{
    int CurSel;
    TBlockSettings ItemTBS;
    DWORD dwItemData;

    CurSel = this->GetCurSelectedItem();

    if( CurSel == LIST_NO_SELECTION )
        return;

    // Can't move item any farther, hilite item and return focus to list
    if( CurSel == 0 )
    {
        this->HiLiteItem( CurSel );
        return;
    }

    // Process for moving item is just removing item and re-adding it at the previous index
    m_pFileList->GetItemText( CurSel, LISTHDR_ID_PATH, m_TempStr2, sizeof(m_TempStr2)-1 );

    // Save settings for this item before removing it
    dwItemData = (DWORD)m_pFileList->GetItemData( CurSel );
    ItemTBS = *((TBlockSettings*)dwItemData);

    this->RemoveItem( CurSel );

    CurSel -= 1;

    this->AddItem( CurSel, m_TempStr2 );

    // restore item settings
    dwItemData = (DWORD)m_pFileList->GetItemData( CurSel );
    *((TBlockSettings*)dwItemData) = ItemTBS;

    this->HiLiteItem( CurSel );

    this->UpdateItemCountDisplay();
    this->UpdateSelectionDisplay();
}

//==========================================================================

void TBlockToolDlg::OnButtonMoveindex( void )
{
    int CurSel;
    int ItemCount;
    int NewIndex;
    TBlockSettings ItemTBS;
    DWORD dwItemData;

    // Check if there's no selection or only one item in list
    CurSel = this->GetCurSelectedItem();

    if( CurSel == LIST_NO_SELECTION )
        return;

    ItemCount = this->GetListItemCount();

    if( ItemCount == 1 )
    {
        this->HiLiteItem( CurSel );
        return;
    }

    // Display dialog for new index input
    CSwapIndexDlg SwapDialog( CurSel, ItemCount - 1, this );

    if( !SwapDialog.DoModal() )
    {
        this->HiLiteItem( CurSel );
        return;
    }

    NewIndex = SwapDialog.GetNewIndex();

    if( NewIndex == CurSel )
        return;

    // Remove and re-insert item into new place in list
    m_pFileList->GetItemText( CurSel, LISTHDR_ID_PATH, m_TempStr2, sizeof(m_TempStr2)-1 );

    dwItemData = (DWORD)m_pFileList->GetItemData( CurSel );
    ItemTBS = *((TBlockSettings*)dwItemData);

    this->RemoveItem( CurSel );

    this->AddItem( NewIndex, m_TempStr2 );

    dwItemData = (DWORD)m_pFileList->GetItemData( CurSel );
    *((TBlockSettings*)dwItemData) = ItemTBS;

    this->HiLiteItem( NewIndex );

    this->UpdateSelectionDisplay();
}

//==========================================================================

void TBlockToolDlg::OnButtonSwapindex( void )
{
    int CurSel;
    int ItemCount;
    int NewIndex;
    CString ItemText;
    CString SwapText;
    TBlockSettings ItemTBS;
    TBlockSettings SwapTBS;

    // Check if there's no selection or only one item in list
    CurSel = this->GetCurSelectedItem();

    if( CurSel == LIST_NO_SELECTION )
        return;

    ItemCount = this->GetListItemCount();

    if( ItemCount == 1 )
    {
        this->HiLiteItem( CurSel );
        return;
    }

    // Display dialog for new index input
    CSwapIndexDlg SwapDialog( CurSel, ItemCount - 1, this );

    if( !SwapDialog.DoModal() )
    {
        this->HiLiteItem( CurSel );
        return;
    }

    NewIndex = SwapDialog.GetNewIndex();

    if( NewIndex == CurSel )
        return;

    // Remove both items and re-insert them in list, swapping their positions
    ItemText = m_pFileList->GetItemText( CurSel, LISTHDR_ID_PATH );
    SwapText = m_pFileList->GetItemText( NewIndex, LISTHDR_ID_PATH );

    ItemTBS = *((TBlockSettings*)m_pFileList->GetItemData( CurSel ));
    SwapTBS = *((TBlockSettings*)m_pFileList->GetItemData( NewIndex ));

    this->RemoveItem( CurSel );
    this->AddItem( CurSel, SwapText );
    *((TBlockSettings*)m_pFileList->GetItemData( CurSel )) = SwapTBS;

    this->RemoveItem( NewIndex );
    this->AddItem( NewIndex, ItemText );
    *((TBlockSettings*)m_pFileList->GetItemData( NewIndex )) = ItemTBS;

    this->HiLiteItem( NewIndex );

    this->UpdateSelectionDisplay();
}

//==========================================================================

void TBlockToolDlg::OnButtonTestExport( void )
{
    CString         ExportFilename;
    TBlockShortName ImageShortname;
    TBlockPath      ImageFilename;
    TBlockSettings* pImageSettings;
    int             FileListID;

    if( !m_bThisReady )
        return;

    if( this->GetListItemCount() <= 0 )
        return;

    FileListID = GetCurSelectedItem();

    if( FileListID < 0 )
        return;

    m_pFileList->GetItemText( FileListID, LISTHDR_ID_PATH, ImageFilename, sizeof(TBlockPath)-1 );

    if( TBLOCKPATH_IsDefault( ImageFilename ) )
        return;

    TBLOCKPATH_GetShortName( ImageFilename, ImageShortname );

    const long FILE_DLG_FLAGS = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    //save file dialog
    CFileDialog SaveDlg( FALSE,                 //Open file dialog
                         TBLOCK_FILE_EXT_TGA,   //Default extension
                         ImageShortname,        //initial filename
                         FILE_DLG_FLAGS,        //Flags
                         FILE_SAVE_TGA_FILTER,  //Filter
                         this );                //Parent Wnd

    SaveDlg.m_ofn.lpstrInitialDir = m_SaveFolder;

    if( IDCANCEL == SaveDlg.DoModal() )
        return;

    // save current folder for TBlock file save
    ::GetCurrentDirectory( sizeof(m_SaveFolder)-1, m_SaveFolder );

    ExportFilename = SaveDlg.GetPathName();

    // store settings
    this->StoreCurSettingFromControls();

    // get texture settings for this item
    pImageSettings = (TBlockSettings*)m_pFileList->GetItemData( FileListID );

    if( pImageSettings->GetUseGlobal() )
        pImageSettings = &m_GlobalSetting;

    // export raw texture
    m_pIMEXObj->TestExport( ExportFilename, this->GetExportTarget(), pImageSettings, ImageFilename );
}


//==========================================================================

void TBlockToolDlg::OnButtonRefreshProps( void )
{
    int ItemCount;

    ItemCount = m_pFileList->GetItemCount();
    s32 Width;
    s32 Height;
    s32 BPP;

    s32 i;
    for (i=0; i<ItemCount; ++i)
    {
        Width = -1;
        Height = -1;
        BPP = -1;
        LVITEM Item;
        Item.iItem=i;
        char ItemText[256];
        m_pFileList->GetItemText( i, LISTHDR_ID_PATH, ItemText, sizeof(TBlockPath)-1 );
        if( 0 != x_strcmp( ItemText, TBLOCK_EMPTY_PATH ) )
        {
            x_bitmap temp;
            AUXBMP_LoadBitmap( temp, ItemText );
            Width = temp.GetWidth();
            Height = temp.GetHeight();
            BPP = temp.GetBPP();
            temp.KillBitmap();
        }
        char temp_str[20];
        x_sprintf( temp_str, "%d", Width );
        m_pFileList->SetItem( i, LISTHDR_ID_WIDTH, LVIF_TEXT, temp_str, 0, 0, 0, 0 );
        x_sprintf( temp_str, "%d", Height );
        m_pFileList->SetItem( i, LISTHDR_ID_HEIGHT, LVIF_TEXT, temp_str, 0, 0, 0, 0 );
        x_sprintf( temp_str, "%d", BPP );
        m_pFileList->SetItem( i, LISTHDR_ID_BPP, LVIF_TEXT, temp_str, 0, 0, 0, 0 );
    }
}

//==========================================================================

BOOL TBlockToolDlg::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
    if( m_bThisReady )
    {
        if( wParam == IDC_LIST_FILES )
        {
            if( !m_bCodeAddRemoveItem )
                this->UpdateSelectionDisplay();
        }
    }

    return CDialog::OnNotify(wParam, lParam, pResult);
}

//==========================================================================

void TBlockToolDlg::OnButtonOpen( void )
{
    s32     RetCode;
    CString FilenameToOpen;

    const long FILE_DLG_FLAGS = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

    CFileDialog OpenDlg( TRUE,              //Open file dialog
                         NULL,              //Default extension
                         NULL,              //initial filename
                         FILE_DLG_FLAGS,    //Flags
                         FILE_OPEN_FILTER,  //Filter
                         this );            //Parent Wnd

    OpenDlg.m_ofn.lpstrInitialDir = m_OpenFolder;

    if( IDCANCEL == OpenDlg.DoModal() )
        return;

    // Save current folder for open file
    ::GetCurrentDirectory( sizeof(m_OpenFolder)-1, m_OpenFolder );
    strcpy( m_SaveFolder, m_OpenFolder );

    FilenameToOpen = OpenDlg.GetPathName();

    // Save current text or TBlock filename to save to
    x_splitpath( FilenameToOpen, NULL, NULL, NULL, m_TempStr );

    if( strstr( m_TempStr, TBLOCK_FILE_EXT_TXT ) )
        strcpy( m_TxtSaveFilename, (LPCTSTR)OpenDlg.GetFileName() );
    else
        strcpy( m_SaveFilename, (LPCTSTR)OpenDlg.GetFileName() );

    // Do the data import
    RetCode = m_pIMEXObj->Import( FilenameToOpen, &m_GlobalSetting, NULL, 0 );

    // remove all items from list before loading
    this->RemoveAllItems();

    this->LoadDataFromImport();
    this->UpdateSettingControls();

    this->UpdateItemCountDisplay();
    this->UpdateSelectionDisplay();
    this->UpdateListHorizSize();
}

//==========================================================================

void TBlockToolDlg::OnButtonSaveTxt( void )
{
    CString TextFilename;

    const long FILE_DLG_FLAGS = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    if( this->GetListItemCount() <= 0 )
        return;

    //save file dialog
    CFileDialog SaveDlg( FALSE,                 //Open file dialog
                         TBLOCK_FILE_EXT_TXT,   //Default extension
                         m_TxtSaveFilename,     //initial filename
                         FILE_DLG_FLAGS,        //Flags
                         FILE_SAVE_TXT_FILTER,  //Filter
                         this );                //Parent Wnd

    SaveDlg.m_ofn.lpstrInitialDir = m_TxtSaveFolder;

    if( IDCANCEL == SaveDlg.DoModal() )
        return;

    // save current folder and filename for text file save
    ::GetCurrentDirectory( sizeof(m_TxtSaveFolder)-1, m_TxtSaveFolder );

    strcpy( m_TxtSaveFilename, (LPCTSTR)SaveDlg.GetFileName() );

    TextFilename = SaveDlg.GetPathName();

    // make sure current settings get stored and then export
    this->StoreCurSettingFromControls();

    this->DoExport( TextFilename );
}

//==========================================================================

void TBlockToolDlg::OnButtonExport( void )
{
    CString ExportFilename;

    const long FILE_DLG_FLAGS = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    if( this->GetListItemCount() <= 0 )
        return;

    //save file dialog
    CFileDialog SaveDlg( FALSE,                 //Open file dialog
                         TBLOCK_FILE_EXT_TEX,   //Default extension
                         m_SaveFilename,        //initial filename
                         FILE_DLG_FLAGS,        //Flags
                         FILE_SAVE_TEX_FILTER,  //Filter
                         this );                //Parent Wnd

    SaveDlg.m_ofn.lpstrInitialDir = m_SaveFolder;

    if( IDCANCEL == SaveDlg.DoModal() )
        return;

    // save current folder and filename for TBlock file save
    ::GetCurrentDirectory( sizeof(m_SaveFolder)-1, m_SaveFolder );

    strcpy( m_SaveFilename, (LPCTSTR)SaveDlg.GetPathName() );

    ExportFilename = SaveDlg.GetPathName();

    // make sure current settings get stored and then export
    this->StoreCurSettingFromControls();

    this->DoExport( ExportFilename );
}

//==========================================================================

void TBlockToolDlg::DoExport( const char* pExportFilename )
{
    int     i;
    int     ItemCount;
    int     RetCode;
    void**  pTempPtrArray;

    TBlockPath**     pTBPathPtrArray;
    TBlockSettings** pTBSPtrArray;
    TBlockPath*      pTBPaths;

    // Get item count
    ItemCount = this->GetListItemCount();
    ASSERT( ItemCount > 0 );
    if( ItemCount <= 0 )
        return;

    // allocate temp pointer array to send to exporter
    pTempPtrArray = new void*[ ItemCount * 2 ];
    ASSERT( pTempPtrArray != NULL );
    if( pTempPtrArray == NULL )
        return;

    // allocate data to retrieve paths from list control
    pTBPaths = new TBlockPath[ ItemCount ];
    ASSERT( pTBPaths != NULL );
    if( pTBPaths == NULL )
    {
        delete[] pTempPtrArray;
        return;
    }

    // set pointers to allocated chunk of pointer array
    pTBSPtrArray    = (TBlockSettings**)(&pTempPtrArray[0]);
    pTBPathPtrArray = (TBlockPath**)(&pTempPtrArray[ItemCount]);

    // get paths and settings from list control
    for( i = 0; i < ItemCount; i++ )
    {
        pTBPathPtrArray[i] = &pTBPaths[i];
        m_pFileList->GetItemText( i, LISTHDR_ID_PATH, pTBPaths[i], sizeof(TBlockPath)-1 );

        pTBSPtrArray[i] = (TBlockSettings*)m_pFileList->GetItemData( i );
    }

    // export
    RetCode = m_pIMEXObj->Export( pExportFilename,
                                  this->GetExportTarget(),
                                  ItemCount,
                                  &m_GlobalSetting,
                                  pTBSPtrArray,
                                  pTBPathPtrArray,
                                  NULL, 0 );

    // free memory
    delete[] pTBPaths;
    delete[] pTempPtrArray;
}

//==========================================================================

void TBlockToolDlg::ClearOutputText( void )
{
    CRichEditCtrl* pREdit;

    pREdit = (CRichEditCtrl*)GetDlgItem( IDC_RICHEDIT_OUTPUT );

    pREdit->SetSel( 0, -1 );
    pREdit->ReplaceSel( "\0" );
}

//==========================================================================

void TBlockToolDlg::OnButtonClearOutput( void )
{
    this->ClearOutputText();
}

//==========================================================================

void TBlockToolDlg::OnCheckCompress( void )
{
/*
    int      CheckOn;
    CButton* pBtn;

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_COMPRESS );
    CheckOn = pBtn->GetCheck();

    // Treat this control like a corresponding radio button to Intensity map option
    if( CheckOn )
    {
        pBtn = (CButton*)GetDlgItem( IDC_CHECK_INTENSITY_MAP );
        pBtn->SetCheck( FALSE );
    }
*/
}

//==========================================================================

void TBlockToolDlg::OnCheckIntensityMap( void )
{
/*
    int      CheckOn;
    CButton* pBtn;

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_INTENSITY_MAP );
    CheckOn = pBtn->GetCheck();

    // Treat this control like a corresponding radio button to Compress option
    if( CheckOn )
    {
        pBtn = (CButton*)GetDlgItem( IDC_CHECK_COMPRESS );
        pBtn->SetCheck( FALSE );
    }
*/
}

//==========================================================================

void TBlockToolDlg::OnCheckAlphaByfile( void )
{
    CButton* pBtn;
    int      CheckOn;
    BOOL     WndEnabled;

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_ALPHA_BYFILE );
    CheckOn    = pBtn->GetCheck();
    WndEnabled = pBtn->IsWindowEnabled();

    // Enable sub-controls based on state of main group button
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_PUNCHTHROUGH );
    pBtn->EnableWindow( !CheckOn && WndEnabled );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_GENTRANSPARENT );
    pBtn->EnableWindow( !CheckOn && WndEnabled );
}

//==========================================================================

void TBlockToolDlg::OnCheckApplyalpha( void )
{
    CButton* pBtn;
    int      CheckOn;
    BOOL     WndEnabled;

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_APPLYALPHA );
    CheckOn    = pBtn->GetCheck();
    WndEnabled = pBtn->IsWindowEnabled();

    // Enable sub-controls based on state of main group button
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_ALPHA_BYFILE );
    pBtn->EnableWindow( CheckOn && WndEnabled );

    OnCheckAlphaByfile();
}

//==========================================================================

void TBlockToolDlg::OnCheckBuildmips( void )
{
    CEdit*   pEdit;
    CButton* pBtn;
    CStatic* pStatic;
    int      CheckOn;
    BOOL     WndEnabled;
    CSpinButtonCtrl* pSpin;

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_BUILDMIPS );
    CheckOn    = pBtn->GetCheck();
    WndEnabled = pBtn->IsWindowEnabled();

    // Enable sub-controls based on state of main group button
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_CUSTOMMIPS );
    pBtn->EnableWindow( CheckOn && WndEnabled );

    pSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_NMIPLEVELS );
    pSpin->EnableWindow( CheckOn && WndEnabled );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_MIPLEVELS );
    pEdit->EnableWindow( CheckOn && WndEnabled );

    pStatic = (CStatic*)GetDlgItem( IDC_STATIC_NMIPLEVELS );
    pStatic->EnableWindow( CheckOn && WndEnabled );
}

//==========================================================================

void TBlockToolDlg::OnCheckChangeintensity( void )
{
    CButton* pBtn;
    CEdit*   pEdit;
    int      CheckOn;
    BOOL     WndEnabled;

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_CHANGEINTENSITY );
    CheckOn    = pBtn->GetCheck();
    WndEnabled = pBtn->IsWindowEnabled();

    // Enable sub-controls based on state of main group button
    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_R );
    pEdit->EnableWindow( CheckOn && WndEnabled );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_G );
    pEdit->EnableWindow( CheckOn && WndEnabled );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_B );
    pEdit->EnableWindow( CheckOn && WndEnabled );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_CSCALE_A );
    pEdit->EnableWindow( CheckOn && WndEnabled );
}

//==========================================================================

void TBlockToolDlg::OnCheckColorspace( void )
{
    CButton*   pBtn;
    CComboBox* pCombo;
    CStatic*   pStatic;
    int        CheckOn;
    BOOL       WndEnabled;

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_COLORSPACE );
    CheckOn    = pBtn->GetCheck();
    WndEnabled = pBtn->IsWindowEnabled();

    // Enable sub-controls based on state of main group button
    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_CSPACE_SRC );
    pCombo->EnableWindow( CheckOn && WndEnabled );

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_WHITEPT_SRC );
    pCombo->EnableWindow( CheckOn && WndEnabled );

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_CSPACE_DST );
    pCombo->EnableWindow( CheckOn && WndEnabled );

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_WHITEPT_DST );
    pCombo->EnableWindow( CheckOn && WndEnabled );

    pStatic = (CStatic*)GetDlgItem( IDC_STATIC_CSPACE_CS_SRC );
    pStatic->EnableWindow( CheckOn && WndEnabled );

    pStatic = (CStatic*)GetDlgItem( IDC_STATIC_CSPACE_WP_SRC );
    pStatic->EnableWindow( CheckOn && WndEnabled );

    pStatic = (CStatic*)GetDlgItem( IDC_STATIC_CSPACE_CS_DST );
    pStatic->EnableWindow( CheckOn && WndEnabled );

    pStatic = (CStatic*)GetDlgItem( IDC_STATIC_CSPACE_WP_DST );
    pStatic->EnableWindow( CheckOn && WndEnabled );
}

//==========================================================================

void TBlockToolDlg::OnCheckGamma( void )
{
    CStatic*   pStatic;
    CComboBox* pCombo;
    CButton*   pBtn;
    CEdit*     pEdit;
    int        CheckOn;
    BOOL       WndEnabled;

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_GAMMA );
    CheckOn    = pBtn->GetCheck();
    WndEnabled = pBtn->IsWindowEnabled();

    // Enable sub-controls based on state of main group button
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_INV_GAMMA );
    pBtn->EnableWindow( CheckOn && WndEnabled );

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_PRESET_GAMMA );
    pCombo->EnableWindow( CheckOn && WndEnabled );

    pEdit= (CEdit*)GetDlgItem( IDC_EDIT_GAMMA_VALUE );
    pEdit->EnableWindow( CheckOn && WndEnabled );

    pStatic = (CStatic*)GetDlgItem( IDC_STATIC_GAMMAVAL );
    pStatic->EnableWindow( CheckOn && WndEnabled );
}

//==========================================================================

void TBlockToolDlg::OnSelchangeComboPresetGamma( void )
{
    DWORD      GammaVal;
    int        CurSel;
    CComboBox* pCombo;
    CEdit*     pEdit;

    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_PRESET_GAMMA );

    CurSel = pCombo->GetCurSel();

    // Get pre-defined gamma value from selection item data
    GammaVal = (DWORD)pCombo->GetItemData( CurSel );

    if( GammaVal == GAMMA_COUNT )
        return;

    // format a string with gamma value and put in edit control
    sprintf( m_TempStr, "%3.2f", BMPCOLOR_GetGammaValue( (EGamma)GammaVal ) );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_GAMMA_VALUE );

    m_ComboUpdateEditBox = TRUE;

    pEdit->SetWindowText( m_TempStr );
}

//==========================================================================

void TBlockToolDlg::OnChangeEditGammaValue( void )
{
    // If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    int        ItemIndex;
    CComboBox* pCombo;

    if( m_ComboUpdateEditBox == TRUE )
    {
        // Text in this edit control is changing due to a combobox selection,
        // So no need to change combo box for user input
        m_ComboUpdateEditBox = FALSE;
        return;
    }

    // User has typed a value into edit control, change combo box selection to
    // the "custom" setting selection.
    pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_PRESET_GAMMA );

    ItemIndex = pCombo->FindStringExact( -1, GAMMA_TEXT[GAMMA_COUNT] );

    pCombo->SetCurSel( ItemIndex );
}

//==========================================================================

void TBlockToolDlg::OnCheckHotcolor( void )
{
    CStatic* pStatic;
    CButton* pBtn;
    int      CheckOn;
    int      CheckOnOther;
    BOOL     WndEnabled;
    BOOL     EnabledOther;

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR );
    CheckOn    = pBtn->GetCheck();
    WndEnabled = pBtn->IsWindowEnabled();

    // Treat this control like a corresponding radio button to
    // other hotcolor option
    if( CheckOn )
    {
        pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR_TEST );
        pBtn->SetCheck( FALSE );
        OnCheckHotcolorTest();
    }

    // Enable sub-controls based on state of main group button
    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_SATURATION );
    pBtn->EnableWindow( CheckOn && WndEnabled );

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_LUMINANCE );
    pBtn->EnableWindow( CheckOn && WndEnabled );

    // Enable shared controls
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR_TEST );
    CheckOnOther = pBtn->GetCheck();
    EnabledOther = pBtn->IsWindowEnabled();

    pStatic = (CStatic*)GetDlgItem( IDC_STATIC_HOTCOLORTV );
    pStatic->EnableWindow( (CheckOn && WndEnabled) || (CheckOnOther && EnabledOther) );

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_NTSC );
    pBtn->EnableWindow( (CheckOn && WndEnabled) || (CheckOnOther && EnabledOther) );

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_PAL );
    pBtn->EnableWindow( (CheckOn && WndEnabled) || (CheckOnOther && EnabledOther) );
}

//==========================================================================

void TBlockToolDlg::OnCheckHotcolorTest( void )
{
    CStatic* pStatic;
    CEdit*   pEdit;
    CButton* pBtn;
    int      CheckOn;
    int      CheckOnOther;
    BOOL     WndEnabled;
    BOOL     EnabledOther;

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR_TEST );
    CheckOn    = pBtn->GetCheck();
    WndEnabled = pBtn->IsWindowEnabled();

    // Treat this control like a corresponding radio button to
    // other hotcolor option
    if( CheckOn )
    {
        pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR );
        pBtn->SetCheck( FALSE );
        OnCheckHotcolor();
    }

    // Enable sub-controls based on state of main group button
    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_R );
    pEdit->EnableWindow( CheckOn && WndEnabled );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_B );
    pEdit->EnableWindow( CheckOn && WndEnabled );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_G );
    pEdit->EnableWindow( CheckOn && WndEnabled );

    pEdit = (CEdit*)GetDlgItem( IDC_EDIT_HCTEST_A );
    pEdit->EnableWindow( CheckOn && WndEnabled );

    // Enable shared controls
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR );
    CheckOnOther = pBtn->GetCheck();
    EnabledOther = pBtn->IsWindowEnabled();

    pStatic = (CStatic*)GetDlgItem( IDC_STATIC_HOTCOLORTV );
    pStatic->EnableWindow( (CheckOn && WndEnabled) || (CheckOnOther && EnabledOther) );

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_NTSC );
    pBtn->EnableWindow( (CheckOn && WndEnabled) || (CheckOnOther && EnabledOther) );

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_HCF_PAL );
    pBtn->EnableWindow( (CheckOn && WndEnabled) || (CheckOnOther && EnabledOther) );
}

//==========================================================================

void TBlockToolDlg::OnRadioViewglobal( void )
{
    CButton* pBtn;
    int      CheckOn;
    int      CurListSel;

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEWGLOBAL );
    CheckOn = pBtn->GetCheck();

    // set custom radio button to opposite of global
    pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEWCUSTOM );
    pBtn->SetCheck( !CheckOn );

    // Enable sub-controls based on state of main group button
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_USEGLOBAL );
    pBtn->EnableWindow( !CheckOn );

    CurListSel = this->GetCurSelectedItem();

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEWCUSTOM );
    pBtn->EnableWindow( CurListSel != LIST_NO_SELECTION );

    OnCheckUseglobal();

    this->StoreCurSettingFromControls();

    m_pDisplayedSetting = &m_GlobalSetting;

    this->UpdateSettingControls();
}

//==========================================================================

void TBlockToolDlg::OnRadioViewcustom( void )
{
    CButton* pBtn;
    int      CurListSel;
    int      CheckOn;
    BOOL     WndEnabled;

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEWCUSTOM );
    CheckOn    = pBtn->GetCheck();
    WndEnabled = pBtn->IsWindowEnabled();

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEWGLOBAL );
    pBtn->SetCheck( !CheckOn );

    // Enable sub-controls based on state of main group button
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_USEGLOBAL );
    pBtn->EnableWindow( CheckOn && WndEnabled );

    CurListSel = this->GetCurSelectedItem();

    if( CurListSel != LIST_NO_SELECTION )
    {
        this->StoreCurSettingFromControls();

        m_pDisplayedSetting = (TBlockSettings*)m_pFileList->GetItemData( CurListSel );

        this->UpdateSettingControls();

        OnCheckUseglobal();
    }
    else
    {
        pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEWGLOBAL );
        pBtn->SetCheck( TRUE );

        OnRadioViewglobal();
    }
}

//==========================================================================

void TBlockToolDlg::OnCheckUseglobal( void )
{
    CButton* pBtn;
    BOOL     WndEnabled;

    pBtn = (CButton*)GetDlgItem( IDC_RADIO_VIEWGLOBAL );
    WndEnabled = pBtn->GetCheck();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_USEGLOBAL );
    WndEnabled = ( WndEnabled || (!pBtn->GetCheck() && pBtn->IsWindowEnabled()) );


    // Enable sub-controls based on state of main group button
    pBtn = (CButton*)GetDlgItem( IDC_CHECK_SWIZZLE );
    pBtn->EnableWindow( WndEnabled );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_COMPRESS );
    pBtn->EnableWindow( WndEnabled );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_INTENSITY_MAP );
    pBtn->EnableWindow( WndEnabled );

    pBtn = (CButton*)GetDlgItem( IDC_4BIT_TEST_CHECK );
    pBtn->EnableWindow( WndEnabled );

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_APPLYALPHA );
    pBtn->EnableWindow( WndEnabled );

    OnCheckApplyalpha();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_BUILDMIPS );
    pBtn->EnableWindow( WndEnabled );

    OnCheckBuildmips();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR );
    pBtn->EnableWindow( WndEnabled );

    OnCheckHotcolor();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_CHANGEINTENSITY );
    pBtn->EnableWindow( WndEnabled );

    OnCheckChangeintensity();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_COLORSPACE );
    pBtn->EnableWindow( WndEnabled );

    OnCheckColorspace();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_GAMMA );
    pBtn->EnableWindow( WndEnabled );

    OnCheckGamma();

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_HOTCOLOR_TEST );
    pBtn->EnableWindow( WndEnabled );

    OnCheckHotcolorTest();
}

//==========================================================================

void TBlockToolDlg::OnCheckTintMips( void )
{
    CButton* pBtn;

    pBtn = (CButton*)GetDlgItem( IDC_CHECK_TINTMIPS );

    if( pBtn->GetCheck() )
        m_pIMEXObj->EnableTintMips( TRUE );
    else
        m_pIMEXObj->EnableTintMips( FALSE );
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// OUTPUT MESSAGE HANDLERS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void TBlockToolDlg::StatusPrintCB( u32 cbParam, s32 MsgType, const char* pMsg )
{
    TBlockToolDlg* pTHIS = (TBlockToolDlg*)cbParam;

    CHARRANGE      CurSel;
    CHARFORMAT     CharFmt;
    CRichEditCtrl* pREdit;

    CharFmt.cbSize = sizeof(CHARFORMAT);

    // Setup text format for type of message displayed
    switch( MsgType )
    {
        case STATUS::MSG_IMPORTANT:
            CharFmt.dwMask      = CFM_COLOR | CFM_BOLD;
            CharFmt.dwEffects   = CFE_BOLD;
            CharFmt.crTextColor = RGB( 0, 0, 0 );
            break;

        case STATUS::MSG_WARNING:
            CharFmt.dwMask      = CFM_COLOR | CFM_BOLD;
            CharFmt.dwEffects   = 0;
            CharFmt.crTextColor = RGB( 0, 0, 255 );
            break;

        case STATUS::MSG_ERROR:
            CharFmt.dwMask      = CFM_COLOR | CFM_BOLD;
            CharFmt.dwEffects   = CFE_BOLD;
            CharFmt.crTextColor = RGB( 255, 0, 0 );
            break;

        case STATUS::MSG_NORMAL:
        default:
            CharFmt.dwMask      = CFM_COLOR | CFM_BOLD;
            CharFmt.dwEffects   = 0; //CFE_AUTOCOLOR;
            CharFmt.crTextColor = RGB( 0, 0, 0 );
            break;
    }

    pREdit = (CRichEditCtrl*)pTHIS->GetDlgItem( IDC_RICHEDIT_OUTPUT );

    // Save current selection
    pREdit->GetSel( CurSel );

    // Set new selection to end of text
    pREdit->SetSel( -1, -1 );

    // Set text format for message and print it
    pREdit->SetSelectionCharFormat( CharFmt );
    pREdit->ReplaceSel( pMsg );

    // restore the previous selection
    pREdit->SetSel( CurSel );
}

//==========================================================================

void TBlockToolDlg::StatusProgStartCB( u32 cbParam, s32 EndValue )
{
    TBlockToolDlg* pTHIS = (TBlockToolDlg*)cbParam;

    CProgressCtrl* pProgress;

    pProgress = (CProgressCtrl*)pTHIS->GetDlgItem( IDC_PROGRESS_EXPORT );

    // Set the new range of the progress bar and start at zero
    pProgress->SetRange32( 0, (int)EndValue );
    pProgress->SetPos( 0 );
}

//==========================================================================

void TBlockToolDlg::StatusProgStepCB( u32 cbParam, s32 NSteps )
{
    TBlockToolDlg* pTHIS = (TBlockToolDlg*)cbParam;

    CProgressCtrl* pProgress;

    pProgress = (CProgressCtrl*)pTHIS->GetDlgItem( IDC_PROGRESS_EXPORT );

    // Increment progress bar position
    pProgress->OffsetPos( NSteps );
}

//==========================================================================

void TBlockToolDlg::OnDblclkListFiles( NMHDR* pNMHDR, LRESULT* pResult )
{
    TBlockPath  SelectedImageFilename;
    int         FileListID;

    if( m_bThisReady )
    {
        FileListID = GetCurSelectedItem();

        if( FileListID >= 0 )
        {
            m_pFileList->GetItemText( FileListID, LISTHDR_ID_PATH, SelectedImageFilename, sizeof(TBlockPath)-1 );
            
            // If the file seems valid, and not an empty bitmap, then try to open it.
            if( TBLOCKPATH_IsDefault( SelectedImageFilename ) == FALSE )
            {
                if( ShellExecute( NULL, NULL, SelectedImageFilename, NULL, NULL, SW_SHOWDEFAULT ) == (HINSTANCE)ERROR_FILE_NOT_FOUND )
                {
                    STATUS::Print( STATUS::MSG_ERROR, "%s, was not found.\n", SelectedImageFilename );
                }
            }
        }
    }

    *pResult = 0;
}

//==========================================================================

void TBlockToolDlg::OnRclickListFiles(NMHDR* pNMHDR, LRESULT* pResult) 
{
    TBlockPath       SelectedImageFilename;
    int              FileListID;
    SHELLEXECUTEINFO FileInfo;

    if( m_bThisReady )
    {
        FileListID = GetCurSelectedItem();

        if( FileListID >= 0 )
        {
            m_pFileList->GetItemText( FileListID, LISTHDR_ID_PATH, SelectedImageFilename, sizeof(TBlockPath)-1 );
            
            memset( &FileInfo, 0, sizeof( FileInfo ) );
            FileInfo.cbSize   = sizeof( FileInfo );
            FileInfo.fMask    = SEE_MASK_INVOKEIDLIST;
            FileInfo.hwnd     = NULL;
            FileInfo.lpVerb   = "properties";
            FileInfo.lpFile   = SelectedImageFilename;

            // If the file seems valid, and not an empty bitmap, then try to open it.
            if( TBLOCKPATH_IsDefault( SelectedImageFilename ) == FALSE )
            {
                //if( ShellExecute( NULL, "open with", SelectedImageFilename, NULL, NULL, SW_SHOWDEFAULT ) == (HINSTANCE)ERROR_FILE_NOT_FOUND )
                if( ShellExecuteEx( &FileInfo ) == FALSE )
                {
                    STATUS::Print( STATUS::MSG_ERROR, "%s, was not found.\n", SelectedImageFilename );
                }
            }
        }
    }
    
    *pResult = 0;
}

//==========================================================================
