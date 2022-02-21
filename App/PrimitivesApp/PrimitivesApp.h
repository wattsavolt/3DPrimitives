#pragma once

#include "Minimal.h"
#include "Input.h"

namespace renderer
{
    class Renderer;
    struct Entity;
    struct SpotLight;
}

class PrimitivesApp
{
public:
    static PrimitivesApp* CreateOrGetExisting(HINSTANCE hInstance, HWND windowHandle, int screenWidth, int screenHeight);
    ~PrimitivesApp();
    bool Update();

private:
    PrimitivesApp(HINSTANCE hInstance, HWND windowHandle, int screenWidth, int screenHeight);
    void CalculateCurrentTime();
    bool InitializeInput(HINSTANCE hInstance, HWND windowHandle);
    bool HandleInput();
    void ShutdownInput();

    bool mInitialized;

    __int64 mStartTime;
    double mCountsPerSecond;
    double mRunTime;
    double mFrameTime;

    std::unique_ptr<renderer::Renderer> mRenderer;

    IDirectInput8* mInput;
    IDirectInputDevice8* mKeyboard;
    IDirectInputDevice8* mMouse;

    int mScreenWidth;
    int mScreenHeight;
    int mMouseX;
    int mMouseY;

    std::shared_ptr<renderer::SpotLight> mSpotLight;
    std::vector<std::shared_ptr<renderer::Entity>> mEntities;

    static PrimitivesApp* mApp;
};