#include "d3dApp.h"
#include "directInput.h"
#include "gfxStats.h"
#include "Vertex.h"
#include <string.h>

class DiffusePyramidDemo : public D3DApp
{
public:
	DiffusePyramidDemo(HINSTANCE hInstance, std::wstring winCaption);
	virtual ~DiffusePyramidDemo();

	virtual bool checkDeviceCaps() override;
	virtual void onLostDevice() override;
	virtual void onResetDevice() override;
	virtual void updateScene(float dt) override;
	virtual void drawScene() override;

private:
	void buildVertexBuffer();
	void buildFX();
	void buildProjMtx();
	void buildViewMtx();

private:
	GfxStats *mGfxStats;

	IDirect3DVertexBuffer9 *mVB;

	ID3DXEffect *mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInverseTranspose;
	D3DXHANDLE   mhLightVecW;
	D3DXHANDLE   mhDiffuseMtrl;
	D3DXHANDLE   mhDiffuseLight;

	D3DXVECTOR3  mLightVecW;
	D3DXCOLOR    mDiffuseMtrl;
	D3DXCOLOR    mDiffuseLight;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined (DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	DiffusePyramidDemo app(hInstance, L"Diffuse Pyramid Demo");
	gd3dApp = &app;

	DirectInput dinput;
	gDInput = &dinput;

	if (gd3dApp->checkDeviceCaps())
		return gd3dApp->run();
	else
		return 0;
}

DiffusePyramidDemo::DiffusePyramidDemo(HINSTANCE hInstance, std::wstring winCaption)
	: D3DApp(hInstance, winCaption)
{
	mGfxStats = new GfxStats();

	mCameraRadius    = 10.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight    = 5.0f;

	mLightVecW    = D3DXVECTOR3(0.0, 1.0f, -1.0f);
	mDiffuseMtrl  = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	mDiffuseLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	D3DXMatrixIdentity(&mWorld);

	buildVertexBuffer();
	buildFX();

	onResetDevice();

	InitAllVertexDeclarations();
}

DiffusePyramidDemo::~DiffusePyramidDemo()
{
	SafeDelete(mGfxStats);

	SafeRelease(mVB);
	SafeRelease(mFX);

	DestroyAllVertexDeclarations();
}

bool DiffusePyramidDemo::checkDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gd3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2,0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2,0))
		return false;

	return true;
}

void DiffusePyramidDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void DiffusePyramidDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());
	buildProjMtx();
}

void DiffusePyramidDemo::updateScene(float dt)
{
	mGfxStats->setVertexCount(8);
	mGfxStats->setTriCount(12);
	mGfxStats->update(dt);

	gDInput->poll();

	if (gDInput->keyDown(DIK_W))
		mCameraHeight += 25.0f * dt;
	if (gDInput->keyDown(DIK_S))
		mCameraHeight -= 25.0f * dt;

	// divide by 50 to make mouse less sensitive
	mCameraRotationY += gDInput->mouseDX() / 50.0f;
	mCameraRadius    += gDInput->mouseDY() / 50.0f;

	if (fabsf(mCameraRotationY) >= 2.0f * D3DX_PI)
		mCameraRotationY = 0.0f;

	if (mCameraRadius < 5.0f)
		mCameraRadius = 5.0f;

	buildViewMtx();
}

void DiffusePyramidDemo::drawScene()
{
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255,255,255), 1.0f, 0));
	HR(gd3dDevice->BeginScene());

	HR(gd3dDevice->SetStreamSource(0, mVB, 0, sizeof(VertexPN)));
	HR(gd3dDevice->SetVertexDeclaration(VertexPN::Decl));

	HR(mFX->SetTechnique(mhTech));

	HR(mFX->SetMatrix(mhWVP, &(mWorld*mView*mProj)));
	D3DXMATRIX worldInverseTranspose;
	D3DXMatrixInverse(&worldInverseTranspose, 0, &mWorld);
	D3DXMatrixTranspose(&worldInverseTranspose, &worldInverseTranspose);
	HR(mFX->SetMatrix(mhWorldInverseTranspose, &worldInverseTranspose));
	HR(mFX->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mDiffuseMtrl, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));
		HR(gd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 4));
		HR(mFX->EndPass());
	}
	HR(mFX->End());

	mGfxStats->display(D3DCOLOR_XRGB(0,0,0));
	HR(gd3dDevice->EndScene());
	HR(gd3dDevice->Present(0, 0, 0, 0));
}


void DiffusePyramidDemo::buildVertexBuffer()
{
	HR(gd3dDevice->CreateVertexBuffer(12*sizeof(VertexPN), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mVB, 0));

	VertexPN *v = 0;
	HR(mVB->Lock(0, 0, (void**)&v, 0));

	// front face
	v[0] = VertexPN(-1.0f, 0.0f, -1.0f, 0.0f, 0.707f, -0.707f);
	v[1] = VertexPN( 0.0f, 1.0f,  0.0f, 0.0f, 0.707f, -0.707f);
	v[2] = VertexPN( 1.0f, 0.0f, -1.0f, 0.0f, 0.707f, -0.707f);

	// left face
	v[3] = VertexPN(-1.0f, 0.0f,  1.0f, -0.707f, 0.707f, 0.0f);
	v[4] = VertexPN( 0.0f, 1.0f,  0.0f, -0.707f, 0.707f, 0.0f);
	v[5] = VertexPN(-1.0f, 0.0f, -1.0f, -0.707f, 0.707f, 0.0f);

	// right face
	v[6] = VertexPN( 1.0f, 0.0f, -1.0f, 0.707f, 0.707f, 0.0f);
	v[7] = VertexPN( 0.0f, 1.0f,  0.0f, 0.707f, 0.707f, 0.0f);
	v[8] = VertexPN( 1.0f, 0.0f,  1.0f, 0.707f, 0.707f, 0.0f);

	// back face
	v[9]  = VertexPN( 1.0f, 0.0f,  1.0f, 0.0f, 0.707f, 0.707f);
	v[10] = VertexPN( 0.0f, 1.0f,  0.0f, 0.0f, 0.707f, 0.707f);
	v[11] = VertexPN(-1.0f, 0.0f,  1.0f, 0.0f, 0.707f, 0.707f);

	HR(mVB->Unlock());
}

void DiffusePyramidDemo::buildFX()
{
	ID3DXBuffer *errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, L"../../src/chap10/DiffusePyramidDemo/diffuse.fx",
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBoxA(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech                  = mFX->GetTechniqueByName("DiffuseTech");
	mhWVP                   = mFX->GetParameterByName(0, "gWVP");
	mhWorldInverseTranspose = mFX->GetParameterByName(0, "gWorldInverseTranspose");
	mhLightVecW             = mFX->GetParameterByName(0, "gLightVecW");
	mhDiffuseMtrl           = mFX->GetParameterByName(0, "gDiffuseMtrl");
	mhDiffuseLight          = mFX->GetParameterByName(0, "gDiffuseLight");
}

void DiffusePyramidDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void DiffusePyramidDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}
