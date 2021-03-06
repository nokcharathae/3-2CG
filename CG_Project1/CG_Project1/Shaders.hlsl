#define X_PI 3.141592

cbuffer TransforBuffer : register(b0) // buffer 0
{
	matrix mWorld;
	matrix mView;
	matrix mProjection;
}

cbuffer LightBuffer : register(b1) // buffer 1
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
	float3 PosCS: POSIION0;
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
	output.PosCS = output.Pos.xyz / output.Pos.w; // camera space의 position

	output.Pos = mul(output.Pos, mProjection);
	output.Color = input.Color;
	output.Normal = normalize(mul(input.Normal,mul(mView,mWorld))); // camera space로 변경 필요
	// Normal vector는 object space지만 righting은 camera space가 효율적이기 때문이기에 위 코드로 작성
	// mul(mView,mWorld) -> col major로 계산이므로 row major로 계산하기 위해서는 vector, matrix 순으로 작성
	// inverse에 transepose 시키면 
	// Normal vector transform 구현 : + 점수
	return output;
}

/*
VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.Pos = mul(float4(input.Pos, 1), WorldViewProjection);
	output.WPos = mul(float4(input.Pos, 1), World);
	output.Color = input.Color;
	output.Normal = normalize(mul(input.Normal, (float3x3) World));

	return output;
}
*/


float3 	PhongLighting(float3 L, float3 N, float3 R, float3 V, float3 mtcAmbient, float3 mtxDiffuse, float3 mtcSpec, float shiness, float3 lightColor)
{
	float n_l = dot(N, L);
	//return mtcAmbient * lightColor + mtxDiffuse * lightColor * max(n_l, 0) + mtcSpec * lightColor * pow(max(dot(R, V), 0), shiness);
	return float3(0, 1, 1);
}

// specular term 안보이게 하기
float3 	PhongLighting2(float3 L, float3 N, float3 R, float3 V, float3 mtcAmbient, float3 mtxDiffuse, float3 mtcSpec, float shiness, float3 lightColor)
{
	float n_l = dot(N, L);
	//if (dot(N, L) <= )R = (float3)0;
	//return mtcAmbient * lightColor + mtxDiffuse * lightColor * max(n_l, 0) + mtcSpec * lightColor * pow(max(dot(R, V), 0), shiness);
	return float3(0, 1, 1);
}

// Pos가 viewport를 통해 screen space로 변환
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
	// L : light vector
	// N : normal vector
	// R : reflection vector
	// V : view vector
	float3 L,N,R,V; // compute to do

	//N= normalize(input.Normal);

	float3 colorOut = PhongLighting(L, N, R, V, mtcAmbient, mtxDiffuse, mtcSpec, shine, lightColor);
	return float4(colorOut, 1);

	//float3 posCS = input.PosCS;
	//float3 unit_nor = normalize(input.Normal);// interpolation 되면 normal이 unit vetor로 들어오지 않을 수 있기 때문에 
	//return float4((unit_nor+(float3)1.f)/2.f, 1.0f);    // 0~1의 값을 값기 위해 /2.f
	//return lightFlag == 777 ? float4(1, 0, 0, 1) : float4(0, 1, 0, 1);
	//return float4(input.Color.xyz, 1.0f); // vertex의 color를 가시화한 것
}

// camera space가 자꾸 바뀌기 때문에 point light 는 apllication level에 들어와야 함

   // float3 colorout=float3(input.Tex.xy,0) : resister(t1);
   // Sampleler 


//{
	// orientation을 바꾸는 tranform 
	// Matrix matR = Matrix::CreateLookAt(Vector3(0,0,0),Vector3(0,-1,0),Vector3(0,0,1));

	// u, v 계산하고 그 u, v로부터 매팡하는 코드
	// colorout= EnvTexuter.Sample(EnvSamplerCpp,input.Tex.xy);
	// Reflection vector
	// vR = 2*dot(N,V)*N-V;
	// vR을 구의 object Space로 transform 시켜야함
	// 
	// Matrix mView2EnvOS;
	// 
	// vR=mul(vR,g_mView2EnvOS);
	// vR=normalize(vR);
	// flaot phi=asin(vR.z);
	// float cosPhi=cos(phi);
	// float theta=acos(max(min(vR.x/cos(phi),0.999f),-0.999f)); 0.00001을 피하려고, 0이나 1값이 나오면 안됨
	// float u=envR.y>=0?theta/(2*x_PI):(2*X_PI=theta)/(2*X_PI);
	// float v=envR.(X_PI/2-phi)/X_PI;

	// colorout= EnvTexture.Sample(EnvSamplerCPP, float2(u,v));
	// R = 2*dot(N,vR)*N-vR;
	// return float4(saturate(color1+color2),1);
//}