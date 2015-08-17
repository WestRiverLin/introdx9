#include "directInput.h"
#include "d3dUtil.h"
#include "d3dApp.h"

DirectInput *gDInput = 0;

DirectInput::DirectInput(DWORD keyboardCoopFlags, DWORD mouseCoopFlags)
{
	ZeroMemory(mKeyboardState, sizeof(mKeyboardState));
	ZeroMemory(&mMouseState, sizeof(mMouseState));

	HR(DirectInput8Create(gd3dApp->getAppInst(), DIRECTINPUT_VERSION,
		IID_IDirectInput8, (void**)&mDInput, 0));

	HR(mDInput->CreateDevice(GUID_SysKeyboard, &mKeyboard, 0));
	HR(mKeyboard->SetDataFormat(&c_dfDIKeyboard));
	HR(mKeyboard->SetCooperativeLevel(gd3dApp->getMainWnd(), keyboardCoopFlags));
	HR(mKeyboard->Acquire());

	HR(mDInput->CreateDevice(GUID_SysMouse, &mMouse, 0));
	HR(mMouse->SetDataFormat(&c_dfDIMouse2));
	HR(mMouse->SetCooperativeLevel(gd3dApp->getMainWnd(), mouseCoopFlags));
	HR(mMouse->Acquire());
}

DirectInput::~DirectInput()
{
	SafeRelease(mDInput);
	mKeyboard->Unacquire();
	mMouse->Unacquire();
	SafeRelease(mKeyboard);
	SafeRelease(mMouse);
}

void DirectInput::poll()
{
	HRESULT hr = mKeyboard->GetDeviceState(sizeof(mKeyboardState), (void**)&mKeyboardState);
	if (FAILED(hr))
	{
		ZeroMemory(mKeyboardState, sizeof(mKeyboardState));
		hr = mKeyboard->Acquire();
	}

	hr = mMouse->GetDeviceState(sizeof(DIMOUSESTATE2), (void**)&mMouseState);
	if (FAILED(hr))
	{
		ZeroMemory(&mMouseState, sizeof(mMouseState));
		hr = mMouse->Acquire();
	}
}

bool DirectInput::keyDown(char key)
{
	return (mKeyboardState[key] & 0x80) != 0;
}

bool DirectInput::mouseButtonDown(int button)
{
	return (mMouseState.rgbButtons[button] & 0x80) != 0;
}

float DirectInput::mouseDX()
{
	return (float)mMouseState.lX;
}

float DirectInput::mouseDY()
{
	return (float)mMouseState.lY;
}

float DirectInput::mouseDZ()
{
	return (float)mMouseState.lZ;
}
