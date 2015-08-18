#include "Vertex.h"

IDirect3DVertexDeclaration9 *VertexPos::Decl = 0;

void InitAllVertexDeclarations()
{
	D3DVERTEXELEMENT9 elems[] = {
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		D3DDECL_END()
	};
	HR(gd3dDevice->CreateVertexDeclaration(elems, &VertexPos::Decl));
}

void DestroyAllVertexDeclaration()
{
	SafeRelease(VertexPos::Decl);
}
