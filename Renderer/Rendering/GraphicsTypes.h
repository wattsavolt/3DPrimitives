#pragma once

#include "Minimal.h"
#include "D3DIncludes.h"

using namespace DirectX;

namespace renderer
{
	/** Wrapper to store mesh buffers */
	struct MeshBuffers
	{
		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* instanceBuffer;
		ID3D11Buffer* indexBuffer;
		uint32_t indexCount = 0;
		uint32_t vertexCount = 0;

		void Release()
		{
			SAFE_RELEASE(vertexBuffer);
			SAFE_RELEASE(instanceBuffer);
			SAFE_RELEASE(indexBuffer);
			indexCount = 0;
			vertexCount = 0;
		}
	};


	/** Wrapper for structured buffer */
	struct StructuredBuffer
	{
		ID3D11Buffer* buffer;
		ID3D11ShaderResourceView* shaderResourceView;

		void Release()
		{
			SAFE_RELEASE(shaderResourceView);
			SAFE_RELEASE(buffer);
		}
	};

	/** Point light for shader */
	struct ShaderPointLight
	{
		XMFLOAT3 pos;
		float range;
		XMFLOAT3 att;
		float pad;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;
	};

	/** Spot light for shader */
	struct ShaderSpotLight
	{
		XMFLOAT3 pos;
		float range;
		XMFLOAT3 dir;
		float cone;
		XMFLOAT3 att;
		float pad;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;
	};

	struct ShaderSceneParams
	{
		XMMATRIX ViewProj;
		XMFLOAT3 camPos;
		float ambient;
		UINT pointLightCount;
		UINT pad1, pad2, pad3;
	};

	struct MeshInstanceData
	{
		XMMATRIX world;
	};
}