#include "Camera.h"

using namespace DirectX;

namespace renderer
{
	Camera::Camera()
	{
		//Set default values.
		mPosition = XMFLOAT3(0, 0, 0);
		mUp = XMFLOAT3(0, 1, 0);
		mTarget = XMFLOAT3(0, 0, 100);
		mFov = 0.4f * 3.14f;
		mWidth = 1920;
		mHeight = 1080;
		mNearZ = 1;
		mFarZ = 4000;
		mWidth = 1920;
		mHeight = 1080;
	}

	Camera::Camera(XMFLOAT3 position, XMFLOAT3 up, XMFLOAT3 target, float fov, float width, float height, float nearZ, float farZ)
		: mPosition(position), mUp(up), mTarget(target), mFov(fov), mWidth(width), mHeight(height), mNearZ(nearZ), mFarZ(farZ)
	{

	}

	Camera::~Camera()
	{

	}

	void Camera::SetYaw(float yaw)
	{
		XMVECTOR vPosition = FV(mPosition);
		XMVECTOR forwardLength = FV(mTarget) - vPosition;
		XMVECTOR rot = XMQuaternionRotationAxis(FV(XMFLOAT3(0, 1, 0)), yaw);
		mUp = VF3(XMVector3Rotate(FV(mUp), rot));
		mTarget = VF3(XMVector3Rotate(forwardLength, rot) + vPosition);
	}

	void Camera::SetPitch(float pitch)
	{
		XMVECTOR vPosition = FV(mPosition);
		XMVECTOR forwardLength = FV(mTarget) - vPosition;
		XMVECTOR forward = XMVector3Normalize(forwardLength);
		XMVECTOR vUp = FV(mUp);
		XMVECTOR right = XMVector3Normalize(XMVector3Cross(forward, vUp));
		XMVECTOR rot = XMQuaternionRotationAxis(right, pitch);
		mUp = VF3(XMVector3Rotate(vUp, rot));
		mTarget = VF3(XMVector3Rotate(forwardLength, rot) + vPosition);
	}

	void Camera::MoveForwardBack(float amount)
	{
		XMVECTOR vPosition = FV(mPosition);
		XMVECTOR vTarget = FV(mTarget);
		XMVECTOR forward = XMVector3Normalize(vTarget - vPosition);
		vPosition = vPosition + forward * amount;
		mPosition = VF3(vPosition);
		mTarget = VF3(vPosition + forward * 100);
	}

	void Camera::MoveRightLeft(float amount)
	{
		XMVECTOR vPosition = FV(mPosition);
		XMVECTOR vTarget = FV(mTarget);
		XMVECTOR forward = XMVector3Normalize(vTarget - vPosition);
		XMVECTOR right = XMVector3Normalize(XMVector3Cross(FV(mUp), forward));
		vPosition = vPosition + right * amount;
		mPosition = VF3(vPosition);
		mTarget = VF3(vPosition + forward * 100);
	}

	void Camera::MoveUpDown(float amount)
	{
		XMVECTOR vPosition = FV(mPosition);
		XMVECTOR vTarget = FV(mTarget);
		XMVECTOR forward = XMVector3Normalize(vTarget - vPosition);
		vPosition = vPosition + FV(mUp) * amount;
		mPosition = VF3(vPosition);
		mTarget = VF3(vPosition + forward * 100);
	}

	void Camera::SetPosition(XMFLOAT3 position)
	{
		XMVECTOR vOldPosition = FV(mPosition);
		XMVECTOR vPosition = FV(position);
		XMVECTOR vTarget = FV(mTarget);
		XMVECTOR forward = XMVector3Normalize(vTarget - vOldPosition);
		mPosition = position;
		mTarget = VF3(vPosition + forward * XMVector3Length(vTarget - vOldPosition));
	}

	XMFLOAT3 Camera::GetPosition() const
	{
		return mPosition;
	}

	void Camera::SetTarget(XMFLOAT3 target)
	{
		XMVECTOR vPosition = FV(mPosition);
		XMVECTOR vTarget = FV(target);
		// Ensure position and target are not the same
		if (XMVector3Equal(vPosition, vTarget))
		{
			return;
		}
		XMVECTOR currentForward = XMVector3Normalize(FV(mTarget) - vPosition);
		XMVECTOR newForward = XMVector3Normalize(vTarget - vPosition);

		float angle = XMConvertToDegrees(XMVectorGetX(XMVector3AngleBetweenNormals(currentForward, newForward)));

		XMVECTOR rot;
		if (angle != 0.0f && angle != 180.0f && angle != 360.0f)
		{
			XMVECTOR axis = XMVector3Normalize(XMVector3Cross(currentForward, newForward));
			rot = XMQuaternionRotationAxis(axis, XMConvertToRadians(angle));
			mUp = VF3(XMVector3Rotate(FV(mUp), rot));
		}
		mTarget = target;
	}

	XMFLOAT3 Camera::GetTarget() const
	{
		return mTarget;
	}

	void Camera::SetUp(XMFLOAT3 up)
	{
		XMVECTOR currentUp = FV(mUp);
		XMVECTOR newUp = FV(up);

		float angle = XMConvertToDegrees(XMVectorGetX(XMVector3AngleBetweenNormals(currentUp, newUp)));

		XMVECTOR rot;
		if (angle != 0.0f && angle != 180.0f && angle != 360.0f)
		{
			XMVECTOR axis = XMVector3Normalize(XMVector3Cross(currentUp, newUp));
			rot = XMQuaternionRotationAxis(axis, XMConvertToRadians(angle));
			mTarget = VF3(XMVector3Rotate(FV(mTarget), rot));
		}

		mUp = up;
	}

	XMFLOAT3 Camera::GetUp() const
	{
		return mUp;
	}

	XMVECTOR Camera::GetForward() const
	{
		return XMVector3Normalize(FV(mTarget) - FV(mPosition));
	}

	XMMATRIX Camera::GetView() const
	{
		return XMMatrixLookAtLH(FV(mPosition), FV(mTarget), FV(mUp));
	}

	XMMATRIX Camera::GetProjection() const
	{
		return XMMatrixPerspectiveFovLH(0.4f * 3.14f, mWidth / mHeight, mNearZ, mFarZ);
	}

	XMMATRIX Camera::GetViewProjection() const
	{
		return GetView() * GetProjection();
	}

	void Camera::SetFOV(float fov)
	{
		mFov = fov;
	}

	float Camera::GetFOV() const
	{
		return mFov;
	}

	void Camera::SetNearZ(float nearZ)
	{
		mNearZ = nearZ;
	}

	float Camera::GetNearZ() const
	{
		return mNearZ;
	}

	void Camera::SetFarZ(float farZ)
	{
		mFarZ = farZ;
	}

	float Camera::GetFarZ() const
	{
		return mFarZ;
	}

	void Camera::SetWidth(float width)
	{
		mWidth = width;
	}

	float Camera::GetWidth() const
	{
		return mWidth;
	}

	void Camera::SetHeight(float height)
	{
		mHeight = height;
	}

	float Camera::GetHeight() const
	{
		return mHeight;
	}
}

