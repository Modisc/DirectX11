//***************************************************************************************
//
// 
//	�������� 6�� 7��
//	���� ������ �Ƕ�̵� �ϳ��� �߰��϶�. ��, ������ ������� �Ƕ�̵��� ��������
//  �ϳ��� Ŀ�ٶ� ���ۿ� �־�� �Ѵ�.
//	����, ������ ���ε�� �Ƕ�̵��� ���ε鵵 �ϳ��� Ŀ�ٶ� ���� ���ۿ� �ִ´�.
//	��, ���� ������ ���������� �� ��
//	
//	�׷� ���� DrawIndexed�� �Ű��������� ������ �����ؼ� ���ڿ� �Ƕ�̵带 �ϳ��� �׸���.
//	����������� �Ƕ�̵�� ���ڰ� �������� �ʵ��� ���� ��ȯ ��ĵ� ������ �����ؾ� �Ѵ�.
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

	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mPyramidWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mBoxVertexOffset;				// ����� ���� ���� �ȿ����� ������ ���� ������ (�� ���������� ���� 0�� �� ����)
	int mPyramidVertexOffset;			// ����� ���� ���� �ȿ����� �Ƕ�̵��� ���� ������ (�� ���������� ���� 8�� �� ����)

	UINT mBoxIndexOffset;				// ����� ���� ���� �ȿ����� ������ ���� ���� (�� ���������� ���� 0)
	UINT mPyramidIndexOffset;			// ����� ���� ���� �ȿ����� �Ƕ�̵��� ���� ���� (�� ���������� ���� 36)

	UINT mBoxIndexCount;				// ������ ���� ���� (�� ���������� ���� 36�� �� ����)
	UINT mPyramidIndexCount;			// �Ƕ�̵��� ���� ���� (�� ���������� ���� 18�� �� ����)

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
	mfxWorldViewProj(0), mInputLayout(0),
	mTheta(1.25f * MathHelper::Pi), mPhi(0.35f * MathHelper::Pi), mRadius(7.0f)
{
	mMainWndCaption = L"Box Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX boxOffset = XMMatrixTranslation(-1.5f, 0.0f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, boxOffset);

	XMMATRIX pyramidOffset = XMMatrixTranslation(+1.5f, 0.0f, 0.0f);
	XMStoreFloat4x4(&mPyramidWorld, pyramidOffset);
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

// ���� ��� �ٽ� ���
void SolutionApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

// ī�޶� �ٶ󺸴� ���� ����(�þ� ��� ����)
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

// ȭ�鿡 �׸� �׸���
void SolutionApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// ���� �׸���
		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world * viewProj)));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// �Ƕ�̵� �׸���
		world = XMLoadFloat4x4(&mPyramidWorld);
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world * viewProj)));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mPyramidIndexCount, mPyramidIndexOffset, mPyramidVertexOffset);
	}

	HR(mSwapChain->Present(0, 0));
}

// ���콺 Ŭ�� ��ǥ ���� �� SetCapture ȣ��
void SolutionApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

// ReleaseCapture ȣ��
void SolutionApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

// ���콺 ��ư Ŭ�� ���¿� �̵��� ���� ī�޶� �̵�
void SolutionApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		mRadius += dx - dy;

		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);

	}

	// ���콺 ������ ��ư�� ������ �ִ� ���¶��
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

// ��������, ���ι��� ����
void SolutionApp::BuildGeometryBuffers()
{
	std::vector<Vertex> boxVertices = 
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Green) },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Green) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Green) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Green) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Green) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Green) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Green) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Green) }
	};

	std::vector<Vertex> pyramidVertices = 
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Red) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Red) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Red) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Red) },
		{ XMFLOAT3( 0.0f, +1.0f,  0.0f), XMFLOAT4((const float*)&Colors::Red) }
	};

	std::vector<UINT> boxIndices =
	{
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

	std::vector<UINT> pyramidIndices =
	{
		// �ظ�
		0, 2, 1,
		2, 3, 1,

		// ����
		0, 2, 4,
		0, 4, 1,
		1, 4, 3,
		3, 4, 2
	};

	// ����� ���� ���� �ȿ����� �� ��ü�� ���� ������ ����.
	mBoxVertexOffset = 0;
	mPyramidVertexOffset = boxVertices.size();

	// �� ��ü�� ���� ���� ����
	mBoxIndexCount = boxIndices.size();
	mPyramidIndexCount = pyramidIndices.size();

	// ����� ���� ���� �ȿ����� �� ��ü�� ���� ���� ����.
	mBoxIndexOffset = 0;
	mPyramidIndexOffset = mBoxIndexCount;

	UINT totalVertexCount = boxVertices.size() + pyramidVertices.size();
	UINT totalIndexCount = boxIndices.size() + pyramidIndices.size();

	std::vector<Vertex> totalVertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < boxVertices.size(); ++i, ++k)
	{
		totalVertices[k].Pos = boxVertices[i].Pos;
		totalVertices[k].Color = boxVertices[i].Color;
	}

	for (size_t i = 0; i < pyramidVertices.size(); ++i, ++k)
	{
		totalVertices[k].Pos = pyramidVertices[i].Pos;
		totalVertices[k].Color = pyramidVertices[i].Color;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &totalVertices[0];

	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	// vector�� ������ �� ũ�⸦ �Բ� ��������ٸ� insert Ű���带 �̿��� ������ ���ϴ´�� �������� ����.
	// totalIndexCount ũ�� ��ŭ �迭�� ���������, 0���� �ʱ�ȭ��.
	// ���� insert, end ���� �Լ��� �츮�� ���ϴ� ��ġ�� ����Ű�� �ʰ� �ǹǷ� ���ϴ´�� �������� �ʰ� �Ǵ� ����.
	// insert ������ �̿��Ϸ��� ���� ũ�� �ʱ�ȭ �κ��� �����ϰ� vector�� �����ϸ� ��.
	std::vector<UINT> totalIndices(totalIndexCount);

	k = 0;
	for (size_t i = 0; i < boxIndices.size(); ++i, ++k)
	{
		totalIndices[k] = boxIndices[i];
	}

	for (size_t i = 0; i < pyramidIndices.size(); ++i, ++k)
	{
		totalIndices[k] = pyramidIndices[i];
	}

	/*totalIndices.insert(totalIndices.end(), boxIndices.begin(), boxIndices.end());
	totalIndices.insert(totalIndices.end(), pyramidIndices.begin(), pyramidIndices.end());*/

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &totalIndices[0];

	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}

// ȿ�� ���� ����(���̴� �ڵ� ������)
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

// �Է� ��ġ ����
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