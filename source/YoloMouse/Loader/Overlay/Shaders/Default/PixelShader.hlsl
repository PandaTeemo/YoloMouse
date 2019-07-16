static const float4 LIGHT_VEC = {0,-1,0,1};

cbuffer ConstantBuffer
{
    float4 light_color;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 normal :   NORMAL;
    float4 color :    COLOR;
};

float4 PS(PS_INPUT input) : SV_Target
{
    //return float4(0,0,1,1);
    float   intensity = dot(input.normal, LIGHT_VEC);
    float4  vintensity = float4(intensity, intensity, intensity, 1);
    //return  input.color * vintensity;
    //return  (input.color + light_color) * vintensity;
    return  light_color * vintensity;
}
