#ifndef SOFTRP_TEXTURE_2D_IMPL_INL_
#define SOFTRP_TEXTURE_2D_IMPL_INL_
#include "Texture2D.h"
#include<cmath>
#include<stdexcept>
#include<cassert>
namespace SoftRP {

	template<typename T>
	inline Texture2D<T>::Texture2D(const unsigned int width, const unsigned int height) {
		resize(width, height);
	}

	template<typename T>
	inline Texture2D<T>::Texture2D(const Texture2D& texture) {
		copy(texture);
	}

	template<typename T>
	inline Texture2D<T>& Texture2D<T>::operator=(const Texture2D& texture) {
		copy(texture);
		return *this;
	}

	template<typename T>
	inline unsigned int Texture2D<T>::width() const {
		return m_width;
	}

	template<typename T>
	inline unsigned int Texture2D<T>::height() const {
		return m_height;
	}

	template<typename T>
	inline T& Texture2D<T>::operator[](const int i) {
		assert(isInRange(i));
		return m_data[i];
	}

	template<typename T>
	inline const T& Texture2D<T>::operator[](const int i) const {
		assert(isInRange(i));
		return m_data[i];
	}

	template<typename T>
	inline void Texture2D<T>::set(const unsigned int i, const unsigned int j, const T& value) {
		const unsigned int index = i*m_width + j;
		checkRange(index);
		m_data[index] = value;
	}

	template<typename T>
	inline void Texture2D<T>::resize(const unsigned int width, const unsigned int height) {
		if (width == 0 || height == 0)
			throw std::invalid_argument{ "Invalid size." };

		if (m_width == width && m_height == height)
			return;

		m_data.reset(nullptr);
		m_width = width;
		m_height = height;
		m_count = width*height;
		m_data.reset(new T[m_count]{});
		m_mipmaps.reset(nullptr);
	}

	template<typename T>
	inline void Texture2D<T>::copy(const Texture2D& texture) {
		resize(texture.m_width, texture.m_height);
		T* first = texture.m_data.get();
		T* dest = m_data.get();
		for (unsigned int i = 0; i < m_count; i++)
			m_data[i] = texture.m_data[i];
		if (texture.m_mipLevels > 0) {
			if (texture.m_mipLevels != m_mipLevels) {
				m_mipLevels = texture.m_mipLevels;
				m_mipmaps.reset(new Texture2D<T>[m_mipLevels]);
			}
			for (unsigned int i = 0; i < m_mipLevels; i++)
				m_mipmaps[i] = texture.m_mipmaps[i];
		} else
			m_mipLevels = texture.m_mipLevels;
	}

	template<typename T>
	inline const T& Texture2D<T>::get(const unsigned int i, const unsigned int j)const {
		return operator[](i*m_width + j);
	}

	template<typename T>
	inline T& Texture2D<T>::get(const unsigned int i, const unsigned int j) {
		return operator[](i*m_width + j);
	}

	template<typename T>
	inline const T& Texture2D<T>::get(const unsigned int i, const unsigned int j, unsigned int mipLevel)const {
		checkMipLevel(mipLevel);
		if (mipLevel == 0)
			return get(i, j);
		return m_mipmaps[mipLevel - 1].get(i, j);
	}

	template<typename T>
	inline const T* Texture2D<T>::getData() const {
		return m_data.get();
	}

	template<typename T>
	inline T* Texture2D<T>::getData() {
		return m_data.get();
	}

	template<typename T>
	inline void Texture2D<T>::clear(T clearValue) {
		for (unsigned int i = 0; i < m_count; i++)
			m_data[i] = clearValue;
	}

#ifdef SOFTRP_MULTI_THREAD
	template<typename T>
	inline void Texture2D<T>::clear(T clearValue, ThreadPool& threadPool) {
		constexpr int tileHeight = 256; //clear tiles size : m_width x tileHeigth
		const int tilesCount = static_cast<int>(std::ceil(static_cast<float>(m_height) / static_cast<float>(tileHeight)));
		const int tileSize = m_width*tileHeight;
		int start = 0;
		for (int i = 0; i < tilesCount - 1; i++, start += tileSize)
			threadPool.addTask([this, tileSize, start, clearValue]() {
			clearTask(tileSize, start, clearValue);
		});

		const int size = std::min<int>(tileSize, m_count - start);
		threadPool.addTask([this, size, start, clearValue]() {
			clearTask(size, start, clearValue);
		});
	}
#endif

	template<typename T>
	inline void Texture2D<T>::clearTask(int size, int start, T clearValue) {
		for (int i = 0; i < size; i++) {
			m_data[start++] = clearValue;
		}
	}

	template<typename T>
	inline void Texture2D<T>::generateMipMaps() {

		computeMipLevels();

		if (m_mipLevels == 0)
			return;

		m_mipmaps.reset(new Texture2D<T>[m_mipLevels]);
		unsigned int width = m_width > 1 ? m_width >> 1 : 1;
		unsigned int height = m_height > 1 ? m_height >> 1 : 1;
		m_mipmaps[0].resize(width, height);
		generateMipMap(m_mipmaps[0], *this);
		for (unsigned int i = 1; i < m_mipLevels; i++) {
			if (width > 1)
				width >>= 1;

			if (height > 1)
				height >>= 1;

			m_mipmaps[i].resize(width, height);
			generateMipMap(m_mipmaps[i], m_mipmaps[i - 1]);
		}
	}

	template<typename T>
	inline unsigned int Texture2D<T>::mipLevels() const {
		return m_mipLevels;
	}

	template<typename T>
	inline unsigned int Texture2D<T>::maxMipLevel() const {
		return m_mipLevels;
	}

	template<typename T>
	inline unsigned int Texture2D<T>::mipLevelWidth(unsigned int mipLevel) const {
		checkMipLevel(mipLevel);
		if (mipLevel == 0)
			return width();
		return m_mipmaps[mipLevel - 1].width();
	}

	template<typename T>
	inline unsigned int Texture2D<T>::mipLevelHeight(unsigned int mipLevel) const {
		checkMipLevel(mipLevel);
		if (mipLevel == 0)
			return height();
		return m_mipmaps[mipLevel - 1].height();
	}

	template<typename T>
	inline void Texture2D<T>::checkMipLevel(unsigned int mipLevel) const {
#ifdef _DEBUG
		if (mipLevel >= m_mipLevels + 1)
			throw std::runtime_error{ "Invalid mip level" };
#endif		
	}

	template<typename T>
	inline void Texture2D<T>::computeMipLevels() {
		m_mipLevels = 0;
		unsigned int width = m_width;
		unsigned int height = m_height;
		while (width > 1 || height > 1) {
			if (width > 1)
				width >>= 1;
			if (height > 1)
				height >>= 1;
			m_mipLevels++;
		}
	}

	template<typename T>
	inline void Texture2D<T>::generateMipMap(Texture2D<T>& mipMap, Texture2D<T>& src) {
		const unsigned int width = mipMap.m_width;
		const unsigned int height = mipMap.m_height;
		const float invBlockSize = 1.0f / 4.0f;
		for (unsigned int i = 0; i < height; i++) {
			unsigned int y = i << 1;
			for (unsigned int j = 0; j < width; j++) {
				T value{};
				unsigned int x = j << 1;
				for (unsigned int k = 0; k < 2; k++) {
					for (unsigned int s = 0; s < 2; s++) {
						unsigned int clampedY = y + k;
						unsigned int clampedX = x + s;
						clampedY = clampedY >= src.m_height ? src.m_height - 1 : clampedY;
						clampedX = clampedX >= src.m_width ? src.m_width - 1 : clampedX;
						value += src.get(clampedY, clampedX);
					}
				}
				value *= invBlockSize;
				mipMap.set(i, j, value);
			}
		}
	}

	template<typename T>
	inline bool Texture2D<T>::isInRange(const int i) const {
		return i >= 0 && static_cast<unsigned int>(i) < m_count;
	}

	template<typename T>
	inline void Texture2D<T>::checkRange(const int i)const {
#ifdef _DEBUG
		if (!isInRange(i))
			throw std::out_of_range{ "Invalid index." };
#endif
	}

}
#endif
