# define X_PI 3.14159265358979323846

cbuffer TransformScene : register(b0)
{
    matrix g_mView;
    matrix g_mProjection;
}

cbuffer TransformModel : register(b1)
{
    matrix g_mWorld;
    int g_mtAmbient, g_mtDiffuse, g_mtSpec;
    float g_shininess;
}


cbuffer LightBuffer : register(b2)
{
    float3 g_posLightCS;
    int g_lightColor;
    float3 g_dirLightCS;
    int g_lightFlag;
    matrix g_mView2EnvOS;
}

// t : texture, shader에서도 사용
Buffer<float3> NormalBuffer : register(t0);

// float3인 이유 : NORM은 GPU에서 쓰는 정밀도가 8bit인 float으로 사용 
Texture2D<float4> EnvTexture : register(t1);

// sampler = s
SamplerState EnvSamplerCpp:register(s0);
// image에서 픽셀 값을 가지고 올 때 
// mip map linea로 sample 하는 것이 나음
// boundary 외를 sampling할 때는 다른 property 적용 필요
// sampler를 불러온 것이 아닌 생성한 것
SamplerState EnvSampler : register(s1)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Border;
    AddressV = Border;
    BorderColor = float4(0, 0, 0, 0);
};

struct VS_INPUT1
{
    float4 Pos : POSITION0;
    float4 Color : COLOR0;
    float3 Nor : NORMAL0;
};

struct VS_INPUT2
{
    float4 Pos : POSITION0;
    float3 Nor : NORMAL0;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT1
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
    float3 PosCS : POSITION0;
    float3 Nor : NORMAL;
};

struct PS_INPUT2
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 PosCS : POSITION0;
    float3 Nor : NORMAL;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT1 VS_P(float4 Pos : POSITION0)
{
    PS_INPUT1 output = (PS_INPUT1)0;
    output.Pos = mul(Pos, g_mWorld);
    output.Pos = mul(output.Pos, g_mView);
    output.PosCS = output.Pos.xyz / output.Pos.w;

    output.Pos = mul(output.Pos, g_mProjection);
    output.Color = (float4)0;
    output.Nor = (float4)0;

    return output;
}

PS_INPUT1 VS_PCN(VS_INPUT1 input)
{
    PS_INPUT1 output = (PS_INPUT1)0;
    output.Pos = mul(input.Pos, g_mWorld);
    output.Pos = mul(output.Pos, g_mView);
    output.PosCS = output.Pos.xyz / output.Pos.w;

    output.Pos = mul(output.Pos, g_mProjection);
    output.Color = input.Color;
    output.Nor = normalize(mul(input.Nor, mul(g_mView, g_mWorld)));

    // 0~15값이 들어와야 함
    //input.primID
    return output;
}

PS_INPUT2 VS_PNT(VS_INPUT2 input)
{
    PS_INPUT2 output = (PS_INPUT2)0;
    output.Pos = mul(input.Pos, g_mWorld);
    output.Pos = mul(output.Pos, g_mView);
    output.PosCS = output.Pos.xyz / output.Pos.w;

    output.Pos = mul(output.Pos, g_mProjection);
    output.Tex = input.Tex;
    output.Nor = normalize(mul(input.Nor, mul(g_mView, g_mWorld)));
    return output;
}

float3 PhongLighting(float3 L, float3 N, float3 R, float3 V,
    float3 mtcAmbient, float3 mtcDiffuse, float3 mtcSpec, float shininess,
    float3 lightColor) {

    float dotNL = dot(N, L);
    //if (dot(N, L) <= 0) R = (float3)0;
    //float3 mtL = mtColor * lColor;
    // in CS, V is (0, 0, -1)
    // dot(R, V) equals to -R.z
    return mtcAmbient * lightColor + mtcDiffuse * lightColor * max(dotNL, 0) + mtcSpec * lightColor * pow(max(dot(R, V), 0), shininess);
}

float3 PhongLighting2(float3 L, float3 N, float3 R, float3 V,
    float3 mtcAmbient, float3 mtcDiffuse, float3 mtcSpec, float shininess,
    float3 lightColor) {

    float dotNL = dot(N, L);
    if (dot(N, L) <= 0) R = (float3)0;
    //float3 mtL = mtColor * lColor;
    // in CS, V is (0, 0, -1)
    // dot(R, V) equals to -R.z
     return mtcAmbient * lightColor + mtcDiffuse * lightColor * max(dotNL, 0) + mtcSpec * lightColor * pow(max(dot(R, V), 0), shininess);
}

float3 ConvertInt2Float3(int iColor) {
    float3 color;
    color.r = (iColor & 0xFF) / 255.f;
    color.g = ((iColor >> 8) & 0xFF) / 255.f;
    color.b = ((iColor >> 16) & 0xFF) / 255.f;
    return color;
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS1(PS_INPUT1 input, uint primID : SV_PrimitiveID) : SV_Target
{
    // Light Color
    float3 lightColor = ConvertInt2Float3(g_lightColor);// float3(1.f, 1.f, 1.f);
    // Material Colors : ambient, diffuse, specular, shininess
    float3 mtcAmbient = ConvertInt2Float3(g_mtAmbient), mtcDiffuse = ConvertInt2Float3(g_mtDiffuse), mtcSpec = ConvertInt2Float3(g_mtSpec);
    float shine = g_shininess;
    // Light type (point)... 
    float3 L, N, R, V; // compute to do

    {
        if (g_lightFlag == 0)
            L = normalize(g_posLightCS - input.PosCS);
        else
            L = g_dirLightCS;
        V = normalize(-input.PosCS);
        
        // vertex의 attribute로 가진 normal 값을 resterizer에서 interporation된 값으로 사용
        // N = normalize(input.Nor);

        // primID = 0, 1 -> 0
        // primID = 1, 2 -> 1
        N = normalize(mul(NormalBuffer[primID], mul(g_mView, g_mWorld)));

        R = 2 * dot(N, L) * N - L;
    }

    float3 colorOut = PhongLighting2(L, N, R, V,
        mtcAmbient, mtcDiffuse, mtcSpec, shine,
        lightColor);

    // if (primID < 4 && primID >= 2)
    //    return float4(1, 1, 1, 1);
     
    return float4(colorOut, 1);

    //float3 posCS = input.PosCS;
    //float3 unit_nor = normalize(input.Nor);
    //return float4(input.Color.xyz, 1.0f);
    //return float4((unit_nor + (float3)1.f) / 2.f, 1.0f);
}

float4 PS2(PS_INPUT2 input) : SV_Target
{
    float3 colorOut = ConvertInt2Float3(g_lightColor);
    
    return float4(colorOut, 1);
}

float4 PS3(PS_INPUT2 input) : SV_Target
{
     float3 colorOut = EnvTexture.Sample(EnvSamplerCpp, input.Tex.xy);

     return float4(colorOut, 1);
}

float4 PS4(PS_INPUT1 input, uint primID : SV_PrimitiveID) : SV_Target
{

    float3 lightColor = ConvertInt2Float3(g_lightColor);// float3(1.f, 1.f, 1.f);
    // Material Colors : ambient, diffuse, specular, shininess
    float3 mtcAmbient = ConvertInt2Float3(g_mtAmbient), mtcDiffuse = ConvertInt2Float3(g_mtDiffuse), mtcSpec = ConvertInt2Float3(g_mtSpec);
    float shine = g_shininess;
    // Light type (point)... 
    float3 L, N, R, V; // compute to do

    {
        if (g_lightFlag == 0)
            L = normalize(g_posLightCS - input.PosCS);
        else
            L = g_dirLightCS;
        V = normalize(-input.PosCS);
        //N = normalize(input.Nor);
        N = normalize(mul(NormalBuffer[primID], mul(g_mView, g_mWorld))); // 면단위로 같은 노멀 백터
        R = 2 * dot(N, L) * N - L;
    }

    float3 color1 = PhongLighting2(L, N, R, V, mtcAmbient, mtcDiffuse, mtcSpec, shine, lightColor);

    float3 vR;
    {
        vR = 2 * dot(N, V) * N - V;
    }

    float3 envR = mul(vR, g_mView2EnvOS);
    envR = normalize(envR);

    float phi = asin(envR.z); // 90~-90
    float cosphi = cos(phi);
    float theta = acos(max(min(envR.x / cos(phi),0.999f),-0.999f));
    float u = envR.y >= 0 ? theta / (2 * X_PI) : (2 * X_PI - theta) / (2 * X_PI);
    float v = (X_PI / 2 - phi) / X_PI;

    float3 texColor = EnvTexture.Sample(EnvSamplerCpp, float2(u, v));

    R = normalize(2 * dot(N, vR) * N - vR);
    float3 color2 = PhongLighting2(vR,N,R,V,mtcAmbient, mtcDiffuse, mtcSpec, shine,texColor);
    
    return float4(saturate(color1 + color2), 1);
}

// camera space가 자꾸 바뀌기 때문에 point light 는 application level에 들어와야 함

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

    // buffer는 sampler 적용이 안됨