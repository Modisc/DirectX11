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

// 참고: 구조체 정렬이 HLSL 구조체 패딩 규칙과 일치하는지 확인.
// 요소는 4차원 벡터 경계를 넘을 수 없다는 제한과 함께 4차원 벡터로 채워집니다.

struct DirectionalLight		// 평행광
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;					// 평행광이 방출하는 주변광의 양
	XMFLOAT4 Diffuse;					// 평행광이 방출하는 분산광의 양
	XMFLOAT4 Specular;					// 평행광이 방출하는 반영광의 양
	XMFLOAT3 Direction;					// 평행광의 방향

	// Pad the last float so we can set an array of lights if we wanted.
	// 필요하다면 빛들의 배열을 설정할 수 있도록, 마지막에 float 하나의 자리를 하나 마련해둔다.
	float Pad; 
};

struct PointLight			// 점광
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;					// 점광이 방출하는 주변광의 양
	XMFLOAT4 Diffuse;					// 점광이 방출하는 분산광의 양
	XMFLOAT4 Specular;					// 점광이 방출하는 반영광의 양;

	// Packed into 4D vector: (Position, Range)
	// (Position, Range) 형태로 하나의 4차원 벡터에 채워 넣는다.
	XMFLOAT3 Position;					// 점광의 위치
	float Range;						// 점광의 범위

	// Packed into 4D vector: (A0, A1, A2, Pad)
	// (A0, A1, A2, Pad) 형태로 하나의 4차원 벡터에 채워 넣는다.
	XMFLOAT3 Att;						// 점광의 감쇠 상수

	float Pad; // Pad the last float so we can set an array of lights if we wanted.
		// 필요하다면 빛들의 배열을 설정할 수 있도록, 마지막에 float 하나의 자리를 하나 마련해둔다.
};

struct SpotLight			// 점적광
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;					// 점적광이 방출하는 주변광의 양
	XMFLOAT4 Diffuse;					// 점적광이 방출하는 분산광의 양
	XMFLOAT4 Specular;					// 점적광이 방출하는 반영광의 양;

	// Packed into 4D vector: (Position, Range)
	// (Position, Range) 형태로 하나의 4차원 벡터에 채워 넣는다.
	XMFLOAT3 Position;					// 점적광의 위치
	float Range;						// 점적광의 범위

	// Packed into 4D vector: (Direction, Spot)
	// (Direction, Spot) 형태로 하나의 4차원 벡터에 채워 넣는다.
	XMFLOAT3 Direction;					// 점적광의 방향
	float Spot;							// 점적광 원뿔 제어에 쓰이는 지수

	// Packed into 4D vector: (Att, Pad)
	// (Att, Pad) 형태로 하나의 4차원 벡터에 채워 넣는다.
	XMFLOAT3 Att;						// 점적광의 감쇠 상수

	float Pad; // Pad the last float so we can set an array of lights if we wanted.
		// 필요하다면 빛들의 배열을 설정할 수 있도록, 마지막에 float 하나의 자리를 하나 마련해둔다.
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

// Ambient : 광원이 방출하는 주변광의 양
// Diffuse : 광원이 방출하는 분산광의 양
// Specular : 광원이 방출하는 반영광의 양
// Direction : 빛의 방향
// Position : 광원의 위치
// Range : 빛의 범위. 광원과의 거리가 이 범위보다 큰 점은 빛을 받지 않는다.
// Attenuation : 빛의 세기가 거리에 따라 감소하는 방식을 제어하는 세 개의 감쇠 상수들을 (a0, a1, a2)의 형태로 저장한다.
// Spot : 점적광 계산에서 점적광 원뿔을 제어하는데 쓰이는 지수