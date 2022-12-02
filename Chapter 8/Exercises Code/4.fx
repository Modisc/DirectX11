#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;

	float gTimeDeltaAcc;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
}; 

//Texture2D gDiffuseMap;
Texture2D gFireMap;
Texture2D gFireAlphaMap;
Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = CLAMP;
	AddressV = CLAMP;
	
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

float2 rotation_texture(float2 uv, float degree) {
	float s = sin(degree);
	float c = cos(degree);

	float2x2 rotationMatrix = float2x2(c, -s, s, c);
	uv -= 0.5f;
	uv = mul(rotationMatrix, uv);
	uv += 0.5f;

	return uv;
}

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
		
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}
 
float4 PS(VertexOut pin, uniform int gLightCount, uniform bool gUseTexure) : SV_Target
{
    pin.NormalW = normalize(pin.NormalW);

	float3 toEye = gEyePosW - pin.PosW;

	float distToEye = length(toEye); 

	toEye /= distToEye;
	
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure)
	{
		// 특정 텍스처만 회전하도록 하기 위해, 텍스처 좌표에
		// 회전 변환 행렬을 적용한 좌표를 구한다.
		float2 uv_rot = rotation_texture(pin.Tex, gTimeDeltaAcc);

		// 입력으로 들어온 회전 변환을 하지 않은 텍스처 좌표를 이용해
		// gFireAlphaMap에 저장된 텍스처에서의 표본을 추출하고,
		// 회전 변환 행렬을 적용한 텍스처 좌표를 이용해
		// gFireMap에 저장된 텍스처에서의 표본을 추출한다.
		// 이러면 이미지 하나만 회전하고 남은 하나는 그대로 유지된다.
		float4 fireTex = gFireMap.Sample(samAnisotropic, uv_rot);
		float4 fireAlphaTex = gFireAlphaMap.Sample(samAnisotropic, pin.Tex);
		//texColor = gDiffuseMap.Sample(samAnisotropic, pin.Tex);
		texColor = fireTex * fireAlphaTex;
	}

	float4 litColor = texColor;
	if( gLightCount > 0  )
	{  
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

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

		litColor = texColor*(ambient + diffuse) + spec;
	}

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
