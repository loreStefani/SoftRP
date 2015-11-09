#pragma once
#include "SRMath.h"
#include "Helpers.h"

namespace SoftRPDemo {
	
	class Camera {

		using Vector3 = SoftRP::Math::Vector3;
		using Matrix4 = SoftRP::Math::Matrix4;

	public:

		//ctor
		Camera() = default;
		//dtor
		~Camera() = default;
		//copy
		Camera(const Camera&) = default;
		Camera& operator=(const Camera&) = default;
		//move
		Camera(Camera&&) = default;
		Camera& operator=(Camera&&) = default;

		/* getters */
		Matrix4& projection();
		Matrix4& view();
		
		/* const getters */
		const Matrix4& projection() const;
		const Matrix4& view() const;
		Matrix4 projView() const;
		Vector3 position()const;

		//define view transform
		void lookAt(Vector3 eyePos, Vector3 target = Vector3{ 0.0f, 0.0f, 0.0f }, Vector3 up = Vector3{ 0.0f, 1.0f, 0.0 });
		//define perspective transform
		void makePerspective(float aspect, float fovY = PI*0.25f, float near = 0.01f, float far = 100.0f);
		void makePerspective(float left, float right, float bottom, float top, float near, float far);		

	private:
		Vector3 m_position{};
		Matrix4 m_view{};
		Matrix4 m_projection{};
	};
	
	inline void Camera::makePerspective(float aspect, float fovY, float near, float far) {
		float top = std::tan(fovY / 2.0f)*near;
		float right = aspect *top;
		makePerspective(-right, right, -top, top, near, far);
	}

	inline void Camera::makePerspective(float left, float right, float bottom, float top, float near, float far) {
		float doubleN = near*2.0f;
		float rightMinusLeft = right - left;
		float topMinusBottom = top - bottom;
		float farMinusNear = far - near;

		m_projection = Matrix4{
			doubleN / rightMinusLeft, 0, (right + left) / rightMinusLeft, 0,
			0, doubleN / topMinusBottom, (top + bottom) / topMinusBottom, 0,
			0, 0, (-far - near) / farMinusNear, -far*doubleN / farMinusNear,
			0, 0, -1, 0
		};
	}

	inline void Camera::lookAt(Vector3 eyePos, Vector3 target, Vector3 up) {
		m_position = eyePos;
		Vector3 look = (eyePos - target).normalize();		
		Vector3 right = cross(up, look).normalize();
		up = cross(look, right);

		m_view = Matrix4{
			right[0], right[1], right[2], -eyePos.dot(right),
			up[0], up[1], up[2], -eyePos.dot(up),
			look[0], look[1], look[2], -eyePos.dot(look),
			0, 0, 0, 1
		};
	}

	inline Camera::Matrix4& Camera::projection() {
		return m_projection;
	}

	inline Camera::Matrix4& Camera::view() {
		return m_view;
	}

	inline Camera::Matrix4 Camera::projView() const{
		return m_projection*m_view;
	}

	inline const Camera::Matrix4& Camera::projection() const{
		return m_projection;
	}

	inline const Camera::Matrix4& Camera::view() const{
		return m_view;
	}

	inline Camera::Vector3 Camera::position()const {
		return m_position;
	}


}
