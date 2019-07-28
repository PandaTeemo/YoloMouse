SamplerState    sampler0: register( s0 );
Texture2D       texture0;

cbuffer ConstantBuffer
{
    float3  light_vector;
    uint    texturing;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color :    COLOR;
    float3 normal :   NORMAL;
    float2 uv :       UV;
};

float4 PS(PS_INPUT input) : SV_Target
{
    //return float4(1,1,1,1);

    float   intensity = (dot(input.normal, light_vector) + 1) / 1;
    float4  vintensity = float4(intensity, intensity, intensity, 1);

    if( texturing != 0 )
        return texture0.Sample(sampler0, input.uv);
    else
        return input.color * vintensity;
}
