//***************************************************************************************
// RenderStates.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireframeRS = 0;
ID3D11RasterizerState* RenderStates::NoCullRS    = 0;
	 
ID3D11BlendState*      RenderStates::AlphaToCoverageBS = 0;
ID3D11BlendState*      RenderStates::TransparentBS     = 0;

// 10�� �������� 7���� ���� �ڵ�
ID3D11BlendState*	   RenderStates::AddBS = 0;
ID3D11DepthStencilState* RenderStates::NoDepthDSS = 0;
//

// 10�� �������� 8���� ���� �ڵ�
ID3D11BlendState* RenderStates::NoRenderTargetWritesBS = 0;
ID3D11DepthStencilState* RenderStates::DepthComplexityDSS = 0;
ID3D11DepthStencilState* RenderStates::DepthLevelDrawDSS = 0;
//

// 10�� �������� 8���� ���� �ڵ�
ID3D11BlendState* RenderStates::DrawDepthComplexityBS = 0;
ID3D11DepthStencilState* RenderStates::DepthDisableDSS = 0;
//

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

	//
	// 10�� �������� 7��
	//

	D3D11_BLEND_DESC addDesc = { 0 };

	addDesc.AlphaToCoverageEnable = false;
	addDesc.IndependentBlendEnable = false;

	addDesc.RenderTarget[0].BlendEnable = true;
	addDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	addDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	addDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	addDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	addDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	addDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	addDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&addDesc, &AddBS));

	D3D11_DEPTH_STENCIL_DESC noDepthDesc;

	noDepthDesc.DepthEnable = true;
	noDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	noDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDepthDesc.StencilEnable = false;

	HR(device->CreateDepthStencilState(&noDepthDesc, &NoDepthDSS));

	//
	//
	//

	//
	// 10�� �������� 8���� ���� �ڵ�
	//

	D3D11_BLEND_DESC noRenderTargetWritesDesc = { 0 };
	noRenderTargetWritesDesc.AlphaToCoverageEnable = false;
	noRenderTargetWritesDesc.IndependentBlendEnable = false;

	noRenderTargetWritesDesc.RenderTarget[0].BlendEnable = false;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	HR(device->CreateBlendState(&noRenderTargetWritesDesc, &NoRenderTargetWritesBS));

	D3D11_DEPTH_STENCIL_DESC depthComplexityDesc;
	depthComplexityDesc.DepthEnable = true;
	depthComplexityDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthComplexityDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthComplexityDesc.StencilEnable = true;
	depthComplexityDesc.StencilReadMask = 0xff;
	depthComplexityDesc.StencilWriteMask = 0xff;

	depthComplexityDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthComplexityDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthComplexityDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	depthComplexityDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthComplexityDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthComplexityDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthComplexityDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	depthComplexityDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR(device->CreateDepthStencilState(&depthComplexityDesc, &DepthComplexityDSS));

	D3D11_DEPTH_STENCIL_DESC depthLevelDrawDesc;
	depthLevelDrawDesc.DepthEnable = true;
	depthLevelDrawDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthLevelDrawDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthLevelDrawDesc.StencilEnable = true;
	depthLevelDrawDesc.StencilReadMask = 0xff;
	depthLevelDrawDesc.StencilWriteMask = 0xff;
	
	depthLevelDrawDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthLevelDrawDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthLevelDrawDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthLevelDrawDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	
	depthLevelDrawDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthLevelDrawDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthLevelDrawDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthLevelDrawDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&depthLevelDrawDesc, &DepthLevelDrawDSS));

	//
	//
	//

	//
	// 10�� �������� 9���� ���� �ڵ�
	//

	D3D11_BLEND_DESC drawDepthComplextiyDesc = { 0 };
	drawDepthComplextiyDesc.AlphaToCoverageEnable = false;
	drawDepthComplextiyDesc.IndependentBlendEnable = false;

	drawDepthComplextiyDesc.RenderTarget[0].BlendEnable = true;
	drawDepthComplextiyDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	drawDepthComplextiyDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	drawDepthComplextiyDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	drawDepthComplextiyDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	drawDepthComplextiyDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	drawDepthComplextiyDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	drawDepthComplextiyDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&drawDepthComplextiyDesc, &DrawDepthComplexityBS));

	D3D11_DEPTH_STENCIL_DESC depthDisableDesc;
	depthDisableDesc.DepthEnable = false;
	depthDisableDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisableDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisableDesc.StencilEnable = false;

	HR(device->CreateDepthStencilState(&depthDisableDesc, &DepthDisableDSS));

	//
	//
	//


}

void RenderStates::DestroyAll()
{
	ReleaseCOM(WireframeRS);
	ReleaseCOM(NoCullRS);
	ReleaseCOM(AlphaToCoverageBS);
	ReleaseCOM(TransparentBS);

	ReleaseCOM(AddBS);
	ReleaseCOM(NoDepthDSS);
	
	ReleaseCOM(NoRenderTargetWritesBS);
	ReleaseCOM(DepthComplexityDSS);
	ReleaseCOM(DepthLevelDrawDSS);

	ReleaseCOM(DrawDepthComplexityBS);
	ReleaseCOM(DepthDisableDSS);
}