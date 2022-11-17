//***************************************************************************************
//
// 
//	연습문제 6장 3-(a)번
//	그림 5.13(a)에 나온 점 목록을 그려라
//
// 
//***************************************************************************************

#include <iostream>

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "MathHelper.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class SolutionApp : public D3DApp
{
public:
	SolutionApp(HINSTANCE hInstance);
	~SolutionApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	ID3DX11Effect* mFX;									
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
	// 디버그 빌드의 경우 실행시점 메모리 점검 기능을 활성화한다.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// 아래 코드들을 작성해주면 디버그용 콘솔이 프로그램 실행시 함께 나오게 된다.
	// 사용 스트림을 stdout(표준출력) 스트림으로 설정했으므로
	// std::cout 등과 같은 출력 함수들로 log를 남길 수가 있다.
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);

#endif

	SolutionApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


SolutionApp::SolutionApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mVB(0), mIB(0), mFX(0), mTech(0),
	mfxWorldViewProj(0), mInputLayout(0),
	mTheta(1.5f * MathHelper::Pi), mPhi(0.5f * MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Box Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

SolutionApp::~SolutionApp()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
	FreeConsole();
}

bool SolutionApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();	
	BuildFX();				
	BuildVertexLayout();	

	return true;
}

// 투영 행렬 다시 계산
void SolutionApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

// 카메라가 바라보는 방향 설정(시야 행렬 구축)
void SolutionApp::UpdateScene(float dt)
{
	
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);
	
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

// 화면에 그림 그리기
void SolutionApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);

	// 기본도형 위상구조를 점 목록으로 설정함.
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		
		// 그리기에 사용할 색인들의 개수 9개
		md3dImmediateContext->DrawIndexed(9, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

// 마우스 클릭 좌표 지정 및 SetCapture 호출
void SolutionApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	// 마우스 클릭 하는 순간의 클릭 좌표를 저장해둠
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	// 마우스가 윈도우창 바깥으로 나갔을 때도, WM_MOUSEMOVE를 받기 위한 함수
	// 이전에 SetCapture 했던 HWND가 있다면 이전 HWND를 반환한다. 없다면 NULL 반환
	// SetCapture를 했다면 후에 ReleaseCapture를 호출해야 한다.
	SetCapture(mhMainWnd);
}

// ReleaseCapture 호출
void SolutionApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	// ReleaseCapture를 하면 영역 밖에서 더이상 WM_MOUSEMOVE가 받아지지 않는다.
	// 함수가 실패하면 return 0
	// 실패한 경우 GetLastError를 호출하여 실패 이유를 확인할 수 있다.
	ReleaseCapture();
}

// 마우스 버튼 클릭 상태와 이동에 따라 카메라 이동
void SolutionApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	// 마우스 왼쪽 버튼을 누르고 있는 상태라면
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		// 1픽셀이 장면의 0.005단위가 되게 한다.
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		// 마우스 입력에 기초해서 궤도 카메라의 반지름을 갱신한다.
		mRadius += dx - dy;

		// Restrict the radius.
		// 반지름을 특정 범위로 제한한다.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);

	}

	// 마우스 오른쪽 버튼을 누르고 있는 상태라면
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		// 1픽셀이 1/4도(디그리 단위)가 되게 한다.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));
		std::cout << "dx : " << dx << ",\t" << "dy : " << dy << std::endl;

		// Update angles based on input to orbit camera around box.
		// 마우스 입력에 기초한 각도로 상자 주변의 궤도 카메라를 갱신한다.
		mTheta -= dx;
		mPhi -= dy;

		std::cout << "mTheta : " << mTheta << ",\t" << "mPhi : " << mPhi << std::endl << std::endl;

		// Restrict the angle mPhi.
		// 각도를 mPhi로 한정한다.
		// 각도 제한이 없으면 카메라가 뒤집힐 수도 있으며, 카메라가 뒤집히게 되면
		// 상향 벡터의 방향(값)이 UpdateScene에서 설정한 값과 달라지게 되므로 도형이 뒤집히는 등의 문제가 발생할 수 있다.
		// ex: Pi - 0.1f에서 - 0.1f를 지우면 위에서 설명한 문제가 발생한다.
		// - 0.1f를 지울거라면 UpdateScene의 상향벡터 up을 조건에 따라 다르게 설정해주는 조건문을 주석에 작성해 두었으니
		// 코드를 수정해야 한다.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

// 정점버퍼, 색인버퍼 생성
void SolutionApp::BuildGeometryBuffers()
{
	Vertex vertices[] =
	{
		{ XMFLOAT3(-3.0f, -1.0f, 0.0f), XMFLOAT4((const float*)&Colors::Red)	},
		{ XMFLOAT3(-2.0f, +1.0f, 0.0f), XMFLOAT4((const float*)&Colors::Red)	},
		{ XMFLOAT3(-1.0f, -0.7f, 0.0f), XMFLOAT4((const float*)&Colors::Red)	},
		{ XMFLOAT3( 0.0f, +0.6f, 0.0f), XMFLOAT4((const float*)&Colors::Red)	},
		{ XMFLOAT3(+1.0f, -0.5f, 0.0f), XMFLOAT4((const float*)&Colors::Red)	},
		{ XMFLOAT3(+2.0f, +0.4f, 0.0f), XMFLOAT4((const float*)&Colors::Red)	},
		{ XMFLOAT3(+3.5f,  0.0f, 0.0f), XMFLOAT4((const float*)&Colors::Red)	},
		{ XMFLOAT3(+4.0f, +1.5f, 0.0f), XMFLOAT4((const float*)&Colors::Red)	},
		{ XMFLOAT3(+5.5f, +1.0f, 0.0f), XMFLOAT4((const float*)&Colors::Red)	},
	};

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;			
	vbd.ByteWidth = sizeof(Vertex) * 9;				// 생성할 정점 버퍼의 크기
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	
	vbd.CPUAccessFlags = 0;						
	vbd.MiscFlags = 0;							
	vbd.StructureByteStride = 0;				
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;							
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	// 점 목록을 그릴 때 색인의 순서가 중요한가?
	// => 색인을 아무렇게나 초기화 해도 문제없이 정상 출력 됨.
	// => 색인은 상관없는 것 같음.
	UINT indices[] = {
		/*0, 1, 2,
		3, 5, 4,
		6, 7, 8*/
		0, 3, 4,
		1, 8, 5,
		2, 7, 6
	};
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 9;				// 생성할 색인 버퍼의 크기
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}

// 효과 파일 생성(셰이더 코드 컴파일)
void SolutionApp::BuildFX()
{
	std::ifstream fin("fx/color.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size,
		0, md3dDevice, &mFX));

	mTech = mFX->GetTechniqueByName("ColorTech");

	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

// 입력 배치 생성
void SolutionApp::BuildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	D3DX11_PASS_DESC passDesc;

	mTech->GetPassByIndex(0)->GetDesc(&passDesc);

	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}
