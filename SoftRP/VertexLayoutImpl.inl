#ifndef SOFTRP_VERTEX_LAYOUT_IMPL_INL_
#define SOFTRP_VERTEX_LAYOUT_IMPL_INL_
#include "VertexLayout.h"
#include<utility>
namespace SoftRP {

	/*  VertexLayout implementation  */

	inline VertexLayout::VertexLayout(const std::vector<size_t>& fieldsSizes) {
		m_vertexStride = 4;
		m_vertexFields.push_back(VertexField{ 0, m_vertexStride }); //position		
		for (size_t s : fieldsSizes) {
			if (s == 0)
				continue;
			m_vertexFields.push_back(VertexField{ m_vertexStride, s });
			m_vertexStride += s;
		}
	}

	inline float* VertexLayout::getVertexData(float* data, size_t vertexIndex) const {
		return data + vertexIndex*m_vertexStride;
	}

	inline float* VertexLayout::getVertexPosition(float* data, size_t vertexIndex)const {
		return getVertexData(data, vertexIndex);
	}

	inline float* VertexLayout::getVertexFieldData(float* data, size_t vertexIndex, size_t vertexFieldIndex)const {
		const size_t offset = m_vertexFields[vertexFieldIndex].offset;
		return getVertexData(data, vertexIndex) + offset;
	}

	inline float* VertexLayout::getVertexFieldData(float* vertexData, size_t vertexFieldIndex) const {
		const size_t offset = m_vertexFields[vertexFieldIndex].offset;
		return vertexData + offset;
	}

	inline float* VertexLayout::getVertexPosition(float* vertexData)const {
		return vertexData;
	}

	inline const float* VertexLayout::getVertexData(const float* data, size_t vertexIndex) const {
		return data + vertexIndex*m_vertexStride;
	}

	inline const float* VertexLayout::getVertexPosition(const float* data, size_t vertexIndex)const {
		return getVertexData(data, vertexIndex);
	}

	inline const float* VertexLayout::getVertexFieldData(const float* data, size_t vertexIndex, size_t vertexFieldIndex)const {
		const size_t offset = m_vertexFields[vertexFieldIndex].offset;
		return getVertexData(data, vertexIndex) + offset;
	}

	inline const float* VertexLayout::getVertexFieldData(const float* vertexData, size_t vertexFieldIndex) const {
		const size_t offset = m_vertexFields[vertexFieldIndex].offset;
		return vertexData + offset;
	}

	inline const float* VertexLayout::getVertexPosition(const float* vertexData)const { return vertexData; }

	inline size_t VertexLayout::fieldCount() const { return m_vertexFields.size(); }

	inline size_t VertexLayout::vertexStride() const { return m_vertexStride; }

	/*  AllocatorVertexLayout implementaion  */

	template<template<typename T>typename A>
	inline AllocatorVertexLayout<A>::AllocatorVertexLayout(const std::vector<size_t>& fieldsSizes, size_t vertexAlignment)
		: VertexLayout{ fieldsSizes }, m_allocator{ vertexStride(), vertexAlignment } {}

	template<template<typename T>typename A>
	inline float* AllocatorVertexLayout<A>::allocateVertex() { return m_allocator.allocate(); }

	template<template<typename T>typename A>
	inline float* AllocatorVertexLayout<A>::allocateVertexArray(size_t count) { return m_allocator.allocateArray(count); }

	template<template<typename T>typename A>
	inline void AllocatorVertexLayout<A>::deallocateVertex(float* vertex) { m_allocator.deallocate(vertex); }

	template<template<typename T>typename A>
	inline void AllocatorVertexLayout<A>::deallocateVertexArray(float* vertexArray) { m_allocator.deallocateArray(vertexArray); }

	/*  InputVertexLayout implementation  */

	inline InputVertexLayout::InputVertexLayout(const std::vector<size_t>& fieldsSizes)
		: AllocatorVertexLayout{ fieldsSizes, 1 } {}

	inline InputVertexLayout InputVertexLayout::create(const std::vector<size_t>& fieldsSizes) {
		return InputVertexLayout{ fieldsSizes };
	}

	/*  OutputVertexLayout implementation  */

	inline OutputVertexLayout::OutputVertexLayout(const std::vector<size_t>& fieldsSizes)
		: AllocatorVertexLayout{ fieldsSizes, 16 } {}

	inline OutputVertexLayout OutputVertexLayout::create(const size_t fieldsCount) {
		std::vector<size_t> vertexFields{};
		if (fieldsCount > 0) {
			vertexFields.resize(fieldsCount);
			for (size_t i = 0; i < fieldsCount; i++)
				vertexFields[i] = 4;
		}
		return OutputVertexLayout{ vertexFields };
	}

}
#endif