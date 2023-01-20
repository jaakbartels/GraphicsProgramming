// -----------------------------------------------------
// Global vars
// -----------------------------------------------------
float4x4 WorldViewProj : WorldViewProjection;
Texture2D DiffuseMap : DiffuseMap;
Texture2D NormalMap : NormalMap;
Texture2D SpecularMap : SpecularMap;
Texture2D GlossinessMap: GlossinessMap;

float4x4 WorldMatrix : World;
float4x4 ViewInverseMatrix : ViewInverse;

float gPI = 3.14159265358979311600;
float3 gLightDirection = normalize(float3(0.577f, -0.577f, 0.577f));
float gLightIntensity = 7.0f;
float gShininess = 25.0f;

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

float CalculatePhong(float ks, float exp, float3 l, float3 v, float3 n)
{
    const float3 reflectVec = reflect(l, n);
    const float alfa = saturate(dot(reflectVec, v));
    float phong = 0;
    if (alfa > 0)
    {
        phong = ks * pow(alfa, exp);
    }

    return phong;
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

float4 PS_Phong(VS_OUTPUT input, SamplerState state) : SV_TARGET
{
    const float3 binormal = cross(input.Normal, input.Tangent);
    const float4x4 tangentSpaceAxis = float4x4(float4(input.Tangent, 0.0f), float4(binormal, 0.0f), float4(input.Normal, 0.0), float4(0.0f, 0.0f, 0.0f, 1.0f));
    const float3 currentNormalMap = 2.0f * NormalMap.Sample(state, input.UV).rgb - float3(1.0f, 1.0f, 1.0f);
    const float3 normal = mul(float4(currentNormalMap, 0.0f), tangentSpaceAxis);

    const float3 viewDirection = normalize(input.WorldPosition.xyz - ViewInverseMatrix[3].xyz);

    const float observedArea = saturate(dot(normal, -gLightDirection));
    const float4 lambert = CalculateLambert(1.0f, DiffuseMap.Sample(state, input.UV));
    const float specularExp = gShininess * GlossinessMap.Sample(state, input.UV).r;
    const float4 specular = SpecularMap.Sample(state, input.UV) * CalculatePhong(1.0f, specularExp, -gLightDirection, viewDirection, input.Normal);

    return (gLightIntensity * lambert + specular) * observedArea;
}

float4 PS_Point(VS_OUTPUT input) : SV_TARGET
{
    return PS_Phong(input,gSamStatePoint);
}

float4 PS_Linear(VS_OUTPUT input) : SV_TARGET
{
    return PS_Phong(input,gSamStateLinear);
}

float4 PS_Anisotropic(VS_OUTPUT input) : SV_TARGET
{
    return PS_Phong(input,gSamStateAnisotropic);
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
