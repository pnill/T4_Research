////////////////////////////////////////////////////////////////////////////
//
// TBlockToolDlg.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCKTOOLDLG_H_INCLUDED
#define TBLOCKTOOLDLG_H_INCLUDED

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCK_STDAFX_H_INCLUDED
    #error "You must include StdAfx.h before this file."
#endif

#include "TBlockTexture.h"
#include "StatusOutput.h"
#include "TBlockIMEXObj.h"

#include "Resource.h"


////////////////////////////////////////////////////////////////////////////
// TBlockToolDlg dialog
////////////////////////////////////////////////////////////////////////////

class TBlockToolDlg : public CDialog
{
public:
     TBlockToolDlg( TBlockIMEXObj* pIMEXObj = NULL, CWnd* pParent = NULL );
    ~TBlockToolDlg( void );

    enum { TBS_ARRAY_SIZE = 128 };

    //{{AFX_DATA(TBlockToolDlg)
    enum { IDD = IDD_TBLOCKTOOL_DIALOG };
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(TBlockToolDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    //}}AFX_VIRTUAL

protected:

    struct SettingArray
    {
        TBlockSettings TBSArray[ TBS_ARRAY_SIZE ];
        char           TBSInUse[ TBS_ARRAY_SIZE ];

        SettingArray*  pNextArray;
    };

    // Data Members
    HICON           m_hIcon;

    BOOL            m_ComboUpdateEditBox;
    BOOL            m_bThisReady;
    BOOL            m_bCodeAddRemoveItem;

    TBlockPath      m_AddFolder;
    TBlockPath      m_SaveFolder;
    TBlockPath      m_SaveFilename;
    TBlockPath      m_OpenFolder;
    TBlockPath      m_TxtSaveFolder;
    TBlockPath      m_TxtSaveFilename;
    TBlockPath      m_TempStr;
    TBlockPath      m_TempStr2;

    CListCtrl*      m_pFileList;

    TBlockSettings  m_GlobalSetting;
    TBlockSettings* m_pDisplayedSetting;

    SettingArray*   m_pSettingPool;

    TBlockIMEXObj*  m_pIMEXObj;
    TBlockIMEXObj   m_IMEXObject;

    // Interface helper functions
    void InitSettingsControlsDefData( void );
    BOOL LoadDataFromImport( void );

    void UpdateSettingControls( void );
    void StoreCurSettingFromControls( void );

    void SetExportTarget( TBLOCK_TARGET Target );
    TBLOCK_TARGET GetExportTarget( void );

    void DoExport( const char* pExportFilename );

    // Output window helpers
    void ClearOutputText( void );

    static void StatusPrintCB    ( u32 cbParam, s32 MsgType, const char* pMsg );
    static void StatusProgStartCB( u32 cbParam, s32 EndValue );
    static void StatusProgStepCB ( u32 cbParam, s32 NSteps );

    // List box interface helper functions
    int  GetCurSelectedItem    ( void );
    int  GetListItemCount      ( void );
    void UpdateListHorizSize   ( void );

    void HiLiteItem    ( int Index );
    int  AddItem       ( int Index, const char* ItemText, TBlockSettings* pTBSSrc = NULL );
    int  RemoveItem    ( int Index );
    void RemoveAllItems( void );

    TBlockSettings* AllocItemSetting( void );
    void            FreeItemSetting ( TBlockSettings* pTBS );

    // General helper functions
    void UpdateItemCountDisplay( void );
    void UpdateSelectionDisplay( void );
    void AddFilesViaWindows    ( BOOL bInsert );

    // Generated message map functions
    //{{AFX_MSG(TBlockToolDlg)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonInsert();
    afx_msg void OnButtonRemove();
    afx_msg void OnButtonRemoveall();
    afx_msg void OnButtonInsertEmpty();
    afx_msg void OnButtonSortalpha();
    afx_msg void OnButtonMovedown();
    afx_msg void OnButtonMoveup();
    afx_msg void OnButtonMoveindex();
    afx_msg void OnButtonSwapindex();
    afx_msg void OnButtonTestExport();
    afx_msg void OnButtonRefreshProps();
    afx_msg void OnButtonOpen();
    afx_msg void OnButtonSaveTxt();
    afx_msg void OnButtonExport();
    afx_msg void OnButtonClearOutput();
    afx_msg void OnCheckAlphaByfile();
    afx_msg void OnCheckApplyalpha();
    afx_msg void OnCheckBuildmips();
    afx_msg void OnCheckChangeintensity();
    afx_msg void OnCheckColorspace();
    afx_msg void OnCheckGamma();
    afx_msg void OnSelchangeComboPresetGamma();
    afx_msg void OnChangeEditGammaValue();
    afx_msg void OnCheckHotcolor();
    afx_msg void OnCheckHotcolorTest();
    afx_msg void OnRadioViewglobal();
    afx_msg void OnRadioViewcustom();
    afx_msg void OnCheckUseglobal();
    afx_msg void OnCheckCompress();
    afx_msg void OnCheckIntensityMap();
    afx_msg void OnCheckTintMips();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnDblclkListFiles(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRclickListFiles(NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // TBLOCKTOOLDLG_H_INCLUDED
