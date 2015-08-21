#include "d3dApp.h"
#include "directInput.h"
#include "gfxStats.h"
#include "Vertex.h"
#include <string.h>

class ColoredCubeDemo : public D3DApp
{
public:
	ColoredCubeDemo(HINSTANCE hInstance, std::wstring winCaption);
	virtual ~ColoredCubeDemo();

	virtual bool checkDeviceCaps() override;
	virtual void onLostDevice() override;
	virtual void onResetDevice() override;
	virtual void updateScene(float dt) override;
	virtual void drawScene() override;

private:
	void buildVertexBuffer();
	void buildIndexBuffer();
	void buildFX();
	void buildProjMtx();
	void buildViewMtx();

private:
	GfxStats *mGfxStats;

	IDirect3DVertexBuffer9 *mVB;
	IDirect3DIndexBuffer9  *mIB;
	ID3DXEffect            *mFX;
	D3DXHANDLE              mhTech;
	D3DXHANDLE              mhWVP;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined (DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ColoredCubeDemo app(hInstance, L"ColoredCubeDemo");
	gd3dApp = &app;

	DirectInput dinput;
	gDInput = &dinput;

	return gd3dApp->run();
}

ColoredCubeDemo::ColoredCubeDemo(HINSTANCE hInstance, std::wstring winCaption)
	: D3DApp(hInstance, winCaption)
{
	mGfxStats = new GfxStats();

	mCameraRadius    = 10.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight    = 5.0f;

	buildVertexBuffer();
	buildIndexBuffer();
	buildFX();

	onResetDevice();

	InitAllVertexDeclarations();
}

ColoredCubeDemo::~ColoredCubeDemo()
{
	SafeDelete(mGfxStats);

	SafeRelease(mVB);
	SafeRelease(mIB);

	DestroyAllVertexDeclarations();
}

void ColoredCubeDemo::buildVertexBuffer()
{
	const D3DCOLOR WHITE   = D3DCOLOR_XRGB(255,255,255);
	const D3DCOLOR BLACK   = D3DCOLOR_XRGB(0,0,0);
	const D3DCOLOR RED     = D3DCOLOR_XRGB(255,0,0);
	const D3DCOLOR GREEN   = D3DCOLOR_XRGB(0,255,0);
	const D3DCOLOR BLUE    = D3DCOLOR_XRGB(0,0,255);
	const D3DCOLOR YELLOW  = D3DCOLOR_XRGB(255,255,0);
	const D3DCOLOR CYAN    = D3DCOLOR_XRGB(0,255,255);
	const D3DCOLOR MAGENTA = D3DCOLOR_XRGB(255,0,255);

	HR(gd3dDevice->CreateVertexBuffer(8*sizeof(VertexCol), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mVB, 0));

	VertexCol *v = 0;
	HR(mVB->Lock(0, 0, (void**)&v, 0));

	v[0] = VertexCol(-1.0f, -1.0f, -1.0f, WHITE);
	v[1] = VertexCol(-1.0f,  1.0f, -1.0f, BLACK);
	v[2] = VertexCol( 1.0f,  1.0f, -1.0f, RED);
	v[3] = VertexCol( 1.0f, -1.0f, -1.0f, GREEN);
	v[4] = VertexCol(-1.0f, -1.0f,  1.0f, BLUE);
	v[5] = VertexCol(-1.0f,  1.0f,  1.0f, YELLOW);
	v[6] = VertexCol( 1.0f,  1.0f,  1.0f, CYAN);
	v[7] = VertexCol( 1.0f, -1.0f,  1.0f, MAGENTA);

	HR(mVB->Unlock());
}

void ColoredCubeDemo::buildIndexBuffer()
{
	HR(gd3dDevice->CreateIndexBuffer(36*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIB, 0));

	WORD *k = 0;
	HR(mIB->Lock(0, 0, (void**)&k, 0));

	// front
	k[0] = 0; k[1] = 1; k[2] = 2;
	k[3] = 0; k[4] = 2; k[5] = 3;

	// back
	k[6] = 4; k[7] = 6; k[8] = 5;
	k[9] = 4; k[10] = 7; k[11] = 6;

	// left
	k[12] = 4; k[13] = 5; k[14] = 1;
	k[15] = 4; k[16] = 1; k[17] = 0;

	// right
	k[18] = 3; k[19] = 2; k[20] = 6;
	k[21] = 3; k[22] = 6; k[23] = 7;

	// top
	k[24] = 1; k[25] = 5; k[26] = 6;
	k[27] = 1; k[28] = 6; k[29] = 2;

	k[30] = 4; k[31] = 0; k[32] = 3;
	k[33] = 4; k[34] = 3; k[35] = 7;

	HR(mIB->Unlock());
}

void ColoredCubeDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

void ColoredCubeDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

bool ColoredCubeDemo::checkDeviceCaps()
{
	return true;
}

void ColoredCubeDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
}

void ColoredCubeDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	buildProjMtx();
}

void ColoredCubeDemo::updateScene(float dt)
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

void ColoredCubeDemo::drawScene()
{
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255,255,255), 1.0f, 0));
	HR(gd3dDevice->BeginScene());

	HR(gd3dDevice->SetStreamSource(0, mVB, 0, sizeof(VertexCol)));
	HR(gd3dDevice->SetIndices(mIB));
	HR(gd3dDevice->SetVertexDeclaration(VertexCol::Decl));

	HR(mFX->SetTechnique(mhTech));
	HR(mFX->SetMatrix(mhWVP, &(mView*mProj)));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));
		HR(gd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12));
		HR(mFX->EndPass());
	}
	HR(mFX->End());


	mGfxStats->display(D3DCOLOR_XRGB(0,0,0));
	HR(gd3dDevice->EndScene());
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void ColoredCubeDemo::buildFX()
{
	ID3DXBuffer *errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, L"../src/chap09/ColoredCubeDemo/color.fx",
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBoxA(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFX->GetTechniqueByName("ColorTech");
	mhWVP  = mFX->GetParameterByName(0, "gWVP");
}
