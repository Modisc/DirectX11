//***************************************************************************************
// LightHelper.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Structures and functions for lighting calculations.
//***************************************************************************************

struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight
{ 
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Att;
	float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float Spot;

	float3 Att;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular; // w = SpecPower
	float4 Reflect;
};

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------

// 교재 식 7.3을 구현한다. (평행광(지향광)구현)
void ComputeDirectionalLight(Material mat, DirectionalLight L, 
                             float3 normal, float3 toEye,
					         out float4 ambient,
						     out float4 diffuse,
						     out float4 spec)
{
	// Initialize outputs.
	// 출력 성분들을 초기화
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);	// 주변광 0
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);	// 분산광 0
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);	// 반영광 0

	// The light vector aims opposite the direction the light rays travel.
	// 빛 벡터는 광선이 나아가는 방향의 반대 방향이다.
	float3 lightVec = -L.Direction;

	// Add ambient term.
	// 주변광 항을 더한다.
	// L.Ambient : 광원에서 간접적으로 표면에 도달한 빛의 양
	// mat.Ambient : 주변광 재질 색상(표면이 주변광을 얼마나 흡수하고 반사하는지 결정)
	ambient = mat.Ambient * L.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// 빛이 막히지 않고 표면에 도달한다는 가정 하에
	// 분산광 항과 반영광 항을 더한다.
	// dot(x, y) : 두 벡터의 내적을 계산한다.
	// 빛 벡터와 법선 벡터의 내적을 구한다. (여기선 둘 다 단위벡터 인 것 같음)
	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	// 동적 분기를 피하기 위해 조건문을 펼친다.
	// 빛 벡터와 법선 벡터의 내적 값은 cos 세타
	// 세타 >= pi/2 라면 빛이 물체의 뒤쪽에서 들어온다는 의미이므로
	// 빛이 반사되어선 안된다.(318p)
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		// reflect(i, n) : 정반사광의 방향 벡터를 구하는 벡터반사 함수
		// 첫 번째 인자로 입사광의 방향벡터를 두 번째 인자로 반사면의 법선을 받는다.
		// lightVec : 광선이 나아가는 방향의 반대방향. 여기선 입사광의 방향벡터를 얻어야 하므로 -를 붙여주었다.
		// normal : 법선벡터
		// reflect 함수로 입사광의 반사 벡터를 구해서 v에 저장함.
		float3 v         = reflect(-lightVec, normal);
		// 반영광 공식
		// 반사 벡터와 시점 벡터의 내적 값에 mat.Specular.w 횟수만큼 제곱한다.
		// 입사 벡터와 법선 벡터가 이루는 각이 90도보다 클 경우 specFactor 값은 무조건 0이 되어야 하지만
		// 여기선 위의 조건문에서 해당 가능성을 제외했으므로 고려할 필요가 없다.(318p)
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
		
		// 분산광 구하기
		// diffuseFactor : 빛 벡터와 표면 법선의 각도에 따라 반사광의 세기를 조율하기 위한 값
		// mat.Diffuse : 분산광 재질 색상(표면이 입사 분산광을 얼마나 반사하고 흡수하는지)
		// L.Diffuse : 분산광의 색
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;

		// 반영광 구하기
		// specFactor : 반영광 세기를 반사 벡터와 시점 벡터 사이의 각도에 따라 조율하기 위한 값
		// mat.Specular : 반영광 재질 색상
		// L.Specular : 반영광이 방출하는 반영광의 양
		spec = specFactor * mat.Specular * L.Specular;
	}
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------

// 교재 식 7.4를 구현한다. (점광 구현)
void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
				   out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
	// 출력 성분 초기화
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	// 빛 벡터(표면 점에서 광원으로의 벡터)
	float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
	// 표면 점과 광원 사이의 거리
	float d = length(lightVec);
	
	// Range test.
	// 범위 판정
	// 일정 범위 밖에 있는 점은 광원으로부터 빛을 전혀 받지 못한다.
	if( d > L.Range )
		return;
		
	// Normalize the light vector.
	// 빛 벡터를 정규화한다.
	lightVec /= d; 
	
	// Ambient term.
	// 주변광 항
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// 빛이 막히지 않고 표면에 도달한다는 가정 하에서
	// 분산광 항과 반영광 항을 더한다.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	// 동적 분기를 피하기 위해 조건문을 펼친다.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}

	// Attenuate
	// 감쇠
	// 거리가 멀어질수록 빛의 세기가 약해져야 한다.
	// 
	//				1
	// ----------------------------
	// a0 * 1 + a1 * d + a2 * d * d
	// 
	// 위 공식의 분모를 만들기 위해 dot(L.Att, float3(1.0, d, d*d)); 를 한다.
	// dot은 위에서도 말했듯 벡터의 내적을 수행한다.
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d*d));

	// 감쇠가 주변광 항에는 영향을 미치지 않는다.
	// 주변광 항은 특정 광원의 빛이 아니라 주변에서 수없이 반사된 간접광을 본뜬 것이기 때문이다.
	diffuse *= att;
	spec    *= att;
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------

// 교재 식 7.5를 구현한다. (점적광 구현)
void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
				  out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
	// 출력 성분 초기화
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	// 빛 벡터(표면 점에서 광원으로의 벡터)
	float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
	// 표면 점과 광원 사이의 거리
	float d = length(lightVec);
	
	// Range test.
	// 범위 판정
	if( d > L.Range )
		return;
		
	// Normalize the light vector.
	// 빛 벡터 정규화
	lightVec /= d; 
	
	// Ambient term.
	// 주변광 항
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// 빛이 막히지 않고 표면에 도달한다는 가정 하에서
	// 분산광 항과 반영광 항을 더한다.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	// 동적 분기를 피하기 위해 조건문을 펼친다.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}
	
	// Scale by spotlight factor and attenuate.
	// 점적광 계수를 계산한다.
	// 광원에서 표면으로 향하는 벡터 -lightVec 와, 점적광의 방향 L.Direction 사이의 내적을 통해
	// 둘 사이의 cos 세타 값을 구하고 L.Spot 횟수만큼 제곱한다.
	// 나온 결과값을 spot 변수에 저장한 후, 점광에서 구했던 값에 곱해줄 것이다.
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	// Scale by spotlight factor and attenuate.
	// 빛의 세기를 점적광 계수로 비례하고 거리에 따라 감쇠시킨다.
	// 점광에선 분자가 1이었지만, 점적광에선 점적광 계수를 곱해줘야 하므로 분자가 spot이다.
	float att = spot / dot(L.Att, float3(1.0f, d, d*d));

	// 점광에선 주변광 항은 감쇠의 영향을 받지 않아 아무런 제어도 하지 않았지만,
	// 점적광에선 빛의 세기에 따라 주변광도 약해질 수 있으므로 주변광에도 점적광 계수를 곱해준다.
	ambient *= spot;
	diffuse *= att;
	spec    *= att;

	// 결론적으로, 점적광 방정식은 점광 방정식에 점적광 원뿔을 기준으로 한 점의 위치에 따른
	// 빛의 감쇠를 흉내 내는 점적광 계수를 곱한 것이다.
}

 
 