#include "PrimitivesApp.h"
#include "Camera/Camera.h"
#include "Rendering/Renderer.h"
#include "Rendering/DataTypes.h"

using namespace renderer;

PrimitivesApp* PrimitivesApp::mApp = nullptr;

PrimitivesApp* PrimitivesApp::CreateOrGetExisting(HINSTANCE hInstance, HWND windowHandle, int screenWidth, int screenHeight)
{
	if (!mApp)
	{
		mApp = new PrimitivesApp(hInstance, windowHandle, screenWidth, screenHeight);
		if (!mApp->mInitialized)
		{
			SAFE_DELETE(mApp);
		}
	}
	return mApp;
}

PrimitivesApp::PrimitivesApp(HINSTANCE hInstance, HWND windowHandle, int screenWidth, int screenHeight)
	: mScreenWidth(screenWidth), mScreenHeight(screenHeight)
{
	mRunTime = 0;
	mFrameTime = 0;
	mMouseX = 0;
	mMouseY = 0;
	mInitialized = false;

	// Determine current performance-counter frequency of the system and the start time
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	mCountsPerSecond = double(frequencyCount.QuadPart);

	QueryPerformanceCounter(&frequencyCount);
	mStartTime = frequencyCount.QuadPart;

	if (!InitializeInput(hInstance, windowHandle))
	{
		MessageBox(NULL, L"PrimitivesApp::PrimitivesApp() - Failed to initialize input devices.",
			L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	GraphicsConfig config;
	config.screenHeight = screenWidth;
	config.screenHeight = screenHeight;
	auto renderer = Renderer::Initialize(windowHandle, config);
	mRenderer.reset(renderer);

	// Create example lights, materials and entities

	// Units are ins metres
	mSpotLight = std::make_shared<SpotLight>();
	mSpotLight->position = { 0, 0, 0 };
	mSpotLight->direction = VF3(XMVector3Normalize(FV({ 0, 0, 1 })));
	mSpotLight->range = 6;
	mSpotLight->cone = 0.8f;
	mSpotLight->attenuation = { 1.0f, 0.85f, 0.6f };
	// White as requested
	mSpotLight->diffuse = { 1, 1, 1 };
	mSpotLight->specular = { 1, 1, 1 };

	mRenderer->AddSpotLight(mSpotLight);

	uint32_t maxDist = 30;

	auto blueSpecMaterial = std::make_shared<Material>();
	blueSpecMaterial->diffuse = { 0.5f, 0.5f, 0.4, 1 };
	blueSpecMaterial->specular = { 0.3, 0.3f, 0.3f };
	blueSpecMaterial->gloss = 1.0f;

	auto glossyMaterial = std::make_shared<Material>();
	glossyMaterial->diffuse = { 0, 0.7f, 0.7f, 1 };
	glossyMaterial->specular = { 0.7f, 0.7f, 0.8f };
	glossyMaterial->gloss = 1.0f;

	for (int i = 0; i < 50; ++i)
	{
		float px = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* maxDist;
		float py = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* maxDist;
		float pz = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* maxDist;
		float r =  (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * maxDist;

		if (i % 5 == 0)
		{
			// Add a green point in front of the entity
			auto pointLight = std::make_shared<PointLight>();
			pointLight->position = { px, py, pz - 2 };
			pointLight->range = 3;
			pointLight->attenuation = { 1.0f, 0.85f, 0.6f };
			pointLight->diffuse = { 0, 1, 0 };
			pointLight->specular = { 0, 0.6f, 0 };

			mRenderer->AddPointLight(pointLight);
		}

		auto entity = std::make_shared<Entity>();

		if (i % 2 == 0)
		{
			entity->material = blueSpecMaterial;
		}
		else
		{
			entity->material = glossyMaterial;
		}

		entity->meshType = MeshType::Cube;
		entity->scale = { 1, 1, 1 };
		entity->position = { px, py, pz };
		entity->rotation = { 0, 1, 0, 45.0f * r };
		
		mEntities.emplace_back(std::move(entity));
		mRenderer->AddEntity(mEntities[i]);
	}
	
	mInitialized = true;
}

PrimitivesApp::~PrimitivesApp()
{
	ShutdownInput();
	mRenderer.reset();
	mApp = nullptr;
}

bool PrimitivesApp::Update()
{
	CalculateCurrentTime();
	// Return if user hit an exit button
	if (!HandleInput())
	{
		return false;
	}

	// Move the spot light with the camera
	auto cam = mRenderer->GetCamera();
	mSpotLight->direction = VF3(cam->GetForward());
	mSpotLight->position = cam->GetPosition();
	
	// Rotate cubes
	for (auto& entity : mEntities)
	{
		entity->rotation.w += 1.0f;
		if (entity->rotation.w >= 360)
		{
			entity->rotation.w = 0;
		}
	}

	mRenderer->Render(mFrameTime);	

	return true;
}

void PrimitivesApp::CalculateCurrentTime()
{
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);
	double oldTime = mRunTime;
	mRunTime = double(current.QuadPart - mStartTime) / mCountsPerSecond;
	// Time since last frame in seconds
	mFrameTime = (mRunTime - oldTime);
}

bool PrimitivesApp::InitializeInput(HINSTANCE hInstance, HWND windowHandle)
{
	mInput = 0;
	mKeyboard = 0;
	mMouse = 0;

	// Create input device
	auto result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&mInput, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the direct input interface for the keyboard.
	result = mInput->CreateDevice(GUID_SysKeyboard, &mKeyboard, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Use predefined input format
	result = mKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}

	// Set the cooperative level of the keyboard to be at the foreground for this app.
	//result = mKeyboard->SetCooperativeLevel(windowHandle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	// Try acquire the keyboard.
	result = mKeyboard->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the input interface for the mouse.
	result = mInput->CreateDevice(GUID_SysMouse, &mMouse, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Use pre-defined data format.
	result = mMouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}

	// Set cooperative level of the mouse to allow sharing with other programs.
	//result = mMouse->SetCooperativeLevel(windowHandle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	// Try acquire the mouse.
	result = mMouse->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void PrimitivesApp::ShutdownInput()
{
	// Release mouse.
	if (mMouse)
	{
		mMouse->Unacquire();
		mMouse->Release();
		mMouse = 0;
	}

	// Release keyboard.
	if (mKeyboard)
	{
		mKeyboard->Unacquire();
		mKeyboard->Release();
		mKeyboard = 0;
	}

	// Release the direct input interface.
	if (mInput)
	{
		mInput->Release();
		mInput = 0;
	}

}

bool PrimitivesApp::HandleInput()
{
	Camera* camera = mRenderer->GetCamera();

	DIMOUSESTATE mouseCurrState;

	BYTE keyboardState[256];

	// Aquire keyboard, mouse and any gamepad from other applications for use.
	mKeyboard->Acquire();
	mMouse->Acquire();

	mMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

	static constexpr float MaxRotate = 6.28319f;

	mKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	// Determine how much to rotate the camera based on mouse movement
	float rotAmountX = (mouseCurrState.lX / (float)mScreenWidth) * MaxRotate;
	float rotAmountY = (mouseCurrState.lY / (float)mScreenHeight) * -MaxRotate;
	float moveAmount = 0.05f;

	mMouseX += mouseCurrState.lX;
	mMouseY += mouseCurrState.lY;

	camera->SetYaw(rotAmountX);
	camera->SetPitch(rotAmountY);

	if (keyboardState[DIK_W] & 0x80 || keyboardState[DIK_UP] & 0x80)
	{
		camera->MoveForwardBack(moveAmount);
	}
	if (keyboardState[DIK_S] & 0x80 || keyboardState[DIK_DOWN] & 0x80)
	{
		camera->MoveForwardBack(-moveAmount);
	}
	if (keyboardState[DIK_D] & 0x80 || keyboardState[DIK_RIGHT] & 0x80)
	{
		camera->MoveRightLeft(moveAmount);
	}
	if (keyboardState[DIK_A] & 0x80 || keyboardState[DIK_LEFT] & 0x80)
	{
		camera->MoveRightLeft(-moveAmount);
	}
	if (keyboardState[DIK_A] & 0x80 || keyboardState[DIK_LEFT] & 0x80)
	{
		camera->MoveRightLeft(-moveAmount);
	}
	// Turn spot light on or off
	// Added this for testing
	if (keyboardState[DIK_L] & 0x80)
	{
		mSpotLight->diffuse = { 1, 1, 1 };
		mSpotLight->specular = { 1, 1, 1 };
	}
	if (keyboardState[DIK_O] & 0x80)
	{
		mSpotLight->diffuse = { 0, 0, 0 };
		mSpotLight->specular = { 0, 0, 0 };
	}
	if (keyboardState[DIK_ESCAPE] & 0x80)
	{
		return false;
	}

	// Ensure saved mouse position doesn't go offscreen ( didn't end up needing it )
	if (mMouseX < mScreenWidth)
	{
		mMouseX = 0;
	}
	else if (mMouseX > mScreenWidth)
	{
		mMouseX = mScreenWidth;
	}
	if (mMouseY < mScreenHeight)
	{
		mMouseY = 0;
	}
	else if (mMouseY > mScreenHeight)
	{
		mMouseY = mScreenHeight;
	}

	return true;
}