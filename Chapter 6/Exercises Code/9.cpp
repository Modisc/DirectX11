//***************************************************************************************
//
// 
//	연습문제 6장 9번
//	상자 예제에서 후면 선발을 비활성화하라(CullMode = None)
//	또한 훈면 선별 대신 전면 선별(CullMode = Front)도 시험해 보라.
//	선별 설정 방법 역시 크게 두가지
//	1. C++ 코드에서 ID3D11DeviceContext::RSSetState를 호출
//	2. 효과 패스의 패스 정의에서 SetRasterizerState()를 호출 (6.8.1절 240p 참고)
// 
// 
//***************************************************************************************

#include <iostream>

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "MathHelper.h"
#include "GameTimer.h"

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
	ID3DX11EffectScalarVariable* mGlobalTime;

	ID3D11InputLayout* mInputLayout;

	ID3D11RasterizerState* mCullModeRS;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mBoxVertexOffset;				// 연결된 정점 버퍼 안에서의 상자의 정점 오프셋 (이 문제에서의 값은 0이 될 것임)
	int mPyramidVertexOffset;			// 연결된 정점 버퍼 안에서의 피라미드의 정점 오프셋 (이 문제에서의 값은 8가 될 것임)

	UINT mBoxIndexOffset;				// 연결된 색인 버퍼 안에서의 상자의 시작 색인 (이 문제에서의 값은 0)
	UINT mPyramidIndexOffset;			// 연결된 색인 버퍼 안에서의 피라미드의 시작 색인 (이 문제에서의 값은 36)

	UINT mBoxIndexCount;				// 상자의 색인 개수 (이 문제에서의 값은 36이 될 것임)
	UINT mPyramidIndexCount;			// 피라미드의 색인 개수 (이 문제에서의 값은 18이 될 것임)

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
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
	mfxWorldViewProj(0), mInputLayout(0), mCullModeRS(0),
	mTheta(1.25f * MathHelper::Pi), mPhi(0.35f * MathHelper::Pi), mRadius(7.0f)
{
	mMainWndCaption = L"Box Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
	XMStoreFloat4x4(&mWorld, I);
}

SolutionApp::~SolutionApp()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mCullModeRS);
	FreeConsole();
}

bool SolutionApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	// 래스터화기

	// 기하구조를 와이어 프레임으로 생성한다
	D3D11_RASTERIZER_DESC cullModeDesc;

	// 설정해줄 값 이외 나머지 값들은 모두 기본값으로
	ZeroMemory(&cullModeDesc, sizeof(D3D11_RASTERIZER_DESC));

	cullModeDesc.FillMode = D3D11_FILL_WIREFRAME;
	//cullModeDesc.CullMode = D3D11_CULL_NONE;
	cullModeDesc.CullMode = D3D11_CULL_FRONT;
	//cullModeDesc.CullMode = D3D11_CULL_BACK;
	cullModeDesc.FrontCounterClockwise = false;	// 시계 방향이 전면

	cullModeDesc.DepthClipEnable = true;

	HR(md3dDevice->CreateRasterizerState(&cullModeDesc, &mCullModeRS));

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
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	md3dImmediateContext->RSSetState(mCullModeRS);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;



	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(36, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

// 마우스 클릭 좌표 지정 및 SetCapture 호출
void SolutionApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

// ReleaseCapture 호출
void SolutionApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

// 마우스 버튼 클릭 상태와 이동에 따라 카메라 이동
void SolutionApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		mRadius += dx - dy;

		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);

	}

	// 마우스 오른쪽 버튼을 누르고 있는 상태라면
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));
		std::cout << "dx : " << dx << ",\t" << "dy : " << dy << std::endl;

		mTheta -= dx;
		mPhi -= dy;

		std::cout << "mTheta : " << mTheta << ",\t" << "mPhi : " << mPhi << std::endl << std::endl;

		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

// 정점버퍼, 색인버퍼 생성
void SolutionApp::BuildGeometryBuffers()
{
	// Create vertex buffer
	Vertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::White)   },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Black)   },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Red)     },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Green)   },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Blue)    },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Yellow)  },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Cyan)    },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Magenta) }
	};

	// 정점 버퍼를 서술하는 구조체를 채운다.
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;						// 버퍼가 쓰이는 방식(여기서는 버퍼 생성 후 변경 없다는 의미)
	vbd.ByteWidth = sizeof(Vertex) * 8;						// 생성할 정점 버퍼의 크기(바이트 단위)
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;				// 정점 버퍼의 경우 해당 플래그 사용하면 됨.
	vbd.CPUAccessFlags = 0;									// CPU가 버퍼에 접근하는 방식을 결정하는 플래그들 지정(버퍼 생성 이후 CPU가 버퍼 읽기, 쓰기를 하지 않는다면 0을 지정)
	vbd.MiscFlags = 0;										// 정점 버퍼에 대해서는 그냥 0 지정
	vbd.StructureByteStride = 0;							// 구조적 버퍼에 저장된 원소 하나의 크기(바이트 단위) (구조적 버퍼를 위해서만 필요함. 그 외 모든 버퍼에 대해서는 0 지정)

	// 정점 버퍼를 초기화할 자료를 지정한다.
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;							// 정점 버퍼를 초기화할 자료를 담은 시스템 메모리 배열을 가리키는 포인터

	// vbd : 생성할 버퍼를 서술하는 구조체
	// vinitData : 버퍼를 초기화하는 데 사용할 자료
	// mBoxVB : 생성된 버퍼가 여기에 설정된다.
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));


	// Create the index buffer

	UINT indices[] = {
		// front face
		1, 2, 3,
		1, 3, 0,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	// 색인 버퍼를 서술하는 구조체를 채운다.
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	// 색인 버퍼를 초기화할 자료를 지정한다.
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	// 색인 버퍼를 생성한다.
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
