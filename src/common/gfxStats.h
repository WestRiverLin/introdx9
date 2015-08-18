#pragma once

#include "d3dUtil.h"

class GfxStats
{
public:
	GfxStats();
	~GfxStats();

	void onLostDevice();
	void onResetDevice();

	void addVertices(DWORD n);
	void subVertices(DWORD n);
	void addTriangles(DWORD n);
	void subTriangles(DWORD n);

	void setTriCount(DWORD n);
	void setVertexCount(DWORD n);

	void update(float dt);
	void display(D3DCOLOR c = D3DCOLOR_XRGB(255,255,255));

private:
	GfxStats(const GfxStats& rhs);
	GfxStats& operator=(const GfxStats& rhs);

private:
	ID3DXFont *mFont;
	float mFPS;
	float mMilliSecPerFrame;
	DWORD mNumTris;
	DWORD mNumVertices;
};
