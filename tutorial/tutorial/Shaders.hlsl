cbuffer TransformBuffer : register(b0)
{
	matrix mWorld;
	matrix mView;
	matrix mProjection;
}

cbuffer LightBuffer : register(b1) // buffer 1
{
	float3 posLightCS;
	float go_con;
	float3 lightColor;
	float dummy2;
	float3 OriginCS;
	float dummy4;

}

cbuffer MaterialBuffer : register(b2)
{
	float3 mtcAmbient;
	float shine;
	float3 mtxDiffuse;
	float dummy3;
	float3 mtcSpec;
	int light_controler;
}

cbuffer GoLightBuffer : register(b3)
{
	float3 goposLightCS;
	float goshad_con;
	float3 golightColor;
	float godummy2;
	float3 goOriginCS;
	float godummy3;
}

cbuffer GoMaterialBuffer : register(b4)
{
	float3 GomtcAmbient;
	float Goshine;
	float3 GomtxDiffuse;
	float Godummy3;
	float3 GomtcSpec;
	int Golight_controler;
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
	float4 Pos : SV_POSITION;  
	float4 Color : COLOR0;
	float3 PosCS: POSIION0;
	float3 Nor : NORMAL0;
};

float3 	PhongLighting(float3 L, float3 N, float3 R, float3 V, float3 mtcAmbient, float3 mtxDiffuse, float3 mtcSpec, float shiness, float3 lightColor)
{
	float3 ambient = mtcAmbient * lightColor;
	float3 diffuse = mtxDiffuse * lightColor * max(dot(N, L), 0);
	float3 specular = mtcSpec * lightColor * pow(max(dot(R, V), 0), shiness);
	return ambient + diffuse + specular;
}

// specular term 안보이게 하기
float3 	PhongLighting2(float3 L, float3 N, float3 R, float3 V, float3 mtcAmbient, float3 mtxDiffuse, float3 mtcSpec, float shiness, float3 lightColor)
{
	if (dot(N, L) <= 0) R = (float3)0;
	float3 ambient = mtcAmbient * lightColor;
	float3 diffuse = mtxDiffuse * lightColor * max(dot(N, L), 0);
	float3 specular = mtcSpec * lightColor * pow(max(dot(R, V), 0), shiness);
	return ambient + diffuse + specular;
}


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
	output.PosCS = output.Pos.xyz / output.Pos.w; // camera space의 position
	
	output.Pos = mul(output.Pos, mProjection);
	output.Color = input.Color;
	output.Nor = normalize(mul(input.Nor, mul(mView, mWorld))); // camera space로 변경 필요


	float3 L = normalize(goposLightCS - output.PosCS);
	float3 N = output.Nor;
	float3 V = normalize(-1 * output.PosCS);
	float3 R = normalize(2 * (dot(L, N) * N - L));
	if (dot(N, L) <= 0) R = (float3)0;
	if (Golight_controler == 1) {
		L = normalize(goposLightCS - output.PosCS);
	}
	else if (Golight_controler == 2) {

		L = normalize(goposLightCS - goOriginCS);
	}

	float3 ambient = GomtcAmbient * golightColor;
	float3 diffuse = GomtxDiffuse * golightColor * max(dot(N, L), 0);
	float3 specular = GomtcSpec * golightColor * pow(max(dot(R, V), 0), Goshine);
	float3 colorout = ambient + diffuse + specular;

	output.Color = float4(colorout, 1);
	
    return output;
}



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
// camera space input 필요
// light가 world space에 고정된 값이면 hard coding이 가능
// but camera가 자꾸 변하므로 light는 application level에 값이 들어가야 함
// 그래서 constant buffer를 추가
float4 PS(PS_INPUT input) : SV_Target
{
		float3 L, N, R, V; // compute to do
		L = normalize(posLightCS - input.PosCS);
		N = input.Nor;
		R = normalize(2 * (dot(L, N) * N - L));
		V = normalize(-1 * input.PosCS);

		if (light_controler == 1 ) {
			L = normalize(posLightCS - input.PosCS);
		}
		else if (light_controler == 2) {

			L = normalize(posLightCS - OriginCS);
		}
		float3 colorOut = PhongLighting2(L, N, R, V, mtcAmbient, mtxDiffuse, mtcSpec, shine, lightColor);
		if (go_con == 1) {
			colorOut = PhongLighting2(L, N, R, V, mtcAmbient, mtxDiffuse, mtcSpec, shine, lightColor);
		}
		if (go_con == 2) {
			colorOut = input.Color;
		}
		return float4(colorOut, 1);

	
	//float3 unit_nor = (input.Nor);// interpolation 되면 normal이 unit vetor로 들어오지 않을 수 있기 때문에
	//return float4((unit_nor+(float3)1.f)/2.f, 1.0f);    // 0~1의 값을 값기 위해 /2.f
	//return lightFlag == 777 ? float4(1, 0, 0, 1) : float4(0, 1, 0, 1);
    //return float4(input.Color.xyz, 1.0f);    // Yellow, with Alpha = 1
}
