#pragma once

#include "Minimal.h"

using namespace DirectX;

namespace renderer
{
	/** Allows for movement of the camera in the scene and calculates view projection matrix */
	class Camera
	{
	public:
		Camera();
		Camera(XMFLOAT3 position, XMFLOAT3 up, XMFLOAT3 target, float fov, float width, float height, float nearZ, float farZ);
		~Camera();
		void SetPitch(float p);
		void SetYaw(float y);
		void MoveForwardBack(float amount);
		void MoveRightLeft(float amount);
		void MoveUpDown(float amount);
		void SetPosition(XMFLOAT3 newPos);
		XMFLOAT3 GetPosition() const;
		void SetTarget(XMFLOAT3 newTarget);
		DirectX::XMFLOAT3 GetTarget() const;
		void SetUp(XMFLOAT3 newUp);
		XMFLOAT3 GetUp() const;
		XMVECTOR GetForward() const;
		XMMATRIX GetView() const;
		XMMATRIX GetProjection() const;
		XMMATRIX GetViewProjection() const;
		void SetFOV(float fov);
		float GetFOV() const;
		void SetNearZ(float nearZ);
		float GetNearZ() const;
		void SetFarZ(float farZ);
		float GetFarZ() const;
		void SetWidth(float width);
		float GetWidth() const;
		void SetHeight(float height);
		float GetHeight() const;

	private:
		XMFLOAT3 mPosition;
		XMFLOAT3 mTarget;
		XMFLOAT3 mUp;
		float mWidth;
		float mHeight;
		float mFov;
		float mNearZ;
		float mFarZ;
	};
}