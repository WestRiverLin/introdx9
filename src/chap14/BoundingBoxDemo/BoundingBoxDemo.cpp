#include "d3dApp.h"
#include "directInput.h"
#include "gfxStats.h"
#include "Vertex.h"
#include <string.h>

class BoundingBoxDemo : public D3DApp
{
public:
	BoundingBoxDemo(HINSTANCE hInstance, std::wstring winCaption);
	virtual ~BoundingBoxDemo();

	virtual bool checkDeviceCaps() override;
	virtual void onLostDevice() override;
	virtual void onResetDevice() override;
	virtual void updateScene(float dt) override;
	virtual void drawScene() override;

private:
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();

private:
	GfxStats *mGfxStats;
	
	ID3DXMesh* mMesh;
	std::vector<Material> mMtrl;
	std::vector<IDirect3DTexture9*> mTex;

	IDirect3DTexture9* mWhiteTex;

	ID3DXMesh* mBox;
	Material   mBoxMtrl;
	AABB       mBoundingBox;
	D3DXMATRIX mBoundingBoxOffset;

	ID3DXEffect *mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;
	D3DXHANDLE   mhMtrl;
	D3DXHANDLE   mhLight;

	DirLight mLight;

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

	BoundingBoxDemo app(hInstance, L"Bounding Box Demo");
	gd3dApp = &app;

	DirectInput dinput;
	gDInput = &dinput;

	if (gd3dApp->checkDeviceCaps())
		return gd3dApp->run();
	else
		return 0;
}

BoundingBoxDemo::BoundingBoxDemo(HINSTANCE hInstance, std::wstring winCaption)
	: D3DApp(hInstance, winCaption)
{
	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	mCameraRadius    = 12.0f;
	mCameraRotationY = 1.2f * D3DX_PI;
	mCameraHeight    = 10.0f;

	mLight.dirW    = D3DXVECTOR3(0.0f, -1.0f, -2.0f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mLight.diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.spec    = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);

	LoadXFile(L"../src/chap14/BoundingBoxDemo/bigship1.x", &mMesh, mMtrl, mTex);
	D3DXMatrixIdentity(&mWorld);

	// Compute the bounding box
	VertexPNT *v = 0;
	HR(mMesh->LockVertexBuffer(0, (void**)&v));

	HR(D3DXComputeBoundingBox(&v[0].pos, mMesh->GetNumVertices(),
		sizeof(VertexPNT), &mBoundingBox.minPt, &mBoundingBox.maxPt));

	HR(mMesh->UnlockVertexBuffer());

	// Build a box mesh so that we can render the bounding box visually
	float width  = mBoundingBox.maxPt.x - mBoundingBox.minPt.x;
	float height = mBoundingBox.maxPt.y - mBoundingBox.minPt.y;
	float depth  = mBoundingBox.maxPt.z - mBoundingBox.minPt.z;
	HR(D3DXCreateBox(gd3dDevice, width, height, depth, &mBox, 0));

	// It is possible that the mesh was not centered about the origin
	// when it was modeled. But the bounding box mesh is built around the
	// origin. So offset the boudning box (mesh) center so that it
	// matches the true mathematical bounding box center.
	D3DXVECTOR3 center = mBoundingBox.center();
	D3DXMatrixTranslation(&mBoundingBoxOffset, center.x, center.y, center.z);

	// Define the box material -- make semi-transparent
	mBoxMtrl.ambient    = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	mBoxMtrl.diffuse    = D3DXCOLOR(0.0f, 0.0f, 1.0f, 0.5f);
	mBoxMtrl.spec       = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMtrl.specPower  = 8.0f;

	// Create the white dummy texture
	HR(D3DXCreateTextureFromFile(gd3dDevice, L"../src/chap14/BoundingBoxDemo/whitetex.dds", &mWhiteTex));

	mGfxStats->addVertices(mMesh->GetNumVertices());
	mGfxStats->addTriangles(mMesh->GetNumFaces());
	mGfxStats->addVertices(mBox->GetNumVertices());
	mGfxStats->addTriangles(mBox->GetNumFaces());

	buildFX();
	
	onResetDevice();
}

BoundingBoxDemo::~BoundingBoxDemo()
{
	SafeDelete(mGfxStats);

	SafeRelease(mFX);
	for (int i = 0; i < mTex.size(); ++i)
	{
		SafeRelease(mTex[i]);
	}

	SafeRelease(mWhiteTex);
	SafeRelease(mMesh);
	SafeRelease(mBox);

	DestroyAllVertexDeclarations();
}

bool BoundingBoxDemo::checkDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gd3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2,0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2,0))
		return false;

	return true;
}

void BoundingBoxDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void BoundingBoxDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());

	buildProjMtx();
}

void BoundingBoxDemo::updateScene(float dt)
{
	mGfxStats->update(dt);

	gDInput->poll();

	if (gDInput->keyDown(DIK_W))
		mCameraHeight += 25.0f * dt;
	if (gDInput->keyDown(DIK_S))
		mCameraHeight -= 25.0f * dt;

	// divide by 50 to make mouse less sensitive
	mCameraRotationY += gDInput->mouseDX() / 100.0f;
	mCameraRadius    += gDInput->mouseDY() / 25.0f;

	if (fabsf(mCameraRotationY) >= 2.0f * D3DX_PI)
		mCameraRotationY = 0.0f;

	if (mCameraRadius < 3.0f)
		mCameraRadius = 3.0f;

	buildViewMtx();
}

void BoundingBoxDemo::drawScene()
{
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));
	HR(gd3dDevice->BeginScene());

	HR(mFX->SetValue(mhLight, &mLight, sizeof(DirLight)));
	HR(mFX->SetMatrix(mhWVP, &(mWorld*mView*mProj)));

	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mWorld));

	HR(mFX->SetTechnique(mhTech));
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	for (int j = 0; j < mMtrl.size(); ++j)
	{
		HR(mFX->SetValue(mhMtrl, &mMtrl[j], sizeof(Material)));

		if (mTex[j] != 0)
		{
			HR(mFX->SetTexture(mhTex, mTex[j]));
		}
		else
		{
			HR(mFX->SetTexture(mhTex, mWhiteTex));
		}

		HR(mFX->CommitChanges());
		HR(mMesh->DrawSubset(j));
	}

	// Draw the bounding box with alpha blending
	HR(gd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	HR(gd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	HR(gd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	HR(mFX->SetMatrix(mhWVP, &(mBoundingBoxOffset*mView*mProj)));
	D3DXMatrixInverse(&worldInvTrans, 0, &mBoundingBoxOffset);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mBoundingBoxOffset));
	HR(mFX->SetValue(mhMtrl, &mBoxMtrl, sizeof(Material)));
	HR(mFX->SetTexture(mhTex, mWhiteTex));
	HR(mFX->CommitChanges());
	HR(mBox->DrawSubset(0));
	HR(gd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));

	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->display(D3DCOLOR_XRGB(0,0,0));
	HR(gd3dDevice->EndScene());
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void BoundingBoxDemo::buildFX()
{
	ID3DXBuffer *errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, L"../src/chap14/BoundingBoxDemo/PhongDirLtTex.fx",
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBoxA(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech          = mFX->GetTechniqueByName("PhongDirLtTexTech");
	mhWVP           = mFX->GetParameterByName(0, "gWVP");
	mhWorldInvTrans = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhMtrl          = mFX->GetParameterByName(0, "gMtrl");
	mhLight         = mFX->GetParameterByName(0, "gLight");
	mhEyePos        = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld         = mFX->GetParameterByName(0, "gWorld");
	mhTex           = mFX->GetParameterByName(0, "gTex");
}

void BoundingBoxDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void BoundingBoxDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}
