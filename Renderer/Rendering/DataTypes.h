#pragma once
#pragma once

#include "Minimal.h"

using namespace DirectX;

namespace renderer
{
	/** Struct with config used for creating the graphics device */
	struct GraphicsConfig
	{
		bool vSyncEnabled = false;
		int screenWidth = 1920;
		int screenHeight = 1080;
		bool windowed = true;
		bool multiSamplingenabled = true;
		std::uint32_t multiSamplingCount = 4;
		std::uint32_t refreshRate = 60;
	};

	/** Point light used in renderer */
	struct PointLight
	{
		XMFLOAT3 position;
		float range;
		XMFLOAT3 attenuation;
		XMFLOAT3 diffuse;
		XMFLOAT3 specular;
	};

	/** Spot light used in renderer */
	struct SpotLight
	{
		XMFLOAT3 position;
		float range;
		XMFLOAT3 direction;
		float cone;
		XMFLOAT3 attenuation;
		XMFLOAT3 diffuse;
		XMFLOAT3 specular;
	};

	/** Struct used for vertex layout */
	struct Vertex
	{
		Vertex() = default;
		constexpr Vertex(float x, float y, float z,
			float nx, float ny, float nz)
			: position(x, y, z),
			normal(nx, ny, nz) {}

		XMFLOAT3 position;
		XMFLOAT3 normal;
	};

	/** Types of primitives the renderer supports */
	enum class MeshType
	{
		Cone,
		Cube,
		Sphere
	};

	/** Phong material used by the renderer */
	struct Material
	{
		XMFLOAT4 diffuse;
		XMFLOAT3 specular;
		float gloss;
	};

	class Cube 
	{
	public:
		static constexpr std::uint32_t numVertices = 24;
		static constexpr std::uint32_t numIndices = 36;

		// Vertices for a unit cube
		static constexpr Vertex vertices[numVertices] =
		{
			// Front Face
			Vertex(-0.5f, -0.5f, -0.5f, 0, 0, -1),
			Vertex(-0.5f, 0.5f, -0.5f, 0, 0, -1),
			Vertex(0.5f, 0.5f, -0.5f, 0, 0, -1),
			Vertex(0.5f, -0.5f, -0.5f, 0, 0, -1),

			// Back Face
			Vertex(-0.5f, -0.5f, 0.5f, 0, 0, 1),
			Vertex(0.5f, -0.5f, 0.5f, 0, 0, 1),
			Vertex(0.5f, 0.5f, 0.5f, 0, 0, 1),
			Vertex(-0.5f, 0.5f, 0.5f, 0, 0, 1),

			// Top Face
			Vertex(-0.5f, 0.5f, -0.5f, 0, 1, 0),
			Vertex(-0.5f, 0.5f, 0.5f, 0, 1, 0),
			Vertex(0.5f, 0.5f, 0.5f, 0, 1, 0),
			Vertex(0.5f, 0.5f, -0.5f, 0, 1, 0),

			// Bottom Face
			Vertex(-0.5f, -0.5f, -0.5f, 0, -1, 0),
			Vertex(0.5f, -0.5f, -0.5f, 0, -1, 0),
			Vertex(0.5f, -0.5f, 0.5f, 0, -1, 0),
			Vertex(-0.5f, -0.5f, 0.5f, 0, -1, 0),

			// Left Face
			Vertex(-0.5f, -0.5f, 0.5f, -1, 0, 0),
			Vertex(-0.5f, 0.5f, 0.5f, -1, 0, 0),
			Vertex(-0.5f, 0.5f, -0.5f, -1, 0, 0),
			Vertex(-0.5f, -0.5f, -0.5f, -1, 0, 0),

			// Right Face
			Vertex(0.5f, -0.5f, -0.5f, 1, 0, 0),
			Vertex(0.5f, 0.5f, -0.5f, 1, 0, 0),
			Vertex(0.5f, 0.5f, 0.5f, 1, 0, 0),
			Vertex(0.5f, -0.5f, 0.5f, 1, 0, 0),
		};

			
		// Indices in clock-wise order
		static constexpr DWORD indices[numIndices] = {
			// Front Face
			0, 1, 2,
			0, 2, 3,

			// Back Face
			4, 5, 6,
			4, 6, 7,

			// Top Face
			8, 9, 10,
			8, 10, 11,

			// Bottom Face
			12, 13, 14,
			12, 14, 15,

			// Left Face
			16, 17, 18,
			16, 18, 19,

			// Right Face
			20, 21, 22,
			20, 22, 23
		};
	};

	/** Mesh object that is passed to the renderer */
	struct Entity
	{
		std::shared_ptr<Material> material;
		MeshType meshType = MeshType::Cube;
		XMFLOAT3 position = { 0, 0, 0 };
		// Axis and Angle in degrees
		XMFLOAT4 rotation = { 0, 0, 0, 0 };
		XMFLOAT3 scale = { 0, 0, 1 };
	};
}