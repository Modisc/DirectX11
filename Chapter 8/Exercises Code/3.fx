//=============================================================================
// Basic.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Basic effect that currently supports transformations, lighting, and texturing.
// 기본 효과. 현재 변환, 조명, 텍스처 매핑을 지원함
//=============================================================================

#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
}; 

// Nonnumeric values cannot be added to a cbuffer.
// 수치가 아닌 객체는 cbuffer에 추가할 수 없다.
Texture2D gFireMap;
Texture2D gFireAlphaMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float2 Tex     : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to world space space.
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
		
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	// 정점 특성들을 출력한다.(이후 삼각형 표면을 따라 보간됨)
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}
 
float4 PS(VertexOut pin, uniform int gLightCount, uniform bool gUseTexure) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
	// 보간 때문에 법선이 더 이상 단위벡터가 아닐 수 있으므로 다시 정규화
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	// toEye 벡터는 조명 계산에 쓰인다.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	// 시점과 이 표면 점 사이의 거리를 보관해둔다.
	float distToEye = length(toEye); 

	// Normalize.
	// 정규화한다.
	toEye /= distToEye;
	
    // Default to multiplicative identity.
	// 기본 텍스처 색상은 곱셈의 항등원.
	// gUseTexure == false 인 경우 아래의 변조 후 가산 연산에서
	// texColor에 영향을 받지 않도록
	// 기본 텍스처 색상은 곱셈의 항등원으로 설정함.
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure)
	{
		// Sample texture.
		// 텍스처에서 표본을 추출한다.
		// 첫번째 매개변수 : SamplerState(8.5절 364p)
		// 두번째 매개변수 : 픽셀의 텍스처 좌표(u,v)
		// 주어진 SamplerState 객체에 지정된 필터링 방법을 이용해서
		// 텍스처 맵의 점 (u,v)에서 추출, 보간한 색상 값을 돌려준다.
		
		float4 fireTex = gFireMap.Sample(samAnisotropic, pin.Tex);
		float4 fireAlphaTex = gFireAlphaMap.Sample(samAnisotropic, pin.Tex);

		texColor = fireTex / fireAlphaTex;
	}
	 
	//
	// Lighting.
	// 조명
	//

	float4 litColor = texColor;
	if( gLightCount > 0  )
	{  
		// Start with a sum of zero. 
		// 합이 0인 재질 속성들로 시작한다.
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Sum the light contribution from each light source.  
		// 각 광원이 기여한 빛을 합한다.
		[unroll]
		for(int i = 0; i < gLightCount; ++i)
		{
			float4 A, D, S;
			ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, 
				A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}

		// Modulate with late add.
		// 변조 후 가산(8.6절 366p)
		litColor = texColor*(ambient + diffuse) + spec;
	}

	// Common to take alpha from diffuse material and texture.
	// 일반적으로 분산광 재질의 알파 성분을 최종 알파 값으로 사용한다.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

technique11 Light1
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, false) ) );
    }
}

technique11 Light2
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, false) ) );
    }
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false) ) );
    }
}

technique11 Light0Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(0, true) ) );
    }
}

technique11 Light1Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, true) ) );
    }
}

technique11 Light2Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, true) ) );
    }
}

technique11 Light3Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true) ) );
    }
}
