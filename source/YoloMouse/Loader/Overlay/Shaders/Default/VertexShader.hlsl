cbuffer ConstantBuffer
{
    float4x4 mvp;
    float4x4 model;
};

struct VS_INPUT
{
    float4 position : POSITION;
    float4 normal :   NORMAL;
    float4 color :    COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 normal :   NORMAL;
    float4 color :    COLOR;
};

VS_OUTPUT VS(const VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul( input.position, mvp );
    output.normal = mul( model, input.normal );
    output.color = input.color;
    return output;
}
