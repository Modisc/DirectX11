//***************************************************************************************
//
// 
//	연습문제 6장 2번
//	상자 예제를 정점 버퍼 두 개를 사용하도록 수정하라.
//
//
//***************************************************************************************

#include <iostream>

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "MathHelper.h"

struct PositionVertex
{
	XMFLOAT3 Pos;
};

struct ColorVertex
{
	XMFLOAT4 Color;
};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	~BoxApp();

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
	ID3D11Buffer* mBoxPosVB;
	ID3D11Buffer* mBoxColVB;
	ID3D11Buffer* mBoxIB;

	ID3DX11Effect* mFX;									// 하나 이상의 렌더링 패스들을 캡슐화하며, 각 패스마다 정점 셰이더가 연관된다.
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

	BoxApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


BoxApp::BoxApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mBoxPosVB(0), mBoxColVB(0), mBoxIB(0), mFX(0), mTech(0),
	mfxWorldViewProj(0), mInputLayout(0),
	mTheta(1.5f * MathHelper::Pi), mPhi(0.25f * MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Box Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

BoxApp::~BoxApp()
{
	ReleaseCOM(mBoxPosVB);
	ReleaseCOM(mBoxColVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
	FreeConsole();
}

bool BoxApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();		// 정점 버퍼, 색인 버퍼 생성
	BuildFX();					// 효과 파일 생성(셰이더 코드 컴파일)
	BuildVertexLayout();		// 입력 배치 생성

	return true;
}

// 투영 행렬 다시 계산
void BoxApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	// 창의 크기가 변했으므로 종횡비를 갱신하고 투영 행렬을 다시 계산한다.
	// XMMatrixPerspectiveFovLH : 원근투영 행렬을 구축해주는 함수
	// 첫번째 매개변수 : 수직 시야각(라디안 단위)
	// 두번째 매개변수 : 종횡비 = 너비 / 높이
	// 세번째 매개변수 : 가까운 평면까지의 거리
	// 네번째 매개변수 :  먼 평면까지의 거리
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

// 카메라가 바라보는 방향 설정(시야 행렬 구축)
void BoxApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	// 구면 좌표를 데카르트 좌표로 변환한다.
	// 구면 좌표계 : 3차원 공간 상의 점들을 나타내는 좌표계. 보통 (r, Theta, Phi)로 나타낸다.
	// 데카르트 좌표계 : 임의의 차원의 유클리드 공간을 나타내는 좌표계. 보통 x, y, z로 나타낸다. 
	// 참고 : https://iskim3068.tistory.com/23
	// x, y, z값을 변경하면 카메라 위치가 바뀜(mPhi, mTheta 변수 값을 변경해보면서 확인함)
	// 여기서 mPhi는 z축 양의 방향과 선분 OP가 이루는 각, mTheta는 x축 양의 방향과 선분 OP가 이루는 각
	// mTheta 값이 작아질수록 카메라가 왼쪽으로 이동, 커질수록 오른쪽으로 이동
	// mPhi 값이 작아질수록 카메라가 위로 이동, 커질수록 카메라가 아래로 이동
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	//std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;

	// Build the view matrix.
	// 시야 행렬을 구축한다.
	// 카메라 위치, 바라볼 대상 위치, 상향 벡터를 알면 시야 행렬을 구축할 수 있다.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();

	// mPhi값 범위 제한을 변경했을 경우 카메라 상하 이동 중에 튀는 문제에 대한 원인을 찾기 위해 테스트 한 코드
	// 해당 주석 코드를 적용하면 튀는 문제를 해결할 수 있다.(완벽하진 않음)
	/*XMVECTOR up;
	if(mPhi >= MathHelper::Pi || mPhi <= 0)
		up = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
	else
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);*/

	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

// 화면에 그림 그리기
void BoxApp::DrawScene()
{
	// 렌더 대상(화면)을 깨끗이 지우라고 명령한다.(지운다는 의미는 대상의 모든 원소를 특정한 하나의 값으로 설정하는 것을 뜻한다) (4.4.7절 151p)
	// => 후면 버퍼를 LightSteelBlue 색으로 지운다.(Colors::LightSteelBlue는 d3dUtil.h에 정의되어 있다.)
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	// 깊이 버퍼를 1.0f, 스텐실 버퍼를 0으로 지운다.
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// BuildVertexLayout 함수에서 생성한 입력 배치는 아직 장치에 묶이지 않은 상태이다.
	// 따라서 해당 입력 배치를 사용하고자 하는 장치에 묶어야 한다.
	md3dImmediateContext->IASetInputLayout(mInputLayout);

	// 기본도형 위상구조(5.5.2절 172p)
	// 기본도형을 형성하는 방식을 Direct3D에게 알려 주는 코드
	// 삼각형 목록을 적용해서 물체들을 그리라는 명령임.
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// BuildGeometryBuffers 함수에서 생성한 정점 버퍼와 색인 버퍼를 실제 파이프라인에 공급하려면
	// 버퍼를 장치의 한 입력 슬롯에 붙여야(묶어야)한다.
	// 아래는 이를 위한 명령이다.(정점버퍼 6.2절 221p, 색인버퍼 6.3절 223p)
	UINT posStride = sizeof(PositionVertex);
	UINT posOffset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxPosVB, &posStride, &posOffset);

	UINT colStride = sizeof(ColorVertex);
	UINT colOffset = 0;
	md3dImmediateContext->IASetVertexBuffers(1, 1, &mBoxColVB, &colStride, &colOffset);

	md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	// 상수 설정(6.8.4절 245p)
	// 세계ㆍ뷰ㆍ투영 결합 행렬을 설정한다.
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Apply의 첫번째 매개변수 : 이후 용도를 위해 예약된 것으로, 현재로서는 항상 0을 지정
		// Apply의 두번째 매개변수 : 패스를 적용할 장치 문맥
		// 효과 기법의 각 패스를 훑으면서 각 패스를 장치문맥에 적용함
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		// 36 indices for the box.
		// 색인 36개로 상자를 그린다.(6.3절 224p)
		// 첫번째 매개변수 : 현재 그리기 호출에서 사용할 색인들의 개수
		// 두번째 매개변수 : 사용할 첫 색인의 위치(버퍼 안에서의 색인)
		// 세번째 매개변수 : 정점들을 가져오기 전에, 이 그리기 호출에서 사용할 색인들이 더해지는 값
		md3dImmediateContext->DrawIndexed(36, 0, 0);
	}

	// 렌더링 처리를 최적화 하기 위한 힌트 전달
	// 스왑체인에서 렌더링 하고있는 윈도우가 최소화 되는 등
	// 표시되는 부분이 존재하지 않는 상태가 되면 DXGI_STATUS_OCCLUDED를 반환한다
	//--------------------------------------------------------------------------------- 
	// 위는 Present를 최적화 하는 방법에 대한 설명인 것 같음(https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=sorkelf&logNo=40161605210)
	// 여기서 Present는 후면 버퍼를 화면에 제시하는 기능이다.
	HR(mSwapChain->Present(0, 0));
}

// 마우스 클릭 좌표 지정 및 SetCapture 호출
void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
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
void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	// ReleaseCapture를 하면 영역 밖에서 더이상 WM_MOUSEMOVE가 받아지지 않는다.
	// 함수가 실패하면 return 0
	// 실패한 경우 GetLastError를 호출하여 실패 이유를 확인할 수 있다.
	ReleaseCapture();
}

// 마우스 버튼 클릭 상태와 이동에 따라 카메라 이동
void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
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
void BoxApp::BuildGeometryBuffers()
{
	// Create vertex buffer
	PositionVertex pos[] =
	{
		XMFLOAT3(-1.0f, -1.0f, -1.0f),
		XMFLOAT3(-1.0f, +1.0f, -1.0f),
		XMFLOAT3(+1.0f, +1.0f, -1.0f),
		XMFLOAT3(+1.0f, -1.0f, -1.0f),
		XMFLOAT3(-1.0f, -1.0f, +1.0f),
		XMFLOAT3(-1.0f, +1.0f, +1.0f),
		XMFLOAT3(+1.0f, +1.0f, +1.0f),
		XMFLOAT3(+1.0f, -1.0f, +1.0f)
	};

	ColorVertex color[] =
	{
		XMFLOAT4((const float*)&Colors::White),
		XMFLOAT4((const float*)&Colors::Black),
		XMFLOAT4((const float*)&Colors::Red),
		XMFLOAT4((const float*)&Colors::Green),
		XMFLOAT4((const float*)&Colors::Blue),
		XMFLOAT4((const float*)&Colors::Yellow),
		XMFLOAT4((const float*)&Colors::Cyan),
		XMFLOAT4((const float*)&Colors::Magenta)
	};


	// 정점 버퍼를 서술하는 구조체를 채운다.
	D3D11_BUFFER_DESC vbd0;
	vbd0.Usage = D3D11_USAGE_IMMUTABLE;						// 버퍼가 쓰이는 방식(여기서는 버퍼 생성 후 변경 없다는 의미)
	vbd0.ByteWidth = sizeof(PositionVertex) * 8;						// 생성할 정점 버퍼의 크기(바이트 단위)
	vbd0.BindFlags = D3D11_BIND_VERTEX_BUFFER;				// 정점 버퍼의 경우 해당 플래그 사용하면 됨.
	vbd0.CPUAccessFlags = 0;									// CPU가 버퍼에 접근하는 방식을 결정하는 플래그들 지정(버퍼 생성 이후 CPU가 버퍼 읽기, 쓰기를 하지 않는다면 0을 지정)
	vbd0.MiscFlags = 0;										// 정점 버퍼에 대해서는 그냥 0 지정
	vbd0.StructureByteStride = 0;							// 구조적 버퍼에 저장된 원소 하나의 크기(바이트 단위) (구조적 버퍼를 위해서만 필요함. 그 외 모든 버퍼에 대해서는 0 지정)

	// 정점 버퍼를 초기화할 자료를 지정한다.
	D3D11_SUBRESOURCE_DATA vinitData0;
	vinitData0.pSysMem = pos;							// 정점 버퍼를 초기화할 자료를 담은 시스템 메모리 배열을 가리키는 포인터

	// vbd : 생성할 버퍼를 서술하는 구조체
	// vinitData : 버퍼를 초기화하는 데 사용할 자료
	// mBoxVB : 생성된 버퍼가 여기에 설정된다.
	HR(md3dDevice->CreateBuffer(&vbd0, &vinitData0, &mBoxPosVB));



	D3D11_BUFFER_DESC vbd1;
	vbd1.Usage = D3D11_USAGE_IMMUTABLE;						
	vbd1.ByteWidth = sizeof(ColorVertex) * 8;			
	vbd1.BindFlags = D3D11_BIND_VERTEX_BUFFER;				
	vbd1.CPUAccessFlags = 0;								
	vbd1.MiscFlags = 0;										
	vbd1.StructureByteStride = 0;							

	D3D11_SUBRESOURCE_DATA vinitData1;
	vinitData1.pSysMem = color;		

	HR(md3dDevice->CreateBuffer(&vbd1, &vinitData1, &mBoxColVB));


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
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}

// 효과 파일 생성(셰이더 코드 컴파일)
void BoxApp::BuildFX()
{
	std::ifstream fin("fx/color.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	// compiledShader : 컴파일된 효과 자료를 가리키는 포인터
	// size : 컴파일된 효과 자료의 바이트 단위 크기
	// 0 : 효과 플래그(D3DX11CompileFromFile 함수의 Flags2에 지정한 것과 일치해야 함)
	// md3dDevice : Direct3D 11 장치를 가리키는 포인터
	// mFX : 생성된 효과 파일을 가리키는 포인터
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size,
		0, md3dDevice, &mFX));

	// 효과 객체에 있는 기법 객체를 가리키는 포인터 얻어오기
	mTech = mFX->GetTechniqueByName("ColorTech");

	// 효과 객체를 통해서 상수 버퍼 변수에 대한 포인터 얻기
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

// 입력 배치 생성
void BoxApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	// 입력 배치 서술 배열 생성
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// 파이프라인 상태를 포함하는 효과 전달에 대해 설명하는 구조체
	D3DX11_PASS_DESC passDesc;

	// ID3DX11EffectTechnique::GetPassByIndex
	// =>  주어진 색인에 해당하는 패스를 나타내는 ID3DX11EffectPass 인터페이스를 가리키는 포인터를 돌려준다.
	// ID3DX11EffectPass::GetDesc
	// =>  리턴형은 HRESULT
	// =>  매개변수 passDesc : 전달 설명에 대한 포인터
	// =>  특정 버퍼에 대한 정보를 얻어온다(?)
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);

	// 입력 배치 생성
	// vertexDesc : 정점 구조체를 서술하는 입력 배치 서술 배열
	// 2 : 입력 배치 서술 배열 원소의 개수
	// passDesc.pIAInputSignature : 정점 셰이더(입력 서명을 포함한)를 컴파일해서 얻은 바이트 코드를 가리키는 포인터
	// passDesc.IAInputSignatureSize : 얻은 바이트 코드의 크기(바이트 단위)
	// mInputLayout : 생성된 입력 배치를 이 포인터를 통해서 돌려준다.
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}
