cbuffer ConstantBuffer
{
    float4x4 mvp;
    float4x4 model;
    float4   variation_color;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color :    COLOR;
    float3 normal :   NORMAL;
    float2 uv :       UV;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color :    COLOR;
    float3 normal :   NORMAL;
    float2 uv :       UV;
};

VS_OUTPUT VS(const VS_INPUT input)
{
    VS_OUTPUT output;

    output.position = mul( float4(input.position, 1), mvp );
    output.normal = mul( model, float4(input.normal, 1) ).xyz;
    output.uv = input.uv;

    if( input.color.r == 0 && input.color.g == 0 && input.color.b == 0 )
        output.color = float4(variation_color.r, variation_color.g, variation_color.b, input.color.a * variation_color.a);
    else
        output.color = input.color;

    return output;
}
