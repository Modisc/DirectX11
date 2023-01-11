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

// 10장 연습문제 5번을 위한 코드
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
	mirrorDesc.DepthEnable      = true;						// 깊이 버퍼링 활성화 or 비활성화 (비활성화시 깊이 버퍼의 픽셀들이 더 이상 갱신되지 않는다)
	
	// D3D11_DEPTH_WRITE_MASK_ZERO or D3D11_DEPTH_WRITE_MASK_ALL 설정 가능 
	// 둘 다 지정할 수는 없다. 깊이 버퍼링이 활성화 된 경우 ZERO를 지정하면
	// 깊이 버퍼 쓰기(갱신)이 방지되나 깊이 판정은 여전히 수행된다.
	// ALL을 지정하면 깊이 버퍼 쓰기가 활성화되어서, 깊이 판정과 스텐실 판정을
	// 모두 통과한 픽셀의 깊이가 깊이 버퍼에 새로이 기록된다.
	mirrorDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ZERO;						
    
	// 깊이 판정에 쓰이는 비교 함수 설정, D3D11_COMPARISON_FUNC 열거형의 한 멤버를 지정해야 한다.
	// 통상적인 깊이 판정은 D3D11_COMPARISON_LESS 이다.
	mirrorDesc.DepthFunc        = D3D11_COMPARISON_LESS;							

    mirrorDesc.StencilEnable    = true;						// 스텐실 판정 활성화 or 비활성화
    mirrorDesc.StencilReadMask  = 0xff;						// 스텐실 판정 조건문(기본값은 그 어떤 비트도 막지 않는 마스크이다.(0xff))
    mirrorDesc.StencilWriteMask = 0xff;						// 스텐실 버퍼를 갱신할 때 특정 비트 값들이 갱신되지 않도록 하는 비트 마스크(기본값은 그 어떤 비트도 막지 않는 마스크이다.(0xff))

	// 전면 삼각형에 대한 스텐실 버퍼 적용 방식
    mirrorDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;			// 픽셀 단편이 스텐실 판정에 실패했을 때의 스텐실 버퍼 갱신 방식을 결정(D3D11_STENCIL_OP 열거형의 한 멤버 지정)
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;			// 픽셀 단편이 스텐실 판정에 통과했으나 깊이 판정에 실패했을 때 스텐실 버퍼 갱신 방식을 결정(D3D11_STENCIL_OP 열거형의 한 멤버 지정)
	mirrorDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;			// 픽셀 단편이 스텐실 판정, 깊이 판정 모두 통과했을 때 스텐실 버퍼 갱신 방식 결정(D3D11_STENCIL_OP 열거형의 한 멤버 지정)
	mirrorDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;			// 스텐실 판정 비교 함수를 결정하는 멤버(D3D11_COMPARISION_FUNC 열거형의 한 멤버 지정)

	// We are not rendering backfacing polygons, so these settings do not matter.
	// 후면 삼각형에 대한 스텐실 버퍼 적용 방식
	// 일반적인 렌더링 시, 후면 선별을 활성화하므로 후면 삼각형은 렌더링되지 않기 때문에,
	// BackFace의 설정은 무시된다.
	// 투명 기하구조(9장 혼합 예제의 철망 상자)처럼 특정 그래픽 알고리즘에서는 실제 효과를 가진다.
    mirrorDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

	// 첫번째 인자 : 생성할 깊이ㆍ스텐실 상태 집합을 서술하는 D3D11_DEPTH_STENCIL_DESC 구조체를 가리키는 포인터.
	// 두번째 인자 : 생성된 ID3D11DepthStencilState 인터페이스를 가리키는 포인터를 돌려준다.
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
	// 10장 연습문제 5번을 위한 코드
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