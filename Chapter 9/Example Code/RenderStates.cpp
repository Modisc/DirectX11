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
	
	// true 설정시, 다중표본화 기법인 알파포괄도(alpha-to-coverage) 변환이 활성화 된다.
	// 이를 위해서는 다중표본화를 활성화 해야한다.
	// 11장에서 알파포괄도 변환을 사용하는 예제가 나온다.
	transparentDesc.AlphaToCoverageEnable = false;		

	// D3D11에서는 장면을 최대 8개까지 렌더 대상들에 동시에 렌더링할 수 있다.
	// 해당 플래그를 true로 설정하면 각 렌더 대상마다 혼합을 개별적으로 수행할 수 있다.
	// false 설정시 D3D11_BLEND_DESC::RenderTarget 배열의 첫 원소에 있는 설정에 따라
	// 모든 렌더 대상의 혼합 연산이 동일하게 수행된다.
	transparentDesc.IndependentBlendEnable = false;

	// D3D11_RENDER_TARGET_BLEND_DESC 원소 여덟 개짜리 배열로, 해당 배열의 i번째 원소는
	// 다중 렌더 대상의 i번째 렌더 대상에 적용할 혼합 설정을 담은 구조체이다.
	//D3D11_BLEND_DESC::RenderTarget

	transparentDesc.RenderTarget[0].BlendEnable = true;								// 혼합의 활성화 여부를 결정한다.
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;				// RGB 성분 혼합의 원본 혼합 계수 Fsrc를 뜻한다. D3D11_BLEND 열거형의 한 멤버를 지정해야 한다.
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;			// RGB 성분 혼합의 대상 혼합 계수 Fdst를 뜻한다. D3D11_BLEND 열거형의 한 멤버를 지정해야 한다.
	transparentDesc.RenderTarget[0].BlendOp =  D3D11_BLEND_OP_ADD;					// RGB 성분 혼합 연산자를 뜻한다. D3D11_BLEND_OP 열거형의 한 멤버를 지정해야 한다.
	transparentDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;				// 알파 성분 혼합의 원본 혼합 계수 Fsrc를 뜻한다. D3D11_BLEND 열거형의 한 멤버를 지정해야 한다.
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;				// 알파 성분 혼합의 대상 혼합 계수 Fdst를 뜻한다. D3D11_BLEND 열거형의 한 멤버를 지정해야 한다.
	transparentDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;			// 알파 성분 혼합의 혼합 연산자를 뜻한다. D3D11_BLEND_OP 열거형의 한 멤버를 지정해야 한다.
	
	// 렌더 대상 쓰기 마스크(write mask)로, 몇몇 플래그들을 하나 이상 조합해서 지정할 수 있다.
	// 해당 플래그들은 혼합의 결과를 후면 버퍼의 어떤 색상 채널들에 기록할 것인지를 결정한다.
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