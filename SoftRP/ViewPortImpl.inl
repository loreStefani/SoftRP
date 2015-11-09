#ifndef SOFTRP_VIEWPORT_IMPL_INL_
#define SOFTRP_VIEWPORT_IMPL_INL_
#include "ViewPort.h"
namespace SoftRP {

	inline ViewPort::ViewPort(unsigned int width, unsigned int height, unsigned int x, unsigned int y, float minZ, float maxZ)
		: m_width{ width }, m_height{ height }, m_x{ x }, m_y{ y }, m_minZ{ minZ }, m_maxZ{ maxZ } {

		if (width == 0 || height == 0)
			throw std::runtime_error{ "Invalid size." };

		updateTransform();
	}

	inline unsigned int ViewPort::getX() const { return m_x; }
	inline unsigned int ViewPort::getY() const { return m_y; }
	inline unsigned int ViewPort::getWidth()const { return m_width; }
	inline unsigned int ViewPort::getHeight()const { return m_height; }
	inline float ViewPort::getMinZ() const { return m_minZ; }
	inline float ViewPort::getMaxZ() const { return m_maxZ; }

	inline const Math::Matrix4& ViewPort::getTransform() { if (m_needsUpdate) updateTransform(); return m_transform; }
	inline const Math::Matrix4& ViewPort::getTransform() const { return m_transform; }

	inline void ViewPort::setX(const unsigned int x) { m_x = x; m_needsUpdate = true; }
	inline void ViewPort::setY(const unsigned int y) { m_y = y; m_needsUpdate = true; }
	inline void ViewPort::setWidth(const unsigned int width) { m_width = width; m_needsUpdate = true; }
	inline void ViewPort::setHeight(const unsigned int height) { m_height = height; m_needsUpdate = true; }
	inline void ViewPort::setMinZ(const float minZ) { m_minZ = minZ; m_needsUpdate = true; }
	inline void ViewPort::setMaxZ(const float maxZ) { m_maxZ = maxZ; m_needsUpdate = true; }

	inline void ViewPort::updateTransform() {

		/*
		Transform the point p_ndc (NDC space) in p_s (Screen space).
		Map [-1, 1]^3 in [x, x + width]*[y + height, y]*[minZ, maxZ]

		Let:
		p_ndc = (x_ndc, y_ndc, z_ndc)
		p_s = (x_s, y_s, z_s)
		x = m_x; y = m_y;
		w = m_width; h = m_height;
		maxZ = m_maxZ; minZ = m_minZ

		then:
		x_s = (w/2)*x_ndc + x + (w/2)
		y_s = -(h/2)*y_ndc + y + (h/2)
		z_s = [(maxZ - minZ)/2]*z_ndc + (minZ + maxZ)/2
		*/

		const float halfWidth = static_cast<float>(m_width) / 2.0f;
		const float halfHeight = static_cast<float>(m_height) / 2.0f;

		m_transform[0] = halfWidth;
		m_transform[3] = static_cast<float>(m_x) + halfWidth;

		m_transform[5] = -halfHeight;
		m_transform[7] = static_cast<float>(m_y) + halfHeight;

		m_transform[10] = (m_maxZ - m_minZ) / 2.0f;
		m_transform[11] = (m_maxZ + m_minZ) / 2.0f;

		m_needsUpdate = false;
	}

	inline ViewPort::ViewPort(const ViewPort& viewPort) {
		copy(viewPort);
	}

	inline ViewPort& ViewPort::operator=(const ViewPort& viewPort) {
		copy(viewPort);
		return *this;
	}

	inline void ViewPort::copy(const ViewPort& viewPort) {
		m_x = viewPort.m_x;
		m_y = viewPort.m_y;
		m_width = viewPort.m_width;
		m_height = viewPort.m_height;
		m_minZ = viewPort.m_minZ;
		m_maxZ = viewPort.m_maxZ;
		m_needsUpdate = viewPort.m_needsUpdate;

		//avoid copy overhead if the transform needs to be computed
		if (!m_needsUpdate)
			m_transform = viewPort.m_transform;
	}
}
#endif
