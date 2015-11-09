#ifndef SOFTRP_BUFFER_H_
#define SOFTRP_BUFFER_H_
namespace SoftRP {

	/*
	Concrete data type which refers to a typed memory buffer, large enough to contain an exact number of element of 
	the type specified as template argument. The memory can be either allocated (and deleted) by an instance, 
	or passed in at construction time. In the latter case, the client is responsible for its deletion.
	An instance can be copied via copy construction or copy assignment. These operations cause memory allocation.
	*/

	template<typename T>
	class Buffer {
	public:

		//ctor, Construct a Buffer allocating memory for size element of type T.
		Buffer(size_t size);
		/*
		ctor, Construct a Buffer which refers to the memory pointed to by data. 
		The memory area is assumed to be large enough to contain size element of type T.
		*/
		Buffer(T* data, size_t size);

		//dtor, the memory is deallocated only if allocated by the instance.
		~Buffer();
			
		//copy
		Buffer(const Buffer&);
		Buffer& operator=(const Buffer&);

		//move
		Buffer(Buffer&& buff);
		Buffer& operator=(Buffer&& buff);

		//access to the memory area
		T* get();
		const T* get()const;

		//size of the Buffer, unit = # elements of type T
		size_t size()const;

	private:
		void copy(const Buffer& buff);
		void move(Buffer&& buff);
		void clean();

		bool m_owned{false};
		T* m_data{nullptr};
		size_t m_size{ 0 };
	};	
}
#include "BufferImpl.inl"
#endif
