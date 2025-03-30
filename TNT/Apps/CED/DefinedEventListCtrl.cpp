// DefinedEventListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DefinedEventListCtrl.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDefinedEventListCtrl

CDefinedEventListCtrl::CDefinedEventListCtrl() : CSortedListCtrl(TRUE, FALSE)
{
	m_pEventList = NULL;
	m_pSkeleton = NULL;
}

CDefinedEventListCtrl::~CDefinedEventListCtrl()
{
}


BEGIN_MESSAGE_MAP(CDefinedEventListCtrl, CSortedListCtrl)
	//{{AFX_MSG_MAP(CDefinedEventListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDefinedEventListCtrl message handlers

void CDefinedEventListCtrl::SetDataPointer(CEventList *pEventList)
{
	m_pEventList = pEventList;
}

void CDefinedEventListCtrl::SetBonePointer(CSkeleton * pSkeleton )
{
	m_pSkeleton = pSkeleton;
}

void CDefinedEventListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	CEvent* pEvent;

	ASSERT( m_pEventList );

	if (pDispInfo->item.mask & LVIF_TEXT) 
	{	
		POSITION EventPOSITION;

		EventPOSITION = m_pEventList->FindIndex( pDispInfo->item.lParam ); 
		if( EventPOSITION )
		{
			pEvent = (CEvent *)m_pEventList->GetAt( EventPOSITION );
			switch (pDispInfo->item.iSubItem) 
			{
				case ELC_EVENT_NAME :
					{
						lstrcpy( pDispInfo->item.pszText, pEvent->GetName() );
						break;
					}
				case ELC_EVENT_RADIUS :
					{
						CString RadiusString;
						float Radius;

						Radius = pEvent->GetRadius();
						RadiusString.Format( "%.2f", Radius );
						lstrcpy( pDispInfo->item.pszText, LPTSTR( LPCTSTR( RadiusString ) ) );
						break;
					}
				case ELC_EVENT_BONE :
					{
						if( m_pSkeleton )
						{
							INT iBone;
							CString BoneName;

							iBone = pEvent->GetBone();
							if( iBone == -1 )
							{
								BoneName = "<no bone>";
							}
							else
							{
								BoneName = m_pSkeleton->GetBoneFromIndex( iBone )->BoneName;
							}
							lstrcpy( pDispInfo->item.pszText, BoneName );
							break;
						}
						else
						{
							break;
						}
					}
			}
		}
	}

	*pResult = 0;
}

int CDefinedEventListCtrl::CompareItems(LPARAM lParam1, LPARAM lParam2)
{
	int nResult = 0;
	CEvent* pEvent1;
	CEvent* pEvent2;
	POSITION EventPOSITION;

	ASSERT( m_pEventList );

	EventPOSITION = m_pEventList->FindIndex( lParam1 ); 
	pEvent1 = (CEvent *)m_pEventList->GetAt( EventPOSITION );
	EventPOSITION = m_pEventList->FindIndex( lParam2 ); 
	pEvent2 = (CEvent *)m_pEventList->GetAt( EventPOSITION );


	switch (GetSortedColumn()) 
	{
		case ELC_EVENT_NAME :
			{
				CString Name1;
				CString Name2;

				Name1 = pEvent1->GetName();
				Name2 = pEvent2->GetName();
				nResult = Name1.Compare( Name2 );
				break;
			}
		case ELC_EVENT_RADIUS :
			{
				float Radius1;
				float Radius2;

				Radius1 = pEvent1->GetRadius();
				Radius2 = pEvent2->GetRadius();
				if( Radius1 < Radius2 )
				{
					nResult = -1;
					break;
				}
				if( Radius1 == Radius2 )
				{
					nResult = 0;
					break;
				}
				if( Radius1 > Radius2 )
				{
					nResult = 1;
					break;
				}
			}
		case ELC_EVENT_BONE :
			{
				if( m_pSkeleton )
				{
					INT iBone1;
					INT iBone2;
					CString BoneName1;
					CString BoneName2;
					
					iBone1 = pEvent1->GetBone();
					iBone2 = pEvent2->GetBone();
					if( iBone1 == -1 )
					{
						BoneName1 = "<no bone>";
					}
					else
					{
						BoneName1 = m_pSkeleton->GetBoneFromIndex( iBone1 )->BoneName;
					}
					if( iBone2 == -1 )
					{
						BoneName2 = "<no bone>";
					}
					else
					{
						BoneName2 = m_pSkeleton->GetBoneFromIndex( iBone2 )->BoneName;
					}
					nResult = BoneName1.Compare( BoneName2 );
					break;
				}
				else
				{
					nResult = 1;
					break;
				}
			}
	}

	return IsAscSorted() ? nResult : -nResult;
}

