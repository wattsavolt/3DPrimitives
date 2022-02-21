#pragma once
#pragma once

#include "DataTypes.h"
#include "GraphicsTypes.h"

namespace renderer
{
    class GraphicsManager;
    class Camera;

    class MeshRenderer
    {
    public:
        static MeshRenderer* Initialize(GraphicsManager* graphicsManager);
        ~MeshRenderer(); 
        void Render(double frameTime, const Camera* camera);
        void AddSpotLight(const std::shared_ptr<SpotLight>& spotLight);
        void AddPointLight(const std::shared_ptr<PointLight>& pointLight);
        void AddEntity(const std::shared_ptr<Entity>& entity);

    private:
        friend class Renderer;
        MeshRenderer(GraphicsManager* graphicsManager);
        void DrawMeshes();
        void DrawMeshes(const MeshBuffers& buffers, const std::vector<std::shared_ptr<Entity>>& entities);
        void LoadShaders();
        void UpdateConstantBuffers(const Camera* camera);
        void UpdateStructuredBuffers();
        void UpdateMeshInstanceBuffers();
        void UpdateMeshInstanceBuffer(MeshType meshType);
        void CreateConstantBuffers();
        void CreateStructuredBuffers();
        void CreateMeshBuffers(MeshType meshType);
        void DeleteMeshBuffers();

        GraphicsManager* mGM;
        std::unordered_map<MeshType, MeshBuffers> mMeshTypeDataMap;
        std::unordered_map<MeshType, std::vector<std::shared_ptr<Entity>>> mMeshTypeEntitiesMap;
        std::set<MeshType> mInstanceBuffersToCreate;


        // Data for buffers
        ShaderSceneParams mSceneParams;
        std::vector<std::shared_ptr<PointLight>> mPointLights;
        std::shared_ptr<SpotLight> mSpotLight;

        // Buffers
        ID3D11Buffer* mSceneConstantBuffer;
        ID3D11Buffer* mMaterialConstantBuffer;
        ID3D11Buffer* mSpotLightConstantBuffer;
        StructuredBuffer mPointLightStructuredBuffer;


        //Shaders
        ID3D11VertexShader* mBaseVertexShader;
        ID3D11InputLayout* mBaseVertexLayout;
        ID3D11PixelShader* mBasePixelShader;
        
        static MeshRenderer* mMeshRenderer;

        static constexpr std::uint32_t MaxPointLightsAllowed = 20;
    };
}