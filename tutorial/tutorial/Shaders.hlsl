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

// VS의 OUT으로 사용 가능
struct PS_INPUT
{
	// HLSL에서의 변수는 semantic을 설정할 수 있음 
	// semantic : 변수의 입력과 출력을 확인 또는 지정하거나 데이터의 출처와 역할에 의미 부여하기 위해 
	// 함수, 변수, 인수 뒤에 선택적으로 붙여서 서술하는 것
	float4 Pos : SV_POSITION; // pos 값이 screen space 값으로 변하여 
	float4 Color : COLOR0;
	float3 PosCS: POSIION0;
	float3 Nor : NORMAL0;
	// float3 VecL : 
};

// Q. rasterization 은 언제 일어나는 건가요?

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
// float4 에 대한 specification이 SV_POSITION
// SV : System Value
// transform을 거치치 않고 바로 rasterize로 보내는 이유 : 세 space 모두 identity matrix로 입력 -> model tran = projection tran
// projection transform은 identity기에 orthognal frustum
// 여러개의 input, output 가능
PS_INPUT VS_TEST(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(input.Pos, mWorld);
	output.Pos = mul(output.Pos, mView);
	//output.VecCs = output.Pos;
	output.PosCS = output.Pos.xyz / output.Pos.w; // camera space의 position
	
	output.Pos = mul(output.Pos, mProjection);
	output.Color = input.Color;
	output.Nor = normalize(mul(input.Nor, mul(mView, mWorld))); // camera space로 변경 필요
	// Normal vector는 object space지만 righting은 camera space가 효율적이기 때문이기에 위 코드로 작성
	// row major에서 저장된 데이터가 들어올 때는 matrix를 곱하는 수식에서는 vector, matrix 순으로 작성
	// 단, mul(mView, mWorld) matrix transform 되는 순서는 <-
	// inverse에 transpose 시키면 
	// Normal vector transform 구현 : + 점수
    return output;
}

float3 	PhongLighting(float3 L, float3 N, float3 R, float3 V, float3 mtcAmbient, float3 mtxDiffuse, float3 mtcSpec, float shiness, float3 lightColor)
{
	return mtcAmbient * lightColor + mtxDiffuse * lightColor * max(dot(N, L), 0) + mtcSpec * lightColor * pow(max(dot(R, V), 0), shiness);
}

// specular term 안보이게 하기
float3 	PhongLighting2(float3 L, float3 N, float3 R, float3 V, float3 mtcAmbient, float3 mtxDiffuse, float3 mtcSpec, float shiness, float3 lightColor)
{
	if (dot(N, L) <= 0) R = (float3)0;
	return mtcAmbient * lightColor + mtxDiffuse * lightColor * max(dot(N, L), 0) + mtcSpec * lightColor * pow(max(dot(R, V), 0), shiness);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
// Pos : screen space 값으로 변환된 값
// camera space input 필요
// light가 world space에 고정된 값이면 hard coding이 가능
// but camera가 자꾸 변하므로 light는 application level에 값이 들어가야 함
// 그래서 constant buffer를 추가
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
	//float3 unit_nor = (input.Nor);// interpolation 되면 normal이 unit vetor로 들어오지 않을 수 있기 때문에
	//return float4((unit_nor+(float3)1.f)/2.f, 1.0f);    // 0~1의 값을 값기 위해 /2.f
	//return lightFlag == 777 ? float4(1, 0, 0, 1) : float4(0, 1, 0, 1);
    //return float4(input.Color.xyz, 1.0f);    // Yellow, with Alpha = 1
}
