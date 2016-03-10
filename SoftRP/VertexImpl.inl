#ifndef SOFTRP_VERTEX_IMPL_INL_
#define SOFTRP_VERTEX_IMPL_INL_
#include "Vertex.h"
#include <cassert>
namespace SoftRP {

	inline Vertex::Vertex(float* vertexData, VertexLayout* vertexLayout) {
		setVertexData(vertexData, vertexLayout);
	}

	inline Vertex::~Vertex() {
		clean();
	}

	inline Vertex::Vertex(const Vertex& v) {
		copy(v);
	}

	inline Vertex& Vertex::operator=(const Vertex& v) {
		copy(v);
		return *this;
	}

	inline Vertex::Vertex(Vertex&& v) {
		move(std::move(v));
	}

	inline Vertex& Vertex::operator=(Vertex&& v) {
		move(std::move(v));
		return *this;
	}

	inline void Vertex::scale(float s) {
		scaleVertexData(m_data, s, 4);
		scaleVertexData(s);
	}

	inline void Vertex::add(const Vertex& v) {
		addVertexData(m_data, v.m_data, 4);
		addVertexData(v);
	}

	inline void Vertex::lerp(float t, const Vertex& v) {
		lerpVertexData(m_data, t, v.m_data, 4);
		lerpVertexData(t, v);
	}

	inline void Vertex::scaleVertexData(float s) {
		scaleVertexData(m_data + 4, s, m_count - 4);
	}

	inline void Vertex::addVertexData(const Vertex& v) {
		assert(m_count == v.m_count);
		addVertexData(m_data + 4, v.m_data + 4, m_count - 4);
	}

	inline void Vertex::lerpVertexData(float t, const Vertex& v) {
		assert(m_count == v.m_count);
		lerpVertexData(m_data + 4, t, v.m_data + 4, m_count - 4);
	}

	inline Math::Vector4& Vertex::position() {
		return *Math::vectorFromPtr<4>(m_data);
	}

	inline const Math::Vector4& Vertex::position()const {
		return *Math::vectorFromPtr<4>(m_data);
	}

	inline const float* Vertex::getField(size_t vertexFieldIndex)const {
		assert(m_vertexLayout != nullptr);
		return m_vertexLayout->getVertexFieldData(m_data, vertexFieldIndex);
	}

	inline float* Vertex::getField(size_t vertexFieldIndex) {
		assert(m_vertexLayout != nullptr);
		return m_vertexLayout->getVertexFieldData(m_data, vertexFieldIndex);
	}

	inline void Vertex::setVertexData(float* vertexData, VertexLayout* vertexLayout) {
		clean();
		if (vertexData) {
			assert(vertexLayout != nullptr);
			m_data = vertexData;
			m_count = vertexLayout->vertexStride();
			m_own = false;
			m_vertexLayout = vertexLayout;
		}
	}

	inline float* Vertex::vertexData() {
		return m_data;
	}

	inline const float* Vertex::vertexData() const {
		return m_data;
	}

	inline size_t Vertex::fieldCount()const {
		return m_vertexLayout->fieldCount();
	}

	inline VertexLayout& Vertex::vertexLayout() const {
		return *m_vertexLayout;
	}

	inline void Vertex::move(Vertex&& v) {
		clean();
		m_vertexLayout = v.m_vertexLayout;
		m_data = v.m_data;
		m_own = v.m_own;
		v.m_data = nullptr;
		v.m_own = false;
		m_count = v.m_count;
		v.m_count = 0;
	}

	inline void Vertex::copy(const Vertex& v) {

		if (v.m_data) {
			m_vertexLayout = v.m_vertexLayout;
			if (!m_own || m_count != v.m_count) {
				clean();
				m_count = v.m_count;
				m_data = m_vertexLayout->allocateVertex();
			}
			m_own = true;
			for (size_t i = 0; i < m_count; i++)
				m_data[i] = v.m_data[i];
		} else {
			clean();
			m_own = false;
			m_data = nullptr;
			m_count = 0;
			m_vertexLayout = nullptr;
		}
	}

	inline void Vertex::clean() {
		if (m_own && m_data) {
			m_vertexLayout->deallocateVertex(m_data);
			m_own = false;
			m_data = nullptr;
			m_vertexLayout = nullptr;
		}
	}

	inline void Vertex::scaleVertexData(float* dest, float s, size_t size) {
		assert(dest != nullptr);
		for (size_t i = 0; i < size; i++)
			dest[i] *= s;
	}

	inline void Vertex::addVertexData(float* dest, const float* src, size_t size) {
		assert(dest != nullptr && src != nullptr);
		for (size_t i = 0; i < size; i++)
			dest[i] += src[i];
	}

	inline void Vertex::lerpVertexData(float* dest, float t, const float* src, size_t size) {
		assert(dest != nullptr && src != nullptr);
		const float oneMinusT = (1.0f - t);
		for (size_t i = 0; i < size; i++) {
			dest[i] *= oneMinusT;
			dest[i] += src[i] * t;
		}
	}

}
#endif
