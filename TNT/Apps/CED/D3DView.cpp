// D3DView.cpp : implementation file
//

#include "x_types.hpp"
#include "stdafx.h"
#include "ced.h"
#include "D3DView.h"
#include "x_plus.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }


/////////////////////////////////////////////////////////////////////////////
// CD3DView static storage

BOOL						CD3DView::m_bAdaptorsRead				= FALSE;	// T/F has the adaptor information been read
CD3DView::D3DAdapterInfo	CD3DView::m_Adapters[10];							// list of adaptors
DWORD						CD3DView::m_dwNumAdapters;							// number of addaptors
DWORD						CD3DView::m_dwAdapter;								// currently selected adapter

BOOL						CD3DView::m_bHALExists					= NULL;
BOOL						CD3DView::m_bHALIsWindowedCompatible	= NULL;
BOOL						CD3DView::m_bHALIsDesktopCompatible		= NULL;
BOOL						CD3DView::m_bHALIsSampleCompatible		= NULL;

BOOL						CD3DView::m_bUseDepthBuffer				= TRUE;
DWORD						CD3DView::m_dwMinDepthBits				= 16;
DWORD						CD3DView::m_dwMinStencilBits			= 0;

/////////////////////////////////////////////////////////////////////////////
// CD3DView

IMPLEMENT_DYNCREATE(CD3DView, CView)

CD3DView::CD3DView()
{
	m_bD3DActive	= FALSE;

	m_pD3D			= NULL;
	m_pd3dDevice	= NULL;

//	m_bUseDepthBuffer	= TRUE;
//	m_dwMinDepthBits	= 16;
//	m_dwMinStencilBits	= 0;
	m_strWindowTitle	= "D3D Setup Error";

	m_bMoved			= FALSE;
	m_bSized			= FALSE;

	m_NearPlane			= 0;
	m_FarPlane			= 10000;
	m_HFOV				= PI/4;
	m_VFOV				= m_HFOV*3.0f/4.0f;

	m_bD3DEnabled		= GetD3DPermission();
}

CD3DView::~CD3DView()
{
}


BEGIN_MESSAGE_MAP(CD3DView, CView)
	//{{AFX_MSG_MAP(CD3DView)
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_ENABLE_D3D, OnEnableD3d)
	ON_WM_CLOSE()
	ON_WM_ACTIVATE()
	ON_WM_ACTIVATEAPP()
	ON_WM_ASKCBFORMATNAME()
	ON_UPDATE_COMMAND_UI(ID_ENABLE_D3D, OnUpdateEnableD3d)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CD3DView drawing

void CD3DView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CD3DView diagnostics

#ifdef _DEBUG
void CD3DView::AssertValid() const
{
	CView::AssertValid();
}

void CD3DView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CD3DView message handlers


/////////////////////////////////////////////////////////////////////////////
// CD3DView Functionality


//-----------------------------------------------------------------------------
// Name: Initialize3DEnvironment()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DView::Initialize3DEnvironment()
{
#ifdef CED_D3D

    HRESULT hr;

    D3DAdapterInfo* pAdapterInfo = &m_Adapters[m_dwAdapter];
    D3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->devices[pAdapterInfo->dwCurrentDevice];
    D3DModeInfo*    pModeInfo    = &pDeviceInfo->modes[pDeviceInfo->dwCurrentMode];

    // Set up the presentation parameters
    ZeroMemory( &m_d3dpp, sizeof(m_d3dpp) );
    m_d3dpp.Windowed				= pDeviceInfo->bWindowed;
    m_d3dpp.BackBufferCount			= 1;
    m_d3dpp.MultiSampleType			= pDeviceInfo->MultiSampleType;
    m_d3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
    m_d3dpp.EnableAutoDepthStencil	= m_bUseDepthBuffer;
    m_d3dpp.AutoDepthStencilFormat	= pModeInfo->DepthStencilFormat;
    m_d3dpp.hDeviceWindow			= GetSafeHwnd();
    m_d3dpp.BackBufferWidth			= m_rcWindowClient.right - m_rcWindowClient.left;
    m_d3dpp.BackBufferHeight		= m_rcWindowClient.bottom - m_rcWindowClient.top;
    m_d3dpp.BackBufferFormat		= pAdapterInfo->d3ddmDesktop.Format;

    // Create the device
    hr = m_pD3D->CreateDevice( m_dwAdapter, pDeviceInfo->DeviceType,
                               GetSafeHwnd(), pModeInfo->dwBehavior, &m_d3dpp,
                               &m_pd3dDevice );
    if( SUCCEEDED(hr) )
    {
        // Store device Caps
        m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );
        m_dwCreateFlags = pModeInfo->dwBehavior;

        // Store render target surface desc
        LPDIRECT3DSURFACE8 pBackBuffer;
        m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
        pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
        pBackBuffer->Release();

		// Set initial projection matrix (window has already been sized, so this should be valid from the OnSize() function
		SetProjection( m_NearPlane, m_FarPlane, m_HFOV, m_DisplayWidth, m_DisplayHeight );
//		SetProjection( m_Camera.ComputeProjMatrixD3D() ) ;

        return S_OK;
    }

	// If that failed, fall back to the reference rasterizer
	if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
	{
		// Let the user know we are switching from HAL to the reference rasterizer
		DisplayErrorMsg( hr, MSGWARN_SWITCHEDTOREF );

		// Select the default adapter
		m_dwAdapter = 0L;
		pAdapterInfo = &m_Adapters[m_dwAdapter];

		// Look for a software device
		for( UINT i=0L; i<pAdapterInfo->dwNumDevices; i++ )
		{
			if( pAdapterInfo->devices[i].DeviceType == D3DDEVTYPE_REF )
			{
				pAdapterInfo->dwCurrentDevice = i;
				pDeviceInfo = &pAdapterInfo->devices[i];
				break;
			}
		}

		// Try again, this time with the reference rasterizer
		if( pAdapterInfo->devices[pAdapterInfo->dwCurrentDevice].DeviceType ==
			D3DDEVTYPE_SW )
		{
			hr = Initialize3DEnvironment();
		}
	}

    return hr;

#else

	return S_OK;

#endif
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DView::Resize3DEnvironment()
{
#ifdef CED_D3D

    HRESULT hr;

	//---	cannot resize until the environment is setup
	if( !m_bD3DActive )
		return S_OK;

    m_dwWindowStyle = GetStyle();
    GetWindowRect( &m_rcWindowBounds );
    GetClientRect( &m_rcWindowClient );

	D3DAdapterInfo* pAdapterInfo = &m_Adapters[m_dwAdapter];
    m_d3dpp.BackBufferWidth			= m_rcWindowClient.right - m_rcWindowClient.left;
    m_d3dpp.BackBufferHeight		= m_rcWindowClient.bottom - m_rcWindowClient.top;
    m_d3dpp.BackBufferFormat		= pAdapterInfo->d3ddmDesktop.Format;

	ReleaseDevices();

	// Reset the device
    if( FAILED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
        return hr;

    // Store render target surface desc
    LPDIRECT3DSURFACE8 pBackBuffer;
    m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
    pBackBuffer->Release();

	AquireDevices();

#endif

    return S_OK;

}


//-----------------------------------------------------------------------------
// Name: Close3DEnvironment()
// Desc: Cleanup scene objects
//-----------------------------------------------------------------------------
void CD3DView::Close3DEnvironment()
{
    m_bD3DActive = FALSE;

#ifdef CED_D3D

    if( m_pd3dDevice )
    {
        m_pd3dDevice->Release();
        m_pD3D->Release();

        m_pd3dDevice = NULL;
        m_pD3D       = NULL;
    }

#endif
}


//-----------------------------------------------------------------------------
// Name: Render3DEnvironment()
// Desc: Draws the scene.
//-----------------------------------------------------------------------------
HRESULT CD3DView::Prepare3DEnvironment( BOOL bClearBuffer, COLORREF ClearColor )
{
#ifdef CED_D3D

    HRESULT hr;

	//---	if the window had moved, or been modifed in some way, display it now.
	if( m_bMoved || m_bSized )
	{
//		hr = Resize3DEnvironment();
		m_bMoved = FALSE;
		m_bSized = FALSE;
	}

    // Test the cooperative level to see if it's okay to render
    if( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
    {
        // If the device was lost, do not render until we get it back
        if( D3DERR_DEVICELOST == hr )
            return S_OK;

        // Check if the device needs to be resized.
        if( D3DERR_DEVICENOTRESET == hr )
        {
            D3DAdapterInfo* pAdapterInfo = &m_Adapters[m_dwAdapter];
            m_pD3D->GetAdapterDisplayMode( m_dwAdapter, &pAdapterInfo->d3ddmDesktop );
            m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;

            if( FAILED( hr = Resize3DEnvironment() ) )
                return hr;
        }
        return hr;
    }

	//---	clear the back buffer if requested
	if( bClearBuffer )
		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(GetRValue(ClearColor),GetGValue(ClearColor),GetBValue(ClearColor)), 1.0f, 0 );

	//---	begin the scene
	return m_pd3dDevice->BeginScene();

#endif

    return S_OK;
}

//-----------------------------------------------------------------------------
void CD3DView::Present3DEnvironment( void )
{
#ifdef CED_D3D

	//---	End the scene
	if( m_pd3dDevice->EndScene() == S_OK )
	{
		//---	Show the frame on the primary surface.
		m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

#endif
}


#ifdef CED_D3D

//-----------------------------------------------------------------------------
// Name: SortModesCallback()
// Desc: Callback function for sorting display modes (used by BuildDeviceList).
//-----------------------------------------------------------------------------
static int SortModesCallback( const VOID* arg1, const VOID* arg2 )
{
    D3DDISPLAYMODE* p1 = (D3DDISPLAYMODE*)arg1;
    D3DDISPLAYMODE* p2 = (D3DDISPLAYMODE*)arg2;

    if( p1->Format > p2->Format )   return -1;
    if( p1->Format < p2->Format )   return +1;
    if( p1->Width  < p2->Width )    return -1;
    if( p1->Width  > p2->Width )    return +1;
    if( p1->Height < p2->Height )   return -1;
    if( p1->Height > p2->Height )   return +1;

    return 0;
}

#endif


//-----------------------------------------------------------------------------
// Name: BuildDeviceList()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DView::BuildDeviceList()
{
#ifdef CED_D3D

	LPDIRECT3D8	pD3D;

	BOOL bDisplayWarning	= FALSE;

	//---	if the adaptors have already been read, do not do it again
	if( !m_bAdaptorsRead )
	{
		// Create the Direct3D object
		pD3D = Direct3DCreate8( D3D_SDK_VERSION );
		if( pD3D == NULL )
			return D3DAPPERR_NODIRECT3D;

		const DWORD dwNumDeviceTypes = 2;
		const D3DDEVTYPE DeviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };

		//---	we will be counting the adaptors as we add them to the list
		m_dwNumAdapters = 0;

		// Loop through all the adapters on the system (usually, there's just one
		// unless more than one graphics card is present).
		for( UINT iAdapter = 0; iAdapter < pD3D->GetAdapterCount(); iAdapter++ )
		{
			// Fill in adapter info
			D3DAdapterInfo* pAdapter  = &m_Adapters[m_dwNumAdapters];
			pD3D->GetAdapterIdentifier( iAdapter, 0, &pAdapter->d3dAdapterIdentifier );
			pD3D->GetAdapterDisplayMode( iAdapter, &pAdapter->d3ddmDesktop );
			pAdapter->dwNumDevices    = 0;
			pAdapter->dwCurrentDevice = 0;

			// Enumerate all display modes on this adapter
			D3DDISPLAYMODE modes[100];
			D3DFORMAT      formats[20];
			DWORD dwNumFormats      = 0;
			DWORD dwNumModes        = 0;
			DWORD dwNumAdapterModes = pD3D->GetAdapterModeCount( iAdapter );

			// Add the adapter's current desktop format to the list of formats
			formats[dwNumFormats++] = pAdapter->d3ddmDesktop.Format;

			for( UINT iMode = 0; iMode < dwNumAdapterModes; iMode++ )
			{
				// Get the display mode attributes
				D3DDISPLAYMODE DisplayMode;
				pD3D->EnumAdapterModes( iAdapter, iMode, &DisplayMode );

				// Filter out low-resolution modes
				if( DisplayMode.Width  < 640 || DisplayMode.Height < 400 )
					continue;

				// Check if the mode already exists (to filter out refresh rates)
				for( DWORD m=0L; m<dwNumModes; m++ )
				{
					if( ( modes[m].Width  == DisplayMode.Width  ) &&
						( modes[m].Height == DisplayMode.Height ) &&
						( modes[m].Format == DisplayMode.Format ) )
						break;
				}

				// If we found a new mode, add it to the list of modes
				if( m == dwNumModes )
				{
					modes[dwNumModes].Width       = DisplayMode.Width;
					modes[dwNumModes].Height      = DisplayMode.Height;
					modes[dwNumModes].Format      = DisplayMode.Format;
					modes[dwNumModes].RefreshRate = 0;
					dwNumModes++;

					// Check if the mode's format already exists
					for( DWORD f=0; f<dwNumFormats; f++ )
					{
						if( DisplayMode.Format == formats[f] )
							break;
					}

					// If the format is new, add it to the list
					if( f== dwNumFormats )
						formats[dwNumFormats++] = DisplayMode.Format;
				}
			}

			// Sort the list of display modes (by format, then width, then height)
			qsort( modes, dwNumModes, sizeof(D3DDISPLAYMODE), SortModesCallback );

			// Add devices to adapter
			for( UINT iDevice = 0; iDevice < dwNumDeviceTypes; iDevice++ )
			{
				// Fill in device info
				D3DDeviceInfo* pDevice;
				pDevice                 = &pAdapter->devices[pAdapter->dwNumDevices];
				pDevice->DeviceType     = DeviceTypes[iDevice];
				pD3D->GetDeviceCaps( iAdapter, DeviceTypes[iDevice], &pDevice->d3dCaps );
				pDevice->dwNumModes     = 0;
				pDevice->dwCurrentMode  = 0;
				pDevice->bCanDoWindowed = FALSE;
				pDevice->bWindowed      = TRUE;
				pDevice->MultiSampleType = D3DMULTISAMPLE_NONE;

				// Examine each format supported by the adapter to see if it will
				// work with this device and meets the needs of the application.
				BOOL  bFormatConfirmed[20];
				DWORD dwBehavior[20];
				D3DFORMAT fmtDepthStencil[20];

				for( DWORD f=0; f<dwNumFormats; f++ )
				{
					bFormatConfirmed[f] = FALSE;
					fmtDepthStencil[f] = D3DFMT_UNKNOWN;

					// Skip formats that cannot be used as render targets on this device
					if( FAILED( pD3D->CheckDeviceType( iAdapter, pDevice->DeviceType,
														 formats[f], formats[f], TRUE ) ) )
						continue;

					if( pDevice->DeviceType == D3DDEVTYPE_HAL )
					{
						// This system has a HAL device
						m_bHALExists = TRUE;

						if( pDevice->d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED )
						{
							// HAL can run in a window for some mode
							m_bHALIsWindowedCompatible = TRUE;

							if( f == 0 )
							{
								// HAL can run in a window for the current desktop mode
								m_bHALIsDesktopCompatible = TRUE;
							}
						}
					}

					// Confirm the device/format for HW vertex processing
					if( pDevice->d3dCaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT )
					{
						if( pDevice->d3dCaps.DevCaps&D3DDEVCAPS_PUREDEVICE )
						{
							dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING |
											D3DCREATE_PUREDEVICE;
							bFormatConfirmed[f] = TRUE;
						}

						if ( FALSE == bFormatConfirmed[f] )
						{
							dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING;
							bFormatConfirmed[f] = TRUE;
						}

						if ( FALSE == bFormatConfirmed[f] )
						{
							dwBehavior[f] = D3DCREATE_MIXED_VERTEXPROCESSING;
							bFormatConfirmed[f] = TRUE;
						}
					}

					// Confirm the device/format for SW vertex processing
					if( FALSE == bFormatConfirmed[f] )
					{
						dwBehavior[f] = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
						bFormatConfirmed[f] = TRUE;
					}

					// Find a suitable depth/stencil buffer format for this device/format
					bFormatConfirmed[f] = TRUE;
					if( m_bUseDepthBuffer )
					{
						if( pD3D, !FindDepthStencilFormat( pD3D, iAdapter, pDevice->DeviceType,
							formats[f], &fmtDepthStencil[f] ) )
						{
							bFormatConfirmed[f] = FALSE;
						}
					}
				}

				// Add all enumerated display modes with confirmed formats to the
				// device's list of valid modes
				for( DWORD m=0L; m<dwNumModes; m++ )
				{
					for( DWORD f=0; f<dwNumFormats; f++ )
					{
						if( modes[m].Format == formats[f] )
						{
							if( bFormatConfirmed[f] == TRUE )
							{
								// Add this mode to the device's list of valid modes
								pDevice->modes[pDevice->dwNumModes].Width      = modes[m].Width;
								pDevice->modes[pDevice->dwNumModes].Height     = modes[m].Height;
								pDevice->modes[pDevice->dwNumModes].Format     = modes[m].Format;
								pDevice->modes[pDevice->dwNumModes].dwBehavior = dwBehavior[f];
								pDevice->modes[pDevice->dwNumModes].DepthStencilFormat = fmtDepthStencil[f];
								pDevice->dwNumModes++;

								if( pDevice->DeviceType == D3DDEVTYPE_HAL )
									m_bHALIsSampleCompatible = TRUE;
							}
						}
					}
				}

				// Select any 640x480 mode for default (but prefer a 16-bit mode)
				for( m=0; m<pDevice->dwNumModes; m++ )
				{
					if( pDevice->modes[m].Width==640 && pDevice->modes[m].Height==480 )
					{
						pDevice->dwCurrentMode = m;
						if( pDevice->modes[m].Format == D3DFMT_R5G6B5 ||
							pDevice->modes[m].Format == D3DFMT_X1R5G5B5 ||
							pDevice->modes[m].Format == D3DFMT_A1R5G5B5 )
						{
							break;
						}
					}
				}

				// Check if the device is compatible with the desktop display mode
				// (which was added initially as formats[0])
				if( bFormatConfirmed[0] && (pDevice->d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) )
				{
					pDevice->bCanDoWindowed = TRUE;
					pDevice->bWindowed      = TRUE;
				}

				// If valid modes were found, keep this device
				if( pDevice->dwNumModes > 0 )
					pAdapter->dwNumDevices++;
			}

			// If valid devices were found, keep this adapter
			if( pAdapter->dwNumDevices > 0 )
				m_dwNumAdapters++;
		}

		//---	flag that the adaptors have been read
		m_bAdaptorsRead = TRUE;

		bDisplayWarning = TRUE;



		// Return an error if no compatible devices were found
		if( 0L == m_dwNumAdapters )
			return D3DAPPERR_NOCOMPATIBLEDEVICES;


		int ca	= -1;
		int cd	= -1;
		int PreferedDevices[3] =
		{	
			D3DDEVTYPE_HAL,
			D3DDEVTYPE_SW,
			D3DDEVTYPE_REF
		};

		// Pick a default device that can render into a window
		// (This code assumes that the HAL device comes before the REF
		// device in the device array).
		for( DWORD a=0; a<m_dwNumAdapters; a++ )
		{
			for( DWORD d=0; d < m_Adapters[a].dwNumDevices; d++ )
			{
				if( m_Adapters[a].devices[d].bWindowed )
				{
					//---	if this is a HAL, it works, use it
					if( m_Adapters[a].devices[d].DeviceType == PreferedDevices[0] )
					{
						m_Adapters[a].dwCurrentDevice = d;
						m_dwAdapter = a;
						pD3D->Release();
						return S_OK;
					}

					//---	make sure that at least one is set
					if( (ca==-1)||(cd==-1) )
					{
						ca = a;
						cd = d;
						continue;
					}

					//---	if the chosen one is a REF, replace it if the new one is SW (SW should be faster)
					if(( m_Adapters[ca].devices[cd].DeviceType == PreferedDevices[2] ) &&
					   ( m_Adapters[a].devices[d].DeviceType == PreferedDevices[1] ))
					{
						ca = a;
						cd = d;
					}
				}
			}
		}

		//---	set the chosen ones
		m_Adapters[ca].dwCurrentDevice = cd;
		m_dwAdapter = ca;
/*
		// Display a warning message
		if( ( bDisplayWarning &&
			  m_Adapters[ca]. devices[cd].DeviceType == D3DDEVTYPE_REF ) )
		{
			if( !m_bHALExists )
				DisplayErrorMsg( D3DAPPERR_NOHARDWAREDEVICE, MSGWARN_SWITCHEDTOREF );
			else if( !m_bHALIsSampleCompatible )
				DisplayErrorMsg( D3DAPPERR_HALNOTCOMPATIBLE, MSGWARN_SWITCHEDTOREF );
			else if( !m_bHALIsWindowedCompatible )
				DisplayErrorMsg( D3DAPPERR_NOWINDOWEDHAL, MSGWARN_SWITCHEDTOREF );
			else if( !m_bHALIsDesktopCompatible )
				DisplayErrorMsg( D3DAPPERR_NODESKTOPHAL, MSGWARN_SWITCHEDTOREF );
			else // HAL is desktop compatible, but not sample compatible
				DisplayErrorMsg( D3DAPPERR_NOHALTHISMODE, MSGWARN_SWITCHEDTOREF );
		}
*/

		pD3D->Release();
	}

	// if the ca and the cd values were set, we found one to use
	if(( m_dwAdapter!=-1 )&&( m_Adapters[m_dwAdapter].dwCurrentDevice!=-1 ))
		return S_OK;

	return D3DAPPERR_NOWINDOWABLEDEVICES;

#else

	return S_OK;

#endif
}


//-----------------------------------------------------------------------------
// Name: FindDepthStencilFormat()
// Desc: Finds a depth/stencil format for the given device that is compatible
//       with the render target format and meets the needs of the app.
//-----------------------------------------------------------------------------
BOOL CD3DView::FindDepthStencilFormat( LPDIRECT3D8 pD3D, UINT iAdapter, D3DDEVTYPE DeviceType,
    D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat )
{
#ifdef CED_D3D

    if( m_dwMinDepthBits <= 16 && m_dwMinStencilBits == 0 )
    {
        if( SUCCEEDED( pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16 ) ) )
        {
            if( SUCCEEDED( pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D16 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D16;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 15 && m_dwMinStencilBits <= 1 )
    {
        if( SUCCEEDED( pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D15S1 ) ) )
        {
            if( SUCCEEDED( pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D15S1 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D15S1;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 24 && m_dwMinStencilBits == 0 )
    {
        if( SUCCEEDED( pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8 ) ) )
        {
            if( SUCCEEDED( pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24X8 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24X8;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 24 && m_dwMinStencilBits <= 8 )
    {
        if( SUCCEEDED( pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) )
        {
            if( SUCCEEDED( pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24S8 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24S8;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 24 && m_dwMinStencilBits <= 4 )
    {
        if( SUCCEEDED( pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4 ) ) )
        {
            if( SUCCEEDED( pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24X4S4 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24X4S4;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 32 && m_dwMinStencilBits == 0 )
    {
        if( SUCCEEDED( pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32 ) ) )
        {
            if( SUCCEEDED( pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D32 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D32;
                return TRUE;
            }
        }
    }

#endif

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DisplayErrorMsg()
// Desc: Displays error messages in a message box
//-----------------------------------------------------------------------------
HRESULT CD3DView::DisplayErrorMsg( HRESULT hr, DWORD dwType )
{
#ifdef CED_D3D

    TCHAR strMsg[512];

    switch( hr )
    {
        case D3DAPPERR_NODIRECT3D:
            _tcscpy( strMsg, _T("Could not initialize Direct3D. You may\n")
                             _T("want to check that the latest version of\n")
                             _T("DirectX is correctly installed on your\n")
                             _T("system.  Also make sure that this program\n")
                             _T("was compiled with header files that match\n")
                             _T("the installed DirectX DLLs.") );
            break;

        case D3DAPPERR_NOCOMPATIBLEDEVICES:
            _tcscpy( strMsg, _T("Could not find any compatible Direct3D\n")
                             _T("devices.") );
            break;

        case D3DAPPERR_NOWINDOWABLEDEVICES:
            _tcscpy( strMsg, _T("This sample cannot run in a desktop\n")
                             _T("window with the current display settings.\n")
                             _T("Please change your desktop settings to a\n")
                             _T("16- or 32-bit display mode and re-run this\n")
                             _T("sample.") );
            break;

        case D3DAPPERR_NOHARDWAREDEVICE:
            _tcscpy( strMsg, _T("No hardware-accelerated Direct3D devices\n")
                             _T("were found.") );
            break;

        case D3DAPPERR_HALNOTCOMPATIBLE:
            _tcscpy( strMsg, _T("This sample requires functionality that is\n")
                             _T("not available on your Direct3D hardware\n")
                             _T("accelerator.") );
            break;

        case D3DAPPERR_NOWINDOWEDHAL:
            _tcscpy( strMsg, _T("Your Direct3D hardware accelerator cannot\n")
                             _T("render into a window.\n")
                             _T("Press F2 while the app is running to see a\n")
                             _T("list of available devices and modes.") );
            break;

        case D3DAPPERR_NODESKTOPHAL:
            _tcscpy( strMsg, _T("Your Direct3D hardware accelerator cannot\n")
                             _T("render into a window with the current\n")
                             _T("desktop display settings.\n")
                             _T("Press F2 while the app is running to see a\n")
                             _T("list of available devices and modes.") );
            break;

        case D3DAPPERR_NOHALTHISMODE:
            _tcscpy( strMsg, _T("This sample requires functionality that is\n")
                             _T("not available on your Direct3D hardware\n")
                             _T("accelerator with the current desktop display\n")
                             _T("settings.\n")
                             _T("Press F2 while the app is running to see a\n")
                             _T("list of available devices and modes.") );
            break;

        case D3DAPPERR_MEDIANOTFOUND:
            _tcscpy( strMsg, _T("Could not load required media." ) );
            break;

        case D3DAPPERR_RESIZEFAILED:
            _tcscpy( strMsg, _T("Could not reset the Direct3D device." ) );
            break;

        case D3DAPPERR_NONZEROREFCOUNT:
            _tcscpy( strMsg, _T("A D3D object has a non-zero reference\n")
                             _T("count (meaning things were not properly\n")
                             _T("cleaned up).") );
            break;

        case E_OUTOFMEMORY:
            _tcscpy( strMsg, _T("Not enough memory.") );
            break;

        case D3DERR_OUTOFVIDEOMEMORY:
            _tcscpy( strMsg, _T("Not enough video memory.") );
            break;

        default:
            _tcscpy( strMsg, _T("Generic application error. Enable\n")
                             _T("debug output for detailed information.") );
    }

    if( MSGERR_APPMUSTEXIT == dwType )
    {
        _tcscat( strMsg, _T("\n\nThis sample will now exit.") );
        MessageBox( strMsg, m_strWindowTitle, MB_ICONERROR|MB_OK );

        // Close the window, which shuts down the app
		SendMessage( WM_CLOSE, 0, 0 );
    }
    else
    {
        if( MSGWARN_SWITCHEDTOREF == dwType )
            _tcscat( strMsg, _T("\n\nSwitching to the reference rasterizer,\n")
                             _T("a software device that implements the entire\n")
                             _T("Direct3D feature set, but runs very slowly.") );
        MessageBox( strMsg, m_strWindowTitle, MB_ICONWARNING|MB_OK );
    }

    return hr;

#else

	return S_OK;

#endif
}

//--------------------------------------------------------------------------
void CD3DView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

#ifdef CED_D3D

    HRESULT hr;
	
    // Create the Direct3D object
    m_pD3D = Direct3DCreate8( D3D_SDK_VERSION );
    if( m_pD3D == NULL )
        DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );

    // Build a list of Direct3D adapters, modes and devices. The
    // ConfirmDevice() callback is used to confirm that only devices that
    // meet the app's requirements are considered.
    if( FAILED( hr = BuildDeviceList() ) )
    {
        SAFE_RELEASE( m_pD3D );
        DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

    // Save window properties
    m_dwWindowStyle = GetStyle();
    GetWindowRect( &m_rcWindowBounds );
    GetClientRect( &m_rcWindowClient );

	if( m_bD3DEnabled )
	{
		// Initialize the 3D environment for the app
		if( FAILED( hr = Initialize3DEnvironment() ) )
		{
			SAFE_RELEASE( m_pD3D );
			DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}
	}

	// The app is ready to go
	m_bD3DActive = m_bD3DEnabled;

#else

    m_bD3DActive = FALSE;

#endif
}

//--------------------------------------------------------------------------
void CD3DView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
#ifdef CED_D3D

//	m_bSized = TRUE;
	Resize3DEnvironment();

#endif
}

//--------------------------------------------------------------------------
void CD3DView::OnMove(int x, int y) 
{
	CView::OnMove(x, y);
	
#ifdef CED_D3D
//	m_bMoved = TRUE;
	Resize3DEnvironment();

#endif
}


void CD3DView::OnDestroy() 
{
#ifdef CED_D3D

	Close3DEnvironment();

#endif

	CView::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

//==========================================================================
void CD3DView::OnEnableD3d() 
{
	if( !m_bD3DEnabled )
		return;

	if( m_bD3DActive )
	{
		m_bD3DActive = FALSE;
	}
	else
	{
		m_bD3DActive = TRUE;
	}
}

//==========================================================================
void CD3DView::OnUpdateEnableD3d( CCmdUI* pCmdUI ) 
{
#ifdef CED_D3D
	pCmdUI->Enable( m_bD3DEnabled ? TRUE : FALSE );
	pCmdUI->SetCheck( m_bD3DActive ? TRUE : FALSE );	
#else
	pCmdUI->Enable( FALSE );
#endif
}

//==========================================================================
void CD3DView::EnableD3D( bool bEnable )
{
	m_bD3DEnabled = bEnable;
	m_bD3DActive = bEnable;
}

//==========================================================================
const D3DMATRIX& CD3DView::matrix4_to_D3DMATRIX( matrix4 m )
{
	static D3DMATRIX M;

	ASSERT( sizeof(M) == sizeof(m) );

	memcpy( &M, &m, sizeof(D3DMATRIX) );
	return M;
}

extern matrix4& ConvertMatrixToD3D( matrix4& rDest );

//==========================================================================
void CD3DView::SetViewMatrix( matrix4& rViewMatrix )
{
#ifdef CED_D3D

	matrix4 m = rViewMatrix;

	//----	SET THE LIGHT BESIDE THE CAMERA
	matrix4 LightMatrix;
	radian3 r = m.GetRotation();
	r.Yaw = -r.Yaw;
	LightMatrix.SetRotation( r );
	LightMatrix.Invert();
	vector3 vLightDir = LightMatrix.Transform( vector3( -2.0f, -1.0f, 5.0f ) );
	vLightDir.Normalize();

    memset( &m_Light, 0, sizeof(D3DLIGHT8) );
    m_Light.Type			= D3DLIGHT_DIRECTIONAL ;
    m_Light.Diffuse.r		= 1.0f;
    m_Light.Diffuse.g		= 1.0f;
    m_Light.Diffuse.b		= 1.0f;
    m_Light.Range			= 1000.0f;
    m_Light.Direction.x		= vLightDir.X;
    m_Light.Direction.y		= vLightDir.Y;
    m_Light.Direction.z		= vLightDir.Z;

	m_pd3dDevice->SetLight( 0, &m_Light );
	m_pd3dDevice->LightEnable( 0, TRUE );

	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE ); // it will get turned on when needed
	m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_XRGB(32,32,32) );

	//---	SET THE VIEW MATRIX
	m_pd3dDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX*)&m );

#endif
}

//#include "d3dx8.h"
//==========================================================================
void CD3DView::SetProjection( f32 NearPlane, f32 FarPlane, f32 HFOV, s32 Width, s32 Height )
{
	m_NearPlane		= NearPlane;
	m_FarPlane		= FarPlane;
	m_DisplayWidth	= Width;
	m_DisplayHeight	= Height;
	m_HFOV			= HFOV;
	m_VFOV			= 2.0f * (f32)atan( (f32)Height / (f32)Width * (f32)tan( HFOV*0.5f ) );

	if( !m_pd3dDevice )
		return;

	f32 w, h;
	f32 Q;

	w = (f32)(1.0/tan(m_HFOV*0.5f));  // 1/tan(x) == cot(x)
	h = (f32)(1.0/tan(m_VFOV*0.5f));   // 1/tan(x) == cot(x)
	Q = m_FarPlane/(m_FarPlane - m_NearPlane);

	D3DMATRIX pmat;
	x_memset(&pmat, 0, sizeof(pmat));

	pmat._11 = w;
	pmat._22 = h;
	pmat._33 = Q;
	pmat._43 = -Q*NearPlane;
	pmat._34 = 1.0f;

	m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &pmat );
}

//==========================================================================
void CD3DView::SetProjection( matrix4& rProjMat )
{
	if( !m_pd3dDevice )
		return;

	m_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&rProjMat );
}
