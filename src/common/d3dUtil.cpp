#include "d3dUtil.h"

void GenTriGrid(int numVertRows, int numVertCols, float dx, float dz, 
				const D3DXVECTOR3 &center, std::vector<D3DXVECTOR3> &verts, std::vector<DWORD> &indices)
{
	int numVertices = numVertRows*numVertCols;
	int numCellRows = numVertRows-1;
	int numCellCols = numVertCols-1;

	int numTris = numCellRows*numCellCols*2;
	float width = (float)numCellCols * dx;
	float depth = (float)numCellRows * dz;

	// vertex generation
	verts.resize(numVertices);

	float xOffset = -width * 0.5f;
	float zOffset =  depth * 0.5f;

	int k = 0;
	for (float i = 0; i < numVertRows; ++i)
	{
		for (float j = 0; j < numVertCols; ++j)
		{
			verts[k].x =  j * dx + xOffset;
			verts[k].z = -i * dz + zOffset;
			verts[k].y = 0.0f;

			D3DXMATRIX T;
			D3DXMatrixTranslation(&T, center.x, center.y, center.z);
			D3DXVec3TransformCoord(&verts[k], &verts[k], &T);

			++k;
		}
	}

	// index generation
	indices.resize(numTris*3);

	k = 0;
	for (DWORD i = 0; i < (DWORD)numCellRows; ++i)
	{
		for (DWORD j = 0; j < (DWORD)numCellCols; ++j)
		{
			indices[k]   =   i   * numVertCols + j;
			indices[k+1] =   i   * numVertCols + j + 1;
			indices[k+2] = (i+1) * numVertCols + j;

			indices[k+3] = (i+1) * numVertCols + j;
			indices[k+4] =   i   * numVertCols + j + 1;
			indices[k+5] = (i+1) * numVertCols + j + 1;

			k += 6;
		}
	}
}