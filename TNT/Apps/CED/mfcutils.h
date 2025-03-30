#ifndef _MFCutils_h_
#define	_MFCutils_h_
// MFCutils.h : header file
//

#include "x_types.hpp"
#include "x_math.hpp"

extern CString MFCU_SetExtension( const char *pFileName, const char *pNewExt ) ;
extern CString MFCU_GetFName( const char *pFileName ) ;

extern CView *MFCU_GetActiveView( ) ;

extern int MFCU_FindListBoxItemByData( CListBox *pList, DWORD Data ) ;
extern int MFCU_FindComboBoxItemByData( CComboBox *pCombo, DWORD Data ) ;

extern void MFCU_UpdateEditControl( CWnd *pWnd, int ID, f32 Value ) ;
extern void MFCU_UpdateEditControl( CWnd *pWnd, int ID, int Value ) ;
extern void MFCU_UpdateEditControl( CWnd *pWnd, int ID, CString String ) ;
extern void MFCU_UpdateEditControl( CEdit &Edit, f32 Value ) ;
extern void MFCU_UpdateEditControl( CEdit &Edit, int Value ) ;
extern void MFCU_UpdateEditControl( CEdit &Edit, CString String ) ;
extern void MFCU_UpdateFromEditControl( CEdit &Edit, f32 &Value ) ;
extern void MFCU_UpdateFromEditControl( CEdit &Edit, s32 &Value ) ;
extern void MFCU_UpdateFromEditControl( CEdit &Edit, CString &String ) ;

extern int MFCU_ListCtrlFind( CListCtrl &List, DWORD Key ) ;
extern int MFCU_ListCtrlGetSelectedIndex( CListCtrl &List ) ;
extern void *MFCU_ListCtrlGetSelected( CListCtrl &List ) ;
extern void *MFCU_ListCtrlGetNext( CListCtrl &List, void *pItem ) ;
extern void *MFCU_ListCtrlGetPrev( CListCtrl &List, void *pItem ) ;

extern bool IntersectRaySphere( vector3  *pRayBase, vector3  *pRayCos, vector3  *pCenter, f32 Radius ) ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_MFCutils_h_
