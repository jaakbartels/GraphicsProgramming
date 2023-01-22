#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}


		Vector3 origin{0.f,0.f,-50.f};
		float fovAngle{ 45.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float nearPlane{ .1f };
		float farPlane{ 100.f };
		float aspect{ 1.f };

		float oldAspect{};
		float oldFov{  };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix;


		inline bool ShouldVertexBeClipped(const Vector4& v) const
		{
			return v.x < -1.f || v.x > 1.f || v.y < -1.f || v.y > 1.f;
		}


		void Initialize(float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f })
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;

			CalculateProjectionMatrix();
		}

		void CalculateViewMatrix()
		{

			viewMatrix = Matrix::CreateLookAtLH(origin, forward, up);
			invViewMatrix = Matrix::Inverse(viewMatrix);
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W2

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspect, nearPlane, farPlane);
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(const Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			float movementSpeed{ 20  };
			float mouseSpeed{ 60  };
			//Keyboard Input
			//const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			const Uint8* pStates{ SDL_GetKeyboardState(nullptr) };
			if (pStates[SDL_SCANCODE_LSHIFT])
			{
				movementSpeed *= 3;
			}
			if (pStates[SDL_SCANCODE_W] || pStates[SDL_SCANCODE_UP])
			{
				origin.z += movementSpeed * deltaTime;
			}
			else if (pStates[SDL_SCANCODE_S] || pStates[SDL_SCANCODE_DOWN])
			{
				origin.z -= movementSpeed * deltaTime;
			}
			else if (pStates[SDL_SCANCODE_D] || pStates[SDL_SCANCODE_RIGHT])
			{
				origin.x += movementSpeed * deltaTime;
			}
			else if (pStates[SDL_SCANCODE_A] || pStates[SDL_SCANCODE_LEFT])
			{
				origin.x -= movementSpeed * deltaTime;
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			const float rotationSpeed{ 4.f };

			if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)))
			{
				origin.y += -mouseY * mouseSpeed * deltaTime;
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				totalPitch += -mouseY * rotationSpeed * deltaTime;
				totalYaw += mouseX * rotationSpeed * deltaTime;
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				totalYaw += mouseX * rotationSpeed * deltaTime;

				origin.z += -mouseY * mouseSpeed * deltaTime;
			}

			const Matrix finalRotation{ Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw) };
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			//Update Matrices
			CalculateViewMatrix();

			if (oldAspect != aspect || oldFov != fov)
			{
				oldAspect = aspect;
				oldFov = fov;
				CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
			}
		}
		const Matrix& GetViewMatrix() const
		{

			return viewMatrix;
		}

		const Matrix& GetInvViewMatrix() const
		{

			return invViewMatrix;
		}

		const Matrix& GetProjectionMatrix() const
		{

			return projectionMatrix;
		}

		Matrix GetWorldViewProjection() const
		{
			return GetViewMatrix() * GetProjectionMatrix();
		}
	};
}
