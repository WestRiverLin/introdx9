#include "d3dApp.h"
#include "directInput.h"
#include "gfxStats.h"
#include "Vertex.h"
#include <string.h>

class AmbientDiffuseSpecularDemo : public D3DApp
{
public:
	AmbientDiffuseSpecularDemo(HINSTANCE hInstance, std::wstring winCaption);
	virtual ~AmbientDiffuseSpecularDemo();

	virtual bool checkDeviceCaps() override;
	virtual void onLostDevice() override;
	virtual void onResetDevice() override;
	virtual void updateScene(float dt) override;
	virtual void drawScene() override;

private:
	void buildFX();
	void buildProjMtx();
	void buildViewMtx();

private:
	GfxStats *mGfxStats;

	ID3DXMesh   *mTeapot;
	ID3DXEffect *mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInverseTranspose;
	D3DXHANDLE   mhLightVecW;
	D3DXHANDLE   mhDiffuseMtrl;
	D3DXHANDLE   mhDiffuseLight;
	D3DXHANDLE   mhAmbientMtrl;
	D3DXHANDLE   mhAmbientLight;
	D3DXHANDLE   mhSpecularMtrl;
	D3DXHANDLE   mhSpecularLight;
	D3DXHANDLE   mhSpecularPower;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;

	D3DXVECTOR3  mLightVecW;
	D3DXCOLOR    mDiffuseMtrl;
	D3DXCOLOR    mDiffuseLight;
	D3DXCOLOR    mAmbientMtrl;
	D3DXCOLOR    mAmbientLight;
	D3DXCOLOR    mSpecularMtrl;
	D3DXCOLOR    mSpecularLight;
	float        mSpecularPower;

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

	AmbientDiffuseSpecularDemo app(hInstance, L"Ambient Diffuse Specular Demo");
	gd3dApp = &app;

	DirectInput dinput;
	gDInput = &dinput;

	if (gd3dApp->checkDeviceCaps())
		return gd3dApp->run();
	else
		return 0;
}

AmbientDiffuseSpecularDemo::AmbientDiffuseSpecularDemo(HINSTANCE hInstance, std::wstring winCaption)
	: D3DApp(hInstance, winCaption)
{
	mGfxStats = new GfxStats();

	mCameraRadius    = 10.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight    = 5.0f;

	mLightVecW     = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	mDiffuseMtrl   = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	mDiffuseLight  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mAmbientMtrl   = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	mAmbientLight  = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	mSpecularMtrl  = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mSpecularLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mSpecularPower = 8.0f;

	D3DXMatrixIdentity(&mWorld);

	HR(D3DXCreateTeapot(gd3dDevice, &mTeapot, 0));
	buildFX();
	
	onResetDevice();

	InitAllVertexDeclarations();
}

AmbientDiffuseSpecularDemo::~AmbientDiffuseSpecularDemo()
{
	SafeDelete(mGfxStats);

	SafeRelease(mTeapot);
	SafeRelease(mFX);

	DestroyAllVertexDeclarations();
}

bool AmbientDiffuseSpecularDemo::checkDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gd3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2,0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2,0))
		return false;

	return true;
}

void AmbientDiffuseSpecularDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void AmbientDiffuseSpecularDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());

	buildProjMtx();
}

void AmbientDiffuseSpecularDemo::updateScene(float dt)
{
	mGfxStats->setVertexCount(mTeapot->GetNumVertices());
	mGfxStats->setTriCount(mTeapot->GetNumFaces());
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

void AmbientDiffuseSpecularDemo::drawScene()
{
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255,255,255), 1.0f, 0));
	HR(gd3dDevice->BeginScene());

	HR(mFX->SetTechnique(mhTech));

	HR(mFX->SetMatrix(mhWVP, &(mWorld*mView*mProj)));
	D3DXMATRIX worldInverseTranspose;
	D3DXMatrixInverse(&worldInverseTranspose, 0, &mWorld);
	D3DXMatrixTranspose(&worldInverseTranspose, &worldInverseTranspose);
	HR(mFX->SetMatrix(mhWorldInverseTranspose, &worldInverseTranspose));
	HR(mFX->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mDiffuseMtrl, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhAmbientMtrl, &mAmbientMtrl, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhAmbientLight, &mAmbientLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularLight, &mSpecularLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularMtrl, &mSpecularMtrl, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecularPower, mSpecularPower));
	HR(mFX->SetMatrix(mhWorld, &mWorld));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));
		HR(mTeapot->DrawSubset(0));
		HR(mFX->EndPass());
	}
	HR(mFX->End());

	mGfxStats->display(D3DCOLOR_XRGB(0,0,0));
	HR(gd3dDevice->EndScene());
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void AmbientDiffuseSpecularDemo::buildFX()
{
	ID3DXBuffer *errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, L"../src/chap10/AmbientDiffuseSpecularDemo/ambientdiffusespec.fx",
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBoxA(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech                  = mFX->GetTechniqueByName("DiffuseTech");
	mhWVP                   = mFX->GetParameterByName(0, "gWVP");
	mhWorldInverseTranspose = mFX->GetParameterByName(0, "gWorldInverseTranspose");
	mhLightVecW             = mFX->GetParameterByName(0, "gLightVecW");
	mhDiffuseMtrl           = mFX->GetParameterByName(0, "gDiffuseMtrl");
	mhDiffuseLight          = mFX->GetParameterByName(0, "gDiffuseLight");
	mhAmbientMtrl           = mFX->GetParameterByName(0, "gAmbientMtrl");
	mhAmbientLight          = mFX->GetParameterByName(0, "gAmbientLight");
	mhSpecularMtrl          = mFX->GetParameterByName(0, "gSpecularMtrl");
	mhSpecularLight         = mFX->GetParameterByName(0, "gSpecularLight");
	mhSpecularPower         = mFX->GetParameterByName(0, "gSpecularPower");
	mhEyePos                = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld                 = mFX->GetParameterByName(0, "gWorld");
}

void AmbientDiffuseSpecularDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void AmbientDiffuseSpecularDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

