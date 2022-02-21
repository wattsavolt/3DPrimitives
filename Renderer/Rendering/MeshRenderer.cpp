#include "MeshRenderer.h"
#include "GraphicsManager.h"
#include "Camera/Camera.h"

namespace renderer
{
    MeshRenderer* MeshRenderer::mMeshRenderer = nullptr;

    MeshRenderer* MeshRenderer::Initialize(GraphicsManager* graphicsManager)
    {
        if (!mMeshRenderer)
        {
            mMeshRenderer = new MeshRenderer(graphicsManager);
        }
        return mMeshRenderer;
    }

    MeshRenderer::MeshRenderer(GraphicsManager* graphicsManager)
    {
        mGM = graphicsManager;

        mSceneParams.ambient = 0.25f;
        mSceneParams.camPos = XMFLOAT3(0, 0, 0);
        mSceneParams.ViewProj = XMMatrixTranspose(XMMatrixIdentity());
        mSceneParams.pad1 = 0;
        mSceneParams.pad2 = 0;
        mSceneParams.pad3 = 0;
        mSceneParams.pointLightCount = 0;

        // Create dummy spot light because one is required by the pixel shader
        // This would have been implemented better if not for time constraints
        mSpotLight = std::make_shared<SpotLight>();
        mSpotLight->position = { 0, 3, -2 };
        mSpotLight->direction = VF3(XMVector3Normalize(FV({ 0, 1, 1 })));
        mSpotLight->range = 10;
        mSpotLight->cone = 1.0f;
        mSpotLight->attenuation = { 1.0f, 0.85f, 0.6f };
        // White as requested
        mSpotLight->diffuse = { 1, 1, 1 };
        mSpotLight->specular = { 1, 1, 1 };

        LoadShaders();
        CreateConstantBuffers();
        CreateStructuredBuffers();
    }

    MeshRenderer::~MeshRenderer()
    {
        // Buffers
        SAFE_RELEASE(mSceneConstantBuffer);
        SAFE_RELEASE(mMaterialConstantBuffer);
        SAFE_RELEASE(mSpotLightConstantBuffer);
        mPointLightStructuredBuffer.Release();
      
        DeleteMeshBuffers();
        
        //Shaders
        SAFE_RELEASE(mBaseVertexShader);
        SAFE_RELEASE(mBaseVertexLayout);
        SAFE_RELEASE(mBasePixelShader);

        mMeshRenderer = nullptr;
    }

    // Note: This is just a forward renderer. Unfortunately due to time I couldn't implement defferred renderer
    void MeshRenderer::Render(double frameTime, const Camera* camera)
    {
        if (mMeshTypeEntitiesMap.empty())
        {
            return;
        }

        UpdateConstantBuffers(camera);
        UpdateStructuredBuffers();

        for (auto meshType : mInstanceBuffersToCreate)
        {
            CreateMeshBuffers(meshType);
        }
        mInstanceBuffersToCreate.clear();

        UpdateMeshInstanceBuffers();

        // Clear the backbuffer with black background colour
        float backgroundColour[4] = { 0, 0, 0, 1 };
        mGM->mDeviceContext->ClearRenderTargetView(mGM->mRenderTargetView, backgroundColour);

        // Refresh the Depth/Stencil view
        mGM->mDeviceContext->ClearDepthStencilView(mGM->mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // Set Render Target and bind depth stencil view to OM stage of pipeline.
        mGM->mDeviceContext->OMSetRenderTargets(1, &mGM->mRenderTargetView, mGM->mDepthStencilView);

        // Set vertex shader and input layout
        mGM->mDeviceContext->VSSetShader(mBaseVertexShader, 0, 0);
        mGM->mDeviceContext->IASetInputLayout(mBaseVertexLayout);

        // Set pixel shader
        mGM->mDeviceContext->PSSetShader(mBasePixelShader, 0, 0);

        // Pixel shader uses all 3 while vertex shader uses scene only 
        ID3D11Buffer* buffers[3] = { mSceneConstantBuffer, mSpotLightConstantBuffer, mMaterialConstantBuffer };
        mGM->mDeviceContext->VSSetConstantBuffers(0, 1, &mSceneConstantBuffer);
        mGM->mDeviceContext->PSSetConstantBuffers(0, 3, buffers);

        // Set shader resources
        ID3D11ShaderResourceView* resources[1] = { mPointLightStructuredBuffer.shaderResourceView };
        mGM->mDeviceContext->PSSetShaderResources(0, 1, resources);

        mGM->EnableClockwiseCulling();

        // Draw meshes
        DrawMeshes();

        // Present the backbuffer to the screen
        mGM->mSwapChain->Present(0, 0);
    }

    void MeshRenderer::DrawMeshes()
    {
        for (const auto& keyValue : mMeshTypeEntitiesMap)
        {
            const auto& buffers = mMeshTypeDataMap[keyValue.first];
            DrawMeshes(buffers, keyValue.second);
        }
    }

    void MeshRenderer::DrawMeshes(const MeshBuffers& buffers, const std::vector<std::shared_ptr<Entity>>& entities)
    {
        // Bind vertex and instance buffer for this mesh type
        ID3D11Buffer* vertexBuffers[2] = { buffers.vertexBuffer, buffers.instanceBuffer };
        UINT strides[2] = { sizeof(Vertex) , sizeof(MeshInstanceData) };
        UINT offsets[2] = { 0, 0 };
        mGM->mDeviceContext->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);

        // Bind index buffer
        mGM->mDeviceContext->IASetIndexBuffer(buffers.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // This could be optimized by passing materials in a structured buffer
        // and drawing all instances at the same time
        for (int i = 0; i < entities.size(); ++i)
        {
            const auto& entity = entities[i];
            // Update material constant buffer with entity material properties
            mGM->UpdateBuffer(mMaterialConstantBuffer, entity->material.get());

            mGM->mDeviceContext->DrawIndexedInstanced(buffers.indexCount, 1, 0, 0, i);
        }
    }

    void MeshRenderer::UpdateConstantBuffers(const Camera* camera)
    {
        // Scene params
        mSceneParams.camPos = camera->GetPosition();
        mSceneParams.ViewProj = XMMatrixTranspose(camera->GetViewProjection());
        mSceneParams.pointLightCount = mPointLights.size();
        mGM->UpdateBuffer(mSceneConstantBuffer, &mSceneParams);

        // Spot light
        ShaderSpotLight shaderSpotLight;
        shaderSpotLight.pos = mSpotLight->position;
        shaderSpotLight.range = mSpotLight->range;
        shaderSpotLight.dir = mSpotLight->direction;
        shaderSpotLight.cone = mSpotLight->cone;
        shaderSpotLight.att = mSpotLight->attenuation;
        shaderSpotLight.pad = 0;
        const auto& d = mSpotLight->diffuse;
        shaderSpotLight.diffuse = { d.x, d.y, d.z, 1 };
        const auto& s = mSpotLight->specular;
        shaderSpotLight.specular = { s.x, s.y, s.z, 1 };
        mGM->UpdateBuffer(mSpotLightConstantBuffer, &shaderSpotLight);
    }

    void MeshRenderer::UpdateStructuredBuffers()
    {
        if (mPointLights.empty())
        {
            return;
        }
        std::vector<ShaderPointLight> lights(MaxPointLightsAllowed);
        for (int i = 0; i < mPointLights.size(); ++i)
        {
            lights[i].pos = mPointLights[i]->position;
            lights[i].range = mPointLights[i]->range;
            lights[i].att = mPointLights[i]->attenuation;
            lights[i].pad = 0;
            const auto& d = mPointLights[i]->diffuse;
            lights[i].diffuse = { d.x, d.y, d.z, 1 };
            const auto& s = mPointLights[i]->specular;
            lights[i].specular = { s.x, s.y, s.z, 1 };
        }
        mGM->UpdateBuffer(mPointLightStructuredBuffer.buffer, lights.data());
    }

    void MeshRenderer::UpdateMeshInstanceBuffers()
    {
        auto iter = mMeshTypeEntitiesMap.begin();
        while (iter != mMeshTypeEntitiesMap.end())
        {
            // Deal with case where entities were removed. Note I had no time to add remove entity functions
            if (iter->second.empty())
            {
                mMeshTypeDataMap[iter->first].Release();
                mMeshTypeDataMap.erase(iter->first);
                iter = mMeshTypeEntitiesMap.erase(iter);
                continue;
            }
            UpdateMeshInstanceBuffer(iter->first);
            ++iter;
        }
    }

    // ** Update the instance buffers with the new world transforms */
    void MeshRenderer::UpdateMeshInstanceBuffer(MeshType meshType)
    {
        const auto& entities = mMeshTypeEntitiesMap[meshType];
        std::vector<MeshInstanceData> instanceData(entities.size());
        for (int i = 0; i < entities.size(); ++i)
        {
            const auto& e = entities[i];
            auto& data = instanceData[i];
            // Change from degrees to radians
            float angleInRad = e->rotation.w * Math::DegToRad;
            auto rotation = XMQuaternionRotationAxis(FV({ e->rotation.x, e->rotation.y, e->rotation.z }), angleInRad);
            auto world = XMMatrixScaling(e->scale.x, e->scale.y, e->scale.z) * XMMatrixRotationQuaternion(rotation) * 
                XMMatrixTranslation(e->position.x, e->position.y, e->position.z);
            // Rows and columns need to be swapped for the shader
            data.world = XMMatrixTranspose(world);
        }
        auto& meshBuffers = mMeshTypeDataMap[meshType];
        mGM->UpdateBuffer(meshBuffers.instanceBuffer, instanceData.data());
    }

    void MeshRenderer::LoadShaders()
    {
        // Base vertex shader
        {
            D3D11_INPUT_ELEMENT_DESC vertexLayout[] =
            {
                // Data for the vertex buffer
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL",	 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },

                // Data for the instance buffer
                { "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
            };

            UINT numElements = ARRAYSIZE(vertexLayout);

            ID3D10Blob* shaderBuffer = nullptr;
            ID3D10Blob* errors = nullptr;
            HRESULT hr = D3DCompileFromFile(L"../../Renderer/Shaders/BaseVS.hlsl", 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &shaderBuffer, &errors);
            if (FAILED(hr))
            {
                MessageBox(NULL, L"D3DCompileFromFile failed with BaseVS.hlsl.",
                    L"Error", MB_OK | MB_ICONERROR);
                SAFE_RELEASE(shaderBuffer);
                return;
            }
            hr = mGM->mDevice->CreateVertexShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, &mBaseVertexShader);
            hr = mGM->mDevice->CreateInputLayout(vertexLayout, numElements, shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), &mBaseVertexLayout); 
            if (FAILED(hr))
            {
                MessageBox(NULL, L"CreateInputLayout failed for BaseVS.hlsl.",
                    L"Error", MB_OK | MB_ICONERROR);
                SAFE_RELEASE(shaderBuffer);
                return;
            }
            SAFE_RELEASE(shaderBuffer);
        }

        // Base pixel shader
        {
            ID3D10Blob* shaderBuffer = nullptr;
            ID3D10Blob* errors = nullptr;
            HRESULT hr = D3DCompileFromFile(L"../../Renderer/Shaders/BasePS.hlsl", 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &shaderBuffer, &errors);
            if (FAILED(hr))
            {
                MessageBox(NULL, L"D3DCompileFromFile failed with BasePS.hlsl.",
                    L"Error", MB_OK | MB_ICONERROR);
                SAFE_RELEASE(shaderBuffer);
                return;
            }
            hr = mGM->mDevice->CreatePixelShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, &mBasePixelShader);
            SAFE_RELEASE(shaderBuffer)
        }
    }

    void MeshRenderer::CreateConstantBuffers()
    {
        // Scene Params
        {
            D3D11_BUFFER_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.ByteWidth = sizeof(ShaderSceneParams);
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;

            mSceneConstantBuffer = mGM->CreateBuffer(desc, &mSceneParams);
        }

        // Spot Light
        {
            // Create with no data originally because the spot light is not set yet
            D3D11_BUFFER_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
            // Using dynamic because faster to update every frame to reflect user changes
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.ByteWidth = sizeof(ShaderSpotLight);
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;

            mSpotLightConstantBuffer = mGM->CreateBuffer(desc);
        }

        // Material
        {
            // Create with no data originally because the material is not set yet
            D3D11_BUFFER_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.ByteWidth = sizeof(Material);
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;

            mMaterialConstantBuffer = mGM->CreateBuffer(desc);
        }
    }

    void MeshRenderer::CreateStructuredBuffers()
    {
        // Point Lights
        D3D11_BUFFER_DESC desc;
        ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = sizeof(ShaderPointLight) * MaxPointLightsAllowed;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.StructureByteStride = sizeof(ShaderPointLight);

        // Buffer of data can be empty until used
        mPointLightStructuredBuffer.buffer = mGM->CreateBuffer(desc);

        if (!mPointLightStructuredBuffer.buffer)
        {
            return; 
        }

        D3D11_BUFFER_SRV srvBuffer;
        srvBuffer.FirstElement = 0;
        // Memory will be allocated for this number of elements but there could be less used
        srvBuffer.NumElements = MaxPointLightsAllowed;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer = srvBuffer;

        mGM->mDevice->CreateShaderResourceView(mPointLightStructuredBuffer.buffer, &srvDesc, &mPointLightStructuredBuffer.shaderResourceView);
    }

    void MeshRenderer::CreateMeshBuffers(MeshType meshType)
    {
        if (mMeshTypeDataMap.find(meshType) == mMeshTypeDataMap.end())
        {
            mMeshTypeDataMap.emplace(meshType, MeshBuffers());

            MeshBuffers& buffers = mMeshTypeDataMap[meshType];

            const Vertex* vertices = nullptr; 
            const DWORD* indices = nullptr;

            // i had no time to add more primitives so just cube but renderer can support any primtive
            if (meshType == MeshType::Cube)
            {
                buffers.vertexCount = Cube::numVertices;
                buffers.indexCount = Cube::numIndices;
                vertices = Cube::vertices;
                indices = Cube::indices;
            }
            else
            {
                return;
            }

            {
                D3D11_BUFFER_DESC desc;
                ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.ByteWidth = sizeof(Vertex) * buffers.vertexCount;
                desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                desc.CPUAccessFlags = 0;
                desc.MiscFlags = 0;

                buffers.vertexBuffer = mGM->CreateBuffer(desc, vertices);
            }
            {
                D3D11_BUFFER_DESC desc;
                ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.ByteWidth = sizeof(DWORD) * buffers.indexCount;
                desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                desc.CPUAccessFlags = 0;
                desc.MiscFlags = 0;

                buffers.indexBuffer = mGM->CreateBuffer(desc, indices);
            }
        }

        MeshBuffers& buffers = mMeshTypeDataMap[meshType];
        
        SAFE_RELEASE(buffers.instanceBuffer);
        
        // The instance buffer is recreated to reflect the current number of entities in the scene
        {
            D3D11_BUFFER_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.ByteWidth = sizeof(MeshInstanceData) * mMeshTypeEntitiesMap[meshType].size();
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;

            // Data is set on the update every frame because entity world transform may change
            buffers.instanceBuffer = mGM->CreateBuffer(desc);
        }
    }

    void MeshRenderer::DeleteMeshBuffers()
    {
        for (auto& keyValue : mMeshTypeDataMap)
        {
            keyValue.second.Release();
        }
        mMeshTypeDataMap.clear();
    }

    void MeshRenderer::AddSpotLight(const std::shared_ptr<SpotLight>& spotLight)
    {
        // Currently just supporting one as per the assignment
        mSpotLight = spotLight;
    }

    void MeshRenderer::AddPointLight(const std::shared_ptr<PointLight>& pointLight)
    {
        if (mPointLights.size() == MaxPointLightsAllowed)
        {
            mPointLights[0] = pointLight;
        }
        else
        {
            mPointLights.push_back(pointLight);
        }   
    }

    void MeshRenderer::AddEntity(const std::shared_ptr<Entity>& entity)
    {
        // Entities are recorded by mesh type because vertex instancing is being used to render them
        if (mMeshTypeEntitiesMap.find(entity->meshType) == mMeshTypeEntitiesMap.end())
        {
            mMeshTypeEntitiesMap.emplace(entity->meshType, std::vector<std::shared_ptr<Entity>>());
        }
        mMeshTypeEntitiesMap[entity->meshType].push_back(entity);
        mInstanceBuffersToCreate.insert(entity->meshType);
    }
}