// -----------------------------------------------------
// Global vars
// -----------------------------------------------------
float4x4 WorldViewProj : WorldViewProjection;
Texture2D DiffuseMap : DiffuseMap;

float4x4 WorldMatrix : World;
float4x4 ViewInverseMatrix : ViewInverse;

float gPI = 3.14159265358979311600;
float3 gLightDirection = normalize(float3(0.577f, -0.577f, 0.577f));
float gLightIntensity = 7.0f;

// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-to-sample
SamplerState gSamStatePoint : SampleState
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; // or Mirror, Clamp, Border
    AddressV = Wrap; // or Mirror, Clamp, Border
};
SamplerState gSamStateLinear : SampleState
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap; // or Mirror, Clamp, Border
    AddressV = Wrap; // or Mirror, Clamp, Border
};
SamplerState gSamStateAnisotropic : SampleState
{
    Filter = ANISOTROPIC;
    AddressU = Wrap; // or Mirror, Clamp, Border
    AddressV = Wrap; // or Mirror, Clamp, Border
};

// -----------------------------------------------------
// Input/Output structs
// -----------------------------------------------------
struct VS_INPUT
{
    float3 Position : POSITION;
    float2 UV : UV;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : COLOR;
    float2 UV : UV;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

//------------------------------------------------
// BRDF
//------------------------------------------------
float4 CalculateLambert(float kd, float4 cd)
{
    return cd * kd / gPI;
}

// -----------------------------------------------------
// Vertex Shader
// -----------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(float4(input.Position, 1.f), WorldViewProj);
    output.UV = input.UV;
    output.Tangent = mul(normalize(input.Tangent), (float3x3)WorldMatrix);
    output.Normal = mul(normalize(input.Normal), (float3x3)WorldMatrix);
    return output;
}

// -----------------------------------------------------
// Pixel Shader
// -----------------------------------------------------

float4 PS_Diffuse(VS_OUTPUT input, SamplerState state) : SV_TARGET
{
    return DiffuseMap.Sample(state, input.UV);
}

float4 PS_Point(VS_OUTPUT input) : SV_TARGET
{
    return PS_Diffuse(input,gSamStatePoint);
}

float4 PS_Linear(VS_OUTPUT input) : SV_TARGET
{
    return PS_Diffuse(input,gSamStateLinear);
}

float4 PS_Anisotropic(VS_OUTPUT input) : SV_TARGET
{
    return PS_Diffuse(input,gSamStateAnisotropic);
}

// -----------------------------------------------------
// Technique (Actual shader)
// -----------------------------------------------------
technique11 PointFilteringTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Point()));
    }
}

technique11 LinearFilteringTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
    }
}

technique11 AnisotropicFilteringTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
    }
}
