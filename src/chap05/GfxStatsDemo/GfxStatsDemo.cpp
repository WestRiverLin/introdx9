#include "d3dApp.h"
#include "gfxStats.h"
#include <string.h>

class GfxStatsDemo : public D3DApp
{
public:
	GfxStatsDemo(HINSTANCE hInstance, std::wstring winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	virtual ~GfxStatsDemo();

	virtual bool checkDeviceCaps() override;
	virtual void onLostDevice() override;
	virtual void onResetDevice() override;
	virtual void updateScene(float dt) override;
	virtual void drawScene() override;

private:
	GfxStats *mGfxStats;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined (DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	GfxStatsDemo app(hInstance, L"GfxStatsDemo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	return gd3dApp->run();
}

GfxStatsDemo::GfxStatsDemo(HINSTANCE hInstance, std::wstring winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	mGfxStats = new GfxStats();
}

GfxStatsDemo::~GfxStatsDemo()
{
	SafeDelete(mGfxStats);
}

bool GfxStatsDemo::checkDeviceCaps()
{
	return true;
}

void GfxStatsDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
}

void GfxStatsDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
}

void GfxStatsDemo::updateScene(float dt)
{
	mGfxStats->update(dt);
}

void GfxStatsDemo::drawScene()
{
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255,255,255), 1.0f, 0));
	HR(gd3dDevice->BeginScene());

	mGfxStats->display();

	HR(gd3dDevice->EndScene());
	HR(gd3dDevice->Present(0, 0, 0, 0));
}
