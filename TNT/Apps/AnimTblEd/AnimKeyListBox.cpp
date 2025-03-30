// AnimKeyListBox.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimKeyListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnimKeyListBox

CAnimKeyListBox::CAnimKeyListBox()
{
	EnableAutomation();
}

CAnimKeyListBox::~CAnimKeyListBox()
{
}

void CAnimKeyListBox::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CListBox::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CAnimKeyListBox, CListBox)
	//{{AFX_MSG_MAP(CAnimKeyListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CAnimKeyListBox, CListBox)
	//{{AFX_DISPATCH_MAP(CAnimKeyListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IAnimKeyListBox to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {D89CD545-386B-11D4-9398-0050DA2C723D}
static const IID IID_IAnimKeyListBox =
{ 0xd89cd545, 0x386b, 0x11d4, { 0x93, 0x98, 0x0, 0x50, 0xda, 0x2c, 0x72, 0x3d } };

BEGIN_INTERFACE_MAP(CAnimKeyListBox, CListBox)
	INTERFACE_PART(CAnimKeyListBox, IID_IAnimKeyListBox, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimKeyListBox message handlers
