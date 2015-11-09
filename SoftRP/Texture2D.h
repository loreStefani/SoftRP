#ifndef SOFTRP_TEXTURE_2D_H_
#define SOFTRP_TEXTURE_2D_H_
#include "SoftRPDefs.h"
#include<memory>
#include "ThreadPool.h"
namespace SoftRP {
	
	/*
	Concrete data type which represents a two-dimensional texture of elements of type T.
	Besides common operations, a Texture2D can generate mipmaps and provide access to them.
	*/

	template<typename T>
	class Texture2D
	{
	public:
		
		//ctor
		Texture2D(const unsigned int width = 1, const unsigned int height = 1);

		//dtor
		~Texture2D() = default;

		//copy
		Texture2D(const Texture2D&);
		Texture2D& operator=(const Texture2D&);
		//move
		Texture2D(Texture2D&&) = default;
		Texture2D& operator=(Texture2D&&) = default;

		/* dimensions */
		unsigned int width() const;
		unsigned int height() const;
		void resize(const unsigned int width, const unsigned int height);

		/* accessors */
		T& operator[](const int i);		
		T* getData();		
		T& get(const unsigned int i, const unsigned int j);

		/* const accessors */
		const T& get(const unsigned int i, const unsigned int j, unsigned int mipLevel)const;
		const T* getData() const;
		const T& operator[](const int i) const;
		const T& get(const unsigned int i, const unsigned int j)const;

		/* setters */
		void set(const unsigned int i, const unsigned int j, const T& value);
		
		/* clearing */
		void clear(T clearValue);
#ifdef SOFTRP_MULTI_THREAD
		void clear(T clearValue, ThreadPool& threadPool);
#endif

		/* mipmaps */
		void generateMipMaps();		
		unsigned int mipLevels() const;
		unsigned int maxMipLevel() const;
		unsigned int mipLevelWidth(unsigned int mipLevel) const;
		unsigned int mipLevelHeight(unsigned int mipLevel) const;
		//TODO: custom mipmaps
	private:

		void checkMipLevel(unsigned int mipLevel) const;
		void computeMipLevels();
		void generateMipMap(Texture2D<T>& mipMap, Texture2D<T>& src);
				
		bool isInRange(const int i) const;
		void checkRange(const int i)const;
		void copy(const Texture2D&);
		void clearTask(int size, int start, T clearValue);
				
		unsigned int m_width{ 0 };
		unsigned int m_height{ 0 };
		unsigned int m_count{ 0 };
		unsigned int m_mipLevels{ 0 };
		std::unique_ptr<T[]> m_data{nullptr};
		std::unique_ptr<Texture2D<T>[]> m_mipmaps{nullptr};
	};
}
#include "Texture2DImpl.inl"
#endif

