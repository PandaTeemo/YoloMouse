// enums
static const uint TYPE_UNLIT_BASIC =    0;
static const uint TYPE_LIT_BASIC =      1;
static const uint TYPE_UNLIT_TEXTURE =  2;

// constants
static const float SATURATION =         1.5;

// texture
SamplerState    sampler0: register( s0 );
Texture2D       texture0;

// constants
cbuffer ConstantBuffer
{
    float3  light_vector;
    uint    type;
};

// input
struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color :    COLOR;
    float3 normal :   NORMAL;
    float2 uv :       UV;
};

// main
float4 PS(PS_INPUT input) : SV_Target
{
    //return float4(1,1,1,1);

    switch( type )
    {
    case TYPE_UNLIT_BASIC:
        return input.color;

    case TYPE_LIT_BASIC:
        {
            float light = dot( input.normal, light_vector );
            return float4(input.color.rgb * SATURATION + light, input.color.a);
        }

    case TYPE_UNLIT_TEXTURE:
        return texture0.Sample( sampler0, input.uv ) * input.color;

    default:
        return float4(1,1,1,1);
    }
}
