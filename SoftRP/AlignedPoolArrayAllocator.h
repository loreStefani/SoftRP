#ifndef SOFTRP_ALIGNED_POOL_ARRAY_ALLOCATOR_H_
#define SOFTRP_ALIGNED_POOL_ARRAY_ALLOCATOR_H_
#include "ArrayAllocator.h"
#include <cassert>
namespace SoftRP {
	
	/*
	Concrete data type which represents an allocator that uses operator new and operator delete
	to manage allocations and satisfy alignment requirements.
	*/
	struct AlignedAllocator {		
		static void* allocate(size_t size, size_t alignment);
		static void deallocate(void* alignedAddress);
	};

	template<typename T>
	using AlignedPoolArrayAllocator = PoolArrayAllocator<T, AlignedAllocator>;	

}
#include "AlignedPoolArrayAllocatorImpl.inl"
#endif
