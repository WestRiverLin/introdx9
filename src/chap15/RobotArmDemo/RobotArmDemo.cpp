//=============================================================================
// RobotArmDemo.cpp by Frank Luna (C) 2005 All Rights Reserved.
//
// Demonstrates how to animate mesh hierarchies.
//
// Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to 
//           alter the height of the camera.
//           Use '1', '2', '3', '4', and '5' keys to select the bone
//           to rotate.  Use the 'A' and 'D' keys to rotate the bone.
//=============================================================================

#include "d3dApp.h"
#include "directInput.h"
#include "gfxStats.h"
#include "Vertex.h"
#include <string.h>

struct BoneFrame
{
	// Note: The root bone's "parent" frame is the world space

	D3DXVECTOR3 pos;  // Relative to parent frame
	float zAngle;     // Relative to parent frame

	D3DXMATRIX toParentXForm;
	D3DXMATRIX toWorldXForm;
};


class RobotArmDemo : public D3DApp
{
public:
	RobotArmDemo(HINSTANCE hInstance, std::wstring winCaption);
	virtual ~RobotArmDemo();

	virtual bool checkDeviceCaps() override;
	virtual void onLostDevice() override;
	virtual void onResetDevice() override;
	virtual void updateScene(float dt) override;
	virtual void drawScene() override;

private:
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();

	void buildBoneWorldTransforms();

private:
	GfxStats *mGfxStats;
	
	// We only need one bone mesh. To draw several bones we just draw the
	// same mesh several times, but with a different transformation
	// applied so that it is drawn in a different place
	ID3DXMesh* mBoneMesh;
	std::vector<Material> mMtrl;
	std::vector<IDirect3DTexture9*> mTex;

	// Our robot arm has five bones.
	static const int NUM_BONES = 5;
	BoneFrame mBones[NUM_BONES];

	// Index into the bone array to the currently selected bone.
	// The user can select a bone and rotate it.
	int mBoneSelected;

	IDirect3DTexture9* mWhiteTex;

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

	RobotArmDemo app(hInstance, L"Robot Arm Demo");
	gd3dApp = &app;

	DirectInput dinput;
	gDInput = &dinput;

	if (gd3dApp->checkDeviceCaps())
		return gd3dApp->run();
	else
		return 0;
}

RobotArmDemo::RobotArmDemo(HINSTANCE hInstance, std::wstring winCaption)
	: D3DApp(hInstance, winCaption)
{
	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	mCameraRadius    = 9.0f;
	mCameraRotationY = 1.5f * D3DX_PI;
	mCameraHeight    = 0.0f;

	mLight.dirW    = D3DXVECTOR3(0.0f, 1.0f, 2.0f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.spec    = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);

	LoadXFile(L"../src/chap15/RobotArmDemo/bone.x", &mBoneMesh, mMtrl, mTex);
	D3DXMatrixIdentity(&mWorld);

	// Create the white dummy texture
	HR(D3DXCreateTextureFromFile(gd3dDevice, L"../src/chap15/RobotArmDemo/whitetex.dds", &mWhiteTex));

	// Initialize the bones relative to their parent frame.
	// The root is special--its parent frame is the world space.
	// As such, its position and angle are ignored--its 
	// toWorldXForm should be set explicitly (that is, the world
	// transform of the entire skeleton).
	//
	// *------*------*------*------
	//    0      1      2      3

	for (int i = 1; i < NUM_BONES; ++i)
	{
		mBones[i].pos    = D3DXVECTOR3(2.0f, 0.0f, 0.0f);
		mBones[i].zAngle = 0.0f;
	}
	mBones[0].pos    = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	mBones[0].zAngle = 0.0f;
	
	// Start off with the last(leaf) bone:
	mBoneSelected = NUM_BONES - 1;

	mGfxStats->addVertices(mBoneMesh->GetNumVertices() * NUM_BONES);
	mGfxStats->addTriangles(mBoneMesh->GetNumFaces() * NUM_BONES);

	buildFX();
	
	onResetDevice();
}

RobotArmDemo::~RobotArmDemo()
{
	SafeDelete(mGfxStats);

	SafeRelease(mFX);
	for (int i = 0; i < mTex.size(); ++i)
	{
		SafeRelease(mTex[i]);
	}

	SafeRelease(mWhiteTex);
	SafeRelease(mBoneMesh);

	DestroyAllVertexDeclarations();
}

bool RobotArmDemo::checkDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gd3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2,0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2,0))
		return false;

	return true;
}

void RobotArmDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void RobotArmDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());

	buildProjMtx();
}

void RobotArmDemo::updateScene(float dt)
{
	mGfxStats->update(dt);

	gDInput->poll();

	if (gDInput->keyDown(DIK_W))
		mCameraHeight += 25.0f * dt;
	if (gDInput->keyDown(DIK_S))
		mCameraHeight -= 25.0f * dt;

	// Allow the user to select a bone (zero based index)
	if (gDInput->keyDown(DIK_1)) mBoneSelected = 0;
	if (gDInput->keyDown(DIK_2)) mBoneSelected = 1;
	if (gDInput->keyDown(DIK_3)) mBoneSelected = 2;
	if (gDInput->keyDown(DIK_4)) mBoneSelected = 3;
	if (gDInput->keyDown(DIK_5)) mBoneSelected = 4;

	// Allow the user to rotate a bone
	if (gDInput->keyDown(DIK_A))
		mBones[mBoneSelected].zAngle += 1.0f * dt;
	if (gDInput->keyDown(DIK_D))
		mBones[mBoneSelected].zAngle -= 1.0f * dt;

	// If we rotate over 360 degrees, just roll back to 0
	if (fabsf(mBones[mBoneSelected].zAngle) >= 2.0f*D3DX_PI)
		mBones[mBoneSelected].zAngle = 0.0f;

	// divide by 50 to make mouse less sensitive
	mCameraRotationY += gDInput->mouseDX() / 100.0f;
	mCameraRadius    += gDInput->mouseDY() / 25.0f;

	if (fabsf(mCameraRotationY) >= 2.0f * D3DX_PI)
		mCameraRotationY = 0.0f;

	if (mCameraRadius < 2.0f)
		mCameraRadius = 2.0f;

	buildViewMtx();
}

void RobotArmDemo::drawScene()
{
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));
	HR(gd3dDevice->BeginScene());

	HR(mFX->SetValue(mhLight, &mLight, sizeof(DirLight)));

	HR(mFX->SetTechnique(mhTech));
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	buildBoneWorldTransforms();
	D3DXMATRIX T;
	D3DXMatrixTranslation(&T, -NUM_BONES, 0.0f, 0.0f);
	for (int i = 0; i < NUM_BONES; ++i)
	{
		// Append the transformation with a slight translation to better
		// center the skeleton at the center of the scene.
		mWorld = mBones[i].toWorldXForm * T;
		HR(mFX->SetMatrix(mhWVP, &(mWorld*mView*mProj)));
		D3DXMATRIX worldInvTrans;
		D3DXMatrixInverse(&worldInvTrans, 0, &mWorld);
		D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
		HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
		HR(mFX->SetMatrix(mhWorld, &mWorld));
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
			HR(mBoneMesh->DrawSubset(j));
		}
	}

	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->display(D3DCOLOR_XRGB(0,0,0));
	HR(gd3dDevice->EndScene());
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void RobotArmDemo::buildFX()
{
	ID3DXBuffer *errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, L"../src/chap15/RobotArmDemo/PhongDirLtTex.fx",
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

void RobotArmDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void RobotArmDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

void RobotArmDemo::buildBoneWorldTransforms()
{
	// First, construct the transformation matrix that transforms
	// the i-th bone into the cooridinate system of its parent

	D3DXMATRIX R, T;
	D3DXVECTOR3 p;
	for (int i = 0; i < NUM_BONES; ++i)
	{
		p = mBones[i].pos;
		D3DXMatrixRotationZ(&R, mBones[i].zAngle);
		D3DXMatrixTranslation(&T, p.x, p.y, p.z);
		mBones[i].toParentXForm = R * T;
	}

	// Now, the i-th object's world transform is given by its
	// to-parent transform, followed by its parent's to-parent transform,
	// followed by its grandparent's to-parent transform, and
	// so on, up to the root's to-parent transform.

	for (int i = 0; i < NUM_BONES; ++i)
	{
		D3DXMatrixIdentity(&mBones[i].toWorldXForm);

		for (int j = i; j >= 0; --j)
		{
			mBones[i].toWorldXForm *= mBones[j].toParentXForm;
		}
	}
}