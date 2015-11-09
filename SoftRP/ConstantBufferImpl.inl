#ifndef SOFTRP_CONSTANT_BUFFER_IMPL_INL_
#define SOFTRP_CONSTANT_BUFFER_IMPL_INL_
#include "ConstantBuffer.h"
#include <utility>
namespace SoftRP {
	
	inline ConstantBuffer::ConstantBuffer(const std::vector<size_t>& fieldsSizes, size_t instanceCount)
		: Buffer{ computeSize(fieldsSizes)*instanceCount }, m_instanceCount{ instanceCount }, m_perInstanceSize{ computeSize(fieldsSizes) } {
		initFields(fieldsSizes);
	}

	inline ConstantBuffer::ConstantBuffer(const std::vector<size_t>& fieldsSizes, float* data, size_t instanceCount)
		: Buffer(data, computeSize(fieldsSizes)*instanceCount), m_instanceCount{ instanceCount }, m_perInstanceSize{ computeSize(fieldsSizes) } {
		initFields(fieldsSizes);
	}

	inline size_t ConstantBuffer::computeSize(const std::vector<size_t>& fieldsSizes) {
		size_t size = 0;
		for (size_t s : fieldsSizes)
			size += s;
		return size;
	}

	inline ConstantBuffer::ConstantBuffer(const ConstantBuffer& cBuff) : Buffer{ cBuff } {
		m_instanceCount = cBuff.m_instanceCount;
		m_perInstanceSize = cBuff.m_perInstanceSize;
		m_offsets = cBuff.m_offsets;
	}

	inline ConstantBuffer& ConstantBuffer::operator=(const ConstantBuffer& cBuff) {
		if (&cBuff == this)
			return *this;
		Buffer::operator=(cBuff);
		m_instanceCount = cBuff.m_instanceCount;
		m_perInstanceSize = cBuff.m_perInstanceSize;
		m_offsets = cBuff.m_offsets;
		return *this;
	}

	inline ConstantBuffer::ConstantBuffer(ConstantBuffer&& cBuff) : Buffer(std::forward<Buffer>(cBuff)) {
		m_offsets = std::move(cBuff.m_offsets);
		m_instanceCount = cBuff.m_instanceCount;
		m_perInstanceSize = cBuff.m_perInstanceSize;
	}

	inline ConstantBuffer& ConstantBuffer::operator=(ConstantBuffer&& cBuff) {
		if (&cBuff == this)
			return *this;
		Buffer::operator=(std::forward<Buffer>(cBuff));
		m_offsets = std::move(cBuff.m_offsets);
		m_instanceCount = cBuff.m_instanceCount;
		m_perInstanceSize = cBuff.m_perInstanceSize;
		return *this;
	}

	inline ConstantBuffer::ConstantBufferField ConstantBuffer::getField(size_t i, size_t instance) {
		const size_t offset = m_offsets[i];
		float* data = get() + offset + instance*m_perInstanceSize;
		return ConstantBufferField{ data };
	}

	inline const ConstantBuffer::ConstantBufferField ConstantBuffer::getField(size_t i, size_t instance) const {
		const size_t offset = m_offsets[i];
		const float* data = get() + offset + instance*m_perInstanceSize;
		return ConstantBufferField{ data };
	}

	inline size_t ConstantBuffer::fieldCount() const {
		return m_offsets.size();
	}

	inline size_t ConstantBuffer::perInstanceSize()const {
		return m_perInstanceSize;
	}

	inline void ConstantBuffer::initFields(const std::vector<size_t>& fieldsSizes) {
		const size_t fieldCount = fieldsSizes.size();
		if (fieldCount == 0)
			throw std::runtime_error{ "Invalid fields count!" };
		m_offsets.resize(fieldCount);
		size_t currSize = 0;
		for (size_t i = 0; i < fieldCount; i++) {
			m_offsets[i] = currSize;
			currSize += fieldsSizes[i];
		}
	}

	inline ConstantBuffer::ConstantBufferField::ConstantBufferField(float* data) : m_data{ data } {}
	inline ConstantBuffer::ConstantBufferField::ConstantBufferField(const float* data) : m_dataConst{ data } {}

	inline Math::Vector2* ConstantBuffer::ConstantBufferField::asVector2() { return Math::vectorFromPtr<2>(m_data); }
	inline Math::Vector3* ConstantBuffer::ConstantBufferField::asVector3() { return Math::vectorFromPtr<3>(m_data); }
	inline Math::Vector4* ConstantBuffer::ConstantBufferField::asVector4() { return Math::vectorFromPtr<4>(m_data); }

	inline Math::Matrix2* ConstantBuffer::ConstantBufferField::asMatrix2() { return Math::MatrixFromPtr<2>(m_data); }
	inline Math::Matrix3* ConstantBuffer::ConstantBufferField::asMatrix3() { return Math::MatrixFromPtr<3>(m_data); }
	inline Math::Matrix4* ConstantBuffer::ConstantBufferField::asMatrix4() { return Math::MatrixFromPtr<4>(m_data); }

	inline const Math::Vector2* ConstantBuffer::ConstantBufferField::asVector2() const { return Math::vectorFromPtr<2>(m_dataConst); }
	inline const Math::Vector3* ConstantBuffer::ConstantBufferField::asVector3() const { return Math::vectorFromPtr<3>(m_dataConst); }
	inline const Math::Vector4* ConstantBuffer::ConstantBufferField::asVector4() const { return Math::vectorFromPtr<4>(m_dataConst); }

	inline const Math::Matrix2* ConstantBuffer::ConstantBufferField::asMatrix2() const { return Math::MatrixFromPtr<2>(m_dataConst); }
	inline const Math::Matrix3* ConstantBuffer::ConstantBufferField::asMatrix3() const { return Math::MatrixFromPtr<3>(m_dataConst); }
	inline const Math::Matrix4* ConstantBuffer::ConstantBufferField::asMatrix4() const { return Math::MatrixFromPtr<4>(m_dataConst); }

	inline float* ConstantBuffer::ConstantBufferField::asFloat() { return m_data; }
	inline const float* ConstantBuffer::ConstantBufferField::asFloat() const { return m_dataConst; }
}
#endif
