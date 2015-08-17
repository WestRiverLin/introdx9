#include "d3dApp.h"
#include "gfxStats.h"

class PageFlipDemo : public D3DApp
{
public:
	PageFlipDemo(HINSTANCE hInstance, std::wstring winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	virtual ~PageFlipDemo();

	virtual bool checkDeviceCaps() override;
	virtual void onLostDevice() override;
	virtual void onResetDevice() override;
	virtual void updateScene(float dt) override;
	virtual void drawScene() override;

private:
	GfxStats *mGfxStats;
	ID3DXSprite *mSprite;
	IDirect3DTexture9 *mFrames;
	D3DXVECTOR3 mSpriteCenter;

	int mCurrFrame;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	PageFlipDemo app(hInstance, L"Page Flip Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	return gd3dApp->run();
}


PageFlipDemo::PageFlipDemo(HINSTANCE hInstance, std::wstring winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	mGfxStats = new GfxStats();

	HR(D3DXCreateSprite(gd3dDevice, &mSprite));
	HR(D3DXCreateTextureFromFile(gd3dDevice, L"../src/chap05/PageFlipDemo/fireatlas.bmp", &mFrames));
	mSpriteCenter = D3DXVECTOR3(32.0f, 32.0f, 0.0f);

	mCurrFrame = 0;

	onResetDevice();
}

PageFlipDemo::~PageFlipDemo()
{
	SafeDelete(mGfxStats);
	SafeRelease(mSprite);
	SafeRelease(mFrames);
}

bool PageFlipDemo::checkDeviceCaps()
{
	return true;
}

void PageFlipDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mSprite->OnLostDevice());
}

void PageFlipDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mSprite->OnResetDevice());

	// Sets up the camera 1000 units back looking at the origin.
	D3DXMATRIX V;
	D3DXVECTOR3 pos(0.0f, 0.0f, -100.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXMatrixLookAtLH(&V, &pos, &target, &up);
	HR(gd3dDevice->SetTransform(D3DTS_VIEW, &V));

	// The following code defines the volume of space the camera sees
	D3DXMATRIX P;
	RECT R;
	GetClientRect(mhMainWnd, &R);
	float width  = (float)R.right;
	float height = (float)R.bottom;
	D3DXMatrixPerspectiveFovLH(&P, D3DX_PI*0.25f, width/height, 1.0f, 5000.0f);
	HR(gd3dDevice->SetTransform(D3DTS_PROJECTION, &P));

	// This code sets texture filters, which helps to smooth out distortions
	// when you scale a texture
	HR(gd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
	HR(gd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
	HR(gd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));

	// lighting off
	HR(gd3dDevice->SetRenderState(D3DRS_LIGHTING, false));

	// alpha blending
	HR(gd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
	HR(gd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
	HR(gd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	HR(gd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
}

void PageFlipDemo::updateScene(float dt)
{
	mGfxStats->setTriCount(2);
	mGfxStats->setVertexCount(4);
	mGfxStats->update(dt);

	// Keep track of how much time has accumulated
	static float timeAccum = 0.0f;
	timeAccum += dt;

	// Play animation at 30 frames per second
	if (timeAccum >= 1.0f / 30.0f)
	{
		++mCurrFrame;
		timeAccum = 0.0f;

		// 0, 1, ..., 29 (frame index)
		if (mCurrFrame > 29)
			mCurrFrame = 0;
	}
}

void PageFlipDemo::drawScene()
{
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0));

	HR(gd3dDevice->BeginScene());
	HR(mSprite->Begin(D3DXSPRITE_OBJECTSPACE|D3DXSPRITE_DONOTMODIFY_RENDERSTATE));

	// Compute rectangle on texture atlas of the current frame
	int i = mCurrFrame / 6; // row
	int j = mCurrFrame % 6; // column
	RECT R = {j*64, i*64, (j+1)*64, (i+1)*64};

	HR(gd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true));

	D3DXMATRIX M;
	D3DXMatrixIdentity(&M);
	HR(mSprite->SetTransform(&M));
	HR(mSprite->Draw(mFrames, &R, &mSpriteCenter, 0, D3DCOLOR_XRGB(255,255,255)));
	HR(mSprite->End());

	HR(gd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false));

	mGfxStats->display();

	HR(gd3dDevice->EndScene());
	HR(gd3dDevice->Present(0,0,0,0));
}
