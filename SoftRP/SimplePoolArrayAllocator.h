#ifndef SOFTRP_SIMPLE_POOL_ARRAY_ALLOCATOR_H_
#define SOFTRP_SIMPLE_POOL_ARRAY_ALLOCATOR_H_
#include "ArrayAllocator.h"
namespace SoftRP {
	
	/*
	Concrete data type which represents an allocator that uses operator new and operator delete
	to manage allocations and ignores any alignment requirement.
	*/
	struct SimpleAllocator {
		static void* allocate(size_t size, size_t alignment);
		static void deallocate(void* ptr);
	};

	template<typename T>
	using SimplePoolArrayAllocator = PoolArrayAllocator<T, SimpleAllocator>;
}
#include "SimplePoolArrayAllocatorImpl.inl"
#endif
