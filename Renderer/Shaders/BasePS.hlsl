#include "Common.hlsli"
#include "LightUtility.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	// The ambient light effecting the material should be calculated in the renderer before the material is drawn.

	// Create a float3 of the input position. Easier to use.
	float3 pixPos = float3(input.worldPos.x, input.worldPos.y, input.worldPos.z);

	// Calculate the direction from the pixel to the camera.
	float3 toEyeVec = normalize(camPos - pixPos);

	float3 colour = float3(0.0f, 0.0f, 0.0f);

	// These variables are reassigned every time a light computational function is called.
	float3 diffuse;
	float3 specular;

	for (int i = 0; i < pointLightCount; i++)
	{
		ComputePointLightEffect(pointLights[i], material, pixPos, input.normal, toEyeVec, diffuse, specular);
		colour += saturate(diffuse + specular);
	}

	ComputeSpotLightEffect(spotLight, material, pixPos, input.normal, toEyeVec, diffuse, specular);
	colour += saturate(diffuse + specular);

	// Add ambient light value
	colour.x += ambient;
	colour.y += ambient;
	colour.z += ambient;

	//Make sure the values are between 0 and 1
	colour = saturate(colour);

	//Return Final Color
	return float4(colour.x, colour.y, colour.z, 1.0f);
}