//***************************************************************************************
// RenderStates.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireframeRS = 0;
ID3D11RasterizerState* RenderStates::NoCullRS    = 0;
	 
ID3D11BlendState*      RenderStates::AlphaToCoverageBS = 0;
ID3D11BlendState*      RenderStates::TransparentBS     = 0;

void RenderStates::InitAll(ID3D11Device* device)
{
	//
	// WireframeRS
	//
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&wireframeDesc, &WireframeRS));

	//
	// NoCullRS
	//
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&noCullDesc, &NoCullRS));

	//
	// AlphaToCoverageBS
	//

	D3D11_BLEND_DESC alphaToCoverageDesc = {0};
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS));

	//
	// TransparentBS
	//

	D3D11_BLEND_DESC transparentDesc = {0};
	
	// true ������, ����ǥ��ȭ ����� ����������(alpha-to-coverage) ��ȯ�� Ȱ��ȭ �ȴ�.
	// �̸� ���ؼ��� ����ǥ��ȭ�� Ȱ��ȭ �ؾ��Ѵ�.
	// 11�忡�� ���������� ��ȯ�� ����ϴ� ������ ���´�.
	transparentDesc.AlphaToCoverageEnable = false;		

	// D3D11������ ����� �ִ� 8������ ���� ���鿡 ���ÿ� �������� �� �ִ�.
	// �ش� �÷��׸� true�� �����ϸ� �� ���� ��󸶴� ȥ���� ���������� ������ �� �ִ�.
	// false ������ D3D11_BLEND_DESC::RenderTarget �迭�� ù ���ҿ� �ִ� ������ ����
	// ��� ���� ����� ȥ�� ������ �����ϰ� ����ȴ�.
	transparentDesc.IndependentBlendEnable = false;

	// D3D11_RENDER_TARGET_BLEND_DESC ���� ���� ��¥�� �迭��, �ش� �迭�� i��° ���Ҵ�
	// ���� ���� ����� i��° ���� ��� ������ ȥ�� ������ ���� ����ü�̴�.
	//D3D11_BLEND_DESC::RenderTarget

	transparentDesc.RenderTarget[0].BlendEnable = true;								// ȥ���� Ȱ��ȭ ���θ� �����Ѵ�.
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;				// RGB ���� ȥ���� ���� ȥ�� ��� Fsrc�� ���Ѵ�. D3D11_BLEND �������� �� ����� �����ؾ� �Ѵ�.
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;			// RGB ���� ȥ���� ��� ȥ�� ��� Fdst�� ���Ѵ�. D3D11_BLEND �������� �� ����� �����ؾ� �Ѵ�.
	transparentDesc.RenderTarget[0].BlendOp =  D3D11_BLEND_OP_ADD;					// RGB ���� ȥ�� �����ڸ� ���Ѵ�. D3D11_BLEND_OP �������� �� ����� �����ؾ� �Ѵ�.
	transparentDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;				// ���� ���� ȥ���� ���� ȥ�� ��� Fsrc�� ���Ѵ�. D3D11_BLEND �������� �� ����� �����ؾ� �Ѵ�.
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;				// ���� ���� ȥ���� ��� ȥ�� ��� Fdst�� ���Ѵ�. D3D11_BLEND �������� �� ����� �����ؾ� �Ѵ�.
	transparentDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;			// ���� ���� ȥ���� ȥ�� �����ڸ� ���Ѵ�. D3D11_BLEND_OP �������� �� ����� �����ؾ� �Ѵ�.
	
	// ���� ��� ���� ����ũ(write mask)��, ��� �÷��׵��� �ϳ� �̻� �����ؼ� ������ �� �ִ�.
	// �ش� �÷��׵��� ȥ���� ����� �ĸ� ������ � ���� ä�ε鿡 ����� �������� �����Ѵ�.
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;				

	HR(device->CreateBlendState(&transparentDesc, &TransparentBS));
}

void RenderStates::DestroyAll()
{
	ReleaseCOM(WireframeRS);
	ReleaseCOM(NoCullRS);
	ReleaseCOM(AlphaToCoverageBS);
	ReleaseCOM(TransparentBS);
}