#include "Common.hlsli"

VS_OUTPUT main(float3 pos : POSITION, float3 normal : NORMAL, float4x4 instWorld : WORLD)
{
	float4x4 wvp = mul(instWorld, ViewProj);

	VS_OUTPUT output = (VS_OUTPUT)0;
	//output.pos.w = 1;
	output.pos = mul(float4(pos, 1.0f), wvp);
	output.worldPos = mul(float4(pos, 1.0f), instWorld);

	output.normal = mul(normal, instWorld);

	return output;
}

