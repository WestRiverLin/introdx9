#pragma once

// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.
// This makes D3D objects work well in the debugger watch window, but slows down
// performance slightly.
#if defined(DEBUG) || defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#include <d3d9.h>
#include <d3dx9.h>
#include "dxerr.h"
#include <string>
#include <sstream>

class D3DApp;
extern D3DApp *gd3dApp;
extern IDirect3DDevice9* gd3dDevice;

#define SafeRelease(x) { if (x) { x->Release(); x = 0; } }
#define SafeDelete(x) { delete x; x = 0; }

#if defined(DEBUG) || defined(_DEBUG)
	#ifndef HR
	#define HR(x) \
	{ \
		HRESULT hr = (x); \
		if (FAILED(hr)) \
		{ \
			DXTRACE_ERR_MSGBOX(L#x, hr); \
		} \
	}
	#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif
