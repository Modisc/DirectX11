//***************************************************************************************
//
//	�������� 6�� 1��
//	���� ���� ����ü�� ���� D3D11_INPUT_ELEMENT_DESC �迭�� �ۼ��϶�
//
//	struct Vertex
//	{
//	XMFLOAT3 Pos;
//	XMFLOAT3 Tangent;
//	XMFLOAT3 Normal;
//	XMFLOAT2 Tex0;
//	XMFLOAT2 Tex1;
//	XMCOLOR COLOR;
//	};
//
//***************************************************************************************

#include <iostream>

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "MathHelper.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Tangent;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex0;
	XMFLOAT2 Tex1;
	XMCOLOR COLOR;
};

class TestApp : public D3DApp
{
private:
	void BuildVertexLayout();
};

void TestApp::BuildVertexLayout() {
	D3D11_INPUT_ELEMENT_DESC desc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 52, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
}