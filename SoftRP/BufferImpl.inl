#ifndef SOFTRP_BUFFER_IMPL_INL_
#define SOFTRP_BUFFER_IMPL_INL_
#include "Buffer.h"
namespace SoftRP {

	template<typename T>
	inline Buffer<T>::Buffer(size_t size)
		: m_owned{ true },
		m_data{ new T[size] },
		m_size{ size } {}

	template<typename T>
	inline Buffer<T>::Buffer(T* data, size_t size) : m_owned{ false }, m_data{ data }, m_size{ size } {}

	template<typename T>
	inline Buffer<T>::~Buffer() {
		clean();
	}

	template<typename T>
	inline Buffer<T>::Buffer(const Buffer& buff) {
		copy(buff);
	}

	template<typename T>
	inline Buffer<T>& Buffer<T>::operator=(const Buffer& buff) {
		copy(buff);
	}

	template<typename T>
	inline void Buffer<T>::copy(const Buffer& buff) {
		clean();
		m_size = buff.m_size;
		if (m_size > 0) {
			m_data = new T[m_size];
			m_owned = true;
			for (size_t i = 0; i < m_size; i++)
				m_data[i] = buff.m_data[i];
		}
	}

	template<typename T>
	inline Buffer<T>::Buffer(Buffer&& buff) {
		move(std::move(buff));
	}

	template<typename T>
	inline Buffer<T>& Buffer<T>::operator=(Buffer&& buff) {
		move(std::move(buff));
		return *this;
	}

	template<typename T>
	inline T* Buffer<T>::get() { return m_data; }

	template<typename T>
	inline const T* Buffer<T>::get()const { return m_data; }

	template<typename T>
	inline size_t Buffer<T>::size()const { return m_size; }

	template<typename T>
	inline void Buffer<T>::move(Buffer&& buff) {
		clean();
		m_owned = buff.m_owned;
		m_data = buff.m_data;
		m_size = buff.m_size;
		buff.m_owned = false;
		buff.m_data = nullptr;
	}

	template<typename T>
	inline void Buffer<T>::clean() {
		if (m_owned && m_data)
			delete[]m_data;
		m_owned = false;
		m_data = nullptr;
	}
}
#endif