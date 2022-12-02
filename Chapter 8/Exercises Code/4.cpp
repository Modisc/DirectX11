//***************************************************************************************
//
// 
// 연습문제 8장 4번
// 연습문제 3을 이어서, 불덩이 텍스처를 시간의 함수로서 회전해서 각 면에 입히도록
// 예제를 수정하라
// 
//
//***************************************************************************************

#include <iostream>

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"

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

private:
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3D11ShaderResourceView* mFireMapSRV;
	ID3D11ShaderResourceView* mFireAlphaMapSRV;
	float mFrame;

	DirectionalLight mDirLights[3];
	Material mBoxMat;

	XMFLOAT4X4 mTexTransform;
	XMFLOAT4X4 mBoxWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mBoxVertexOffset;
	UINT mBoxIndexOffset;
	UINT mBoxIndexCount;

	XMFLOAT3 mEyePosW;
	
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

	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
#endif

	SolutionApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


SolutionApp::SolutionApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mFireMapSRV(0), mFireAlphaMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f),
	mFrame(0), mTheta(1.3f * MathHelper::Pi), mPhi(0.4f * MathHelper::Pi), mRadius(2.5f)
{
	mMainWndCaption = L"Crate Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mBoxWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mTexTransform, I);
	XMStoreFloat4x4(&mProj, I);

	mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[1].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}

SolutionApp::~SolutionApp()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mFireMapSRV);
	ReleaseCOM(mFireAlphaMapSRV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
}

bool SolutionApp::Init()
{
	if (!D3DApp::Init())
		return false;

	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);

	ID3D11Resource* texResource = nullptr;
	std::cout << texResource << std::endl;
	HR(CreateDDSTextureFromFile(md3dDevice,
		L"../flare.dds", &texResource, &mFireMapSRV));
	HR(CreateDDSTextureFromFile(md3dDevice,
		L"../flarealpha.dds", &texResource, &mFireAlphaMapSRV));
	ReleaseCOM(texResource)

		BuildGeometryBuffers();

	return true;
}

void SolutionApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 0.01f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void SolutionApp::UpdateScene(float dt)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	mFrame += dt * 2;

	// 아래의 과정을 여기서 실행시키고 basic.fx에서 정점셰이더에서 텍스처 좌표 자체를 움직여버리면
	// 합쳐놓은 이미지 2개 모두가 회전하게 된다. 이미지 2개 중 flare 이미지만 회전하는 것을 원하므로
	// basic.fx 파일 안에서 함수를 하나 짜서, 같은 로직으로 하되, 회전행렬 등을 직접 구현한다.
	// => basic.fx 파일에서는 XMMatrixRotationZ 등 지원되는 함수가 없기 때문이다. 
	// (없는건지 못찾는건지 모르겠음. 어려운 행렬이 아니니 간단하게 직접 구현하는 걸로 함)

	// 도형 면의 좌상단을 (0, 0) 우하단을 (1, 1)로 가정한다.
	// 텍스처 이미지 바깥쪽인 (-0.5, -0.5) 좌표를 현재 텍스처 이미지의 (0, 0) 위치로 가져온다.
	// 그러면 실제 텍스처 이미지는 좌상단 1/4 지역만큼만 현재 도형 면의 우하닥 1/4 지역에 남게되고
	// 나머지 이미지는 보이지 않는다. 남은 빈 공간들은 사용자가 설정한 텍스처 좌표 지정 모드에 따라
	// 다르게 보인다.
	//XMMATRIX fireTranslateToCenter = XMMatrixTranslation(-0.5f, -0.5f, 0.0f);
	
	// 텍스처 이미지의 중심 좌표 (0, 0)은 도형 면의 우하단 1/4 지역에 남은 이미지의 좌상단 점이다. 즉, 도형 면의 (0.5, 0.5) 위치이다.
	// 해당 점을 중심으로 텍스처를 반시계방향으로 회전시킨다.
	//XMMATRIX fireRotOffset = XMMatrixRotationZ(mFrame);

	// 현재 회전한 텍스처 이미지의 (0.5, 0.5) 좌표를 현재 텍스처 이미지의 (0, 0) 위치로 가져온다.
	// 현재 텍스처 이미지가 회전되어 있다고 해도, 현재 텍스처 이미지의 (0, 0) 위치에서 해당 이미지를 따라
	// (0.5, 0.5) 위치에 있는 점을 잡고 이미지를 가져오게 되며, 현재 텍스처 이미지의 (0, 0) 위치는 
	// 도형 면의 우하단 1/4 지역에 남은 이미지의 좌상단 점이다. 즉, 도형 면의 (0.5, 0.5) 위치이다.
	// 텍스처 이미지의 정중앙인 (0.5, 0.5) 좌표가 도형 면의 (0.5, 0.5) 위치로 이동하게 되었으므로
	// 텍스처 이미지는 처음 위치 그대로 배치되었다.
	// 현재 텍스처 이미지의 중심 좌표는 도형 면의 좌상단 모서리와 일치한다.
	//XMMATRIX fireTranslateToPrevPos = XMMatrixTranslation(0.5f, 0.5f, 0.0f);

	// 위의 3개의 변환행렬을 결합하여 mTexTransform 변수에 넣어준다.
	//XMStoreFloat4x4(&mTexTransform, fireTranslateToCenter * fireRotOffset *fireTranslateToPrevPos);
}

void SolutionApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);

	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetFireMap(mFireMapSRV);
		Effects::BasicFX->SetFireAlphaMap(mFireAlphaMapSRV);
		Effects::BasicFX->SetDegree(mFrame);

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);
	}

	HR(mSwapChain->Present(0, 0));
}

void SolutionApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void SolutionApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void SolutionApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mTheta += dx;
		mPhi += dy;

		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = 0.01f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f * static_cast<float>(y - mLastMousePos.y);

		mRadius += dx - dy;

		mRadius = MathHelper::Clamp(mRadius, 1.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SolutionApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	mBoxVertexOffset = 0;

	mBoxIndexCount = box.Indices.size();

	mBoxIndexOffset = 0;

	UINT totalVertexCount = box.Vertices.size();

	UINT totalIndexCount = mBoxIndexCount;

	std::vector<Vertex::Basic32> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}

