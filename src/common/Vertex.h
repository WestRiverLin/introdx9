#pragma once

#include "d3dUtil.h"

struct VertexPos
{
	VertexPos() : pos(0.0f, 0.0f, 0.0f) {}
	VertexPos(float x, float y, float z) : pos(x,y,z) {}
	VertexPos(const D3DXVECTOR3& v) : pos(v) {}

	D3DXVECTOR3 pos;
	static IDirect3DVertexDeclaration9 *Decl;
};

void InitAllVertexDeclarations();
void DestroyAllVertexDeclaration();
