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

// ���� �� 7.3�� �����Ѵ�. (���౤(���Ɽ)����)
void ComputeDirectionalLight(Material mat, DirectionalLight L, 
                             float3 normal, float3 toEye,
					         out float4 ambient,
						     out float4 diffuse,
						     out float4 spec)
{
	// Initialize outputs.
	// ��� ���е��� �ʱ�ȭ
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);	// �ֺ��� 0
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);	// �л걤 0
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);	// �ݿ��� 0

	// The light vector aims opposite the direction the light rays travel.
	// �� ���ʹ� ������ ���ư��� ������ �ݴ� �����̴�.
	float3 lightVec = -L.Direction;

	// Add ambient term.
	// �ֺ��� ���� ���Ѵ�.
	// L.Ambient : �������� ���������� ǥ�鿡 ������ ���� ��
	// mat.Ambient : �ֺ��� ���� ����(ǥ���� �ֺ����� �󸶳� ����ϰ� �ݻ��ϴ��� ����)
	ambient = mat.Ambient * L.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// ���� ������ �ʰ� ǥ�鿡 �����Ѵٴ� ���� �Ͽ�
	// �л걤 �װ� �ݿ��� ���� ���Ѵ�.
	// dot(x, y) : �� ������ ������ ����Ѵ�.
	// �� ���Ϳ� ���� ������ ������ ���Ѵ�. (���⼱ �� �� �������� �� �� ����)
	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	// ���� �б⸦ ���ϱ� ���� ���ǹ��� ��ģ��.
	// �� ���Ϳ� ���� ������ ���� ���� cos ��Ÿ
	// ��Ÿ >= pi/2 ��� ���� ��ü�� ���ʿ��� ���´ٴ� �ǹ��̹Ƿ�
	// ���� �ݻ�Ǿ �ȵȴ�.(318p)
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		// reflect(i, n) : ���ݻ籤�� ���� ���͸� ���ϴ� ���͹ݻ� �Լ�
		// ù ��° ���ڷ� �Ի籤�� ���⺤�͸� �� ��° ���ڷ� �ݻ���� ������ �޴´�.
		// lightVec : ������ ���ư��� ������ �ݴ����. ���⼱ �Ի籤�� ���⺤�͸� ���� �ϹǷ� -�� �ٿ��־���.
		// normal : ��������
		// reflect �Լ��� �Ի籤�� �ݻ� ���͸� ���ؼ� v�� ������.
		float3 v         = reflect(-lightVec, normal);
		// �ݿ��� ����
		// �ݻ� ���Ϳ� ���� ������ ���� ���� mat.Specular.w Ƚ����ŭ �����Ѵ�.
		// �Ի� ���Ϳ� ���� ���Ͱ� �̷�� ���� 90������ Ŭ ��� specFactor ���� ������ 0�� �Ǿ�� ������
		// ���⼱ ���� ���ǹ����� �ش� ���ɼ��� ���������Ƿ� ����� �ʿ䰡 ����.(318p)
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
		
		// �л걤 ���ϱ�
		// diffuseFactor : �� ���Ϳ� ǥ�� ������ ������ ���� �ݻ籤�� ���⸦ �����ϱ� ���� ��
		// mat.Diffuse : �л걤 ���� ����(ǥ���� �Ի� �л걤�� �󸶳� �ݻ��ϰ� ����ϴ���)
		// L.Diffuse : �л걤�� ��
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;

		// �ݿ��� ���ϱ�
		// specFactor : �ݿ��� ���⸦ �ݻ� ���Ϳ� ���� ���� ������ ������ ���� �����ϱ� ���� ��
		// mat.Specular : �ݿ��� ���� ����
		// L.Specular : �ݿ����� �����ϴ� �ݿ����� ��
		spec = specFactor * mat.Specular * L.Specular;
	}
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------

// ���� �� 7.4�� �����Ѵ�. (���� ����)
void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
				   out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
	// ��� ���� �ʱ�ȭ
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	// �� ����(ǥ�� ������ ���������� ����)
	float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
	// ǥ�� ���� ���� ������ �Ÿ�
	float d = length(lightVec);
	
	// Range test.
	// ���� ����
	// ���� ���� �ۿ� �ִ� ���� �������κ��� ���� ���� ���� ���Ѵ�.
	if( d > L.Range )
		return;
		
	// Normalize the light vector.
	// �� ���͸� ����ȭ�Ѵ�.
	lightVec /= d; 
	
	// Ambient term.
	// �ֺ��� ��
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// ���� ������ �ʰ� ǥ�鿡 �����Ѵٴ� ���� �Ͽ���
	// �л걤 �װ� �ݿ��� ���� ���Ѵ�.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	// ���� �б⸦ ���ϱ� ���� ���ǹ��� ��ģ��.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}

	// Attenuate
	// ����
	// �Ÿ��� �־������� ���� ���Ⱑ �������� �Ѵ�.
	// 
	//				1
	// ----------------------------
	// a0 * 1 + a1 * d + a2 * d * d
	// 
	// �� ������ �и� ����� ���� dot(L.Att, float3(1.0, d, d*d)); �� �Ѵ�.
	// dot�� �������� ���ߵ� ������ ������ �����Ѵ�.
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d*d));

	// ���谡 �ֺ��� �׿��� ������ ��ġ�� �ʴ´�.
	// �ֺ��� ���� Ư�� ������ ���� �ƴ϶� �ֺ����� ������ �ݻ�� �������� ���� ���̱� �����̴�.
	diffuse *= att;
	spec    *= att;
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------

// ���� �� 7.5�� �����Ѵ�. (������ ����)
void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
				  out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
	// ��� ���� �ʱ�ȭ
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	// �� ����(ǥ�� ������ ���������� ����)
	float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
	// ǥ�� ���� ���� ������ �Ÿ�
	float d = length(lightVec);
	
	// Range test.
	// ���� ����
	if( d > L.Range )
		return;
		
	// Normalize the light vector.
	// �� ���� ����ȭ
	lightVec /= d; 
	
	// Ambient term.
	// �ֺ��� ��
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// ���� ������ �ʰ� ǥ�鿡 �����Ѵٴ� ���� �Ͽ���
	// �л걤 �װ� �ݿ��� ���� ���Ѵ�.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	// ���� �б⸦ ���ϱ� ���� ���ǹ��� ��ģ��.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}
	
	// Scale by spotlight factor and attenuate.
	// ������ ����� ����Ѵ�.
	// �������� ǥ������ ���ϴ� ���� -lightVec ��, �������� ���� L.Direction ������ ������ ����
	// �� ������ cos ��Ÿ ���� ���ϰ� L.Spot Ƚ����ŭ �����Ѵ�.
	// ���� ������� spot ������ ������ ��, �������� ���ߴ� ���� ������ ���̴�.
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	// Scale by spotlight factor and attenuate.
	// ���� ���⸦ ������ ����� ����ϰ� �Ÿ��� ���� �����Ų��.
	// �������� ���ڰ� 1�̾�����, ���������� ������ ����� ������� �ϹǷ� ���ڰ� spot�̴�.
	float att = spot / dot(L.Att, float3(1.0f, d, d*d));

	// �������� �ֺ��� ���� ������ ������ ���� �ʾ� �ƹ��� ��� ���� �ʾ�����,
	// ���������� ���� ���⿡ ���� �ֺ����� ������ �� �����Ƿ� �ֺ������� ������ ����� �����ش�.
	ambient *= spot;
	diffuse *= att;
	spec    *= att;

	// ���������, ������ �������� ���� �����Ŀ� ������ ������ �������� �� ���� ��ġ�� ����
	// ���� ���踦 �䳻 ���� ������ ����� ���� ���̴�.
}

 
 