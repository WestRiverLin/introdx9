#pragma once

#include "d3dUtil.h"
#include <string>

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance, std::wstring winCaption, D3DDEVTYPE devType = D3DDEVTYPE_HAL, DWORD requestedVP = D3DCREATE_HARDWARE_VERTEXPROCESSING);
	virtual ~D3DApp();

	HINSTANCE getAppInst();
	HWND      getMainWnd();

	// Framework methods.  Derived client class overrides these methods to 
	// implement specific application requirements.
	virtual bool checkDeviceCaps()     { return true; }
	virtual void onLostDevice()        {}
	virtual void onResetDevice()       {}
	virtual void updateScene(float dt) {}
	virtual void drawScene()           {}

	// Override these methods only if you do not like the default window creation,
	// direct3D device creation, window procedure, or message loop.  In general,
	// for the sample programs of this book, we will not need to modify these.
	virtual void initMainWindow();
	virtual void initDirect3D();
	virtual int run();
	virtual LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void enableFullScreenMode(bool enable);
	bool isDeviceLost();

protected:
	// Derived client class can modify these data members in the constructor to
	// customize the application.
	std::wstring mMainWndCaption;
	D3DDEVTYPE   mDevType;
	DWORD        mRequestedVP;

	HINSTANCE mhAppInst;
	HWND mhMainWnd;
	IDirect3D9* md3dObject;
	bool mAppPaused;
	D3DPRESENT_PARAMETERS md3dPP;
};