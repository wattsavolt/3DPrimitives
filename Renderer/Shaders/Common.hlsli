#ifndef __COMMON_HLSL__
#define __COMMON_HLSL__

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 worldPos : POSITION;
	float3 normal : NORMAL;
};

struct Material
{
	float4 diffuse;
	float3 specular;
	float gloss;
};

struct PointLight
{
	float3 pos;
	float  range;
	float3 att;
	float pad;
	float4 diffuse;
	float4 specular;
};

struct SpotLight
{
	float3 pos;
	float  range;
	float3 dir;
	float cone;
	float3 att;
	float pad;
	float4 diffuse;
	float4 specular;
};

cbuffer SceneParamsCBuffer : register(b0)
{
	float4x4 ViewProj;
	float3 camPos;
	float ambient;
	uint pointLightCount;
	uint3 pad;
}

cbuffer SpotLightCBuffer : register(b1)
{
	SpotLight spotLight;
};

cbuffer MaterialCBuffer : register(b2)
{
	Material material;
};

StructuredBuffer<PointLight> pointLights : register(t0);

Texture2D shadowMap : register(t1);

#endif // __COMMON_HLSL__