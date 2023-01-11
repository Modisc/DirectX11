//***************************************************************************************
// RenderStates.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireframeRS     = 0;
ID3D11RasterizerState* RenderStates::NoCullRS        = 0;
ID3D11RasterizerState* RenderStates::CullClockwiseRS = 0;
	 
ID3D11BlendState*      RenderStates::AlphaToCoverageBS      = 0;
ID3D11BlendState*      RenderStates::TransparentBS          = 0;
ID3D11BlendState*      RenderStates::NoRenderTargetWritesBS = 0;

ID3D11DepthStencilState* RenderStates::MarkMirrorDSS     = 0;
ID3D11DepthStencilState* RenderStates::DrawReflectionDSS = 0;
ID3D11DepthStencilState* RenderStates::NoDoubleBlendDSS  = 0;

// 10�� �������� 5���� ���� �ڵ�
ID3D11DepthStencilState* RenderStates::NonDepthForWallDSS = 0;
ID3D11DepthStencilState* RenderStates::NonStencilForSkullDSS = 0;
ID3D11DepthStencilState* RenderStates::OnlyDepthForWallDSS = 0;
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
	// CullClockwiseRS
	//

	// Note: Define such that we still cull backfaces by making front faces CCW.
	// If we did not cull backfaces, then we have to worry about the BackFace
	// property in the D3D11_DEPTH_STENCIL_DESC.
	D3D11_RASTERIZER_DESC cullClockwiseDesc;
	ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
	cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
	cullClockwiseDesc.FrontCounterClockwise = false;
	cullClockwiseDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&cullClockwiseDesc, &CullClockwiseRS));

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
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;

	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&transparentDesc, &TransparentBS));

	//
	// NoRenderTargetWritesBS
	//

	D3D11_BLEND_DESC noRenderTargetWritesDesc = {0};
	noRenderTargetWritesDesc.AlphaToCoverageEnable = false;
	noRenderTargetWritesDesc.IndependentBlendEnable = false;

	noRenderTargetWritesDesc.RenderTarget[0].BlendEnable    = false;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	HR(device->CreateBlendState(&noRenderTargetWritesDesc, &NoRenderTargetWritesBS));

	//
	// MarkMirrorDSS
	//

	D3D11_DEPTH_STENCIL_DESC mirrorDesc;
	mirrorDesc.DepthEnable      = true;						// ���� ���۸� Ȱ��ȭ or ��Ȱ��ȭ (��Ȱ��ȭ�� ���� ������ �ȼ����� �� �̻� ���ŵ��� �ʴ´�)
	
	// D3D11_DEPTH_WRITE_MASK_ZERO or D3D11_DEPTH_WRITE_MASK_ALL ���� ���� 
	// �� �� ������ ���� ����. ���� ���۸��� Ȱ��ȭ �� ��� ZERO�� �����ϸ�
	// ���� ���� ����(����)�� �����ǳ� ���� ������ ������ ����ȴ�.
	// ALL�� �����ϸ� ���� ���� ���Ⱑ Ȱ��ȭ�Ǿ, ���� ������ ���ٽ� ������
	// ��� ����� �ȼ��� ���̰� ���� ���ۿ� ������ ��ϵȴ�.
	mirrorDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ZERO;						
    
	// ���� ������ ���̴� �� �Լ� ����, D3D11_COMPARISON_FUNC �������� �� ����� �����ؾ� �Ѵ�.
	// ������� ���� ������ D3D11_COMPARISON_LESS �̴�.
	mirrorDesc.DepthFunc        = D3D11_COMPARISON_LESS;							

    mirrorDesc.StencilEnable    = true;						// ���ٽ� ���� Ȱ��ȭ or ��Ȱ��ȭ
    mirrorDesc.StencilReadMask  = 0xff;						// ���ٽ� ���� ���ǹ�(�⺻���� �� � ��Ʈ�� ���� �ʴ� ����ũ�̴�.(0xff))
    mirrorDesc.StencilWriteMask = 0xff;						// ���ٽ� ���۸� ������ �� Ư�� ��Ʈ ������ ���ŵ��� �ʵ��� �ϴ� ��Ʈ ����ũ(�⺻���� �� � ��Ʈ�� ���� �ʴ� ����ũ�̴�.(0xff))

	// ���� �ﰢ���� ���� ���ٽ� ���� ���� ���
    mirrorDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;			// �ȼ� ������ ���ٽ� ������ �������� ���� ���ٽ� ���� ���� ����� ����(D3D11_STENCIL_OP �������� �� ��� ����)
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;			// �ȼ� ������ ���ٽ� ������ ��������� ���� ������ �������� �� ���ٽ� ���� ���� ����� ����(D3D11_STENCIL_OP �������� �� ��� ����)
	mirrorDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;			// �ȼ� ������ ���ٽ� ����, ���� ���� ��� ������� �� ���ٽ� ���� ���� ��� ����(D3D11_STENCIL_OP �������� �� ��� ����)
	mirrorDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;			// ���ٽ� ���� �� �Լ��� �����ϴ� ���(D3D11_COMPARISION_FUNC �������� �� ��� ����)

	// We are not rendering backfacing polygons, so these settings do not matter.
	// �ĸ� �ﰢ���� ���� ���ٽ� ���� ���� ���
	// �Ϲ����� ������ ��, �ĸ� ������ Ȱ��ȭ�ϹǷ� �ĸ� �ﰢ���� ���������� �ʱ� ������,
	// BackFace�� ������ ���õȴ�.
	// ���� ���ϱ���(9�� ȥ�� ������ ö�� ����)ó�� Ư�� �׷��� �˰��򿡼��� ���� ȿ���� ������.
    mirrorDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

	// ù��° ���� : ������ ���̤����ٽ� ���� ������ �����ϴ� D3D11_DEPTH_STENCIL_DESC ����ü�� ����Ű�� ������.
	// �ι�° ���� : ������ ID3D11DepthStencilState �������̽��� ����Ű�� �����͸� �����ش�.
	HR(device->CreateDepthStencilState(&mirrorDesc, &MarkMirrorDSS));

	//
	// DrawReflectionDSS
	//

	D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
	drawReflectionDesc.DepthEnable      = true;
	drawReflectionDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
	drawReflectionDesc.DepthFunc        = D3D11_COMPARISON_LESS; 
	drawReflectionDesc.StencilEnable    = true;
    drawReflectionDesc.StencilReadMask  = 0xff;
    drawReflectionDesc.StencilWriteMask = 0xff;

	drawReflectionDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilFunc   = D3D11_COMPARISON_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	drawReflectionDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilFunc   = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&drawReflectionDesc, &DrawReflectionDSS));

	//
	// NoDoubleBlendDSS
	//

	D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;
	noDoubleBlendDesc.DepthEnable      = true;
	noDoubleBlendDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
	noDoubleBlendDesc.DepthFunc        = D3D11_COMPARISON_LESS; 
	noDoubleBlendDesc.StencilEnable    = true;
    noDoubleBlendDesc.StencilReadMask  = 0xff;
    noDoubleBlendDesc.StencilWriteMask = 0xff;

	noDoubleBlendDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.FrontFace.StencilFunc   = D3D11_COMPARISON_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	noDoubleBlendDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.BackFace.StencilFunc   = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&noDoubleBlendDesc, &NoDoubleBlendDSS));


	//
	// 10�� �������� 5���� ���� �ڵ�
	//

	D3D11_DEPTH_STENCIL_DESC noDepthDesc;
	noDepthDesc.DepthEnable = false;
	noDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	noDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDepthDesc.StencilEnable = false;

	HR(device->CreateDepthStencilState(&noDepthDesc, &NonDepthForWallDSS));
	
	D3D11_DEPTH_STENCIL_DESC noStencilDesc;
	noStencilDesc.DepthEnable = true;
	noStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	noStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noStencilDesc.StencilEnable = false;

	HR(device->CreateDepthStencilState(&noStencilDesc, &NonStencilForSkullDSS));

	D3D11_DEPTH_STENCIL_DESC onlyDepthDesc;
	onlyDepthDesc.DepthEnable = true;
	onlyDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	onlyDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	onlyDepthDesc.StencilEnable = false;

	HR(device->CreateDepthStencilState(&onlyDepthDesc, &OnlyDepthForWallDSS));

	//
	//
	//
}

void RenderStates::DestroyAll()
{
	ReleaseCOM(WireframeRS);
	ReleaseCOM(NoCullRS);
	ReleaseCOM(CullClockwiseRS);

	ReleaseCOM(AlphaToCoverageBS);
	ReleaseCOM(TransparentBS);
	ReleaseCOM(NoRenderTargetWritesBS);

	ReleaseCOM(MarkMirrorDSS);
	ReleaseCOM(DrawReflectionDSS);
	ReleaseCOM(NoDoubleBlendDSS);
}