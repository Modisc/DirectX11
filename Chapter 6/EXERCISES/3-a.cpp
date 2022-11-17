//***************************************************************************************
//
// 
//	�������� 6�� 3-(a)��
//	�׸� 5.13(a)�� ���� �� ����� �׷���
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
	// ����� ������ ��� ������� �޸� ���� ����� Ȱ��ȭ�Ѵ�.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// �Ʒ� �ڵ���� �ۼ����ָ� ����׿� �ܼ��� ���α׷� ����� �Բ� ������ �ȴ�.
	// ��� ��Ʈ���� stdout(ǥ�����) ��Ʈ������ ���������Ƿ�
	// std::cout ��� ���� ��� �Լ���� log�� ���� ���� �ִ�.
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

	// �⺻���� �������� �� ������� ������.
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
		
		// �׸��⿡ ����� ���ε��� ���� 9��
		md3dImmediateContext->DrawIndexed(9, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

// ���콺 Ŭ�� ��ǥ ���� �� SetCapture ȣ��
void SolutionApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	// ���콺 Ŭ�� �ϴ� ������ Ŭ�� ��ǥ�� �����ص�
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	// ���콺�� ������â �ٱ����� ������ ����, WM_MOUSEMOVE�� �ޱ� ���� �Լ�
	// ������ SetCapture �ߴ� HWND�� �ִٸ� ���� HWND�� ��ȯ�Ѵ�. ���ٸ� NULL ��ȯ
	// SetCapture�� �ߴٸ� �Ŀ� ReleaseCapture�� ȣ���ؾ� �Ѵ�.
	SetCapture(mhMainWnd);
}

// ReleaseCapture ȣ��
void SolutionApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	// ReleaseCapture�� �ϸ� ���� �ۿ��� ���̻� WM_MOUSEMOVE�� �޾����� �ʴ´�.
	// �Լ��� �����ϸ� return 0
	// ������ ��� GetLastError�� ȣ���Ͽ� ���� ������ Ȯ���� �� �ִ�.
	ReleaseCapture();
}

// ���콺 ��ư Ŭ�� ���¿� �̵��� ���� ī�޶� �̵�
void SolutionApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	// ���콺 ���� ��ư�� ������ �ִ� ���¶��
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		// 1�ȼ��� ����� 0.005������ �ǰ� �Ѵ�.
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		// ���콺 �Է¿� �����ؼ� �˵� ī�޶��� �������� �����Ѵ�.
		mRadius += dx - dy;

		// Restrict the radius.
		// �������� Ư�� ������ �����Ѵ�.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);

	}

	// ���콺 ������ ��ư�� ������ �ִ� ���¶��
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		// 1�ȼ��� 1/4��(��׸� ����)�� �ǰ� �Ѵ�.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));
		std::cout << "dx : " << dx << ",\t" << "dy : " << dy << std::endl;

		// Update angles based on input to orbit camera around box.
		// ���콺 �Է¿� ������ ������ ���� �ֺ��� �˵� ī�޶� �����Ѵ�.
		mTheta -= dx;
		mPhi -= dy;

		std::cout << "mTheta : " << mTheta << ",\t" << "mPhi : " << mPhi << std::endl << std::endl;

		// Restrict the angle mPhi.
		// ������ mPhi�� �����Ѵ�.
		// ���� ������ ������ ī�޶� ������ ���� ������, ī�޶� �������� �Ǹ�
		// ���� ������ ����(��)�� UpdateScene���� ������ ���� �޶����� �ǹǷ� ������ �������� ���� ������ �߻��� �� �ִ�.
		// ex: Pi - 0.1f���� - 0.1f�� ����� ������ ������ ������ �߻��Ѵ�.
		// - 0.1f�� ����Ŷ�� UpdateScene�� ���⺤�� up�� ���ǿ� ���� �ٸ��� �������ִ� ���ǹ��� �ּ��� �ۼ��� �ξ�����
		// �ڵ带 �����ؾ� �Ѵ�.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

// ��������, ���ι��� ����
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
	vbd.ByteWidth = sizeof(Vertex) * 9;				// ������ ���� ������ ũ��
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	
	vbd.CPUAccessFlags = 0;						
	vbd.MiscFlags = 0;							
	vbd.StructureByteStride = 0;				
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;							
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	// �� ����� �׸� �� ������ ������ �߿��Ѱ�?
	// => ������ �ƹ����Գ� �ʱ�ȭ �ص� �������� ���� ��� ��.
	// => ������ ������� �� ����.
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
	ibd.ByteWidth = sizeof(UINT) * 9;				// ������ ���� ������ ũ��
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

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
