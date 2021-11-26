cbuffer TransforBuffer : register(b0)
{
	matrix mWorld;
	matrix mView;
	matrix mProjection;
}

cbuffer LightBuffer : register(b1)
{
	float3 posLightCS;
	int lightFlag;
}


struct VS_INPUT
{
	float4 Pos : POSITION0;
	float4 Color : COLOR0;
	float3 Normal : NORMAL;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Color: COLOR0;
	float3 PosCS:POSIION0;
	float3 Normal : NORMAL;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS_TEST(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(input.Pos, mWorld);
	output.Pos = mul(output.Pos, mView);
	output.PosCS = output.Pos.xyz / output.Pos.w;

	output.Pos = mul(output.Pos, mProjection);
	output.Color = input.Color;
	output.Normal = normalize(mul(input.Normal,mul(mView,mWorld))); // camera space�� ���� �ʿ�
	// mul(mView,mWorld) -> col major�� ����̹Ƿ� row major�� ����ϱ� ���ؼ��� data, matrix ������ �ۼ�
	return output;
}

float3 	PhongLighting(float3 L, float3 N, float3 R, float3 V, float3 mtcAmbient, float3 mtxDiffuse, float3 mtcSpec, float3 shiness, float3 lightColor)
{
	return float3(0, 1, 1);
}

// specular term �Ⱥ��̰� �ϱ�
float3 	PhongLighting2(float3 L, float3 N, float3 R, float3 V, float3 mtcAmbient, float3 mtxDiffuse, float3 mtcSpec, float3 shiness, float3 lightColor)
{
	return float3(0, 1, 1);
}

// Pos�� viewport�� ���� screen space�� ��ȯ
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	// Light Color
	float3 lightColor = float3(1.f,1.f,1.f);
	// Material Colors : ambient, diffuse, specular, shininess
	float3 mtcAmbient = float3(0.1f,0.1f,0.1f),mtxDiffuse = float3(0.7f,0.7f,0.f),mtcSpec = float3(0.2f,0.f,0.2f);
	float shine = 10.f;
	// Light type (point) ....
	float3 L,N,R,V; // compute to do

	float3 colorOut= PhongLighting(L, N, R, V, mtcAmbient, mtxDiffuse, mtcSpec, shine, lightColor);
	return float4(colorOut, 1);

	//float3 posCS = input.PosCS;
	//float3 unit_nor = normalize(input.Normal);// interpolation �Ǹ� normal�� unit vetor�� ������ ���� �� �ֱ� ������ 
	//return float4((unit_nor+(float3)1.f)/2.f, 1.0f);    // 0~1�� ���� ���� ���� /2.f
	//return lightFlag == 777 ? float4(1, 0, 0, 1) : float4(0, 1, 0, 1);z
	//return float4(input.Color.xyz, 1.0f);
}

