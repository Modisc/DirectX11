//***************************************************************************************
//
// 
// �������� 8�� 4��
// �������� 3�� �̾, �ҵ��� �ؽ�ó�� �ð��� �Լ��μ� ȸ���ؼ� �� �鿡 ��������
// ������ �����϶�
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

	// �Ʒ��� ������ ���⼭ �����Ű�� basic.fx���� �������̴����� �ؽ�ó ��ǥ ��ü�� ������������
	// ���ĳ��� �̹��� 2�� ��ΰ� ȸ���ϰ� �ȴ�. �̹��� 2�� �� flare �̹����� ȸ���ϴ� ���� ���ϹǷ�
	// basic.fx ���� �ȿ��� �Լ��� �ϳ� ¥��, ���� �������� �ϵ�, ȸ����� ���� ���� �����Ѵ�.
	// => basic.fx ���Ͽ����� XMMatrixRotationZ �� �����Ǵ� �Լ��� ���� �����̴�. 
	// (���°��� ��ã�°��� �𸣰���. ����� ����� �ƴϴ� �����ϰ� ���� �����ϴ� �ɷ� ��)

	// ���� ���� �»���� (0, 0) ���ϴ��� (1, 1)�� �����Ѵ�.
	// �ؽ�ó �̹��� �ٱ����� (-0.5, -0.5) ��ǥ�� ���� �ؽ�ó �̹����� (0, 0) ��ġ�� �����´�.
	// �׷��� ���� �ؽ�ó �̹����� �»�� 1/4 ������ŭ�� ���� ���� ���� ���ϴ� 1/4 ������ ���Եǰ�
	// ������ �̹����� ������ �ʴ´�. ���� �� �������� ����ڰ� ������ �ؽ�ó ��ǥ ���� ��忡 ����
	// �ٸ��� ���δ�.
	//XMMATRIX fireTranslateToCenter = XMMatrixTranslation(-0.5f, -0.5f, 0.0f);
	
	// �ؽ�ó �̹����� �߽� ��ǥ (0, 0)�� ���� ���� ���ϴ� 1/4 ������ ���� �̹����� �»�� ���̴�. ��, ���� ���� (0.5, 0.5) ��ġ�̴�.
	// �ش� ���� �߽����� �ؽ�ó�� �ݽð�������� ȸ����Ų��.
	//XMMATRIX fireRotOffset = XMMatrixRotationZ(mFrame);

	// ���� ȸ���� �ؽ�ó �̹����� (0.5, 0.5) ��ǥ�� ���� �ؽ�ó �̹����� (0, 0) ��ġ�� �����´�.
	// ���� �ؽ�ó �̹����� ȸ���Ǿ� �ִٰ� �ص�, ���� �ؽ�ó �̹����� (0, 0) ��ġ���� �ش� �̹����� ����
	// (0.5, 0.5) ��ġ�� �ִ� ���� ��� �̹����� �������� �Ǹ�, ���� �ؽ�ó �̹����� (0, 0) ��ġ�� 
	// ���� ���� ���ϴ� 1/4 ������ ���� �̹����� �»�� ���̴�. ��, ���� ���� (0.5, 0.5) ��ġ�̴�.
	// �ؽ�ó �̹����� ���߾��� (0.5, 0.5) ��ǥ�� ���� ���� (0.5, 0.5) ��ġ�� �̵��ϰ� �Ǿ����Ƿ�
	// �ؽ�ó �̹����� ó�� ��ġ �״�� ��ġ�Ǿ���.
	// ���� �ؽ�ó �̹����� �߽� ��ǥ�� ���� ���� �»�� �𼭸��� ��ġ�Ѵ�.
	//XMMATRIX fireTranslateToPrevPos = XMMatrixTranslation(0.5f, 0.5f, 0.0f);

	// ���� 3���� ��ȯ����� �����Ͽ� mTexTransform ������ �־��ش�.
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

