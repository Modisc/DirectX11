//***************************************************************************************
// LightHelper.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper classes for lighting.
//***************************************************************************************

#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

#include <Windows.h>
#include <DirectXmath.h>
#include <DirectXPackedVector.h>
using namespace DirectX;
using namespace DirectX::PackedVector;

// Note: Make sure structure alignment agrees with HLSL structure padding rules. 
//   Elements are packed into 4D vectors with the restriction that an element
//   cannot straddle a 4D vector boundary.

// ����: ����ü ������ HLSL ����ü �е� ��Ģ�� ��ġ�ϴ��� Ȯ��.
// ��Ҵ� 4���� ���� ��踦 ���� �� ���ٴ� ���Ѱ� �Բ� 4���� ���ͷ� ä�����ϴ�.

struct DirectionalLight		// ���౤
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;					// ���౤�� �����ϴ� �ֺ����� ��
	XMFLOAT4 Diffuse;					// ���౤�� �����ϴ� �л걤�� ��
	XMFLOAT4 Specular;					// ���౤�� �����ϴ� �ݿ����� ��
	XMFLOAT3 Direction;					// ���౤�� ����

	// Pad the last float so we can set an array of lights if we wanted.
	// �ʿ��ϴٸ� ������ �迭�� ������ �� �ֵ���, �������� float �ϳ��� �ڸ��� �ϳ� �����صд�.
	float Pad; 
};

struct PointLight			// ����
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;					// ������ �����ϴ� �ֺ����� ��
	XMFLOAT4 Diffuse;					// ������ �����ϴ� �л걤�� ��
	XMFLOAT4 Specular;					// ������ �����ϴ� �ݿ����� ��;

	// Packed into 4D vector: (Position, Range)
	// (Position, Range) ���·� �ϳ��� 4���� ���Ϳ� ä�� �ִ´�.
	XMFLOAT3 Position;					// ������ ��ġ
	float Range;						// ������ ����

	// Packed into 4D vector: (A0, A1, A2, Pad)
	// (A0, A1, A2, Pad) ���·� �ϳ��� 4���� ���Ϳ� ä�� �ִ´�.
	XMFLOAT3 Att;						// ������ ���� ���

	float Pad; // Pad the last float so we can set an array of lights if we wanted.
		// �ʿ��ϴٸ� ������ �迭�� ������ �� �ֵ���, �������� float �ϳ��� �ڸ��� �ϳ� �����صд�.
};

struct SpotLight			// ������
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;					// �������� �����ϴ� �ֺ����� ��
	XMFLOAT4 Diffuse;					// �������� �����ϴ� �л걤�� ��
	XMFLOAT4 Specular;					// �������� �����ϴ� �ݿ����� ��;

	// Packed into 4D vector: (Position, Range)
	// (Position, Range) ���·� �ϳ��� 4���� ���Ϳ� ä�� �ִ´�.
	XMFLOAT3 Position;					// �������� ��ġ
	float Range;						// �������� ����

	// Packed into 4D vector: (Direction, Spot)
	// (Direction, Spot) ���·� �ϳ��� 4���� ���Ϳ� ä�� �ִ´�.
	XMFLOAT3 Direction;					// �������� ����
	float Spot;							// ������ ���� ��� ���̴� ����

	// Packed into 4D vector: (Att, Pad)
	// (Att, Pad) ���·� �ϳ��� 4���� ���Ϳ� ä�� �ִ´�.
	XMFLOAT3 Att;						// �������� ���� ���

	float Pad; // Pad the last float so we can set an array of lights if we wanted.
		// �ʿ��ϴٸ� ������ �迭�� ������ �� �ֵ���, �������� float �ϳ��� �ڸ��� �ϳ� �����صд�.
};

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular; // w = SpecPower
	XMFLOAT4 Reflect;
};

#endif // LIGHTHELPER_H

// Ambient : ������ �����ϴ� �ֺ����� ��
// Diffuse : ������ �����ϴ� �л걤�� ��
// Specular : ������ �����ϴ� �ݿ����� ��
// Direction : ���� ����
// Position : ������ ��ġ
// Range : ���� ����. �������� �Ÿ��� �� �������� ū ���� ���� ���� �ʴ´�.
// Attenuation : ���� ���Ⱑ �Ÿ��� ���� �����ϴ� ����� �����ϴ� �� ���� ���� ������� (a0, a1, a2)�� ���·� �����Ѵ�.
// Spot : ������ ��꿡�� ������ ������ �����ϴµ� ���̴� ����