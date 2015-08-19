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

struct VertexCol
{
	VertexCol() : pos(0.0f, 0.0f, 0.0f), col(0x00000000) {}
	VertexCol(float x, float y, float z, D3DCOLOR c) : pos(x,y,z), col(c) {}
	VertexCol(const D3DXVECTOR3 &v, D3DCOLOR c) : pos(v), col(c) {}

	D3DXVECTOR3 pos;
	D3DCOLOR    col;
	static IDirect3DVertexDeclaration9 *Decl;
};

void InitAllVertexDeclarations();
void DestroyAllVertexDeclaration();
