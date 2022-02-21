#ifndef __LIGHT_UTILITY_HLSL__
#define __LIGHT_UTILITY_HLSL__

#include "Common.hlsli"

void ComputePointLightEffect(PointLight light, Material mat, float3 pos, float3 normal, float3 toEyeVec, out float3 diffuse, out float3 specular)
{
	//Initialize outputs
	diffuse = float3(0.0f, 0.0f, 0.0f);
	specular = float3(0.0f, 0.0f, 0.0f);

	//The direction from the surface to the light.
	float3 lightVec = light.pos - pos;

	//The distance from the surface to the light.
	float d = length(lightVec);

	//Go no further if the distance of the surface point from the light is greater than the range of the light.
	if (d > light.range)
	{
		return;
	}

	//Normalize the light vector
	lightVec /= d;

	//Diffuse and specular term are only added if the surface is in the line of sight of the light.
	float diffuseFactor = dot(lightVec, normal);

	//Flatten to avoid dynamic branching
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 reflectVec = reflect(-lightVec, normal);
		float specularFactor = pow(max(dot(reflectVec, toEyeVec), 0.0f), mat.gloss);
		diffuse = diffuseFactor * mat.diffuse * light.diffuse;
		specular = specularFactor * mat.specular * light.specular;

		//Calculate Light's Distance Falloff factor
		float att = 1.0f / (light.att[0] + (light.att[1] * d)) + (light.att[2] * (d * d));

		diffuse *= att;
		specular *= att;
	}
}

void ComputeSpotLightEffect(SpotLight light, Material mat, float3 pos, float3 normal, float3 toEyeVec, out float3 diffuse, out float3 specular)
{
	//Initialize outputs
	diffuse = float3(0.0f, 0.0f, 0.0f);
	specular = float3(0.0f, 0.0f, 0.0f);

	//The direction from the surface to the light.
	float3 lightVec = light.pos - pos;

	//The distance from the surface to the light.
	float d = length(lightVec);

	//Go no further if the distance of the surface point from the light is greater than the range of the light.
	if (d > light.range)
	{
		return;
	}

	//Normalize the light vector
	lightVec /= d;

	//Diffuse and specular term are only added if the surface is in the line of sight of the light.
	float diffuseFactor = dot(lightVec, normal);

	//Flatten to avoid dynamic branching
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 reflectVec = reflect(-lightVec, normal);
		float specularFactor = pow(max(dot(reflectVec, toEyeVec), 0.0f), mat.gloss);
		diffuse = diffuseFactor * mat.diffuse * light.diffuse;
		specular = specularFactor * mat.specular * light.specular;

		//Calculate falloff from center to edge of point light cone
		float falloff = pow(max(dot(-lightVec, light.dir), 0.0f), light.cone);

		//Calculate the light's distance falloff factor.
		float att = falloff / (light.att[0] + (light.att[1] * d)) + (light.att[2] * (d * d));

		diffuse *= att;
		specular *= att;
	}
}

#endif // __LIGHT_UTILITY_HLSL__