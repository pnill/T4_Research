#if !defined(AFX_D3DVIEW_H__5AC58A41_F37F_452F_9EDE_B007AC1B4DD3__INCLUDED_)
#define AFX_D3DVIEW_H__5AC58A41_F37F_452F_9EDE_B007AC1B4DD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// D3DView.h : header file
//

#include "x_math.hpp"

#ifdef CED_D3D
#include "D3D8.h"
#endif



/////////////////////////////////////////////////////////////////////////////
// CD3DView view

class CD3DView : public CView
{
public:
	//-----------------------------------------------------------------------------
	// Error codes
	//-----------------------------------------------------------------------------
	enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOREF };

	#define D3DAPPERR_NODIRECT3D          0x82000001
	#define D3DAPPERR_NOWINDOW            0x82000002
	#define D3DAPPERR_NOCOMPATIBLEDEVICES 0x82000003
	#define D3DAPPERR_NOWINDOWABLEDEVICES 0x82000004
	#define D3DAPPERR_NOHARDWAREDEVICE    0x82000005
	#define D3DAPPERR_HALNOTCOMPATIBLE    0x82000006
	#define D3DAPPERR_NOWINDOWEDHAL       0x82000007
	#define D3DAPPERR_NODESKTOPHAL        0x82000008
	#define D3DAPPERR_NOHALTHISMODE       0x82000009
	#define D3DAPPERR_NONZEROREFCOUNT     0x8200000a
	#define D3DAPPERR_MEDIANOTFOUND       0x8200000b
	#define D3DAPPERR_RESIZEFAILED        0x8200000c

private:
	//-----------------------------------------------------------------------------
	// Name: struct D3DModeInfo
	// Desc: Structure for holding information about a display mode
	//-----------------------------------------------------------------------------
	struct D3DModeInfo
	{
		DWORD      Width;      // Screen width in this mode
		DWORD      Height;     // Screen height in this mode
		D3DFORMAT  Format;     // Pixel format in this mode
		DWORD      dwBehavior; // Hardware / Software / Mixed vertex processing
		D3DFORMAT  DepthStencilFormat; // Which depth/stencil format to use with this mode
	};


	//-----------------------------------------------------------------------------
	// Name: struct D3DDeviceInfo
	// Desc: Structure for holding information about a Direct3D device, including
	//       a list of modes compatible with this device
	//-----------------------------------------------------------------------------
	struct D3DDeviceInfo
	{
		// Device data
		D3DDEVTYPE   DeviceType;      // Reference, HAL, etc.
		D3DCAPS8     d3dCaps;         // Capabilities of this device
		const TCHAR* strDesc;         // Name of this device
		BOOL         bCanDoWindowed;  // Whether this device can work in windowed mode

		// Modes for this device
		DWORD        dwNumModes;
		D3DModeInfo  modes[150];

		// Current state
		DWORD        dwCurrentMode;
		BOOL         bWindowed;
		D3DMULTISAMPLE_TYPE MultiSampleType;
	};


	//-----------------------------------------------------------------------------
	// Name: struct D3DAdapterInfo
	// Desc: Structure for holding information about an adapter, including a list
	//       of devices available on this adapter
	//-----------------------------------------------------------------------------
	struct D3DAdapterInfo
	{
		// Adapter data
		D3DADAPTER_IDENTIFIER8 d3dAdapterIdentifier;
		D3DDISPLAYMODE d3ddmDesktop;      // Desktop display mode for this adapter

		// Devices for this adapter
		DWORD          dwNumDevices;
		D3DDeviceInfo  devices[5];

		// Current state
		DWORD          dwCurrentDevice;
	};



protected:
	CD3DView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CD3DView)

// Attributes
public:
	// System adaptor information (what video cards are installed and what are their capabilities)
	static BOOL				m_bAdaptorsRead;	// T/F has the adaptor information been read
    static D3DAdapterInfo	m_Adapters[10];		// list of adaptors
    static DWORD			m_dwNumAdapters;	// number of addaptors
    static DWORD			m_dwAdapter;		// currently selected adapter

	static BOOL				m_bHALExists;
	static BOOL				m_bHALIsWindowedCompatible;
	static BOOL				m_bHALIsDesktopCompatible;
	static BOOL				m_bHALIsSampleCompatible;

	BOOL					m_bD3DEnabled;		// T/F is D3D allowed

	// Light
	D3DLIGHT8				m_Light;			// light used in scene	

	// Projection values
	f32						m_FarPlane;
	f32						m_NearPlane;
	s32						m_DisplayWidth;
	s32						m_DisplayHeight;
	f32						m_HFOV;
	f32						m_VFOV;
    D3DMATRIX				m_ProjMat;			// projection matrix

    // Main objects used for creating and rendering the 3D scene
	BOOL					m_bD3DActive;		// Is the system ready to go?
    LPDIRECT3D8				m_pD3D;             // The main D3D object
    LPDIRECT3DDEVICE8		m_pd3dDevice;       // The D3D rendering device
    D3DPRESENT_PARAMETERS	m_d3dpp;			// Parameters for CreateDevice/Reset
    D3DCAPS8				m_d3dCaps;          // Caps for the device
    D3DSURFACE_DESC			m_d3dsdBackBuffer;  // Surface desc of the backbuffer
    DWORD					m_dwCreateFlags;    // Indicate sw or hw vertex processing
    DWORD					m_dwWindowStyle;    // Saved window style for mode switches
    RECT					m_rcWindowBounds;   // Saved window bounds for mode switches
    RECT					m_rcWindowClient;   // Saved client area size for mode switches
	BOOL					m_bSized;
	BOOL					m_bMoved;

    // Overridable variables for the app
    TCHAR*					m_strWindowTitle;   // Title for the app's window
    static BOOL				m_bUseDepthBuffer;  // Whether to autocreate depthbuffer
    static DWORD			m_dwMinDepthBits;   // Minimum number of bits needed in depth buffer
    static DWORD			m_dwMinStencilBits; // Minimum number of bits needed in stencil buffer

// Operations
public:
	HRESULT		Initialize3DEnvironment	( void );
	HRESULT		Resize3DEnvironment		( void );
	void		Close3DEnvironment		( void );
	HRESULT		Prepare3DEnvironment	( BOOL bClearBuffer, COLORREF ClearColor );
	void		Present3DEnvironment	( void );

	static HRESULT	BuildDeviceList		( void );
	HRESULT		DisplayErrorMsg			( HRESULT hr, DWORD dwType );
	static BOOL	FindDepthStencilFormat	( LPDIRECT3D8 pD3D, UINT iAdapter, D3DDEVTYPE DeviceType,
										  D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat );

	void		SetViewMatrix			( matrix4& rViewMatrix );
	void		SetProjection			( matrix4& rProjMat );
	void		SetProjection			( f32 NearPlane, f32 FarPlane, f32 HFOV, s32 Width, s32 Height );

	virtual bool GetD3DPermission		( void ) { return TRUE; }	// used to ask the application if D3D is allowed at startup
	void		EnableD3D				( bool bEnable );

	//---	virtual functions
	virtual void ReleaseDevices			( void ){}
	virtual void AquireDevices			( void ){}

protected:
	const D3DMATRIX&	matrix4_to_D3DMATRIX	( matrix4 m );

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CD3DView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CD3DView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CD3DView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnDestroy();
	afx_msg void OnEnableD3d();
	afx_msg void OnUpdateEnableD3d(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_D3DVIEW_H__5AC58A41_F37F_452F_9EDE_B007AC1B4DD3__INCLUDED_)
