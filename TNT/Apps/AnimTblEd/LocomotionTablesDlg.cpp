// LocomotionTablesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"
#include "LocomotionTablesDlg.h"
#include "AnimIDSelectionDlg.h"
#include "Math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	ABS(a)			(((a)<0)?-(a):(a))

#define	BRUSH_WIDTH		24
#define	BRUSH_HEIGHT	13
#define	BRUSH_SPACE		0

#define	DEG_TO_RAD(d)	((d)*3.14159f/180.0f)
#define	RAD_TO_DEG(r)	((r)*180.0f/3.14159f)

#define	BASIC_COLOR			RGB(0x00,0x00,0x00)	// BLACK
#define	SELECT_COLOR		RGB(0xFC,0xFC,0x24)	// BRIGHT YELLOW
#define	SHADE_COLOR			RGB(0x80,0x80,0x80)	// GRAY (ANIM_NONE)
#define	BACKGROUND_COLOR	RGB(0x80,0x80,0x80)	// GRAY (ANIM_NONE) //RGB(0xFF,0xFF,0xFF)	// WHITE

#define	NUM_ANIM_COLORS		16
#define	NUM_HATCH_PATTERS	7

#define	TABLE_PHASE_SHIFT	3.14159f

// difference between the arc radius and its overlap radius when the overlap will be flood filled rather than repeatedly arced.
#define	OVERLAP_FILLAREARADIUS		6

unsigned long AnimColors[NUM_ANIM_COLORS] =
{
	RGB(0x00,0x00,0xFC),	// BLUE
	RGB(0x24,0xFC,0x24),	// GREEN
	RGB(0x00,0xFC,0xFC),	// CYAN
	RGB(0xFC,0x14,0x14),	// RED
	RGB(0xB0,0x00,0xFC),	// MAGENTA
	RGB(0x70,0x48,0x00),	// BROWN
	RGB(0xFF,0xFF,0xFF),	// WHITE
	RGB(0x00,0x00,0x70),	// LT. BLUE
	RGB(0x00,0x70,0x00),	// LT. GREEN
	RGB(0x00,0x70,0x70),	// LT. CYAN
	RGB(0x70,0x00,0x00),	// LT. RED
	RGB(0x70,0x00,0x70),	// LT. MAGENTA
	RGB(0x70,0x70,0x14),	// DIRTY YELLOW
	RGB(0x00,0x00,0x34),	// DARK BLUE
	RGB(0x00,0x34,0x00),	// DARK GREEN
	RGB(0x34,0x00,0x00),	// DARK RED
};

/////////////////////////////////////////////////////////////////////////////
// StringToFloat

float StringToFloat( CString& Text )
{
	CString DecText;
	int PointIndex;
	int i;
	float Decimal;
	float Whole;
	int Neg;

	Text.TrimLeft();

	//---	store the sign of the value
	if (Text[0] == '-')
	{
		Neg = TRUE;
		Text = Text.Right( Text.GetLength() - 1 );
	}
	else
		Neg = FALSE;

	//---	if the string has a decimal point, calculate the decimal value
	PointIndex = Text.Find( '.' );
	if (PointIndex != -1)
	{
		DecText = Text.Right( Text.GetLength() - PointIndex - 1 );

		Decimal = 0.0f;
		for( i=DecText.GetLength()-1; i>=0; i-- )
		{
			if ((DecText[i] >= '0') && (DecText[i] <= '9'))
				Decimal = (Decimal / 10.0f) + DecText[i] - '0';
		}
		Decimal /= 10.0f;

		Text = Text.Left( PointIndex );
	}

	//---	calculate the whole value
	Whole = 0.0f;
	for( i=0; i<Text.GetLength(); i++ )
	{
		if ((Text[i] >= '0') && (Text[i] <= '9'))
			Whole = (Whole * 10.0f) + Text[i] - '0';
	}

	Whole += Decimal;
	if (Neg)
		Whole = -Whole;

	return Whole;
}


#define	FLOAT_STEP_SIZE	0.01f

void CSelectValue::Inc( float Steps )
{
	float Value;

	switch( m_pSelected->GetType() )
	{
	case LATOBJECT_TYPE_TABLE:		Value = ((CLocoAnimTable*)m_pSelected)->m_CenterNSR;	break;
	case LATOBJECT_TYPE_ARC:		Value = ((CLocoArcData*)m_pSelected)->m_Angle;			break;
	case LATOBJECT_TYPE_ANIMNSR:	Value = ((CLocoAnimNSR*)m_pSelected)->m_NSR;			break;
	default: ASSERT(0);
	}

	//---	update the value
	Value += Steps;
	if (Value < *m_pMin) Value = *m_pMin;
	if (Value > *m_pMax) Value = *m_pMax;

	switch( m_pSelected->GetType() )
	{
	case LATOBJECT_TYPE_TABLE:		((CLocoAnimTable*)m_pSelected)->m_CenterNSR = Value;	break;
	case LATOBJECT_TYPE_ARC:		((CLocoArcData*)m_pSelected)->m_Angle = Value;			break;
	case LATOBJECT_TYPE_ANIMNSR:	((CLocoAnimNSR*)m_pSelected)->m_NSR = Value;			break;
	default: ASSERT(0);
	}
}

void CSelectValue::Set( float Value )
{
	//---	update the value
	if (Value < *m_pMin) Value = *m_pMin;
	if (Value > *m_pMax) Value = *m_pMax;

	switch( m_pSelected->GetType() )
	{
	case LATOBJECT_TYPE_TABLE:		((CLocoAnimTable*)m_pSelected)->m_CenterNSR = Value;	break;
	case LATOBJECT_TYPE_ARC:		((CLocoArcData*)m_pSelected)->m_Angle = Value;			break;
	case LATOBJECT_TYPE_ANIMNSR:	((CLocoAnimNSR*)m_pSelected)->m_NSR = Value;			break;
	default: ASSERT(0);
	}
}

float CSelectValue::Get( void )
{
	switch( m_pSelected->GetType() )
	{
	case LATOBJECT_TYPE_TABLE:		return (float) ((CLocoAnimTable*)m_pSelected)->m_CenterNSR;
	case LATOBJECT_TYPE_ARC:		return (float) ((CLocoArcData*)m_pSelected)->m_Angle;
	case LATOBJECT_TYPE_ANIMNSR:	return (float) ((CLocoAnimNSR*)m_pSelected)->m_NSR;
	default: ASSERT(0);
	}

	return 0.0f;
}

void CSelectValue::SetDisplay( CDialog* pDialog )
{
	//---	change the displayed value
	CString str;
	float Value;

	switch( m_pSelected->GetType() )
	{
	case LATOBJECT_TYPE_TABLE:		Value = ((CLocoAnimTable*)m_pSelected)->m_CenterNSR;	break;
	case LATOBJECT_TYPE_ARC:		Value = ((CLocoArcData*)m_pSelected)->m_Angle;			break;
	case LATOBJECT_TYPE_ANIMNSR:	Value = ((CLocoAnimNSR*)m_pSelected)->m_NSR;			break;
	default: ASSERT(0);
	}

	if ((m_ValueDisplayConversion == RAD_TO_DEG(1.0f)) && !((CLocomotionTablesDlg*) pDialog)->DisplayRadian())
		Value = RAD_TO_DEG(Value);
	if		(Value <= 1.0f)		str.Format( "%6.4f", Value );
	else if (Value <= 10.0f)	str.Format( "%6.3f", Value );
	else						str.Format( "%6.2f", Value );
	pDialog->SetDlgItemText( IDC_VALUE, str );
}

CSelectValue::CSelectValue( CLATObject* pSelected, float* pMin, float* pMax, float ValueDisplayConversion )
{
	m_pSelected = pSelected;
	m_pMin = pMin;
	m_pMax = pMax;
	m_ValueDisplayConversion = ValueDisplayConversion;
}

void CSelectAnim::Inc( float Steps )
{
	int Anim;

	switch( m_pSelected->GetType() )
	{
	case LATOBJECT_TYPE_TABLE:		Anim = ((CLocoAnimTable*)m_pSelected)->m_CenterAnim;	break;
	case LATOBJECT_TYPE_ARC:		ASSERT(0);												break;
	case LATOBJECT_TYPE_ANIMNSR:	Anim = ((CLocoAnimNSR*)m_pSelected)->m_Anim;			break;
	default: ASSERT(0);
	}

	//---	update the value
	Anim += (int)Steps;
	if (Anim < m_Min) Anim = m_Min;
	if (Anim > m_Max) Anim = m_Max;

	switch( m_pSelected->GetType() )
	{
	case LATOBJECT_TYPE_TABLE:		((CLocoAnimTable*)m_pSelected)->m_CenterAnim = Anim;	break;
	case LATOBJECT_TYPE_ARC:		ASSERT(0);												break;
	case LATOBJECT_TYPE_ANIMNSR:	((CLocoAnimNSR*)m_pSelected)->m_Anim = Anim;			break;
	default: ASSERT(0);
	}
}

void CSelectAnim::Set( float Value )
{
	int Anim;

	//---	update the value
	Anim = (int) Value;
	if (Anim < m_Min) Anim = m_Min;
	if (Anim > m_Max) Anim = m_Max;

	switch( m_pSelected->GetType() )
	{
	case LATOBJECT_TYPE_TABLE:		((CLocoAnimTable*)m_pSelected)->m_CenterAnim = Anim;	break;
	case LATOBJECT_TYPE_ARC:		ASSERT(0);												break;
	case LATOBJECT_TYPE_ANIMNSR:	((CLocoAnimNSR*)m_pSelected)->m_Anim = Anim;			break;
	default: ASSERT(0);
	}
}

float CSelectAnim::Get( void )
{
	switch( m_pSelected->GetType() )
	{
	case LATOBJECT_TYPE_TABLE:		return (float)((CLocoAnimTable*)m_pSelected)->m_CenterAnim;
	case LATOBJECT_TYPE_ARC:		ASSERT(0);
	case LATOBJECT_TYPE_ANIMNSR:	return (float) ((CLocoAnimNSR*)m_pSelected)->m_Anim;;
	default: ASSERT(0);
	}

	return 0.0f;
}

void CSelectAnim::SetDisplay( CDialog* pDialog )
{
	int Anim;

	switch( m_pSelected->GetType() )
	{
	case LATOBJECT_TYPE_TABLE:		Anim = ((CLocoAnimTable*)m_pSelected)->m_CenterAnim;	break;
	case LATOBJECT_TYPE_ARC:		ASSERT(0);												break;
	case LATOBJECT_TYPE_ANIMNSR:	Anim = ((CLocoAnimNSR*)m_pSelected)->m_Anim;			break;
	default: ASSERT(0);
	}

	CListBox* pUsedAnims = ((CListBox*) pDialog->GetDlgItem( IDC_USEDANIMS ));
	int OldTopIndex = pUsedAnims->GetTopIndex();
	pUsedAnims->SetCurSel( Anim );
	if (OldTopIndex != pUsedAnims->GetTopIndex())	
		((CLocomotionTablesDlg*)pDialog)->ScrollUsedAnims();
}

CSelectAnim::CSelectAnim( CLATObject* pSelected, int Min, int Max )
{
	m_pSelected = pSelected;
	m_Min = Min;
	m_Max = Max;
}



/////////////////////////////////////////////////////////////////////////////
// CLocomotionTablesDlg dialog

CLocomotionTablesDlg::CLocomotionTablesDlg(CAnimTblEdDoc* pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CLocomotionTablesDlg::IDD, pParent)
{
	m_MouseSensitivity = 2;

	m_pCurLocoTable = NULL;
	m_pCurArc = NULL;
	m_pCurZone = NULL;
	m_pAnimBrushes = NULL;

	m_Zero	= 0.0f;
	m_One	= 1.0f;
	m_2Pi	= 2*3.14159265358979f;

	m_pDoc = pDoc;

	//{{AFX_DATA_INIT(CLocomotionTablesDlg)
	//}}AFX_DATA_INIT
}


CLocomotionTablesDlg::~CLocomotionTablesDlg(void)
{
	ClearAllSelectedItems();

	if (m_pAnimBrushes)
		delete[]m_pAnimBrushes;
}


void CLocomotionTablesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocomotionTablesDlg)
	DDX_Control(pDX, IDC_TABLETYPE, m_TableTypeListBoxCtrl);
	DDX_Control(pDX, IDC_RADIAN, m_Radian);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocomotionTablesDlg, CDialog)
	//{{AFX_MSG_MAP(CLocomotionTablesDlg)
	ON_WM_PAINT()
	ON_WM_CANCELMODE()
	ON_LBN_SELCHANGE(IDC_LOCOMOTIONTABLES, OnSelchangeLocomotiontables)
	ON_LBN_SELCANCEL(IDC_LOCOMOTIONTABLES, OnSelcancelLocomotiontables)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_PHASE, OnPhase)
	ON_BN_CLICKED(IDC_SIDEFOREWARD, OnSideforeward)
	ON_BN_CLICKED(IDC_POINTFOREWARD, OnPointforeward)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PHASESHIFTSPIN, OnDeltaposPhaseshiftspin)
	ON_LBN_SETFOCUS(IDC_LOCOMOTIONTABLES, OnSetfocusLocomotiontables)
	ON_LBN_KILLFOCUS(IDC_LOCOMOTIONTABLES, OnKillfocusLocomotiontables)
	ON_EN_KILLFOCUS(IDC_PHASESHIFT, OnKillfocusPhaseshift)
	ON_EN_KILLFOCUS(IDC_NAME, OnKillfocusName)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	ON_EN_KILLFOCUS(IDC_MAXNSR, OnKillfocusMaxnsr)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MAXNSRSPIN, OnDeltaposMaxnsrspin)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_ADDANIM, OnAddanim)
	ON_BN_CLICKED(IDC_REMOVEANIM, OnRemoveanim)
	ON_WM_MOUSEWHEEL()
	ON_LBN_SELCHANGE(IDC_USEDANIMS, OnSelchangeUsedanims)
	ON_BN_CLICKED(IDC_PLUS, OnPlus)
	ON_BN_CLICKED(IDC_MINUS, OnMinus)
	ON_BN_CLICKED(IDC_EQUAL, OnEqual)
	ON_EN_CHANGE(IDC_VALUE, OnChangeValue)
	ON_NOTIFY(UDN_DELTAPOS, IDC_VALUESPIN, OnDeltaposValuespin)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_RADIAN, OnRadian)
	ON_EN_CHANGE(IDC_OVERLAP, OnChangeOverlap)
	ON_EN_KILLFOCUS(IDC_OVERLAP, OnKillfocusOverlap)
	ON_NOTIFY(UDN_DELTAPOS, IDC_OVERLAPSPIN, OnDeltaposOverlapspin)
	ON_EN_KILLFOCUS(IDC_VALUE, OnKillfocusValue)
	ON_BN_CLICKED(IDC_CHANGEANIM, OnChangeanim)
	ON_BN_CLICKED(IDC_REORIENTANIMCHANGE, OnReorientanimchange)
	ON_LBN_DBLCLK(IDC_USEDANIMS, OnDblclkUsedanims)
	ON_CBN_SELCHANGE(IDC_TABLETYPE, OnSelchangeTabletype)
	ON_BN_CLICKED(IDC_FIXEDONFIELD, OnFixedonfield)
	ON_BN_CLICKED(IDC_FACEDIR, OnFacedir)
	ON_CBN_KILLFOCUS(IDC_CALLBACK, OnKillfocusCallback)
	ON_EN_CHANGE(IDC_OVERLAPARC, OnChangeOverlaparc)
	ON_EN_KILLFOCUS(IDC_OVERLAPARC, OnKillfocusOverlaparc)
	ON_NOTIFY(UDN_DELTAPOS, IDC_OVERLAPARCSPIN, OnDeltaposOverlaparcspin)
	ON_BN_CLICKED(IDC_LIMITEDNSR, OnLimitednsr)
	ON_BN_CLICKED(IDC_DONOTEXPORT, OnDonotexport)
	ON_CBN_SELCHANGE(IDC_STARTANGLE, OnSelchangeStartangle)
	ON_CBN_SELCHANGE(IDC_ENDANGLE, OnSelchangeEndangle)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocomotionTablesDlg message handlers

BOOL CLocomotionTablesDlg::OnInitDialog() 
{
	CListBox*	pLocoTablesList;
	CListBox*	pUsedAnimsList;
	CEdit*		pName;
	CRect		Rect;
	CString		Name;
	int			i, Index;

	CDialog::OnInitDialog();

	m_DrawAnimKeyOnly = FALSE;
	m_DrawTableOnly = FALSE;

	//---	fill the display list with all the table names
	CListBox* pList = (CListBox*) GetDlgItem( IDC_LOCOMOTIONTABLES );

	for (i=0; i<m_LocomotionTables.GetCount(); i++)
	{
		Name = m_LocomotionTables.GetTable( i )->m_Name;
		pList->AddString( Name );
		Index = pList->FindStringExact( -1, Name );
		pList->SetItemData( Index, (unsigned int)m_LocomotionTables.GetTable( i ) );
	}
	
	//---	get pointers to key dialog elements
	pName = (CEdit*) GetDlgItem( IDC_NAME );
	pLocoTablesList = (CListBox*) GetDlgItem( IDC_LOCOMOTIONTABLES );
	pUsedAnimsList = (CListBox*) GetDlgItem( IDC_USEDANIMS );	

	//---	determine the rectangle of the locomotion table display
	GetDlgItem( IDC_TABLERECT )->GetWindowRect( &m_TableRect );
	ScreenToClient( &m_TableRect );

	//---	determine the arc center
	m_ArcCenter.x = m_TableRect.left + m_TableRect.Width() / 2;
	m_ArcCenter.y = m_TableRect.top + m_TableRect.Height() / 2;

	//---	determine the arc radius
	m_ArcRadius = (m_TableRect.Height() < m_TableRect.Width()) ? m_TableRect.Height() : m_TableRect.Width();
	m_ArcRadius /= 2;

	//---	make the table rect just a bit bigger to make sure it covers all the thick lines for redrawing
	m_TableRect.left -= 1;
	m_TableRect.top -= 1;
	m_TableRect.right += 1;
	m_TableRect.bottom += 1;

	//---	create the basic brush for the table
	m_BlackBrush.CreateSolidBrush( 0 );
	m_BackgroundBrush.CreateSolidBrush( BACKGROUND_COLOR );
	m_ShadeBrush.CreateSolidBrush( SHADE_COLOR );
	m_SelectBrush.CreateSolidBrush( SELECT_COLOR );
	m_BasicPen.CreatePen( PS_SOLID | PS_COSMETIC, 2, BASIC_COLOR );
	m_SelectPen.CreatePen( PS_SOLID | PS_COSMETIC, 2, SELECT_COLOR );
	m_BrushKeyPen.CreatePen( PS_SOLID | PS_COSMETIC, 1, BASIC_COLOR );
	m_DottedLinePen.CreatePen( PS_DASH | PS_COSMETIC, 1, BASIC_COLOR );

	//---	the scroll bar for the used aniamtion list box had to be implemented at the dialog level so that it could also
	//		be used to set the animation color key.  We need to determine the size of the used animation scroll bar hear
	//		as well as the size and location of the scroll bar when it is needed.
	GetDlgItem( IDC_USEDANIMS )->GetWindowRect( &m_UsedAnimsList_NoScroll );
	this->ScreenToClient( &m_UsedAnimsList_NoScroll );
	m_UsedAnimsList_Scroll = m_UsedAnimsList_NoScroll;
	m_UsedAnimsList_Scroll.right -= 14;
	m_UsedAnimsList_ScrollBar = m_UsedAnimsList_NoScroll;
	m_UsedAnimsList_ScrollBar.left = m_UsedAnimsList_Scroll.right;

	//---	get the animation key brush rectangle
	CListBox* pUsedAnims = (CListBox*) GetDlgItem( IDC_USEDANIMS );
	pUsedAnims->GetWindowRect( &m_AnimBrushKeyRect );
	ScreenToClient( &m_AnimBrushKeyRect );
	m_AnimBrushKeyRect.top += 1;
	m_AnimBrushKeyRect.right = m_AnimBrushKeyRect.left - BRUSH_SPACE;
	m_AnimBrushKeyRect.left = m_AnimBrushKeyRect.right - BRUSH_WIDTH;

	//---	set the displayed table
	if (m_LocomotionTables.GetCount())	pLocoTablesList->SetCurSel( 0 );
	SetDisplayToTable( ((CListBox*) GetDlgItem( IDC_LOCOMOTIONTABLES ))->GetCurSel() );


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLocomotionTablesDlg::EnableDisplay( int EnableBool )
{
	GetDlgItem( IDC_NAMETITLE			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_NAME				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_NUMARCSTITLE		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_NUMARCS				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_SELECTEDARCTITLE	)->EnableWindow( EnableBool );
	GetDlgItem( IDC_SELECTEDARC			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_NUMZONESTITLE		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_NUMZONES			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_SELECTEDZONETITLE	)->EnableWindow( EnableBool );
	GetDlgItem( IDC_SELECTEDZONE		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_PLUS				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_MINUS				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_EQUAL				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_ORIENTATION			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_POINTFOREWARD		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_SIDEFOREWARD		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_PHASE				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_PHASESHIFT			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_PHASESHIFTSPIN		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_USEDANIMSTITLE		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_USEDANIMS			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_ADDANIM				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_REMOVEANIM			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_CHANGEANIM			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_MAXNSRTITLE			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_MAXNSR				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_MAXNSRSPIN			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_OVERLAPTITLE		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_OVERLAP				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_OVERLAPSPIN			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_VALUETITLE			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_VALUE				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_VALUESPIN			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_LINEONLY			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_AREAONLY			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_NSRINFO				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_REORIENTANIM		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_REORIENTANIMTITLE	)->EnableWindow( EnableBool );
	GetDlgItem( IDC_REORIENTANIMCHANGE	)->EnableWindow( EnableBool );
	GetDlgItem( IDC_TABLETYPE			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_TABLETYPETITLE		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_CALLBACK			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_LOCOCALLBACKTITLE	)->EnableWindow( EnableBool );
	GetDlgItem( IDC_LIMITEDNSR			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_DONOTEXPORT			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_OVERLAPARC			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_OVERLAPARCTITLE		)->EnableWindow( EnableBool );

#ifdef OLDWAY
	GetDlgItem( IDC_FACEDIR				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_FIXEDONFIELD		)->EnableWindow( EnableBool );
#else
	GetDlgItem( IDC_STARTANGLETITLE		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_STARTANGLE			)->EnableWindow( EnableBool );
	GetDlgItem( IDC_ENDANGLETITLE		)->EnableWindow( EnableBool );
	GetDlgItem( IDC_ENDANGLE			)->EnableWindow( EnableBool );
#endif

	GetDlgItem( IDC_COPY				)->EnableWindow( EnableBool );
	GetDlgItem( IDC_REMOVE				)->EnableWindow( EnableBool );
}

void CLocomotionTablesDlg::ScrollUsedAnims( void )
{
	CListBox* pUsedAnims = (CListBox*) GetDlgItem( IDC_USEDANIMS );
	SetVScroll( pUsedAnims->GetTopIndex(), (CScrollBar*) GetDlgItem( IDC_USEDANIMSBAR ));
}

void CLocomotionTablesDlg::SetDisplayToTable(int Table)
{
	CString Text;

	if (Table == -1)
	{
		RedrawTable();
		RedrawAnimKey();

		m_pCurLocoTable = NULL;
		SetSelected( -1, -1, -1, FALSE, FALSE );
		EnableDisplay( FALSE );
		return;
	}

	EnableDisplay( TRUE );
	
	CListBox* pList = (CListBox*) GetDlgItem( IDC_LOCOMOTIONTABLES );
	pList->SetCurSel( Table );

	if( m_pCurLocoTable != (CLocoAnimTable*) pList->GetItemData( Table ) )
	{
		m_pCurLocoTable = (CLocoAnimTable*) pList->GetItemData( Table );

		RedrawTable();
		RedrawAnimKey();

		SetSelected( -1, -1, -1, FALSE, FALSE );

		SetDlgItemText( IDC_NAME, m_pCurLocoTable->m_Name );
		m_TableTypeListBoxCtrl.SetCurSel( m_pCurLocoTable->m_Type );
		SetDlgItemInt( IDC_NUMARCSEDIT, m_pCurLocoTable->m_ArcDataList.GetCount() );
		Text.Format( "%5.3f", m_pCurLocoTable->m_MaxNSR );
		SetDlgItemText( IDC_MAXNSR, Text );
		Text.Format( "%6.3f", m_pCurLocoTable->m_OverlapNSR );
		SetDlgItemText( IDC_OVERLAP, Text );
		Text.Format( "%6.3f", m_pCurLocoTable->m_OverlapArc );
		SetDlgItemText( IDC_OVERLAPARC, Text );

#ifdef OLDWAY
		if (m_pCurLocoTable->m_Type != TABLETYPE_LOCOMOTION)
		{
			((CButton*)GetDlgItem( IDC_FIXEDONFIELD ))->EnableWindow( FALSE );
			((CButton*)GetDlgItem( IDC_FIXEDONFIELD ))->SetCheck( FALSE );

			((CButton*)GetDlgItem( IDC_FACEDIR ))->EnableWindow( TRUE );
			((CButton*)GetDlgItem( IDC_FACEDIR ))->SetCheck( m_pCurLocoTable->m_Flags & TABLEFLAG_FACEDIR ? TRUE : FALSE );
		}
		else
		{
			((CButton*)GetDlgItem( IDC_FIXEDONFIELD ))->EnableWindow( TRUE );
			((CButton*)GetDlgItem( IDC_FIXEDONFIELD ))->SetCheck( m_pCurLocoTable->m_Flags & TABLEFLAG_FIXED ? TRUE : FALSE );

			((CButton*)GetDlgItem( IDC_FACEDIR ))->EnableWindow( FALSE );
			((CButton*)GetDlgItem( IDC_FACEDIR ))->SetCheck( FALSE );
		}
#else
		SetIndexAngles();
#endif
		((CButton*)GetDlgItem( IDC_LIMITEDNSR ))->SetCheck( m_pCurLocoTable->m_Flags & TABLEFLAG_LIMITNSR ? TRUE : FALSE );
		((CButton*)GetDlgItem( IDC_DONOTEXPORT ))->SetCheck( m_pCurLocoTable->m_bDoNotExport ? TRUE : FALSE );

		SetDlgItemText( IDC_REORIENTANIM, m_pCurLocoTable->m_ReorientAnim );

		SetSelectedCallback();
		SetOrientation();
		SetUsedAnimations();
	}

	//---	if this table is not exported, turn off everything now
	if( m_pCurLocoTable->m_bDoNotExport )
	{
		EnableDisplay( FALSE );
		GetDlgItem( IDC_DONOTEXPORT )->EnableWindow( TRUE );
	}
}

///////////////////////////////////////////////////////////////////////////////
void CLocomotionTablesDlg::SetIndexAngles( void )
{
	CComboBox* pStartAngleOptions = (CComboBox*)GetDlgItem( IDC_STARTANGLE );
	CComboBox* pEndAngleOptions = (CComboBox*)GetDlgItem( IDC_ENDANGLE );

	pStartAngleOptions->ResetContent();
	pStartAngleOptions->AddString( "<fixed>" );
	pStartAngleOptions->AddString( "CurFaceDir" );
	pStartAngleOptions->AddString( "CurMoveDir" );
	pStartAngleOptions->AddString( "DestFaceDir" );
	pStartAngleOptions->AddString( "DestMoveDir" );

	pEndAngleOptions->ResetContent();
	pEndAngleOptions->AddString( "CurFaceDir" );
	pEndAngleOptions->AddString( "CurMoveDir" );
	pEndAngleOptions->AddString( "DestFaceDir" );
	pEndAngleOptions->AddString( "DestMoveDir" );

	if( m_pCurLocoTable )
	{
		pStartAngleOptions->SetCurSel( m_pCurLocoTable->m_IndexAngleStart );
		pEndAngleOptions->SetCurSel( m_pCurLocoTable->m_IndexAngleEnd );
	}
	else
	{
		pStartAngleOptions->SetCurSel( -1 );
		pEndAngleOptions->SetCurSel( -1 );
	}
}

void CLocomotionTablesDlg::SetSelectedCallback( void )
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem( IDC_CALLBACK );
	int i;

	//---	add the callback functions to the combo box
	CStringArray StringArray;
	m_LocomotionTables.FillCallbackStringArray( StringArray );
	pComboBox->ResetContent();
	for( i=0; i<StringArray.GetSize(); i++ )
		pComboBox->AddString( StringArray[i] );

	//---	set the callback function name and combo box options
	SetDlgItemText( IDC_CALLBACK, m_pCurLocoTable->m_Callback );
}

void CLocomotionTablesDlg::SetOrientation( void )
{
	RedrawTable();

	if (m_pCurLocoTable->m_Orientation == ORIENTATION_POINTFOREWARD)
	{
		((CButton*) GetDlgItem( IDC_POINTFOREWARD ))->SetCheck( TRUE );
		((CButton*) GetDlgItem( IDC_SIDEFOREWARD ))->SetCheck( FALSE );
		((CButton*) GetDlgItem( IDC_PHASE ))->SetCheck( FALSE );
		GetDlgItem( IDC_PHASESHIFT )->EnableWindow( FALSE );
		GetDlgItem( IDC_PHASESHIFTSPIN )->EnableWindow( FALSE );
	}
	else if (m_pCurLocoTable->m_Orientation == ORIENTATION_SIDEFOREWARD)
	{
		((CButton*) GetDlgItem( IDC_POINTFOREWARD ))->SetCheck( FALSE );
		((CButton*) GetDlgItem( IDC_SIDEFOREWARD ))->SetCheck( TRUE );
		((CButton*) GetDlgItem( IDC_PHASE ))->SetCheck( FALSE );
		GetDlgItem( IDC_PHASESHIFT )->EnableWindow( FALSE );
		GetDlgItem( IDC_PHASESHIFTSPIN )->EnableWindow( FALSE );
	}
	else
	{
		((CButton*) GetDlgItem( IDC_POINTFOREWARD ))->SetCheck( FALSE );
		((CButton*) GetDlgItem( IDC_SIDEFOREWARD ))->SetCheck( FALSE );
		((CButton*) GetDlgItem( IDC_PHASE ))->SetCheck( TRUE );
		GetDlgItem( IDC_PHASESHIFT )->EnableWindow( TRUE );
		GetDlgItem( IDC_PHASESHIFTSPIN )->EnableWindow( TRUE );

		CString PhaseShift;
		PhaseShift.Format( "%6.2f", RAD_TO_DEG(m_pCurLocoTable->m_PhaseShift) );
		SetDlgItemText( IDC_PHASESHIFT, PhaseShift );
	}
}

void CLocomotionTablesDlg::SetUsedAnimations( void )
{
	CListBox* pUsedAnims = (CListBox*)GetDlgItem( IDC_USEDANIMS );
	CScrollBar* pUsedAnimsBar = (CScrollBar*) GetDlgItem( IDC_USEDANIMSBAR );
	int Anim, Hatch;
	int NumAnims=m_pCurLocoTable->m_AnimationList.GetSize();

	//---	if the dialog control has beem removed, do nothing
	ASSERT(pUsedAnims != NULL);
	
	//---	create the animation brushes storage
	if (m_pAnimBrushes)	delete[] m_pAnimBrushes;
	if (NumAnims > 0)	m_pAnimBrushes = new CBrush[NumAnims];
	else				m_pAnimBrushes = NULL;
	
	//---	clear the current list
	pUsedAnims->ResetContent();

	//---	fill the list with the used animations
	for (Anim=0; Anim<NumAnims; Anim++)
	{
		//---	add the animation to the list
		pUsedAnims->AddString( m_pCurLocoTable->m_AnimationList[Anim] );

		//---	create the animations brush
		Hatch = (Anim / NUM_ANIM_COLORS) % 7;
		switch( Hatch )
		{
		case 0:	m_pAnimBrushes[Anim].CreateSolidBrush( AnimColors[Anim%NUM_ANIM_COLORS] );					break;
		case 1:	m_pAnimBrushes[Anim].CreateHatchBrush( HS_BDIAGONAL,	AnimColors[Anim%NUM_ANIM_COLORS] );	break;
		case 2:	m_pAnimBrushes[Anim].CreateHatchBrush( HS_CROSS,		AnimColors[Anim%NUM_ANIM_COLORS] );	break;
		case 3:	m_pAnimBrushes[Anim].CreateHatchBrush( HS_DIAGCROSS,	AnimColors[Anim%NUM_ANIM_COLORS] );	break;
		case 4:	m_pAnimBrushes[Anim].CreateHatchBrush( HS_FDIAGONAL,	AnimColors[Anim%NUM_ANIM_COLORS] );	break;
		case 5:	m_pAnimBrushes[Anim].CreateHatchBrush( HS_HORIZONTAL,	AnimColors[Anim%NUM_ANIM_COLORS] );	break;
		case 6:	m_pAnimBrushes[Anim].CreateHatchBrush( HS_VERTICAL,		AnimColors[Anim%NUM_ANIM_COLORS] );	break;
		}
	}

	//---	set the scroll bar
	CRect AnimBrushKeyRect;
	int BoxListLen;
	pUsedAnims->GetWindowRect( &AnimBrushKeyRect );
	BoxListLen = (int)AnimBrushKeyRect.Height() / BRUSH_HEIGHT;
	if (BoxListLen >= NumAnims)
	{
		pUsedAnimsBar->ModifyStyle( WS_VISIBLE, 0, SWP_NOACTIVATE );
		pUsedAnims->SetWindowPos( &wndTop, m_UsedAnimsList_NoScroll.left,
										m_UsedAnimsList_NoScroll.top,
										m_UsedAnimsList_NoScroll.Width(),
										m_UsedAnimsList_NoScroll.Height(), 0 );
	}
	else
	{
		pUsedAnimsBar->SetScrollRange( 0, NumAnims - BoxListLen );
		pUsedAnimsBar->SetScrollPos( 0 );
		pUsedAnimsBar->ModifyStyle( 0, WS_VISIBLE, SWP_NOACTIVATE );

		pUsedAnims->SetWindowPos( &wndTop, m_UsedAnimsList_Scroll.left,
										m_UsedAnimsList_Scroll.top,
										m_UsedAnimsList_Scroll.Width(),
										m_UsedAnimsList_Scroll.Height(), 0 );

		pUsedAnimsBar->SetWindowPos( &wndTop, m_UsedAnimsList_ScrollBar.left,
										m_UsedAnimsList_ScrollBar.top,
										m_UsedAnimsList_ScrollBar.Width(),
										m_UsedAnimsList_ScrollBar.Height(), 0 );
	}

	//---	redraw the animation key
	RedrawAnimKey();
}

void CLocomotionTablesDlg::SetSelectedButtonDisplay( bool Display )
{
	//---	setup the plus/minus/equal buttons.
	GetDlgItem( IDC_PLUS )->EnableWindow( Display );
	GetDlgItem( IDC_MINUS )->EnableWindow( Display );
	GetDlgItem( IDC_EQUAL )->EnableWindow( Display && (m_SelectedItems.GetCount() > 1) );
}

void CLocomotionTablesDlg::SetSelectedArcZoneDisplay( int Arc, int Zone )
{
	CString str;
	POSITION pos;
	bool Display = (Arc != -1) || (Zone != -1);
	bool ArcDisplay=TRUE;
	bool ZoneDisplay=TRUE;

	//---	set arc display
	if (m_pCurLocoTable)
	{
		if ((Arc == -1) && (Zone != -1))
			str = "ALL";
		else if (Arc == -1)
		{
			ArcDisplay = FALSE;
			if (m_CenterSelected)
				str = "CTR";
			else
				str = "NIL";
		}
		else
			str.Format("%d", Arc);
		SetDlgItemText( IDC_SELECTEDARC, str );
		str.Format("%d", m_pCurLocoTable->m_ArcDataList.GetCount());
		SetDlgItemText( IDC_NUMARCS, str );
	}
	else
		ArcDisplay = FALSE;

/*	GetDlgItem( IDC_SELECTEDARCTITLE )->ModifyStyle( ArcDisplay?0:WS_VISIBLE, ArcDisplay?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_SELECTEDARCTITLE )->Invalidate();
	GetDlgItem( IDC_SELECTEDARC )->ModifyStyle( ArcDisplay?0:WS_VISIBLE, ArcDisplay?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_SELECTEDARC )->Invalidate();
	GetDlgItem( IDC_NUMARCSTITLE )->ModifyStyle( ArcDisplay?0:WS_VISIBLE, ArcDisplay?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_NUMARCSTITLE )->Invalidate();
	GetDlgItem( IDC_NUMARCS )->ModifyStyle( ArcDisplay?0:WS_VISIBLE, ArcDisplay?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_NUMARCS )->Invalidate();
*/

	//---	set zone Display
	if (m_pCurLocoTable)
	{
		if ((Zone == -1) && (Arc != -1))
		{
			if (m_LineSelected)
				ZoneDisplay = FALSE;
			else
				str = "ALL";
		}
		else if (Zone == -1)
		{
			ZoneDisplay = FALSE;
			if (m_CenterSelected)
				str = "CTR";
			else
				str = "NIL";
		}
		else
			str.Format("%d", Zone);

		SetDlgItemText( IDC_SELECTEDZONE, str );
		if (m_pCurLocoTable->m_ArcDataList.GetCount() > 0)
		{
			pos = m_pCurLocoTable->m_ArcDataList.FindIndex( 0 );
			str.Format("%d", m_pCurLocoTable->m_ArcDataList.GetAt( pos )->m_AnimNSRList.GetCount());
		}
		else
			str.Format("%d", 0 );
		SetDlgItemText( IDC_NUMZONES, str );
	}
	else
		ZoneDisplay = FALSE;
/*
	GetDlgItem( IDC_SELECTEDZONETITLE )->ModifyStyle( ZoneDisplay?0:WS_VISIBLE, ZoneDisplay?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_SELECTEDZONETITLE )->Invalidate();
	GetDlgItem( IDC_SELECTEDZONE )->ModifyStyle( ZoneDisplay?0:WS_VISIBLE, ZoneDisplay?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_SELECTEDZONE )->Invalidate();
	GetDlgItem( IDC_NUMZONESTITLE )->ModifyStyle( ZoneDisplay?0:WS_VISIBLE, ZoneDisplay?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_NUMZONESTITLE )->Invalidate();
	GetDlgItem( IDC_NUMZONES )->ModifyStyle( ZoneDisplay?0:WS_VISIBLE, ZoneDisplay?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_NUMZONES )->Invalidate();*/
}

void CLocomotionTablesDlg::SetSelectedValueDisplay( bool Display, float Value, bool Radian )
{
	CString str;

	GetDlgItem( IDC_VALUETITLE )->ModifyStyle( Display?0:WS_VISIBLE, Display?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_VALUETITLE )->Invalidate();
	GetDlgItem( IDC_VALUE )->ModifyStyle( Display?0:WS_VISIBLE, Display?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_VALUE )->Invalidate();
	GetDlgItem( IDC_VALUESPIN )->ModifyStyle( Display?0:WS_VISIBLE, Display?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_VALUESPIN )->Invalidate();
	GetDlgItem( IDC_RADIAN )->ModifyStyle( Display&&Radian?0:WS_VISIBLE, Display&&Radian?WS_VISIBLE:0, SWP_NOACTIVATE );
	GetDlgItem( IDC_RADIAN )->Invalidate();

	if (!Display)
		return;

	if ((Value > 1.0f) || (Value < 0.0f))
		str.Format( "%6.2f", Value );
	else if (Value < 10.0f)
		str.Format( "%6.3f", Value );
	else
		str.Format( "%6.4f", Value );

	SetDlgItemText( IDC_VALUE, str );
}

void CLocomotionTablesDlg::SetSelectedAnimationDisplay( bool Display, int AnimationIndex )
{
	CListBox* pUsedAnims = (CListBox*)GetDlgItem( IDC_USEDANIMS );

	if (Display && (AnimationIndex != -1))
		pUsedAnims->SetCurSel( AnimationIndex );
	else
		pUsedAnims->SetCurSel( -1 );

	ScrollUsedAnims();
	RedrawAnimKey();
}

void CLocomotionTablesDlg::SetSelected( int OriginArc, int Arc, int Zone, bool Line, bool Center )
{
	CLocoArcData* pArc;
	CLocoAnimNSR* pAnimNSR;
	int NumArcs;
	POSITION pos;
	int Anim;

	RedrawTable();

	//---	set the selected value
	m_SelectedArc		= Arc;
	m_SelectedZone		= Zone;
	m_LineSelected		= Line;
	m_CenterSelected	= Center;

	//---	Set the selected items
	ClearAllSelectedItems();

	//---	select
	if (m_pCurLocoTable == NULL)
	{
		m_SelectedArc		= -1;
		m_SelectedZone		= -1;
		m_LineSelected		= FALSE;
		m_CenterSelected	= FALSE;

		//---	Set the selected items
		ClearAllSelectedItems();

		//---	set the dialog display
		SetSelectedButtonDisplay( FALSE );
		SetSelectedArcZoneDisplay( -1, -1 );
		SetSelectedValueDisplay( FALSE, 0, FALSE );
		SetSelectedAnimationDisplay( FALSE, -1 );
	}
	else if (m_CenterSelected)
	{
		//---	clear any illigal selection values
		m_SelectedArc = -1;
		m_SelectedZone = -1;
		
		//---	Set the selected items
		ClearAllSelectedItems();

		//---	set the dialog display
		SetSelectedButtonDisplay( FALSE );
		SetSelectedArcZoneDisplay( m_SelectedArc, m_SelectedZone );
		SetSelectedValueDisplay( m_LineSelected, m_pCurLocoTable->m_CenterNSR, FALSE );
		SetSelectedAnimationDisplay( !m_LineSelected, m_pCurLocoTable->m_CenterAnim );

		//---	Set the selected items
		CSelect* pCenter;
		if (m_LineSelected)
			pCenter = new CSelectValue( m_pCurLocoTable, &m_Zero, &m_pCurLocoTable->m_MaxNSR, 1.0f );
		else
			pCenter = new CSelectAnim( m_pCurLocoTable, -1, m_pCurLocoTable->m_AnimationList.GetSize()-1 );
		AddSelectedItem( pCenter );
	}
	else if ((m_SelectedArc == -1) && (m_SelectedZone == -1))
	{
		//---	Set the selected items
		ClearAllSelectedItems();

		if (m_LineSelected)
		{
			//---	select all the arcs
			int ArcIndex, ActualArc;
			CSelectValue* pSelectArc;
			float *pMin, *pMax;

			NumArcs = m_pCurLocoTable->m_ArcDataList.GetCount();
			for( ArcIndex=0; ArcIndex<NumArcs; ArcIndex++ )
			{
				ActualArc = (OriginArc + ArcIndex)%NumArcs;
				pos = m_pCurLocoTable->m_ArcDataList.FindIndex( ActualArc );
				pArc = m_pCurLocoTable->m_ArcDataList.GetAt( pos );

				if (ActualArc == 0)	pMin = &m_Zero;
				else				pMin = &m_pCurLocoTable->m_ArcDataList.GetAt( m_pCurLocoTable->m_ArcDataList.FindIndex( ActualArc-1 ) )->m_Angle;

				if (ActualArc == (NumArcs-1))	pMax = &m_2Pi;
				else							pMax = &m_pCurLocoTable->m_ArcDataList.GetAt( m_pCurLocoTable->m_ArcDataList.FindIndex( ActualArc+1 ) )->m_Angle;
	
				pSelectArc = new CSelectValue( pArc, pMin, pMax, RAD_TO_DEG(1.0f) );
				AddSelectedItem( pSelectArc );
			}

			SetSelectedButtonDisplay( TRUE );
			SetSelectedArcZoneDisplay( -1, -1 );
			SetSelectedValueDisplay( FALSE, 0.0f, FALSE );
			SetSelectedAnimationDisplay( FALSE, -1 );
		}
		else
		{
			SetSelectedButtonDisplay( FALSE );
			SetSelectedArcZoneDisplay( -1, -1 );
			SetSelectedValueDisplay( FALSE, 0.0f, FALSE );
			SetSelectedAnimationDisplay( FALSE, -1 );
		}
	}
	else if (m_SelectedArc != -1)
	{
		pos = m_pCurLocoTable->m_ArcDataList.FindIndex( Arc );
		pArc = m_pCurLocoTable->m_ArcDataList.GetAt( pos );
		NumArcs = m_pCurLocoTable->m_ArcDataList.GetCount();

		if (m_SelectedZone == -1)
		{
			if (m_LineSelected)
			{
				//---	Set the selected items
				ClearAllSelectedItems();

				float *pMin, *pMax;
				if (Arc == 0)	pMin = &m_Zero;
				else			pMin = &m_pCurLocoTable->m_ArcDataList.GetAt( m_pCurLocoTable->m_ArcDataList.FindIndex( Arc-1 ) )->m_Angle;

				if (Arc == (NumArcs-1))	pMax = &m_2Pi;
				else					pMax = &m_pCurLocoTable->m_ArcDataList.GetAt( m_pCurLocoTable->m_ArcDataList.FindIndex( Arc+1 ) )->m_Angle;
	
				//---	they are not allowed to change the position of the last arc from 2*Pi
				if (Arc != (NumArcs-1))
				{
					pArc = m_pCurLocoTable->m_ArcDataList.GetAt( m_pCurLocoTable->m_ArcDataList.FindIndex( Arc ) );
					CSelectValue* pSelectArc = new CSelectValue( pArc, pMin, pMax, RAD_TO_DEG(1.0f) );
					AddSelectedItem( pSelectArc );
				}

				//---	set the selected dialog components
				SetSelectedButtonDisplay( TRUE );
				SetSelectedArcZoneDisplay( m_SelectedArc, -1 );
				SetSelectedValueDisplay( TRUE, m_Radian.GetCheck() ? pArc->m_Angle : RAD_TO_DEG(pArc->m_Angle), TRUE ); 
				SetSelectedAnimationDisplay( FALSE, -1 );
			}
			else
			{
				//---	Set the selected items
				ClearAllSelectedItems();

				//---	add the zones to the selection list
				Anim = -2;
				pos = pArc->m_AnimNSRList.GetHeadPosition();
				while(pos)
				{
					pAnimNSR = pArc->m_AnimNSRList.GetNext( pos );

					//---	add this zone to the selection list
					CSelectAnim* pSelectZone = new CSelectAnim( pAnimNSR, -1, m_pCurLocoTable->m_AnimationList.GetSize()-1 );
					AddSelectedItem( pSelectZone );

					//---	keep a check on what anims are used for setting the selected animations
					if (Anim != -1)
					{
						if (Anim == -2) Anim = pAnimNSR->m_Anim;
						if (pAnimNSR->m_Anim != Anim) Anim = -1;	// if the animations are not homogenous, set them to -1
					}
				}

				//---	set the selected dialog components
				SetSelectedButtonDisplay( TRUE );
				SetSelectedArcZoneDisplay( m_SelectedArc, -1 );
				SetSelectedValueDisplay( FALSE, -1, FALSE ); 
				SetSelectedAnimationDisplay( TRUE, Anim );
			}
		}
		else
		{
			//---	Set the selected items
			ClearAllSelectedItems();

			if (m_LineSelected)
			{
				int NumZones = pArc->m_AnimNSRList.GetCount();
				float *pMin, *pMax;

				if (m_SelectedZone == 0)	pMin = &m_pCurLocoTable->m_CenterNSR;
				else						pMin = &pArc->m_AnimNSRList.GetAt( pArc->m_AnimNSRList.FindIndex( m_SelectedZone-1 ))->m_NSR;

				if (m_SelectedZone == (NumZones-1))	pMax = &m_pCurLocoTable->m_MaxNSR;
				else								pMax = &pArc->m_AnimNSRList.GetAt( pArc->m_AnimNSRList.FindIndex( m_SelectedZone+1 ) )->m_NSR;
	
				pos = pArc->m_AnimNSRList.FindIndex( m_SelectedZone );
				pAnimNSR = pArc->m_AnimNSRList.GetAt( pos );

				CSelectValue* pSelectArc = new CSelectValue( pAnimNSR, pMin, pMax, 1.0f );
				AddSelectedItem( pSelectArc );

				//---	set the selected dialog components
				SetSelectedButtonDisplay( TRUE );
				SetSelectedArcZoneDisplay( m_SelectedArc, m_SelectedZone );
				SetSelectedValueDisplay( TRUE, pAnimNSR->m_NSR, FALSE ); 
				SetSelectedAnimationDisplay( FALSE, -1 );
			}
			else
			{
				pos = pArc->m_AnimNSRList.FindIndex( m_SelectedZone );
				pAnimNSR = pArc->m_AnimNSRList.GetAt( pos );

				//---	add this zone to the selection list
				CSelectAnim* pSelectZone = new CSelectAnim( pAnimNSR, -1, m_pCurLocoTable->m_AnimationList.GetSize()-1 );
				AddSelectedItem( pSelectZone );

				//---	set the selected dialog components
				SetSelectedButtonDisplay( TRUE );
				SetSelectedArcZoneDisplay( m_SelectedArc, m_SelectedZone );
				SetSelectedValueDisplay( FALSE, -1, FALSE ); 
				SetSelectedAnimationDisplay( TRUE, pAnimNSR->m_Anim );
			}
		}

	}
	else if (m_SelectedZone != -1)
	{
		POSITION pos;
		int NumArcs = m_pCurLocoTable->m_ArcDataList.GetCount();
		int ArcIndex;

		//---	Set the selected items
		ClearAllSelectedItems();

		//---	select a zone within all the arcs
		for( ArcIndex=0; ArcIndex<NumArcs; ArcIndex++ )
		{
			pos = m_pCurLocoTable->m_ArcDataList.FindIndex( (OriginArc + ArcIndex)%NumArcs );
			pArc = m_pCurLocoTable->m_ArcDataList.GetAt( pos );

			//---	get a pointer to the zone
			int NumZones = pArc->m_AnimNSRList.GetCount();

			//---	skip this arc if it does not have the zone
			if (m_SelectedZone >= NumZones)
				continue;

			//---	look for the correct zone within this arc
			pos = pArc->m_AnimNSRList.FindIndex( m_SelectedZone );
			pAnimNSR = pArc->m_AnimNSRList.GetAt( pos );
			if (m_LineSelected)
			{
				float *pMin, *pMax;

				if (m_SelectedZone == 0)	pMin = &m_pCurLocoTable->m_CenterNSR;
				else						pMin = &pArc->m_AnimNSRList.GetAt( pArc->m_AnimNSRList.FindIndex( m_SelectedZone-1 ))->m_NSR;

				if (m_SelectedZone == (NumZones-1))	pMax = &m_pCurLocoTable->m_MaxNSR;
				else								pMax = &pArc->m_AnimNSRList.GetAt( pArc->m_AnimNSRList.FindIndex( m_SelectedZone+1 ) )->m_NSR;
	
				pos = pArc->m_AnimNSRList.FindIndex( m_SelectedZone );
				pAnimNSR = pArc->m_AnimNSRList.GetAt( pos );

				CSelectValue* pSelectArc = new CSelectValue( pAnimNSR, pMin, pMax, 1.0f );
				AddSelectedItem( pSelectArc );
			}
			else
			{
				//---	add this zone to the selection list
				CSelectAnim* pSelectZone = new CSelectAnim( pAnimNSR, -1, m_pCurLocoTable->m_AnimationList.GetSize()-1 );
				AddSelectedItem( pSelectZone );
			}
		}

		//---	set the selected dialog components
		SetSelectedButtonDisplay( TRUE );
		SetSelectedArcZoneDisplay( -1, m_SelectedZone );
		SetSelectedValueDisplay( TRUE, pAnimNSR->m_NSR, FALSE ); 
		SetSelectedAnimationDisplay( FALSE, -1 );
	}
}

void CLocomotionTablesDlg::ClearAllSelectedItems()
{
	POSITION pos = m_SelectedItems.GetHeadPosition();

	while( pos )
	{
		CSelect* pSelect = m_SelectedItems.GetNext( pos );
		delete pSelect;
	}

	m_SelectedItems.RemoveAll();

	RedrawTable();
}

void CLocomotionTablesDlg::AddSelectedItem( CSelect* pItem )
{
	RedrawTable();
	m_SelectedItems.AddTail( pItem );
}

void CLocomotionTablesDlg::IncSelected( int Steps )
{
	//---	loop through all selected items and increment them
	CSelect* pSelect;
	POSITION pos;
	int First=TRUE;

	RedrawTable();

	pos = m_SelectedItems.GetHeadPosition();
	while( pos != NULL )
	{
		pSelect = m_SelectedItems.GetNext( pos );
		pSelect->Inc( (float)Steps );
		if (First)
		{
			pSelect->SetDisplay( this );
			First = FALSE;
		}
	}
}

void CLocomotionTablesDlg::SetSelectedValue( float Value )
{
	//---	loop through all selected items and increment them
	CSelect* pSelect;
	POSITION pos;

	if (m_SelectedItems.GetCount() <= 0)
		return;

	//---	tell the table to redraw
	if (m_LineSelected)
		RedrawTable();
	else
		RedrawAnimKey();

	//---	update the selected item values
	pos = m_SelectedItems.GetHeadPosition();
	while( pos != NULL )
	{
		pSelect = m_SelectedItems.GetNext( pos );
		pSelect->Set( Value );
	}

	//---	update the display
	m_SelectedItems.GetHead()->SetDisplay( this );
}


void CLocomotionTablesDlg::IncSelectedValue( float Value )
{
	//---	loop through all selected items and increment them
	CSelect* pSelect;
	POSITION pos;

	//---	tell the table to redraw
	if (m_LineSelected)
		RedrawTable();
	else
		RedrawAnimKey();

	//---	update the selected item values
	pos = m_SelectedItems.GetHeadPosition();
	while( pos != NULL )
	{
		pSelect = m_SelectedItems.GetNext( pos );
		pSelect->Inc( Value );
	}

	//---	update the display
	m_SelectedItems.GetHead()->SetDisplay( this );
}



///////////////////////////////////////////////////////////////////////////////////////
//						Drawing functions
///////////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::SetBrushForAnim( CPaintDC& dc, int Anim, bool Selected )
{
	//---	if this item is selected (and the LineSelect flag is not on), set the brush to the selected brush
	if (Selected)
		dc.SelectObject( &m_SelectBrush );
	else if (Anim == -1) // ANIM_NONE
		dc.SelectObject( &m_ShadeBrush );
	else
		dc.SelectObject( &m_pAnimBrushes[ Anim ] );
}

bool CLocomotionTablesDlg::GetArcRadius( float NSR, float MaxNSR, int& rArcRadius )
{
	if (NSR >= MaxNSR)
	{
		rArcRadius = (int)((float)m_ArcRadius * MaxNSR);
		return TRUE;
	}
	else
	{
		rArcRadius = (int)((float)m_ArcRadius * NSR);
		return FALSE;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::OnPaint() 
{
	int radius;
	float MaxNSR;
	int ExitEarly;
	POSITION pos;
	int Count;
	CLocoArcData* pFirstArc;
	CLocoArcData* pLastArc;
	CLocoArcData* pThisArc;
	CRect UpdateRect, Intersection;

	CPaintDC dc(this); // device context for painting

	//---	stop now if there is no selected table
	if (m_pCurLocoTable == NULL)
	{
		//---	draw center circle
		dc.SelectObject( &m_BackgroundBrush );
		dc.SelectObject( &m_BasicPen );
		dc.Ellipse( m_ArcCenter.x - m_ArcRadius, m_ArcCenter.y - m_ArcRadius, m_ArcCenter.x + m_ArcRadius, m_ArcCenter.y + m_ArcRadius );
		return;
	}

	//---	Draw the Anim/Brush key
	m_DrawTableOnly = FALSE;
	if (!m_DrawTableOnly || m_DrawAnimKeyOnly)
	{
		m_DrawAnimKeyOnly = FALSE;
		DrawAnimBrushKey( dc );
		if (m_DrawAnimKeyOnly && !m_DrawTableOnly)
			return;
	}

	//---	draw entire circle (clear center to the background color
	if (m_pCurLocoTable->m_MaxNSR != 1.0f)
		dc.SelectObject( &m_ShadeBrush );
	else
		dc.SelectObject( &m_BackgroundBrush );
	dc.SelectObject( &m_BasicPen );
	dc.Ellipse( m_ArcCenter.x - m_ArcRadius, m_ArcCenter.y - m_ArcRadius, m_ArcCenter.x + m_ArcRadius, m_ArcCenter.y + m_ArcRadius );

	//---	set the phase shift if the orientation is set on one of the calculated settings
	if (m_pCurLocoTable->m_Orientation == ORIENTATION_POINTFOREWARD)
		m_pCurLocoTable->m_PhaseShift = 0.0f;
	else if (m_pCurLocoTable->m_Orientation == ORIENTATION_SIDEFOREWARD)
	{
		if (m_pCurLocoTable->m_ArcDataList.GetCount())
			m_pCurLocoTable->m_PhaseShift = -m_pCurLocoTable->m_ArcDataList.GetHead()->m_Angle / 2.0f;
		else
			m_pCurLocoTable->m_PhaseShift = 0.0f;
	}

	//---	draw MaxNSR circle
	MaxNSR = m_pCurLocoTable->m_MaxNSR;
	if (MaxNSR != 1.0f)
	{
		dc.SelectObject( &m_BackgroundBrush );
		radius = (int)((float)m_ArcRadius * MaxNSR);
		dc.Ellipse( m_ArcCenter.x - radius, m_ArcCenter.y - radius, m_ArcCenter.x + radius, m_ArcCenter.y + radius );
	}

	//---	draw the center circle
	if (m_pCurLocoTable->m_CenterNSR > 0.0f)
	{
		//---	draw the center circle
		if (m_CenterSelected && m_LineSelected)	dc.SelectObject( &m_SelectPen );
		else									dc.SelectObject( &m_BasicPen );
		SetBrushForAnim( dc, m_pCurLocoTable->m_CenterAnim, m_CenterSelected && !m_LineSelected );
		ExitEarly = GetArcRadius( m_pCurLocoTable->m_CenterNSR, MaxNSR, radius );
		dc.Ellipse( m_ArcCenter.x - radius, m_ArcCenter.y - radius, m_ArcCenter.x + radius, m_ArcCenter.y + radius );
		if (ExitEarly) return;

		//---	draw the overlap
		if (m_pCurLocoTable->m_OverlapNSR != 0.0f)
		{
			int Overlap = (int)((float)m_ArcRadius*m_pCurLocoTable->m_OverlapNSR);
			radius += Overlap;
			dc.Arc( m_ArcCenter.x - radius, m_ArcCenter.y - radius, m_ArcCenter.x + radius, m_ArcCenter.y + radius,
					m_ArcCenter.x - radius, m_ArcCenter.y, m_ArcCenter.x - radius, m_ArcCenter.y );

			radius = radius - Overlap/2;

			//---	if the overlap is large, fill the area
			if (ABS(Overlap) < OVERLAP_FILLAREARADIUS)
			{
				//---	flood fill is dangerous for these small areas, lets just draw another arc
				dc.Arc( m_ArcCenter.x - radius, m_ArcCenter.y - radius, m_ArcCenter.x + radius, m_ArcCenter.y + radius,
						m_ArcCenter.x - radius, m_ArcCenter.y, m_ArcCenter.x - radius, m_ArcCenter.y );
			}
			else
			{
				//---	fill it with a selected color or a non-selected color
				if (m_CenterSelected && m_LineSelected)	
					dc.SelectObject( &m_SelectBrush );
				else
					dc.SelectObject( &m_BlackBrush );

				dc.ExtFloodFill( m_ArcCenter.x + radius, m_ArcCenter.y, BACKGROUND_COLOR, FLOODFILLSURFACE);//SELECT_COLOR );
			}
		}
	}

	//---	loop through the arcs and draw each one
	if (m_pCurLocoTable->m_ArcDataList.GetCount() > 0)
	{
		pos = m_pCurLocoTable->m_ArcDataList.GetHeadPosition();
		pFirstArc = pLastArc = m_pCurLocoTable->m_ArcDataList.GetNext( pos );
		Count = 1;
		while( pos )
		{
			pThisArc = m_pCurLocoTable->m_ArcDataList.GetNext( pos );
			DrawArc( dc, pLastArc, pThisArc, Count++ ); 
			pLastArc = pThisArc;
		}
		DrawArc( dc, pLastArc, pFirstArc, 0 );
	}

	//---	draw the line
	dc.SelectObject( &m_BasicPen );
	dc.MoveTo( m_ArcCenter.x, m_ArcCenter.y - m_ArcRadius );
	dc.LineTo( m_ArcCenter.x, m_ArcCenter.y - m_ArcRadius - 15 );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::DrawArc( CPaintDC& dc, CLocoArcData* pLastArc, CLocoArcData* pArc, int Arc ) 
{
	CLocoAnimNSR* pAnimNSR;
	POSITION pos;
	CRect BoundRect;
	CPoint LineStart;
	CPoint LineEnd;
	CPoint LastZoneLineEnd;
	float MinRadius, MaxRadius, FillRadius, LastRadius;
	float angle1, angle2, FillAngle;
	bool OverlapSelect;
	int LastArc;
	int Zone;
	float SinA1, SinA2, CosA1, CosA2, SinO, CosO;
	float FillSin, FillCos;
	CPoint FillSeed;
	float PhaseShift = m_pCurLocoTable->m_PhaseShift - (m_pCurLocoTable->m_OverlapArc/2.0f) + TABLE_PHASE_SHIFT;
	float CenterRadius = m_pCurLocoTable->m_CenterNSR*m_ArcRadius;
	float OuterRadius = m_pCurLocoTable->m_MaxNSR*m_ArcRadius;
	bool LineSelected, AreaSelected;
	float OverlapArc = m_pCurLocoTable->m_OverlapArc;
	float FillOverlapAngle;
	int Overlap = (int)(m_pCurLocoTable->m_OverlapNSR*(float)m_ArcRadius);
	int OverlapMin = (Overlap < 0) ? Overlap : 0;
	int OverlapMax = (Overlap < 0) ? 0 : Overlap;

	//---	get the sine/cosine values
	angle1 = pLastArc->m_Angle;
	angle2 = pArc->m_Angle;
	if (Arc == 0)	FillAngle = angle2 / 2.0f;
	else			FillAngle = (angle1 + angle2) / 2.0f;
	if (FillAngle == 0.0f)	FillAngle = m_2Pi/2.0f;

	angle1 += PhaseShift;
	while(angle1 < 0) angle1 += m_2Pi;
	while(angle1 >= m_2Pi) angle1 -= m_2Pi;

	angle2 += PhaseShift;
	while(angle2 < 0) angle2 += m_2Pi;
	while(angle2 >= m_2Pi) angle2 -= m_2Pi;

	FillAngle += PhaseShift;
	while(FillAngle < 0) FillAngle += m_2Pi;
	while(FillAngle >= m_2Pi) FillAngle -= m_2Pi;

	SinA1 = (float)sin( (double)angle1 );
	CosA1 = (float)cos( (double)angle1 );

	SinA2 = (float)sin( (double)angle2 );
	CosA2 = (float)cos( (double)angle2 );

	FillSin = (float)sin( (double)FillAngle );
	FillCos = (float)cos( (double)FillAngle );

	//---	draw in the basic color
	dc.SelectObject( &m_BasicPen );

	//---	Only draw arc border lines if the angles are different
	if (pLastArc->m_Angle != pArc->m_Angle)
	{
		//---	draw the first line if this is the first arc drawn
		if (Arc == 1)
		{
			//---	set the pen to selected if required
			if (m_LineSelected && !m_CenterSelected
				&& (m_SelectedZone == -1)
				&& ((m_SelectedArc == -1) || (m_SelectedArc == 0)))
				dc.SelectObject( &m_SelectPen );
			else
				dc.SelectObject( &m_BasicPen );

			LineStart.x = m_ArcCenter.x + (long)((CenterRadius + OverlapMax)*SinA1);
			LineStart.y = m_ArcCenter.y + (long)((CenterRadius + OverlapMax)*CosA1);
			LineEnd.x = m_ArcCenter.x + (long)(OuterRadius*SinA1);
			LineEnd.y = m_ArcCenter.y + (long)(OuterRadius*CosA1);
			dc.MoveTo( LineStart );
			dc.LineTo( LineEnd );
		}

		//---	if there is an arc overlap, draw it now
		if( OverlapArc != 0.0f )
		{
			LastArc = Arc - 1;
			if( LastArc == -1 )
				LastArc = m_pCurLocoTable->m_ArcDataList.GetCount()-1;

			//---	set the pen to selected if required
			if (m_LineSelected && !m_CenterSelected
				&& (m_SelectedZone == -1)
				&& ((m_SelectedArc == -1) || (m_SelectedArc == LastArc)))
				OverlapSelect = TRUE;
			else
				OverlapSelect = FALSE;

			//---	set the pen to selected if required
			if (OverlapSelect)
				dc.SelectObject( &m_SelectPen );
			else
				dc.SelectObject( &m_BasicPen );

			//---	draw the arc line
			SinO = (float)sin( (double)(angle1 + OverlapArc) );
			CosO = (float)cos( (double)(angle1 + OverlapArc) );
			LineStart.x = m_ArcCenter.x + (long)((CenterRadius + OverlapMax)*SinO);
			LineStart.y = m_ArcCenter.y + (long)((CenterRadius + OverlapMax)*CosO);
			LineEnd.x = m_ArcCenter.x + (long)(OuterRadius*SinO);
			LineEnd.y = m_ArcCenter.y + (long)(OuterRadius*CosO);
			dc.MoveTo( LineStart );
			dc.LineTo( LineEnd );

			//---	if the arc is wide enough, fill it
			if( OverlapArc > 0.0175f )
			{
				//---	set the pen to selected if required
				if (OverlapSelect)
					dc.SelectObject( &m_SelectBrush );
				else
					dc.SelectObject( &m_BlackBrush );

				FillOverlapAngle = angle1 + (OverlapArc*0.5f);
				FillRadius = OuterRadius*0.95f;
				FillSeed.x = m_ArcCenter.x + (long)(FillRadius*sin( (double)FillOverlapAngle) );
				FillSeed.y = m_ArcCenter.y + (long)(FillRadius*cos( (double)FillOverlapAngle) );
				dc.ExtFloodFill( FillSeed.x, FillSeed.y, BACKGROUND_COLOR, FLOODFILLSURFACE );
			}
		}

		//---	draw the second line, unless it has already been drawn
		if (Arc != 0)
		{
			//---	set the pen to selected if required
			if (m_LineSelected && !m_CenterSelected
				&& (m_SelectedZone == -1)
				&& ((m_SelectedArc == -1) || (m_SelectedArc == Arc)))
				dc.SelectObject( &m_SelectPen );
			else
				dc.SelectObject( &m_BasicPen );

			LineStart.x = m_ArcCenter.x + (long)((CenterRadius + OverlapMax)*SinA2);
			LineStart.y = m_ArcCenter.y + (long)((CenterRadius + OverlapMax)*CosA2);
			LineEnd.x = m_ArcCenter.x + (long)(OuterRadius*SinA2);
			LineEnd.y = m_ArcCenter.y + (long)(OuterRadius*CosA2);
			dc.MoveTo( LineStart );
			dc.LineTo( LineEnd );
		}
	}

	//---	initialze the LastZoneLineEnd variable to the Center arc
	LastRadius = CenterRadius;

	//---	draw each zone
	pos = pArc->m_AnimNSRList.GetHeadPosition();
	Zone = 0;
	while( pos )
	{
		pAnimNSR = pArc->m_AnimNSRList.GetNext( pos );

		LineSelected = m_LineSelected
			&& (m_SelectedZone == Zone)
			&& ((m_SelectedArc == -1) || (m_SelectedArc == Arc));

		//---	Draw Bounding Arc for zone
		if (pos == NULL)
			MaxRadius = MinRadius = ((float)m_ArcRadius * pAnimNSR->m_NSR);
		else
		{
			MinRadius = ((float)m_ArcRadius * pAnimNSR->m_NSR) + OverlapMin;
			MaxRadius = ((float)m_ArcRadius * pAnimNSR->m_NSR) + OverlapMax;
		}

		if (MinRadius > OuterRadius) MinRadius = OuterRadius;
		if (MaxRadius > OuterRadius) MaxRadius = OuterRadius;

		//---	draw the arc line
		if( OverlapArc != 0 )
			DrawArcLine( dc, MinRadius, MaxRadius, SinO, CosO, SinA2, CosA2, FillSin, FillCos, LineSelected );
		else
			DrawArcLine( dc, MinRadius, MaxRadius, SinA1, CosA1, SinA2, CosA2, FillSin, FillCos, LineSelected );

		//---	fill the animation region
		AreaSelected =	!m_LineSelected
					&& (((m_SelectedArc == -1) && (m_SelectedZone == Zone))
					|| ((m_SelectedArc == Arc) && ((m_SelectedZone == Zone) || (m_SelectedZone == -1))));
		SetBrushForAnim( dc, pAnimNSR->m_Anim, AreaSelected );
		FillRadius = LastRadius + (MinRadius - LastRadius)/2.0f;
		FillSeed.x = m_ArcCenter.x + (long)(FillRadius*FillSin);
		FillSeed.y = m_ArcCenter.y + (long)(FillRadius*FillCos);
		dc.ExtFloodFill( FillSeed.x, FillSeed.y, BACKGROUND_COLOR, FLOODFILLSURFACE );

		//LastZoneLineEnd = LineEnd;
		LastRadius = MaxRadius;
		Zone++;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::DrawArcLine( CPaintDC& dc,
									   float MinRadius, float MaxRadius,
									   float SinA1, float CosA1,
									   float SinA2, float CosA2,
									   float FillSin, float FillCos,
									   bool LineSelected )
{
	CRect BoundRect;
	CPoint FillSeed;
	CPoint LineStart;
	CPoint LineEnd;
	float FillRadius;
	bool Circle = (SinA1 == 0.0f) && (SinA2 == 0.0f) && (CosA1 == 0.0f) && (CosA2 == 0.0f);

	//---	set the pen to selected if required
	if (LineSelected)
	{
		dc.SelectObject( &m_SelectPen );
		dc.SelectObject( &m_SelectBrush );
	}
	else
	{
		dc.SelectObject( &m_BasicPen );
		dc.SelectObject( &m_BlackBrush );
	}

	//---	define arc bounding box
	BoundRect.left = m_ArcCenter.x - (long)MinRadius;
	BoundRect.top = m_ArcCenter.y - (long)MinRadius;
	BoundRect.right = m_ArcCenter.x + (long)MinRadius;
	BoundRect.bottom = m_ArcCenter.y + (long)MinRadius;

	//---	define arc start and end points
	if (!Circle)
	{
		LineStart.x = m_ArcCenter.x + (long)(MinRadius * SinA1);
		LineStart.y = m_ArcCenter.y + (long)(MinRadius * CosA1);
		LineEnd.x = m_ArcCenter.x + (long)(MinRadius * SinA2);
		LineEnd.y = m_ArcCenter.y + (long)(MinRadius * CosA2);
	}
	else
	{
		LineStart.x = LineEnd.x = 1;
		LineStart.y = LineEnd.y = 0;
	}

	//---	draw the arc
	dc.Arc( &BoundRect, LineStart, LineEnd );

	if (MinRadius != MaxRadius)
	{
		//---	define arc bounding box
		BoundRect.left = m_ArcCenter.x - (long)MaxRadius;
		BoundRect.top = m_ArcCenter.y - (long)MaxRadius;
		BoundRect.right = m_ArcCenter.x + (long)MaxRadius;
		BoundRect.bottom = m_ArcCenter.y + (long)MaxRadius;

		//---	define arc start and end points
		if (!Circle)
		{
			LineStart.x = m_ArcCenter.x + (long)(MaxRadius * SinA1);
			LineStart.y = m_ArcCenter.y + (long)(MaxRadius * CosA1);
			LineEnd.x = m_ArcCenter.x + (long)(MaxRadius * SinA2);
			LineEnd.y = m_ArcCenter.y + (long)(MaxRadius * CosA2);
		}

		//---	draw the arc
		dc.Arc( &BoundRect, LineStart, LineEnd );

		//---	fill the region
		FillRadius = MinRadius + (MaxRadius - MinRadius)/2.0f;

		if (ABS(MinRadius - MaxRadius) < OVERLAP_FILLAREARADIUS)
		{
			//---	define arc bounding box
			BoundRect.left = m_ArcCenter.x - (long)FillRadius;
			BoundRect.top = m_ArcCenter.y - (long)FillRadius;
			BoundRect.right = m_ArcCenter.x + (long)FillRadius;
			BoundRect.bottom = m_ArcCenter.y + (long)FillRadius;

			//---	define arc start and end points
			if (!Circle)
			{
				LineStart.x = m_ArcCenter.x + (long)(FillRadius * SinA1);
				LineStart.y = m_ArcCenter.y + (long)(FillRadius * CosA1);
				LineEnd.x = m_ArcCenter.x + (long)(FillRadius * SinA2);
				LineEnd.y = m_ArcCenter.y + (long)(FillRadius * CosA2);
			}

			//---	draw the arc
			dc.Arc( &BoundRect, LineStart, LineEnd );
		}
		else
		{
			FillSeed.x = m_ArcCenter.x + (long)(FillRadius*FillSin);
			FillSeed.y = m_ArcCenter.y + (long)(FillRadius*FillCos);
			dc.ExtFloodFill( FillSeed.x, FillSeed.y, BACKGROUND_COLOR, FLOODFILLSURFACE );
		}
	}
}

void CLocomotionTablesDlg::DrawAnimBrushKey( CPaintDC& dc )
{
	CListBox* pUsedAnims;
	int i, Count, Offset;
	CRect AnimBrushKeyRect;

	if (!m_pAnimBrushes) return;

	pUsedAnims = (CListBox*) GetDlgItem( IDC_USEDANIMS );

	//---	get the animation key brush rectangle
	AnimBrushKeyRect = m_AnimBrushKeyRect;
	AnimBrushKeyRect.bottom = AnimBrushKeyRect.top + BRUSH_HEIGHT;

	Offset = pUsedAnims->GetTopIndex();

	//---	display each brush
	dc.SelectObject( &m_BrushKeyPen );
	Count = (int)m_AnimBrushKeyRect.Height() / BRUSH_HEIGHT;
	if (Count> pUsedAnims->GetCount())
		Count = pUsedAnims->GetCount();
	for(i=0; i<Count; i++)
	{
		dc.SelectObject( &m_pAnimBrushes[i + Offset] );
		dc.Rectangle( &AnimBrushKeyRect );
		AnimBrushKeyRect.top = AnimBrushKeyRect.bottom;
		AnimBrushKeyRect.bottom += BRUSH_HEIGHT;
	}
}


//////////////////////////////////////////////////////////////////////
//						Message Handling
//////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Add your message handler code here
	
}

void CLocomotionTablesDlg::OnSelchangeLocomotiontables() 
{
	//---	do some last chance saving of modified data before switching to another table
//	if (m_pCurLocoTable)
//		OnKillfocusCallback();
	SetDisplayToTable( ((CListBox*)GetDlgItem( IDC_LOCOMOTIONTABLES ))->GetCurSel() );	
}

void CLocomotionTablesDlg::OnSelcancelLocomotiontables() 
{
}

void CLocomotionTablesDlg::OnSetfocusLocomotiontables() 
{
/*
	if (m_pCurLocoTable)
		OnKillfocusCallback();
	SetDisplayToTable( ((CListBox*)GetDlgItem( IDC_LOCOMOTIONTABLES ))->GetCurSel() );	
	*/
}

void CLocomotionTablesDlg::OnKillfocusLocomotiontables() 
{
}

void CLocomotionTablesDlg::OnAdd() 
{
	CListBox* pTables = (CListBox*)GetDlgItem( IDC_LOCOMOTIONTABLES );
	int i;
	CString NewName;

	CLocoAnimTable* pTable = new CLocoAnimTable;

	//---	make sure that this locomotion table does not share a name with another table.
	i=2;
	NewName = pTable->m_Name;
	while (pTables->FindStringExact( -1, pTable->m_Name ) != -1)
		pTable->m_Name.Format( "%s_%d", NewName, i++ );

	//---	add the new one to the list
	m_LocomotionTables.AddTable( pTable );
	pTables->AddString( pTable->m_Name );
	i = pTables->FindStringExact( -1, pTable->m_Name );
	pTables->SetItemData( i, (unsigned int) pTable );

	pTables->SetCurSel( pTables->FindStringExact( -1, pTable->m_Name ) );
	OnSelchangeLocomotiontables();
}

void CLocomotionTablesDlg::OnCopy() 
{
	CListBox* pTables = (CListBox*)GetDlgItem( IDC_LOCOMOTIONTABLES );
	int i;
	CString Name, NewName;

	CLocoAnimTable* pTable = new CLocoAnimTable;

	if (pTables->GetCurSel() == -1)
		return;

	pTables->GetText( pTables->GetCurSel(), Name );
	*pTable = *m_LocomotionTables.GetTable( Name );

	//---	make sure that this locomotion table does not share a name with another table.
	i=2;
	NewName = pTable->m_Name;
	while (pTables->FindStringExact( -1, pTable->m_Name ) != -1)
		pTable->m_Name.Format( "%s_%d", NewName, i++ );

	//---	add the new one to the list
	m_LocomotionTables.AddTable( pTable );
	pTables->AddString( pTable->m_Name );
	i = pTables->FindStringExact( -1, pTable->m_Name );
	pTables->SetItemData( i, (unsigned int) pTable );

	pTables->SetCurSel( pTables->FindStringExact( -1, pTable->m_Name ) );
	OnSelchangeLocomotiontables();
}

void CLocomotionTablesDlg::OnRemove() 
{
	CListBox* pTables = (CListBox*)GetDlgItem( IDC_LOCOMOTIONTABLES );
	CString Name;
	int CurSel = pTables->GetCurSel();

	if (CurSel == -1)
		return;

	pTables->GetText( pTables->GetCurSel(), Name );
	m_LocomotionTables.Delete( Name );
	pTables->DeleteString( CurSel );
	pTables->SetCurSel( CurSel );
	if (pTables->GetCurSel() == -1)
		pTables->SetCurSel( pTables->GetCount() - 1 );
	OnSelchangeLocomotiontables();
}

void CLocomotionTablesDlg::OnKillfocusName() 
{
	CListBox* pTables = (CListBox*)GetDlgItem( IDC_LOCOMOTIONTABLES );
	CString Name;
	GetDlgItemText( IDC_NAME, Name );

	if (Name.GetLength() == 0)
	{
		MessageBox( "Must give the table a name.", "ERROR" );
		GetDlgItem( IDC_NAME )->SetFocus();
		return;
	}

	//---	make sure that there are no other tables with the same name
	if ((pTables->FindStringExact( -1, Name ) != pTables->GetCurSel())
		&& (pTables->FindStringExact( pTables->GetCurSel(), Name ) != -1))
	{
		MessageBox( "New name matches the name of an existing table.", "ERROR" );
		GetDlgItem( IDC_NAME )->SetFocus();
		return;
	}

	CLocoAnimTable* pTable;
	pTable = (CLocoAnimTable*) pTables->GetItemData( pTables->GetCurSel() );
	pTable->m_Name = Name;

	//---	update the tables lists
	pTables->DeleteString( pTables->GetCurSel() );
	pTables->AddString( Name );
	int Index = pTables->FindStringExact( -1, Name );
	pTables->SetItemData( Index, (unsigned int) pTable );
	pTables->SetCurSel( Index );
}

void CLocomotionTablesDlg::OnChangeName() 
{
	CListBox* pTables = (CListBox*)GetDlgItem( IDC_LOCOMOTIONTABLES );
	CString Name;
	GetDlgItemText( IDC_NAME, Name );

	if (Name.GetLength() == 0)
		Name = "##########";

	//---	make sure that there are no other tables with the same name
	if ((pTables->FindStringExact( -1, Name ) != pTables->GetCurSel())
		&& (pTables->FindStringExact( pTables->GetCurSel(), Name ) != -1))
		return;

	CLocoAnimTable* pTable;
	pTable = (CLocoAnimTable*) pTables->GetItemData( pTables->GetCurSel() );
	pTable->m_Name = Name;

	//---	update the tables lists
	pTables->DeleteString( pTables->GetCurSel() );
	pTables->AddString( Name );
	int Index = pTables->FindStringExact( -1, Name );
	pTables->SetItemData( Index, (unsigned int) pTable );
	pTables->SetCurSel( Index );
}

void CLocomotionTablesDlg::OnPointforeward() 
{
	m_pCurLocoTable->m_Orientation = ORIENTATION_POINTFOREWARD;
	SetOrientation();
}

void CLocomotionTablesDlg::OnSideforeward() 
{
	m_pCurLocoTable->m_Orientation = ORIENTATION_SIDEFOREWARD;
	SetOrientation();
}

void CLocomotionTablesDlg::OnPhase() 
{
	m_pCurLocoTable->m_Orientation = ORIENTATION_PHASESHIFT;
	SetOrientation();
}

void CLocomotionTablesDlg::OnDeltaposPhaseshiftspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	float Value;
	CString Text;
	
	float Change = -1.0f*(float)pNMUpDown->iDelta;

	GetDlgItemText( IDC_PHASESHIFT, Text );
	Value = StringToFloat( Text );
	Value += Change;
	if (Value > 360.0f) Value = 360.0f;
	else if (Value < -360.0f) Value = -360.0f;
	Text.Format( "%6.2f", Value );
	SetDlgItemText( IDC_PHASESHIFT, Text );

	//---	store the phase shift
	m_pCurLocoTable->m_PhaseShift = DEG_TO_RAD(Value);
	RedrawTable();
	
	*pResult = 0;
}

void CLocomotionTablesDlg::OnOK( void )
{
	if (m_pCurLocoTable)
		OnKillfocusCallback();

	CDialog::OnOK();
}

void CLocomotionTablesDlg::OnKillfocusPhaseshift() 
{
	float Value;
	CString Text;
	
	GetDlgItemText( IDC_PHASESHIFT, Text );
	Value = StringToFloat( Text );
	if (Value > 360.0f) Value = 360.0f;
	else if (Value < -360.0f) Value = -360.0f;
	Text.Format( "%6.2f", Value );
	SetDlgItemText( IDC_PHASESHIFT, Text );
	RedrawTable();

	//---	store the phase shift
	m_pCurLocoTable->m_PhaseShift = DEG_TO_RAD(Value);
}

void CLocomotionTablesDlg::OnDeltaposMaxnsrspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	float Value;
	CString Text;
	
	float Change = -0.01f*(float)pNMUpDown->iDelta;

	GetDlgItemText( IDC_MAXNSR, Text );
	Value = StringToFloat( Text );
	Value += Change;
	if (Value > 1.0f) Value = 1.0f;
	else if (Value < 0.0f) Value = 0.0f;
	Text.Format( "%5.3f", Value );
	SetDlgItemText( IDC_MAXNSR, Text );

	//---	deselect whatever is selected
	SetSelected( -1, -1, -1, FALSE, FALSE );

	//---	store the new MaxNSR value
	m_pCurLocoTable->m_MaxNSR = Value;
	RedrawTable();
	
	*pResult = 0;
}

void CLocomotionTablesDlg::OnKillfocusMaxnsr() 
{
	float Value;
	CString Text;
	
	GetDlgItemText( IDC_MAXNSR, Text );
	Value = StringToFloat( Text );
	if (Value > 1.0f) Value = 1.0f;
	else if (Value < 0.0f) Value = 0.0f;
	Text.Format( "%5.3f", Value );
	SetDlgItemText( IDC_MAXNSR, Text );

	//---	store the new MaxNSR value
	m_pCurLocoTable->m_MaxNSR = Value;
	RedrawTable();
}


void CLocomotionTablesDlg::OnChangeOverlaparc() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void CLocomotionTablesDlg::OnKillfocusOverlaparc() 
{
	float Value;
	CString Text;
	
	GetDlgItemText( IDC_OVERLAPARC, Text );
	Value = StringToFloat( Text );
	if (Value > 0.2f) Value = 0.2f;
	else if (Value < -0.2f) Value = -0.2f;
	Text.Format( "%6.3f", Value );
	SetDlgItemText( IDC_OVERLAPARC, Text );

	//---	store the new MaxNSR value
	m_pCurLocoTable->m_OverlapArc = Value;
	RedrawTable();
}

void CLocomotionTablesDlg::OnDeltaposOverlaparcspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	float Value;
	CString Text;
	
	float Change = -0.01f*(float)pNMUpDown->iDelta;

	GetDlgItemText( IDC_OVERLAPARC, Text );
	Value = StringToFloat( Text );
	Value += Change;
	if (Value > 0.2f) Value = 0.2f;
	else if (Value < -0.2f) Value = -0.2f;
	Text.Format( "%6.3f", Value );
	SetDlgItemText( IDC_OVERLAPARC, Text );

	//---	deselect whatever is selected
	SetSelected( -1, -1, -1, FALSE, FALSE );

	//---	store the new MaxNSR value
	m_pCurLocoTable->m_OverlapArc = Value;
	RedrawTable();
	
	*pResult = 0;
}

void CLocomotionTablesDlg::OnChangeOverlap() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void CLocomotionTablesDlg::OnKillfocusOverlap() 
{
	float Value;
	CString Text;
	
	GetDlgItemText( IDC_OVERLAP, Text );
	Value = StringToFloat( Text );
	if (Value > 0.2f) Value = 0.2f;
	else if (Value < -0.2f) Value = -0.2f;
	Text.Format( "%6.3f", Value );
	SetDlgItemText( IDC_OVERLAP, Text );

	//---	store the new MaxNSR value
	m_pCurLocoTable->m_OverlapNSR = Value;
	RedrawTable();
}


void CLocomotionTablesDlg::OnDeltaposOverlapspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	float Value;
	CString Text;
	
	float Change = -0.01f*(float)pNMUpDown->iDelta;

	GetDlgItemText( IDC_OVERLAP, Text );
	Value = StringToFloat( Text );
	Value += Change;
	if (Value > 0.2f) Value = 0.2f;
	else if (Value < -0.2f) Value = -0.2f;
	Text.Format( "%6.3f", Value );
	SetDlgItemText( IDC_OVERLAP, Text );

	//---	deselect whatever is selected
	SetSelected( -1, -1, -1, FALSE, FALSE );

	//---	store the new MaxNSR value
	m_pCurLocoTable->m_OverlapNSR = Value;
	RedrawTable();
	
	*pResult = 0;
}

void CLocomotionTablesDlg::RedrawTable()
{
	if (this->GetSafeHwnd() != NULL)
	{
		InvalidateRect( &m_TableRect, FALSE );
		m_DrawTableOnly = TRUE;
	}
}

void CLocomotionTablesDlg::RedrawAnimKey()
{
	if (this->GetSafeHwnd() != NULL)
	{
		InvalidateRect( &m_AnimBrushKeyRect, FALSE );
		m_DrawAnimKeyOnly = TRUE;
	}
}


////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::OnReorientanimchange() 
{
	CListBox* pUsedAnims=(CListBox*)GetDlgItem( IDC_USEDANIMS );
	CAnimIDSelectionDlg		AnimIDSelectionDlg;

	AnimIDSelectionDlg.m_pDoc = this->m_pDoc;

	if (AnimIDSelectionDlg.DoModal() == IDOK)
	{
		m_pCurLocoTable->m_ReorientAnim = AnimIDSelectionDlg.m_SelectedAnim;
		SetDlgItemText( IDC_REORIENTANIM, m_pCurLocoTable->m_ReorientAnim );
	}
}

////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::OnAddanim() 
{
	CListBox* pUsedAnims=(CListBox*)GetDlgItem( IDC_USEDANIMS );
	CAnimIDSelectionDlg		AnimIDSelectionDlg;
	int i;

	AnimIDSelectionDlg.m_pDoc = this->m_pDoc;

	if (AnimIDSelectionDlg.DoModal() == IDOK)
	{
		//---	look for this string
		for( i=0; i<m_pCurLocoTable->m_AnimationList.GetSize(); i++ )
			if (m_pCurLocoTable->m_AnimationList[i] == AnimIDSelectionDlg.m_SelectedAnim )
				break;

		//---	if it was not found, add it
		if (i==m_pCurLocoTable->m_AnimationList.GetSize())
		{
			m_pCurLocoTable->InsertAnim( pUsedAnims->GetCurSel(), AnimIDSelectionDlg.m_SelectedAnim );
			SetUsedAnimations();
			RedrawAnimKey();
			RedrawTable();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::OnRemoveanim() 
{
	CListBox* pUsedAnims=(CListBox*)GetDlgItem( IDC_USEDANIMS );
	CString Anim;
	int AnimIndex;
	
	//---	if there is no selection return
	if (pUsedAnims->GetCurSel() == -1)
		return;

	//---	remove the item from the list
	AnimIndex = pUsedAnims->GetCurSel();
	pUsedAnims->GetText( AnimIndex, Anim );
	m_pCurLocoTable->RemoveAnim( Anim );
	SetUsedAnimations();
	RedrawAnimKey();
	RedrawTable();
}


////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::OnChangeanim() 
{
	CListBox* pUsedAnims=(CListBox*)GetDlgItem( IDC_USEDANIMS );
	CAnimIDSelectionDlg		AnimIDSelectionDlg;
	int i;

	if (pUsedAnims->GetCurSel() == -1)
		return;

	AnimIDSelectionDlg.m_pDoc = this->m_pDoc;

	if (AnimIDSelectionDlg.DoModal() == IDOK)
	{
		//---	look for this string
		for( i=0; i<m_pCurLocoTable->m_AnimationList.GetSize(); i++ )
			if (m_pCurLocoTable->m_AnimationList[i] == AnimIDSelectionDlg.m_SelectedAnim )
				break;

		//---	if it was not found, add it
		if (i==m_pCurLocoTable->m_AnimationList.GetSize())
		{
			m_pCurLocoTable->ChangeAnim( m_pCurLocoTable->m_AnimationList[pUsedAnims->GetCurSel()], AnimIDSelectionDlg.m_SelectedAnim );
			SetUsedAnimations();
			RedrawAnimKey();
			RedrawTable();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::OnSelchangeUsedanims() 
{
	//---	if lines are selected, then return now
	if (m_LineSelected)
		return;

	//---	set the selected zones to the currently selected animation
	SetSelectedValue( (float)((CListBox*) GetDlgItem( IDC_USEDANIMS ))->GetCurSel() );
}

////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::CopyArc( int Arc )
{
	POSITION pos = m_pCurLocoTable->m_ArcDataList.FindIndex( Arc );
	CLocoArcData* pArc = m_pCurLocoTable->m_ArcDataList.GetAt( pos );

	//---	make the new arc
	CLocoArcData* pNewArc = new CLocoArcData;

	//---	set the new arc up with the previous arc zones
	*pNewArc = *pArc;

	//---	set the new arcs angle
	float Min, Max, New;
	if (m_pCurLocoTable->m_ArcDataList.GetCount() == 1)
		Min = 0.0f;
	else if (Arc == 0)
		Min = m_pCurLocoTable->m_ArcDataList.GetTail()->m_Angle ;
	else
		Min = m_pCurLocoTable->m_ArcDataList.GetAt( m_pCurLocoTable->m_ArcDataList.FindIndex( Arc - 1 ))->m_Angle ;
	Max = pArc->m_Angle;
	while (Min >= Max) Min -= m_2Pi;
	New = (Max - Min)/2 + Min;
	while (New < 0.0f) New += m_2Pi;
	pNewArc->m_Angle = New;

	//---	add the new arc into the arc list
	m_pCurLocoTable->m_ArcDataList.InsertBefore( pos, pNewArc );
}

////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::CopyZone( int Arc, int Zone )
{
	POSITION apos = m_pCurLocoTable->m_ArcDataList.FindIndex( Arc );
	CLocoArcData* pArc = m_pCurLocoTable->m_ArcDataList.GetAt( apos );

	//---	return if the given zone does not exist
	if (pArc->m_AnimNSRList.GetCount() <= Zone)
		return;

	POSITION pos = pArc->m_AnimNSRList.FindIndex( m_SelectedZone );
	CLocoAnimNSR* pZone = pArc->m_AnimNSRList.GetAt( pos );

	//---	make the new arc
	CLocoAnimNSR* pNewZone = new CLocoAnimNSR;

	//---	set the new arc up with the previous arc zones
	*pNewZone = *pZone;

	//---	set the new arcs angle
	float Min, Max, New;
	if ((pArc->m_AnimNSRList.GetCount() == 1) || (m_SelectedZone == 0))
		Min = m_pCurLocoTable->m_CenterNSR;
	else
		Min = pArc->m_AnimNSRList.GetAt( pArc->m_AnimNSRList.FindIndex( m_SelectedZone - 1 ))->m_NSR ;
	Max = pZone->m_NSR;
	ASSERT(Max > Min);
	New = (Max - Min)/2 + Min;
	pNewZone->m_NSR = New;

	//---	add the new arc into the arc list
	pArc->m_AnimNSRList.InsertBefore( pos, pNewZone );
}

////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::DeleteArc( int Arc )
{
	//---	cannot delete what is not there
	if (m_pCurLocoTable->m_ArcDataList.GetCount() <= Arc)
		return;

	//---	remove the arc
	POSITION pos = m_pCurLocoTable->m_ArcDataList.FindIndex( Arc );
	CLocoArcData* pArc = m_pCurLocoTable->m_ArcDataList.GetAt( pos );
	m_pCurLocoTable->m_ArcDataList.RemoveAt( pos );
	delete pArc;

	if (m_pCurLocoTable->m_ArcDataList.GetCount() == 0 )
	{
		m_pCurLocoTable->m_CenterNSR = m_pCurLocoTable->m_MaxNSR;
		SetSelected( -1, -1, -1, m_LineSelected, FALSE );
	}
	else if (m_SelectedArc == m_pCurLocoTable->m_ArcDataList.GetCount())
		SetSelected( m_SelectedArc-1, m_SelectedArc-1, m_SelectedZone, m_LineSelected, FALSE );
}

////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::DeleteZone( int Arc, int Zone )
{
	//---	cannot delete what is not there
	if (m_pCurLocoTable->m_ArcDataList.GetCount() <= Arc)
		return;

	//---	if this is the last zone, remove the arc
	if (m_pCurLocoTable->m_ArcDataList.GetCount() == 0)
	{
		DeleteArc( Arc );
		return;
	}

	//---	get the arc
	POSITION apos = m_pCurLocoTable->m_ArcDataList.FindIndex( Arc );
	CLocoArcData* pArc = m_pCurLocoTable->m_ArcDataList.GetAt( apos );

	//---	return if the given zone does not exist
	if (pArc->m_AnimNSRList.GetCount() <= Zone)
		return;

	//---	remove the zone
	POSITION pos = pArc->m_AnimNSRList.FindIndex( m_SelectedZone );
	CLocoAnimNSR* pZone = pArc->m_AnimNSRList.GetAt( pos );
	pArc->m_AnimNSRList.RemoveAt( pos );
	delete pZone;
}

////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::OnPlus() 
{
	int i;

	//---	Split whatever is selected into two equal parts

	//---	if the center is selected, return
	if (m_CenterSelected)
		return;

	//---	determine if all the zones of an arc are selected
	if (m_SelectedArc != -1)
	{
		POSITION apos = m_pCurLocoTable->m_ArcDataList.FindIndex( m_SelectedArc );
		CLocoArcData* pArc = m_pCurLocoTable->m_ArcDataList.GetAt( apos );

		if ((m_SelectedZone == -1) || ((m_SelectedZone == 0) && (pArc->m_AnimNSRList.GetCount() == 1)))
			CopyArc( m_SelectedArc );
		else
			CopyZone( m_SelectedArc, m_SelectedZone );
	}
	else if (m_SelectedZone != -1)
	{
		//---	make copies of this respecting zone in all of the arcs
		for (i=0; i<m_pCurLocoTable->m_ArcDataList.GetCount(); i++ )
			CopyZone( i, m_SelectedZone );
	}

	//---	we will want to redraw the table after this
	RedrawTable();
}

////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::OnMinus() 
{
	int i;

	//---	Remove whatever is selected

	//---	if the center is selected, return
	if (m_CenterSelected)
		return;

	//---	determine if all the zones of an arc are selected
	if (m_SelectedArc != -1)
	{
		POSITION apos = m_pCurLocoTable->m_ArcDataList.FindIndex( m_SelectedArc );
		CLocoArcData* pArc = m_pCurLocoTable->m_ArcDataList.GetAt( apos );

		if ((m_SelectedZone == -1) || ((m_SelectedZone == 0) && (pArc->m_AnimNSRList.GetCount() == 1)))
			DeleteArc( m_SelectedArc );
		else
			DeleteZone( m_SelectedArc, m_SelectedZone );
	}
	else if (m_SelectedZone != -1)
	{
		//---	make copies of this respecting zone in all of the arcs
		for (i=0; i<m_pCurLocoTable->m_ArcDataList.GetCount(); i++ )
			DeleteZone( i, m_SelectedZone );
	}

	//---	we will want to redraw the table after this
	RedrawTable();
}

////////////////////////////////////////////////////////////////////////////////

void CLocomotionTablesDlg::OnEqual() 
{
	//---	make whatever is selected equal

	//---	if the center is selected, return
	if (m_CenterSelected)
		return;

	//---	return if nothing is selected
	if (m_SelectedItems.GetCount() == 0)
		return;

	//---	we will want to redraw the table after this
	RedrawTable();

	//---	if this is not a line selection, then they all need to have the same animation
	if (!m_LineSelected)
	{
		POSITION pos = m_SelectedItems.GetHeadPosition();
		float value = m_SelectedItems.GetAt( pos )->Get();
		while( pos ) m_SelectedItems.GetNext( pos )->Set( value );
		return;
	}

	//---	handle each case of line selection specifically
	if ((m_SelectedArc != -1) && (m_SelectedZone == -1))
	{
		float Min, Max, New, Inc;
		Min = m_pCurLocoTable->m_CenterNSR;
		Max = m_pCurLocoTable->m_MaxNSR;
		Inc = (Max - Min)/m_pCurLocoTable->m_ArcDataList.GetCount();
		New = Min;

		POSITION pos = m_SelectedItems.GetHeadPosition();
		while( pos ) m_SelectedItems.GetNext( pos )->Set( New += Inc );
		m_SelectedItems.GetTail()->Set( Max ); // deal with accumulated roundoff error
	}
	else if ((m_SelectedArc == -1) && (m_SelectedZone != -1))
	{
		POSITION pos = m_SelectedItems.GetHeadPosition();
		float value = m_SelectedItems.GetAt( pos )->Get();
		while( pos ) m_SelectedItems.GetNext( pos )->Set( value );
	}
	else if ((m_SelectedArc == -1) && (m_SelectedZone == -1))
	{
		if (m_SelectedItems.GetHead()->m_pSelected->GetType() == LATOBJECT_TYPE_ARC)
		{
			CWordArray SelectedIndecies;

			//---	get a list of selected arc indicies
			SelectedIndecies.SetSize( m_SelectedItems.GetCount() );
			POSITION pos = m_SelectedItems.GetHeadPosition();
			POSITION foundpos;
			CLocoArcData* pArc;
			int count=0,j,i;
			while( pos )
			{
				CLATObject* pSelect=m_SelectedItems.GetNext( pos )->m_pSelected;
				ASSERT(pSelect->GetType() == LATOBJECT_TYPE_ARC);
				pArc = (CLocoArcData*)pSelect;
				foundpos = m_pCurLocoTable->m_ArcDataList.Find( pArc );
				for(i=0; i<m_pCurLocoTable->m_ArcDataList.GetCount(); i++)
					if (foundpos == m_pCurLocoTable->m_ArcDataList.FindIndex( i ))
						break;
				SelectedIndecies[count++] = i;
			}

			//---	sort the array
			for( i=0; i<count; i++ )
				for( j=i+1; j<count; j++ )
					if( SelectedIndecies[i] < SelectedIndecies[j] )
					{
						unsigned short temp = SelectedIndecies[i];
						SelectedIndecies[i] = SelectedIndecies[j];
						SelectedIndecies[j] = temp;
					}

			//---	deterine the start and step size of arcs
			//---	always work from the last arc counter clockwize (keeps last arc on m_2Pi)
			pos = m_pCurLocoTable->m_ArcDataList.FindIndex( SelectedIndecies[0] );//count-1] );
			float Start = m_pCurLocoTable->m_ArcDataList.GetAt( pos )->m_Angle;
			float Inc = m_2Pi / (float)count;

			float check = Start - (Inc*(count - 1));
			if (check < 0.0f)
				Start -= check;

			//---	set the arc angles
			j = 0;
			for( i=m_pCurLocoTable->m_ArcDataList.GetCount()-1; i>=0; i-- )
			{
				int smallcount = i - SelectedIndecies[j];
				int v;

				//---	if this is the last section, make sure that it completes by 0.0f
				if (j == (count-1))
					Inc = Start / smallcount;

				//---	set the sub arcs which exist between the selected arcs
				for( v=0; v<smallcount; v++ )
				{
					pos = m_pCurLocoTable->m_ArcDataList.FindIndex( i );
					pArc = m_pCurLocoTable->m_ArcDataList.GetAt( pos );
					
					pArc->m_Angle = Start - v*(Inc/(float)count);
					i--;
				}

				//---	set the selected arc's angle
				pos = m_pCurLocoTable->m_ArcDataList.FindIndex( i );
				pArc = m_pCurLocoTable->m_ArcDataList.GetAt( pos );
				pArc->m_Angle = Start;
				Start -= Inc;
			}
		}
		else
		{
			POSITION pos = m_SelectedItems.GetHeadPosition();
			float value = m_SelectedItems.GetAt( pos )->Get();
			while( pos ) m_SelectedItems.GetNext( pos )->Set( value );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//					MOUSE INTERFACE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
bool CLocomotionTablesDlg::MouseButtonPressed( int WhichButton, bool DoubleClick, bool Shift, bool Ctrl, CPoint Point )
{
	float Angle;
	int Radius;
	float NSR;
	float LineAngOff;
	int LineRadOff;
	float AngDist1;
	float AngDist2;
	float ZoneRadius;
	POSITION pos;
	CLocoArcData* pArc2;
	CLocoArcData* pArc;
	int Arc;
	CLocoAnimNSR* pZone;
	CLocoAnimNSR* pZone2;
	int Zone;
	float OverlapArc = m_pCurLocoTable ? m_pCurLocoTable->m_OverlapArc : 0.0f;
	bool LineOnly=Ctrl;
	bool AreaOnly=Shift;

	if (m_pCurLocoTable == NULL)
		return FALSE;

	if( m_pCurLocoTable->m_bDoNotExport )
		return FALSE;

	int Overlap = (int)((float)m_ArcRadius*m_pCurLocoTable->m_OverlapNSR);
	int OverlapMin = (Overlap < 0) ? Overlap : 0;
	int OverlapMax = (Overlap > 0) ? Overlap : 0;

	//---	check to determine if the color key was hit
	if (m_AnimBrushKeyRect.PtInRect( Point ))
	{
		//---	if lines are selected, do nothing
		if (!m_LineSelected)
		{
			//---	determine which one was hit
			int Anim = ((CListBox*)GetDlgItem( IDC_USEDANIMS ))->GetTopIndex() + (int)(Point.y - m_AnimBrushKeyRect.top)/BRUSH_HEIGHT;
			SetSelectedValue( (float)Anim );
			((CListBox*)GetDlgItem( IDC_USEDANIMS ))->SetCurSel( Anim );
		}
		return TRUE;
	}

	//---	determine if the mouse was pressed within the table display
	if (!m_TableRect.PtInRect( Point ))
	{
		return FALSE;
	}

	//---	deselect any dialog controls with focus
	SetFocus();

	//---	if there is no current table, return now
	if (m_pCurLocoTable == NULL)
		return TRUE;

	//---	capture the mouse unless this is a double click
	if (!DoubleClick)
		SetCapture();
	m_MouseLastPos = Point;

	//---	get the click points to coordinates reletive to the table
	Point.x -= m_ArcCenter.x;
	Point.y -= m_ArcCenter.y;

	//---	calculate the angle and radius of the button press
	Angle = (float)atan2( (double)Point.x, (double)Point.y );
	Angle -= m_pCurLocoTable->m_PhaseShift - (m_pCurLocoTable->m_OverlapArc/2.0f) + TABLE_PHASE_SHIFT;
	while(Angle < 0.0f) Angle += m_2Pi;
	Radius = (int)sqrt( (double)(Point.x*Point.x + Point.y*Point.y) );
	NSR = (float)Radius / (float)m_ArcRadius;

	//---	reject this press if it is substantially outside the circle
	if (Radius > (m_ArcRadius+6))
	{
		SetSelected( -1, -1, -1, FALSE, FALSE );
		return TRUE;
	}

	//---	determine if the center was selected
	float CenterRad = m_pCurLocoTable->m_CenterNSR * m_ArcRadius;
	if (Radius < (CenterRad+OverlapMax+(AreaOnly?0:2)))
	{
		//---	no special effect from double clicking the center
		if (DoubleClick)
			return TRUE;

		//---	determine if the line or the center was selected
		if (LineOnly || (!AreaOnly && (Radius >= (CenterRad + OverlapMin - 3))))
		{
			SetSelected( -1, -1, -1, TRUE, TRUE );
			m_CursorRadiusOffset = Radius - (int)CenterRad;
		}
		else
			SetSelected( -1, -1, -1, FALSE, TRUE );

		return TRUE;
	}

	//---	determine which angle of the table was clicked within
	pos = m_pCurLocoTable->m_ArcDataList.GetHeadPosition();
	Arc = 0;
	int NumArcs = m_pCurLocoTable->m_ArcDataList.GetCount();
	while( pos )
	{
		pArc = m_pCurLocoTable->m_ArcDataList.GetNext( pos );
		if (Angle <= pArc->m_Angle)
			break;

		pArc2 = pArc;
		Arc++;
	}
	if (Arc == 0)
		pArc2 = m_pCurLocoTable->m_ArcDataList.GetAt( m_pCurLocoTable->m_ArcDataList.FindIndex( NumArcs-1 ) );
	else if (Arc == NumArcs)
		pArc = m_pCurLocoTable->m_ArcDataList.GetHead();

	//---	determine which zone of the arc was clicked within
	pos = pArc->m_AnimNSRList.GetHeadPosition();
	Zone = 0;
	pZone2 = NULL;
	int NumZones = pArc->m_AnimNSRList.GetCount();
	while( pos )
	{
		pZone = pArc->m_AnimNSRList.GetNext( pos );
		if (NSR <= pZone->m_NSR)
			break;

		pZone2 = pZone;
		Zone++;
	}
	if (Zone == NumZones)
		pZone = NULL;

	if (Arc == NumArcs) Arc = 0;

	//---	determine if the click hit a line
	if (!AreaOnly)
	{
		//---	determine the degree of error in selecting a line
		if (LineOnly)
		{
			LineAngOff = 10000.0f;
			LineRadOff = 10000;
		}
		else
		{
			LineAngOff = m_2Pi/200.0f;
			LineRadOff = 2;
		}

		//---	determine which line was hit
		if (NumArcs > 1)
		{
			AngDist1 = ABS(pArc->m_Angle - Angle);
			AngDist2 = ABS(pArc2->m_Angle - Angle);
			if ((AngDist1 < LineAngOff) || (AngDist2 < (LineAngOff+OverlapArc)))
			{
				if (Arc == NumArcs) Arc = 0;

				//---	if it is a double click, select all arcs
				if (AngDist1 <= AngDist2)
					SetSelected( Arc, DoubleClick ? -1 : Arc, -1, TRUE, FALSE );
				else
					SetSelected( Arc-1, DoubleClick ? -1 : Arc-1, -1, TRUE, FALSE );

				return TRUE;
			}
		}

		//---	if no zone was clicked, then assume that the last zone line was clicked
		if (pZone == NULL)
		{
			//---	if it is a double click, select all zones
			SetSelected( Arc, DoubleClick ? -1 : Arc, NumZones-1, TRUE, FALSE );

			m_CursorRadiusOffset = Radius - (int)((float)m_ArcRadius*m_pCurLocoTable->m_MaxNSR);
			return TRUE;
		}

		//---	dont shift the zone radius for the last zone
		if ((Zone == (NumZones-1)) || (Zone == NumZones))
			OverlapMin = 0;

		//---	check for zone1's line to have been clicked
		ZoneRadius = m_ArcRadius*pZone->m_NSR;
		if (Radius > (ZoneRadius - 2 + OverlapMin))
		{
			//---	if it is a double click, select all zones
			SetSelected( Arc, DoubleClick ? -1 : Arc, Zone, TRUE, FALSE );
			m_CursorRadiusOffset = Radius - (int)ZoneRadius;
			return TRUE;
		}

		//---	if there is a pZone2, then check for pZone2's line to have been clicked
		if (pZone2)
		{
			ZoneRadius = m_ArcRadius*pZone2->m_NSR;
			if (Radius < (ZoneRadius +2 + OverlapMax))
			{
				//---	if it is a double click, select all zones
				SetSelected( Arc, DoubleClick ? -1 : Arc, Zone-1, TRUE, FALSE );
				m_CursorRadiusOffset = Radius - (int)ZoneRadius;
				return TRUE;
			}
		}
/*
		if (pZone2)
			Dist2 = ABS(m_ArcRadius*pZone2->m_NSR - Radius);
		else
			Dist2 = (float)LineRadOff + 1.0f; // this is not a facter

		if ((Dist1 < (float)LineRadOff) || (Dist2 < LineRadOff))
		{
			if (Dist1 <= Dist2)
			{
				SetSelected( Arc, Zone, TRUE, FALSE );
				return TRUE;
			}
			else
			{
				SetSelected( Arc, Zone-1, TRUE, FALSE );
				return TRUE;
			}
		}
*/
	}

	//---	an area was selected

	//---	if no zone was hit, set the zone to a -1, this will let the user click just outside all the zones to select an entire arc
	if (pZone == NULL) Zone = -1;

	//---	make the selection
	SetSelected( Arc, DoubleClick ? -1 : Arc, Zone, ((Arc != -1) && (Zone == -1) && !AreaOnly) ? TRUE : FALSE, FALSE );

	return TRUE;
}

void CLocomotionTablesDlg::OnLButtonDown(UINT nFlags, CPoint Point) 
{
	bool LineOnly=(nFlags&MK_CONTROL)?TRUE:FALSE;
	bool AreaOnly=(nFlags&MK_SHIFT)?TRUE:FALSE;

	if (!CLocomotionTablesDlg::MouseButtonPressed( 0, FALSE, (nFlags&MK_SHIFT)?TRUE:FALSE, (nFlags&MK_CONTROL)?TRUE:FALSE, Point ))
		CDialog::OnLButtonDown(nFlags, Point);
}

void CLocomotionTablesDlg::OnLButtonDblClk(UINT nFlags, CPoint Point) 
{
	bool LineOnly=(nFlags&MK_CONTROL)?TRUE:FALSE;
	bool AreaOnly=(nFlags&MK_SHIFT)?TRUE:FALSE;

	if (!CLocomotionTablesDlg::MouseButtonPressed( 0, TRUE, (nFlags&MK_SHIFT)?TRUE:FALSE, (nFlags&MK_CONTROL)?TRUE:FALSE, Point ))
		CDialog::OnLButtonDblClk(nFlags, Point);
}

void CLocomotionTablesDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if (GetCapture() == this)
	{
		CLocoArcData* pArc;
		CLocoAnimNSR* pAnimNSR;
		POSITION pos, pos2, pos3;
		float	MaxNSR = m_pCurLocoTable->m_MaxNSR;
		float	CurMaxNSR;
		bool	Remove;

		ReleaseCapture();

		//---	if the center has been moved to fill the entire table, limit it and get out
		if (m_pCurLocoTable->m_CenterNSR >= MaxNSR)
		{
			m_pCurLocoTable->m_CenterNSR = MaxNSR;
/*			pos = m_pCurLocoTable->m_ArcDataList.GetHeadPosition();
			while (pos != NULL)
			{
				pArc = m_pCurLocoTable->m_ArcDataList.GetNext( pos );
				pArc->EmptyList();
			}*/

			//---	we will want to redraw the table after this
			RedrawTable();

			m_pCurLocoTable->EmptyList();
			return;
		}
		else if (m_pCurLocoTable->m_ArcDataList.GetCount() == 0)
		{
			//---	add an arc above the center arc
			pArc = new CLocoArcData;
			m_pCurLocoTable->m_ArcDataList.AddTail( pArc );

			//---	we will want to redraw the table after this
			RedrawTable();
		}

		//---	make sure that all the zones exist all the way to the m_NSR value
		pos = m_pCurLocoTable->m_ArcDataList.GetHeadPosition();
		while (pos != NULL)
		{
			Remove = FALSE;
			CurMaxNSR = m_pCurLocoTable->m_CenterNSR;
			pArc = m_pCurLocoTable->m_ArcDataList.GetNext( pos );
			pos2 = pArc->m_AnimNSRList.GetHeadPosition();
			while (pos2)
			{
				pos3 = pos2;
				pAnimNSR = pArc->m_AnimNSRList.GetNext( pos2 );
				if (Remove || (pAnimNSR->m_NSR <= CurMaxNSR))
				{
					delete pAnimNSR;
					pArc->m_AnimNSRList.RemoveAt( pos3 );

					//---	we will want to redraw the table after this
					RedrawTable();
				}
				else
				{
					CurMaxNSR = pAnimNSR->m_NSR;
					if (pAnimNSR->m_NSR >= MaxNSR)
					{
						//---	set the max for this zone
						pAnimNSR->m_NSR = MaxNSR;
						Remove = TRUE;
					}
				}
			}

			//---	if the zones of this arc do not stretch to the MaxNSR, add another zone
			if (CurMaxNSR < MaxNSR)
			{
				pAnimNSR = new CLocoAnimNSR;
				pAnimNSR->m_Anim = -1;
				pAnimNSR->m_NSR = MaxNSR;
				pArc->m_AnimNSRList.AddTail( pAnimNSR );

				//---	we will want to redraw the table after this
				RedrawTable();
			}
		}
	}
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CLocomotionTablesDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	if( !m_pCurLocoTable )
		return;

	if( m_pCurLocoTable->m_bDoNotExport )
		return;

	if (nFlags & MK_LBUTTON)
	{
		if (m_SelectedItems.GetCount() == 0)
			return;

		if (m_LineSelected)
		{
			point.x -= m_ArcCenter.x;
			point.y -= m_ArcCenter.y;

			//---	if an arc angle is selected, calculate the new angle for the selected arc
			if ((m_SelectedZone != -1) || m_CenterSelected)
			{
				//---	get the new radius
				int Radius = (int)sqrt( (double)(point.x*point.x + point.y*point.y) );
				float NSR = (float)(Radius - m_CursorRadiusOffset) / (float)m_ArcRadius;

				//---	get the current radius of the first selected zone
				float OldNSR = m_SelectedItems.GetHead()->Get();

				IncSelectedValue( NSR - OldNSR );
			}
			else if (m_SelectedZone == -1)
			{
				//---	get the angle from the cursor to the center
				float Angle = (float)atan2( (double)point.x, (double)point.y );
				Angle -= m_pCurLocoTable->m_PhaseShift - (m_pCurLocoTable->m_OverlapArc/2.0f) + TABLE_PHASE_SHIFT;
				while(Angle < 0.0f) Angle += m_2Pi;

				//---	get the current angle of the first selected arc
				float OldAngle = m_SelectedItems.GetHead()->Get();

				IncSelectedValue( Angle - OldAngle );
			}
		}
		else
		{
			if (ABS(m_MouseLastPos.y - point.y) >= m_MouseSensitivity)
			{
				int Incs = (point.y - m_MouseLastPos.y) / m_MouseSensitivity;
				int NewAnim = (int)m_SelectedItems.GetHead()->Get() + Incs;


				SetSelectedValue( (float)NewAnim );
				m_MouseLastPos.y += Incs*m_MouseSensitivity;
			}
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}

BOOL CLocomotionTablesDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	IncSelected( -zDelta/WHEEL_DELTA );
	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CLocomotionTablesDlg::OnDeltaposValuespin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	float Value;
	CString Text;
	bool RadiusSelected = m_CenterSelected || (m_SelectedZone != -1);
	 
	*pResult = 0;

	if (m_SelectedItems.GetCount() == 0)
		return;

	float Change = 0.01f*(float)pNMUpDown->iDelta;

	GetDlgItemText( IDC_VALUE, Text );
	Value = StringToFloat( Text );
	if (!m_Radian.GetCheck() && !RadiusSelected) Value = DEG_TO_RAD( Value );
	Value += Change;
	SetSelectedValue( Value );
	Value = m_SelectedItems.GetHead()->Get();
	if (!m_Radian.GetCheck() && !RadiusSelected) Value = RAD_TO_DEG( Value );
	if (Value <= 1.0f)			Text.Format( "%6.4f", Value );
	else if (Value <= 10.0f)	Text.Format( "%6.3f", Value );
	else						Text.Format( "%6.2f", Value );
	SetDlgItemText( IDC_VALUE, Text );

	RedrawTable();
	
	*pResult = 0;
}

void CLocomotionTablesDlg::OnChangeValue() 
{
}


void CLocomotionTablesDlg::OnKillfocusValue() 
{
	float Value;
	CString Text;
	static InThis = FALSE;
	bool RadiusSelected = m_CenterSelected || (m_SelectedZone != -1);//((m_SelectedZone != -1) || (m_SelectedArc == -1));

	if (!m_LineSelected)
		return;

	if (m_SelectedItems.GetCount() == 0)
		return;

	if (InThis) return;
	InThis = TRUE;
	
	//---	get the value from the dialog control
	GetDlgItemText( IDC_VALUE, Text );
	Value = StringToFloat( Text );

	//---	set the selected value
	if (!m_Radian.GetCheck() && !RadiusSelected)
		Value = DEG_TO_RAD( Value );
	SetSelectedValue( Value );

	//---	reset the dialog control to the new value which may have been modified
	Value = m_SelectedItems.GetHead()->Get();
	if (!m_Radian.GetCheck() && !RadiusSelected)
		Value = RAD_TO_DEG( Value );
	if (Value <= 1.0f)			Text.Format( "%6.4f", Value );
	else if (Value <= 10.0f)	Text.Format( "%6.3f", Value );
	else						Text.Format( "%6.2f", Value );
	SetDlgItemText( IDC_VALUE, Text );

	RedrawTable();

	InThis = FALSE;
}


void CLocomotionTablesDlg::OnRadian() 
{
	float Value;
	CString Text;

	if (!m_LineSelected)
		return;

	if (m_SelectedItems.GetCount() == 0)
		return;

	if (m_CenterSelected || (m_SelectedZone != -1))
		return;

	Value = m_SelectedItems.GetHead()->Get();
	if ( !m_Radian.GetCheck() )
		Value = RAD_TO_DEG( Value );

	if (Value <= 1.0f)			Text.Format( "%6.4f", Value );
	else if (Value <= 10.0f)	Text.Format( "%6.3f", Value );
	else						Text.Format( "%6.2f", Value );
	SetDlgItemText( IDC_VALUE, Text );
}

void CLocomotionTablesDlg::SetVScroll( UINT nPos, CScrollBar* pScrollBar )
{
	CListBox* pUsedAnims = (CListBox*) GetDlgItem( IDC_USEDANIMS );
	int ScrollMin, ScrollMax;
	pScrollBar->GetScrollRange( &ScrollMin, &ScrollMax );

	if (pScrollBar != GetDlgItem( IDC_USEDANIMSBAR ))
		return;

	pUsedAnims->SetTopIndex( nPos );

	pScrollBar->SetScrollPos( pUsedAnims->GetTopIndex() );
	RedrawAnimKey();
}

void CLocomotionTablesDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CListBox* pUsedAnims = (CListBox*) GetDlgItem( IDC_USEDANIMS );
	int ScrollMin, ScrollMax, NewPos;
	pScrollBar->GetScrollRange( &ScrollMin, &ScrollMax );

	if (pScrollBar != GetDlgItem( IDC_USEDANIMSBAR ))
		return;

	switch (nSBCode)
	{
		case SB_TOP:			// Scroll to the top
			pUsedAnims->SetTopIndex( ScrollMin );
			break;

		case SB_BOTTOM:			// Scroll to bottom
			pUsedAnims->SetTopIndex( ScrollMax );
			break;

		case SB_ENDSCROLL:		// End scroll.
			break;

		case SB_LINEDOWN:		// Scroll one line down.
			pUsedAnims->SetTopIndex( pUsedAnims->GetTopIndex()+1 );
			break;

		case SB_LINEUP:			// Scroll one line up.
			pUsedAnims->SetTopIndex( pUsedAnims->GetTopIndex()-1 );
			break;

		case SB_PAGEDOWN:		// Scroll one page down.
			NewPos = pUsedAnims->GetTopIndex() + (int)(m_AnimBrushKeyRect.Height() / BRUSH_HEIGHT)-1;
			if (NewPos > ScrollMax) NewPos = ScrollMax;
			pUsedAnims->SetTopIndex( NewPos );
			break;

		case SB_PAGEUP:			// Scroll one page up.
			NewPos = pUsedAnims->GetTopIndex() - (int)(m_AnimBrushKeyRect.Height() / BRUSH_HEIGHT)-1;
			if (NewPos < ScrollMin) NewPos = ScrollMin;
			pUsedAnims->SetTopIndex( NewPos );
			break;

		case SB_THUMBPOSITION:	// Scroll to the absolute position. The current position is provided in nPos.
			pUsedAnims->SetTopIndex( nPos );
			break;

		case SB_THUMBTRACK:		// Drag scroll box to specified position. The current position is provided in nPos.
			pUsedAnims->SetTopIndex( nPos );
			break;
	}

	pScrollBar->SetScrollPos( pUsedAnims->GetTopIndex() );
	RedrawAnimKey();
	
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CLocomotionTablesDlg::OnReltoactor() 
{
}

void CLocomotionTablesDlg::OnDblclkUsedanims() 
{
	OnChangeanim();	
}

void CLocomotionTablesDlg::OnSelchangeTabletype() 
{
	if( !m_pCurLocoTable )
		return;

	m_pCurLocoTable->m_Type = m_TableTypeListBoxCtrl.GetCurSel();

#ifdef OLDWAY

	if (m_pCurLocoTable->m_Type != TABLETYPE_LOCOMOTION)
	{
		((CButton*)GetDlgItem( IDC_FIXEDONFIELD ))->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_FIXEDONFIELD ))->SetCheck( FALSE );

		((CButton*)GetDlgItem( IDC_FACEDIR ))->EnableWindow( TRUE );
		((CButton*)GetDlgItem( IDC_FACEDIR ))->SetCheck( m_pCurLocoTable->m_Flags & TABLEFLAG_FACEDIR ? TRUE : FALSE );
	}
	else
	{
		((CButton*)GetDlgItem( IDC_FIXEDONFIELD ))->EnableWindow( TRUE );
		((CButton*)GetDlgItem( IDC_FIXEDONFIELD ))->SetCheck( m_pCurLocoTable->m_Flags & TABLEFLAG_FIXED ? TRUE : FALSE );

		((CButton*)GetDlgItem( IDC_FACEDIR ))->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_FACEDIR ))->SetCheck( FALSE );
	}

#else

	switch( m_pCurLocoTable->m_Type )
	{
	case TABLETYPE_LOCOMOTION:
		m_pCurLocoTable->m_IndexAngleStart	= INDEXANGLESTART_DESTFACEDIR;
		m_pCurLocoTable->m_IndexAngleEnd	= INDEXANGLEEND_CURMOVEDIR;
		break;

	case TABLETYPE_CUT:
		m_pCurLocoTable->m_IndexAngleStart	= INDEXANGLESTART_CURMOVEDIR;
		m_pCurLocoTable->m_IndexAngleEnd	= INDEXANGLEEND_DESTMOVEDIR;
		break;

	case TABLETYPE_TURN:
		m_pCurLocoTable->m_IndexAngleStart	= INDEXANGLESTART_CURFACEDIR;
		m_pCurLocoTable->m_IndexAngleEnd	= INDEXANGLEEND_DESTFACEDIR;
		break;

	case TABLETYPE_UNDEFINED:
		m_pCurLocoTable->m_IndexAngleStart	= INDEXANGLESTART_CURMOVEDIR;
		m_pCurLocoTable->m_IndexAngleEnd	= INDEXANGLEEND_DESTFACEDIR;
		break;

	}

	SetIndexAngles();

#endif
}

void CLocomotionTablesDlg::OnFixedonfield() 
{
	if (!m_pCurLocoTable)
		return;

	if (((CButton*)GetDlgItem( IDC_FIXEDONFIELD ))->GetCheck())
		m_pCurLocoTable->m_Flags |=  TABLEFLAG_FIXED;
	else
		m_pCurLocoTable->m_Flags &=  ~TABLEFLAG_FIXED;
}

void CLocomotionTablesDlg::OnFacedir() 
{
	if (!m_pCurLocoTable)
		return;

	if (((CButton*)GetDlgItem( IDC_FACEDIR ))->GetCheck())
		m_pCurLocoTable->m_Flags |=  TABLEFLAG_FACEDIR;
	else
		m_pCurLocoTable->m_Flags &=  ~TABLEFLAG_FACEDIR;
}

void CLocomotionTablesDlg::OnLimitednsr() 
{
	if (!m_pCurLocoTable)
		return;

	if (((CButton*)GetDlgItem( IDC_FACEDIR ))->GetCheck())
		m_pCurLocoTable->m_Flags |=  TABLEFLAG_LIMITNSR;
	else
		m_pCurLocoTable->m_Flags &=  ~TABLEFLAG_LIMITNSR;
}

void CLocomotionTablesDlg::OnKillfocusCallback() 
{
	CString Callback;
	CString Build;

	GetDlgItemText( IDC_CALLBACK, Callback );	

	//---	remove any extra spaces from the callback
	Callback.TrimLeft();
	Callback.TrimRight();

	//---	remove illegal characters
	Build = Callback.SpanIncluding( "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );
	while (Build != Callback)
	{
		if (Callback[ Build.GetLength() ] == ' ')
		{
			char* pStr = Callback.GetBuffer( Callback.GetLength() );
			pStr[ Build.GetLength() ] = '_';
			Callback.ReleaseBuffer();
		}
		else
			Callback = Callback.Left( Build.GetLength() ) + Callback.Right( Callback.GetLength() - Build.GetLength() - 1 );

		Build = Callback.SpanIncluding( "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );
	}

	//---	set the string
	SetDlgItemText( IDC_CALLBACK, Callback );	
	m_pCurLocoTable->m_Callback = Callback;
}

void CLocomotionTablesDlg::OnDonotexport() 
{
	m_pCurLocoTable->m_bDoNotExport = ((CButton*)GetDlgItem(IDC_DONOTEXPORT))->GetCheck() ? TRUE : FALSE;	
	SetDisplayToTable( ((CListBox*)GetDlgItem( IDC_LOCOMOTIONTABLES ))->GetCurSel() );	
}

void CLocomotionTablesDlg::OnSelchangeStartangle() 
{
	if( !m_pCurLocoTable )
		return;

	m_pCurLocoTable->m_IndexAngleStart = ((CComboBox*)GetDlgItem(IDC_STARTANGLE))->GetCurSel();
}

void CLocomotionTablesDlg::OnSelchangeEndangle() 
{
	if( !m_pCurLocoTable )
		return;

	m_pCurLocoTable->m_IndexAngleStart = ((CComboBox*)GetDlgItem(IDC_ENDANGLE))->GetCurSel();
}
