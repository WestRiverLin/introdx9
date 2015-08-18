#include "d3dApp.h"
#include "directInput.h"
#include "gfxStats.h"
#include "Vertex.h"
#include <string.h>

class MeshDemo : public D3DApp
{
public:
	MeshDemo(HINSTANCE hInstance, std::wstring winCaption);
	virtual ~MeshDemo();

	virtual bool checkDeviceCaps() override;
	virtual void onLostDevice() override;
	virtual void onResetDevice() override;
	virtual void updateScene(float dt) override;
	virtual void drawScene() override;

private:
	void buildGeoBuffers();
	void buildFX();
	void buildProjMtx();
	void buildViewMtx();

	void drawCylinders();
	void drawSpheres();

private:
	GfxStats *mGfxStats;

	DWORD mNumGridVertices;
	DWORD mNumGridTriangles;

	ID3DXMesh *mCylinder;
	ID3DXMesh *mSphere;

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

	MeshDemo app(hInstance, L"Mesh Demo");
	gd3dApp = &app;

	DirectInput dinput;
	gDInput = &dinput;

	if (gd3dApp->checkDeviceCaps())
		return gd3dApp->run();
	else
		return 0;
}

MeshDemo::MeshDemo(HINSTANCE hInstance, std::wstring winCaption)
	: D3DApp(hInstance, winCaption)
{
	mGfxStats = new GfxStats();

	mCameraRadius    = 10.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight    = 5.0f;

	HR(D3DXCreateCylinder(gd3dDevice, 1.0f, 1.0f, 6.0f, 20, 20, &mCylinder, 0));
	HR(D3DXCreateSphere(gd3dDevice, 1.0f, 20, 20, &mSphere, 0));

	buildGeoBuffers();
	buildFX();

	int numCylVerts    = mCylinder->GetNumVertices() * 14;
	int numSphereVerts = mSphere->GetNumVertices() * 14;
	int numCylTris     = mCylinder->GetNumFaces() * 14;
	int numSphereTris  = mSphere->GetNumFaces() * 14;

	mGfxStats->addVertices(mNumGridVertices);
	mGfxStats->addVertices(numCylVerts);
	mGfxStats->addVertices(numSphereVerts);
	mGfxStats->addTriangles(mNumGridTriangles);
	mGfxStats->addTriangles(numCylTris);
	mGfxStats->addTriangles(numSphereTris);

	onResetDevice();

	InitAllVertexDeclarations();
}

MeshDemo::~MeshDemo()
{
	SafeDelete(mGfxStats);

	SafeRelease(mVB);
	SafeRelease(mIB);
	SafeRelease(mFX);
	SafeRelease(mCylinder);
	SafeRelease(mSphere);

	DestroyAllVertexDeclaration();
}

bool MeshDemo::checkDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gd3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2,0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2,0))
		return false;

	return true;

	return true;
}

void MeshDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void MeshDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());

	buildProjMtx();
}

void MeshDemo::updateScene(float dt)
{
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

void MeshDemo::drawScene()
{
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255,255,255), 1.0f, 0));
	HR(gd3dDevice->BeginScene());

	HR(gd3dDevice->SetStreamSource(0, mVB, 0, sizeof(VertexPos)));
	HR(gd3dDevice->SetIndices(mIB));
	HR(gd3dDevice->SetVertexDeclaration(VertexPos::Decl));

	HR(mFX->SetTechnique(mhTech));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		HR(mFX->SetMatrix(mhWVP, &(mView*mProj)));
		HR(mFX->CommitChanges());
		HR(gd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mNumGridVertices, 0, mNumGridTriangles));

		drawCylinders();
		drawSpheres();

		HR(mFX->EndPass());
	}
	HR(mFX->End());

	mGfxStats->display(D3DCOLOR_XRGB(0,0,0));
	HR(gd3dDevice->EndScene());
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void MeshDemo::buildGeoBuffers()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	GenTriGrid(100, 100, 1.0f, 1.0f, D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	mNumGridVertices  = 100*100;
	mNumGridTriangles = 99*99*2;

	HR(gd3dDevice->CreateVertexBuffer(mNumGridVertices * sizeof(VertexPos),
		D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mVB, 0));

	VertexPos *v = 0;
	HR(mVB->Lock(0, 0, (void**)&v, 0));
	for (DWORD i = 0; i < mNumGridVertices; ++i) v[i] = verts[i];
	HR(mVB->Unlock());

	HR(gd3dDevice->CreateIndexBuffer(mNumGridTriangles*3*sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIB, 0));

	WORD *k = 0;
	HR(mIB->Lock(0, 0, (void**)&k, 0));
	for (DWORD i = 0; i < mNumGridTriangles*3; ++i) k[i] = (WORD)indices[i];
	HR(mIB->Unlock());
}

void MeshDemo::buildFX()
{
	ID3DXBuffer *errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, L"../src/chap08/MeshDemo/transform.fx",
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBoxA(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFX->GetTechniqueByName("TransformTech");
	mhWVP  = mFX->GetParameterByName(0, "gWVP");
}

void MeshDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

void MeshDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void MeshDemo::drawCylinders()
{
	D3DXMATRIX T, R;

	D3DXMatrixRotationX(&R, D3DX_PI*0.5f);

	for (int z = -30; z <= 30; z += 10)
	{
		D3DXMatrixTranslation(&T, -10.0f, 3.0f, (float)z);
		HR(mFX->SetMatrix(mhWVP, &(R*T*mView*mProj)));
		HR(mFX->CommitChanges());
		HR(mCylinder->DrawSubset(0));

		D3DXMatrixTranslation(&T, 10.0f, 3.0f, (float)z);
		HR(mFX->SetMatrix(mhWVP, &(R*T*mView*mProj)));
		HR(mFX->CommitChanges());
		HR(mCylinder->DrawSubset(0));
	}
}

void MeshDemo::drawSpheres()
{
	D3DXMATRIX T;

	for(int z = -30; z <= 30; z+= 10)
	{
		D3DXMatrixTranslation(&T, -10.0f, 7.5f, (float)z);
		HR(mFX->SetMatrix(mhWVP, &(T*mView*mProj)));
		HR(mFX->CommitChanges());
		HR(mSphere->DrawSubset(0));

		D3DXMatrixTranslation(&T, 10.0f, 7.5f, (float)z);
		HR(mFX->SetMatrix(mhWVP, &(T*mView*mProj)));
		HR(mFX->CommitChanges());
		HR(mSphere->DrawSubset(0));
	}
}
