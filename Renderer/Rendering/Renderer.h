#pragma once

#include "DataTypes.h"

namespace renderer
{
    class Renderer
    {
    public:
        static Renderer* Initialize(HWND windowHandle, const GraphicsConfig& config);
        ~Renderer();
        void Render(double frameTime);
        // Pass shared pointers for lights to allow app to change position and colour
        void AddSpotLight(const std::shared_ptr<SpotLight>& spotLight);
        void AddPointLight(const std::shared_ptr<PointLight>& pointLight);
        void AddEntity(const std::shared_ptr<Entity>& entity);
        class Camera* GetCamera() const;

    private:
        Renderer(HWND windowHandle, const GraphicsConfig& config);

        class GraphicsManager* mGM;
        class MeshRenderer* mMR;
        Camera* mCamera;

        std::vector<std::shared_ptr<Entity>> mEntities;

        static Renderer* mRenderer;
    };
}