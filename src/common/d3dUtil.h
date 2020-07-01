#pragma once

// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.
// This makes D3D objects work well in the debugger watch window, but slows down
// performance slightly.
#if defined(DEBUG) || defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <d3d9.h>
#include <d3dx9.h>
#include "dxerr.h"
#include <string>
#include <sstream>
#include <vector>
#include <float.h>

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

//===============================================================
// Geometry generation.

void GenTriGrid(int numVertRows, int numVertCols, float dx, float dz, 
				const D3DXVECTOR3 &center, std::vector<D3DXVECTOR3> &verts, std::vector<DWORD> &indices);

//===============================================================
// Colors and Materials

const D3DXCOLOR WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR BLACK(0.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR RED(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const D3DXCOLOR BLUE(0.0f, 0.0f, 1.0f, 1.0f);

struct Material
{
	Material() : ambient(WHITE), diffuse(WHITE), spec(WHITE), specPower(8.0f) {}
	Material(const D3DXCOLOR& a, const D3DXCOLOR& d, const D3DXCOLOR& s, float power) :
		ambient(a), diffuse(d), spec(s), specPower(power) {}
	
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	float specPower;
};

struct DirLight
{
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	D3DXVECTOR3 dirW;
};

//===============================================================
// .X Files

void LoadXFile(
	const std::wstring& filename,
	ID3DXMesh** meshOut,
	std::vector<Material>& materials,
	std::vector<IDirect3DTexture9*>& textures);

//===============================================================
// Math Constants

const float MY_INFINITY = FLT_MAX;
const float MY_EPSILON  = 0.001f;

//===============================================================
// Bounding Volumes

struct AABB
{
	AABB() : minPt(MY_INFINITY, MY_INFINITY, MY_INFINITY), maxPt(-MY_INFINITY, -MY_INFINITY, -MY_INFINITY) {}

	D3DXVECTOR3 center()
	{
		return 0.5f*(minPt + maxPt);
	}

	D3DXVECTOR3 minPt;
	D3DXVECTOR3 maxPt;
};

struct BoundingSphere
{
	BoundingSphere() : pos(0.0f, 0.0f, 0.0f), radius(0.0f) {}

	D3DXVECTOR3 pos;
	float radius;
};

