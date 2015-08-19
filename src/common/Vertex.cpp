#include "Vertex.h"

IDirect3DVertexDeclaration9 *VertexPos::Decl = 0;
IDirect3DVertexDeclaration9 *VertexCol::Decl = 0;

void InitAllVertexDeclarations()
{
	D3DVERTEXELEMENT9 VertexPosElems[] = {
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		D3DDECL_END()
	};
	HR(gd3dDevice->CreateVertexDeclaration(VertexPosElems, &VertexPos::Decl));

	D3DVERTEXELEMENT9 VertexColElems[] = {
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END()
	};
	HR(gd3dDevice->CreateVertexDeclaration(VertexColElems, &VertexCol::Decl));
}

void DestroyAllVertexDeclaration()
{
	SafeRelease(VertexPos::Decl);
	SafeRelease(VertexCol::Decl);
}
