#include "Renderer.h"
#include "GraphicsManager.h"
#include "MeshRenderer.h"
#include "Camera/Camera.h"

namespace renderer
{
    Renderer* Renderer::mRenderer = nullptr;

    Renderer* Renderer::Initialize(HWND windowHandle, const GraphicsConfig& config)
    {
        if (!mRenderer)
        {
            mRenderer = new Renderer(windowHandle, config);
        }
        return mRenderer;
    }

    Renderer::Renderer(HWND windowHandle, const GraphicsConfig& config)
    {
        mGM = GraphicsManager::Initialize(windowHandle, config);
        mMR = MeshRenderer::Initialize(mGM);

        mCamera = new Camera(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 1, 0), XMFLOAT3(0, 0, 100), 0.4f * 3.14f, config.screenWidth, config.screenHeight, 0.01f, 2000.0f);
    }

    Renderer::~Renderer()
    {
        SAFE_DELETE(mMR);
        SAFE_DELETE(mGM);
        SAFE_DELETE(mCamera);
        mRenderer = nullptr;
    }

    void Renderer::Render(double frameTime)
    {
        mMR->Render(frameTime, mCamera);
    }

    void Renderer::AddSpotLight(const std::shared_ptr<SpotLight>& spotLight)
    {
        mMR->AddSpotLight(spotLight);
    }

    void Renderer::AddPointLight(const std::shared_ptr<PointLight>& pointLight)
    {
        mMR->AddPointLight(pointLight);
    }

    void Renderer::AddEntity(const std::shared_ptr<Entity>& entity)
    {
        mMR->AddEntity(entity);
    }

    Camera* Renderer::GetCamera() const
    {
        return mCamera;
    }
}