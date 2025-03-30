
#include "stdafx.h"

#include "mfcutils.h"

//////////////////////////////////////////////////////////////////////////////
//	Set File Extension in PathName

CString MFCU_SetExtension( const char *pFileName, const char *pNewExt )
{
	CString	String ;
	char	Path[_MAX_PATH] ;
	char	Drive[_MAX_DRIVE] ;
	char	Dir[_MAX_DIR] ;
	char	FName[_MAX_FNAME] ;
	char	Ext[_MAX_EXT] ;

	_splitpath( pFileName, Drive, Dir, FName, Ext ) ;
	_makepath( Path, Drive, Dir, FName, pNewExt ) ;
	String = Path ;
	return String ;
}

CString MFCU_GetFName( const char *pFileName )
{
	CString	String ;
	char	FName[_MAX_FNAME] ;

	_splitpath( pFileName, NULL, NULL, FName, NULL ) ;
	String = FName ;
	return String ;
}

CView *MFCU_GetActiveView( )
{
	CView *pView = NULL ;

	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd ;
	if( pFrame )
	{
		//---	Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame() ;
		if( pChild )
		{
			//---	Get the active view attached to the active MDI child window.
			pView = (CView *) pChild->GetActiveView() ;
		}
	}

	return pView ;
}

int MFCU_FindListBoxItemByData( CListBox *pList, DWORD Data )
{
	int FoundItem = -1 ;

	for( int i = 0 ; i < pList->GetCount() ; i++ )
	{
		if( pList->GetItemData( i ) == Data )
		{
			FoundItem = i ;
			break ;
		}
	}

	return FoundItem ;
}

int MFCU_FindComboBoxItemByData( CComboBox *pCombo, DWORD Data )
{
	int FoundItem = -1 ;

	for( int i = 0 ; i < pCombo->GetCount() ; i++ )
	{
		if( pCombo->GetItemData( i ) == Data )
		{
			FoundItem = i ;
			break ;
		}
	}

	return FoundItem ;
}


void MFCU_UpdateEditControl( CWnd *pWnd, int ID, f32 Value )
{
	CString OldStr, NewStr ;

	pWnd->GetDlgItemText( ID, OldStr ) ;
	if( (atof( OldStr ) != Value) || (OldStr.GetLength() == 0) )
	{
		NewStr.Format( "%.03f", Value ) ;
		pWnd->SetDlgItemText( ID, NewStr ) ;
		pWnd->GetDlgItem( ID )->RedrawWindow() ;
	}
}

void MFCU_UpdateEditControl( CWnd *pWnd, int ID, int Value )
{
	CString OldStr, NewStr ;

	pWnd->GetDlgItemText( ID, OldStr ) ;
	if( (atoi( OldStr ) != Value) || (OldStr.GetLength() == 0) )
	{
		NewStr.Format( "%d", Value ) ;
		pWnd->SetDlgItemText( ID, NewStr ) ;
		pWnd->GetDlgItem( ID )->RedrawWindow() ;
	}
}

void MFCU_UpdateEditControl( CWnd *pWnd, int ID, CString String )
{
	CString OldStr ;

	pWnd->GetDlgItemText( ID, OldStr ) ;
	if( OldStr != String )
	{
		pWnd->SetDlgItemText( ID, String ) ;
		pWnd->GetDlgItem( ID )->RedrawWindow() ;
	}
}

void MFCU_UpdateEditControl( CEdit &Edit, f32 Value )
{
	CString OldStr, NewStr ;

	Edit.GetWindowText( OldStr ) ;
	if( (atof( OldStr ) != Value) || (OldStr.GetLength() == 0) )
	{
		NewStr.Format( "%.03f", Value ) ;
		Edit.SetWindowText( NewStr ) ;
		Edit.RedrawWindow( ) ;
	}
}

void MFCU_UpdateEditControl( CEdit &Edit, int Value )
{
	CString OldStr, NewStr ;

	Edit.GetWindowText( OldStr ) ;
	if( (atoi( OldStr ) != Value) || (OldStr.GetLength() == 0) )
	{
		NewStr.Format( "%d", Value ) ;
		Edit.SetWindowText( NewStr ) ;
		Edit.RedrawWindow( ) ;
	}
}

void MFCU_UpdateEditControl( CEdit &Edit, CString String )
{
	CString OldStr ;

	Edit.GetWindowText( OldStr ) ;
	if( OldStr != String )
	{
		Edit.SetWindowText( String ) ;
		Edit.RedrawWindow() ;
	}
}

void MFCU_UpdateFromEditControl( CEdit &Edit, f32 &Value )
{
	CString Str ;

	Edit.GetWindowText( Str ) ;
	if( (atof( Str ) != Value) )
	{
		Value = (f32)atof( Str ) ;
        MFCU_UpdateEditControl( Edit, Value ) ;
	}
}

void MFCU_UpdateFromEditControl( CEdit &Edit, s32 &Value )
{
	CString Str ;

	Edit.GetWindowText( Str ) ;
	if( (atoi( Str ) != Value) )
	{
		Value = atoi( Str ) ;
        MFCU_UpdateEditControl( Edit, Value ) ;
	}
}

void MFCU_UpdateFromEditControl( CEdit &Edit, CString &String )
{
	CString Str ;

	Edit.GetWindowText( Str ) ;
	if( Str != String )
	{
        String = Str ;
        MFCU_UpdateEditControl( Edit, String ) ;
	}
}


int MFCU_ListCtrlFind( CListCtrl &List, DWORD Key )
{
	int Found = -1 ;
	for( int i = 0 ; i < List.GetItemCount() ; i++ )
	{
		if( List.GetItemData( i ) == Key )
		{
			Found = i ;
			break ;
		}
	}
	return Found ;
}

int MFCU_ListCtrlGetSelectedIndex( CListCtrl &List )
{
	int Found = -1 ;
	for( int i = 0 ; i < List.GetItemCount() ; i++ )
	{
		if( List.GetItemState( i, LVIS_SELECTED ) )
		{
			Found = i ;
			break ;
		}
	}
	return Found ;
}

void *MFCU_ListCtrlGetSelected( CListCtrl &List )
{
	void *pFound = NULL ;
	for( int i = 0 ; i < List.GetItemCount() ; i++ )
	{
		if( List.GetItemState( i, LVIS_SELECTED ) )
		{
			pFound = (void*)List.GetItemData( i ) ;
			break ;
		}
	}
	return pFound ;
}

void *MFCU_ListCtrlGetNext( CListCtrl &List, void *pItem )
{
	void	*pFound = NULL ;
	LV_FINDINFO	fi ;

	fi.flags = LVFI_PARAM ;
	fi.lParam = (DWORD)pItem ;

	int Index = List.FindItem( &fi, -1 ) ;
	if( Index != -1 )
	{
		Index++ ;
		if( Index < List.GetItemCount() )
		{
			pFound = (void*)List.GetItemData( Index ) ;
		}
	}

	return pFound ;
}

void *MFCU_ListCtrlGetPrev( CListCtrl &List, void *pItem )
{
	void	*pFound = NULL ;
	LV_FINDINFO	fi ;

	fi.flags = LVFI_PARAM ;
	fi.lParam = (DWORD)pItem ;

	int Index = List.FindItem( &fi, -1 ) ;
	if( Index != -1 )
	{
		Index-- ;
		if( Index >= 0 )
		{
			pFound = (void*)List.GetItemData( Index ) ;
		}
	}

	return pFound ;
}


bool IntersectRaySphere( vector3  *pRayBase, vector3  *pRayCos, vector3  *pCenter, f32 Radius )
{
	bool		hit ;				// True if ray intersects sphere
	vector3 	d ;					// Ray base to sphere center
	double		bsq, u, disc ;
//	double		root ;

	d = *pRayBase - *pCenter;
//	V3_Sub( &d, pRayBase, pCenter ) ;
	bsq  = d.X*pRayCos->X + d.Y*pRayCos->Y + d.Z*pRayCos->Z ;
	u    = d.X*d.X + d.Y*d.Y + d.Z*d.Z - Radius*Radius ;
	disc = bsq*bsq - u ;
  
	hit  = (disc >= 0.0) ;

/*
	if  (hit) { 					// If ray hits sphere
	    root  =  sqrt(disc);
	    *rin  = -bsq - root;		//    entering distance
	    *rout = -bsq + root;		//    leaving distance
	}
*/
	return hit ;
}
