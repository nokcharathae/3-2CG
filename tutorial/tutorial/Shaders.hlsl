cbuffer TransformBuffer : register(b0)
{
	matrix mWorld;
	matrix mView;
	matrix mProjection;
}

cbuffer LightBuffer : register(b0) // buffer 1
{
	float3 posLightCS;
	float dummy;
	float3 lightColor;
	float dummy2;

}

cbuffer MaterialBuffer : register(b1)
{
	float3 mtcAmbient;
	float shine;
	float3 mtxDiffuse;
	float dummy3;
	float3 mtcSpec;
	int light_controler;
}

struct VS_INPUT
{
	float4 Pos : POSITION0;
	float4 Color : COLOR0;
	float3 Nor : NORMAL;
};

// VS�� OUT���� ��� ����
struct PS_INPUT
{
	// HLSL������ ������ semantic�� ������ �� ���� 
	// semantic : ������ �Է°� ����� Ȯ�� �Ǵ� �����ϰų� �������� ��ó�� ���ҿ� �ǹ� �ο��ϱ� ���� 
	// �Լ�, ����, �μ� �ڿ� ���������� �ٿ��� �����ϴ� ��
	float4 Pos : SV_POSITION; // pos ���� screen space ������ ���Ͽ� 
	float4 Color : COLOR0;
	float3 PosCS: POSIION0;
	float3 Nor : NORMAL0;
	// float3 VecL : 
};

// Q. rasterization �� ���� �Ͼ�� �ǰ���?

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
// float4 �� ���� specification�� SV_POSITION
// SV : System Value
// transform�� ��ġġ �ʰ� �ٷ� rasterize�� ������ ���� : �� space ��� identity matrix�� �Է� -> model tran = projection tran
// projection transform�� identity�⿡ orthognal frustum
// �������� input, output ����
PS_INPUT VS_TEST(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(input.Pos, mWorld);
	output.Pos = mul(output.Pos, mView);
	//output.VecCs = output.Pos;
	output.PosCS = output.Pos.xyz / output.Pos.w; // camera space�� position
	
	output.Pos = mul(output.Pos, mProjection);
	output.Color = input.Color;
	output.Nor = normalize(mul(input.Nor, mul(mView, mWorld))); // camera space�� ���� �ʿ�
	// Normal vector�� object space���� righting�� camera space�� ȿ�����̱� �����̱⿡ �� �ڵ�� �ۼ�
	// row major���� ����� �����Ͱ� ���� ���� matrix�� ���ϴ� ���Ŀ����� vector, matrix ������ �ۼ�
	// ��, mul(mView, mWorld) matrix transform �Ǵ� ������ <-
	// inverse�� transpose ��Ű�� 
	// Normal vector transform ���� : + ����
    return output;
}

float3 	PhongLighting(float3 L, float3 N, float3 R, float3 V, float3 mtcAmbient, float3 mtxDiffuse, float3 mtcSpec, float shiness, float3 lightColor)
{
	return mtcAmbient * lightColor + mtxDiffuse * lightColor * max(dot(N, L), 0) + mtcSpec * lightColor * pow(max(dot(R, V), 0), shiness);
}

// specular term �Ⱥ��̰� �ϱ�
float3 	PhongLighting2(float3 L, float3 N, float3 R, float3 V, float3 mtcAmbient, float3 mtxDiffuse, float3 mtcSpec, float shiness, float3 lightColor)
{
	if (dot(N, L) <= 0) R = (float3)0;
	return mtcAmbient * lightColor + mtxDiffuse * lightColor * max(dot(N, L), 0) + mtcSpec * lightColor * pow(max(dot(R, V), 0), shiness);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
// Pos : screen space ������ ��ȯ�� ��
// camera space input �ʿ�
// light�� world space�� ������ ���̸� hard coding�� ����
// but camera�� �ڲ� ���ϹǷ� light�� application level�� ���� ���� ��
// �׷��� constant buffer�� �߰�
float4 PS(PS_INPUT input) : SV_Target
{
	float3 L, N, R, V; // compute to do
	L = normalize(posLightCS - input.PosCS);
	N = input.Nor;
	R = normalize(2*(dot(L, N) * N - L));
	V = normalize(-1*input.PosCS);
	
	if (light_controler == 1) {
		L = normalize(posLightCS - input.PosCS);
	}
	else if (light_controler == 2) {
		L = normalize(float3(0,-1,0)--input.PosCS);
	}

	float3 colorOut = PhongLighting2(L, N, R, V, mtcAmbient, mtxDiffuse, mtcSpec, shine, lightColor);
	
	return float4(colorOut, 1);
	//float3 unit_nor = (input.Nor);// interpolation �Ǹ� normal�� unit vetor�� ������ ���� �� �ֱ� ������
	//return float4((unit_nor+(float3)1.f)/2.f, 1.0f);    // 0~1�� ���� ���� ���� /2.f
	//return lightFlag == 777 ? float4(1, 0, 0, 1) : float4(0, 1, 0, 1);
    //return float4(input.Color.xyz, 1.0f);    // Yellow, with Alpha = 1
}
