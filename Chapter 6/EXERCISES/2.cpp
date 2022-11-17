//***************************************************************************************
//
// 
//	�������� 6�� 2��
//	���� ������ ���� ���� �� ���� ����ϵ��� �����϶�.
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

	ID3DX11Effect* mFX;									// �ϳ� �̻��� ������ �н����� ĸ��ȭ�ϸ�, �� �н����� ���� ���̴��� �����ȴ�.
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

	BuildGeometryBuffers();		// ���� ����, ���� ���� ����
	BuildFX();					// ȿ�� ���� ����(���̴� �ڵ� ������)
	BuildVertexLayout();		// �Է� ��ġ ����

	return true;
}

// ���� ��� �ٽ� ���
void BoxApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	// â�� ũ�Ⱑ �������Ƿ� ��Ⱦ�� �����ϰ� ���� ����� �ٽ� ����Ѵ�.
	// XMMatrixPerspectiveFovLH : �������� ����� �������ִ� �Լ�
	// ù��° �Ű����� : ���� �þ߰�(���� ����)
	// �ι�° �Ű����� : ��Ⱦ�� = �ʺ� / ����
	// ����° �Ű����� : ����� �������� �Ÿ�
	// �׹�° �Ű����� :  �� �������� �Ÿ�
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

// ī�޶� �ٶ󺸴� ���� ����(�þ� ��� ����)
void BoxApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	// ���� ��ǥ�� ��ī��Ʈ ��ǥ�� ��ȯ�Ѵ�.
	// ���� ��ǥ�� : 3���� ���� ���� ������ ��Ÿ���� ��ǥ��. ���� (r, Theta, Phi)�� ��Ÿ����.
	// ��ī��Ʈ ��ǥ�� : ������ ������ ��Ŭ���� ������ ��Ÿ���� ��ǥ��. ���� x, y, z�� ��Ÿ����. 
	// ���� : https://iskim3068.tistory.com/23
	// x, y, z���� �����ϸ� ī�޶� ��ġ�� �ٲ�(mPhi, mTheta ���� ���� �����غ��鼭 Ȯ����)
	// ���⼭ mPhi�� z�� ���� ����� ���� OP�� �̷�� ��, mTheta�� x�� ���� ����� ���� OP�� �̷�� ��
	// mTheta ���� �۾������� ī�޶� �������� �̵�, Ŀ������ ���������� �̵�
	// mPhi ���� �۾������� ī�޶� ���� �̵�, Ŀ������ ī�޶� �Ʒ��� �̵�
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	//std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;

	// Build the view matrix.
	// �þ� ����� �����Ѵ�.
	// ī�޶� ��ġ, �ٶ� ��� ��ġ, ���� ���͸� �˸� �þ� ����� ������ �� �ִ�.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();

	// mPhi�� ���� ������ �������� ��� ī�޶� ���� �̵� �߿� Ƣ�� ������ ���� ������ ã�� ���� �׽�Ʈ �� �ڵ�
	// �ش� �ּ� �ڵ带 �����ϸ� Ƣ�� ������ �ذ��� �� �ִ�.(�Ϻ����� ����)
	/*XMVECTOR up;
	if(mPhi >= MathHelper::Pi || mPhi <= 0)
		up = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
	else
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);*/

	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

// ȭ�鿡 �׸� �׸���
void BoxApp::DrawScene()
{
	// ���� ���(ȭ��)�� ������ ������ ����Ѵ�.(����ٴ� �ǹ̴� ����� ��� ���Ҹ� Ư���� �ϳ��� ������ �����ϴ� ���� ���Ѵ�) (4.4.7�� 151p)
	// => �ĸ� ���۸� LightSteelBlue ������ �����.(Colors::LightSteelBlue�� d3dUtil.h�� ���ǵǾ� �ִ�.)
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	// ���� ���۸� 1.0f, ���ٽ� ���۸� 0���� �����.
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// BuildVertexLayout �Լ����� ������ �Է� ��ġ�� ���� ��ġ�� ������ ���� �����̴�.
	// ���� �ش� �Է� ��ġ�� ����ϰ��� �ϴ� ��ġ�� ����� �Ѵ�.
	md3dImmediateContext->IASetInputLayout(mInputLayout);

	// �⺻���� ������(5.5.2�� 172p)
	// �⺻������ �����ϴ� ����� Direct3D���� �˷� �ִ� �ڵ�
	// �ﰢ�� ����� �����ؼ� ��ü���� �׸���� �����.
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// BuildGeometryBuffers �Լ����� ������ ���� ���ۿ� ���� ���۸� ���� ���������ο� �����Ϸ���
	// ���۸� ��ġ�� �� �Է� ���Կ� �ٿ���(�����)�Ѵ�.
	// �Ʒ��� �̸� ���� ����̴�.(�������� 6.2�� 221p, ���ι��� 6.3�� 223p)
	UINT posStride = sizeof(PositionVertex);
	UINT posOffset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxPosVB, &posStride, &posOffset);

	UINT colStride = sizeof(ColorVertex);
	UINT colOffset = 0;
	md3dImmediateContext->IASetVertexBuffers(1, 1, &mBoxColVB, &colStride, &colOffset);

	md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	// ��� ����(6.8.4�� 245p)
	// ������������ ���� ����� �����Ѵ�.
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Apply�� ù��° �Ű����� : ���� �뵵�� ���� ����� ������, ����μ��� �׻� 0�� ����
		// Apply�� �ι�° �Ű����� : �н��� ������ ��ġ ����
		// ȿ�� ����� �� �н��� �����鼭 �� �н��� ��ġ���ƿ� ������
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		// 36 indices for the box.
		// ���� 36���� ���ڸ� �׸���.(6.3�� 224p)
		// ù��° �Ű����� : ���� �׸��� ȣ�⿡�� ����� ���ε��� ����
		// �ι�° �Ű����� : ����� ù ������ ��ġ(���� �ȿ����� ����)
		// ����° �Ű����� : �������� �������� ����, �� �׸��� ȣ�⿡�� ����� ���ε��� �������� ��
		md3dImmediateContext->DrawIndexed(36, 0, 0);
	}

	// ������ ó���� ����ȭ �ϱ� ���� ��Ʈ ����
	// ����ü�ο��� ������ �ϰ��ִ� �����찡 �ּ�ȭ �Ǵ� ��
	// ǥ�õǴ� �κ��� �������� �ʴ� ���°� �Ǹ� DXGI_STATUS_OCCLUDED�� ��ȯ�Ѵ�
	//--------------------------------------------------------------------------------- 
	// ���� Present�� ����ȭ �ϴ� ����� ���� ������ �� ����(https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=sorkelf&logNo=40161605210)
	// ���⼭ Present�� �ĸ� ���۸� ȭ�鿡 �����ϴ� ����̴�.
	HR(mSwapChain->Present(0, 0));
}

// ���콺 Ŭ�� ��ǥ ���� �� SetCapture ȣ��
void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
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
void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	// ReleaseCapture�� �ϸ� ���� �ۿ��� ���̻� WM_MOUSEMOVE�� �޾����� �ʴ´�.
	// �Լ��� �����ϸ� return 0
	// ������ ��� GetLastError�� ȣ���Ͽ� ���� ������ Ȯ���� �� �ִ�.
	ReleaseCapture();
}

// ���콺 ��ư Ŭ�� ���¿� �̵��� ���� ī�޶� �̵�
void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
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


	// ���� ���۸� �����ϴ� ����ü�� ä���.
	D3D11_BUFFER_DESC vbd0;
	vbd0.Usage = D3D11_USAGE_IMMUTABLE;						// ���۰� ���̴� ���(���⼭�� ���� ���� �� ���� ���ٴ� �ǹ�)
	vbd0.ByteWidth = sizeof(PositionVertex) * 8;						// ������ ���� ������ ũ��(����Ʈ ����)
	vbd0.BindFlags = D3D11_BIND_VERTEX_BUFFER;				// ���� ������ ��� �ش� �÷��� ����ϸ� ��.
	vbd0.CPUAccessFlags = 0;									// CPU�� ���ۿ� �����ϴ� ����� �����ϴ� �÷��׵� ����(���� ���� ���� CPU�� ���� �б�, ���⸦ ���� �ʴ´ٸ� 0�� ����)
	vbd0.MiscFlags = 0;										// ���� ���ۿ� ���ؼ��� �׳� 0 ����
	vbd0.StructureByteStride = 0;							// ������ ���ۿ� ����� ���� �ϳ��� ũ��(����Ʈ ����) (������ ���۸� ���ؼ��� �ʿ���. �� �� ��� ���ۿ� ���ؼ��� 0 ����)

	// ���� ���۸� �ʱ�ȭ�� �ڷḦ �����Ѵ�.
	D3D11_SUBRESOURCE_DATA vinitData0;
	vinitData0.pSysMem = pos;							// ���� ���۸� �ʱ�ȭ�� �ڷḦ ���� �ý��� �޸� �迭�� ����Ű�� ������

	// vbd : ������ ���۸� �����ϴ� ����ü
	// vinitData : ���۸� �ʱ�ȭ�ϴ� �� ����� �ڷ�
	// mBoxVB : ������ ���۰� ���⿡ �����ȴ�.
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

	// ���� ���۸� �����ϴ� ����ü�� ä���.
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	// ���� ���۸� �ʱ�ȭ�� �ڷḦ �����Ѵ�.
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	// ���� ���۸� �����Ѵ�.
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}

// ȿ�� ���� ����(���̴� �ڵ� ������)
void BoxApp::BuildFX()
{
	std::ifstream fin("fx/color.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	// compiledShader : �����ϵ� ȿ�� �ڷḦ ����Ű�� ������
	// size : �����ϵ� ȿ�� �ڷ��� ����Ʈ ���� ũ��
	// 0 : ȿ�� �÷���(D3DX11CompileFromFile �Լ��� Flags2�� ������ �Ͱ� ��ġ�ؾ� ��)
	// md3dDevice : Direct3D 11 ��ġ�� ����Ű�� ������
	// mFX : ������ ȿ�� ������ ����Ű�� ������
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size,
		0, md3dDevice, &mFX));

	// ȿ�� ��ü�� �ִ� ��� ��ü�� ����Ű�� ������ ������
	mTech = mFX->GetTechniqueByName("ColorTech");

	// ȿ�� ��ü�� ���ؼ� ��� ���� ������ ���� ������ ���
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

// �Է� ��ġ ����
void BoxApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	// �Է� ��ġ ���� �迭 ����
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// ���������� ���¸� �����ϴ� ȿ�� ���޿� ���� �����ϴ� ����ü
	D3DX11_PASS_DESC passDesc;

	// ID3DX11EffectTechnique::GetPassByIndex
	// =>  �־��� ���ο� �ش��ϴ� �н��� ��Ÿ���� ID3DX11EffectPass �������̽��� ����Ű�� �����͸� �����ش�.
	// ID3DX11EffectPass::GetDesc
	// =>  �������� HRESULT
	// =>  �Ű����� passDesc : ���� ���� ���� ������
	// =>  Ư�� ���ۿ� ���� ������ ���´�(?)
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);

	// �Է� ��ġ ����
	// vertexDesc : ���� ����ü�� �����ϴ� �Է� ��ġ ���� �迭
	// 2 : �Է� ��ġ ���� �迭 ������ ����
	// passDesc.pIAInputSignature : ���� ���̴�(�Է� ������ ������)�� �������ؼ� ���� ����Ʈ �ڵ带 ����Ű�� ������
	// passDesc.IAInputSignatureSize : ���� ����Ʈ �ڵ��� ũ��(����Ʈ ����)
	// mInputLayout : ������ �Է� ��ġ�� �� �����͸� ���ؼ� �����ش�.
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}
