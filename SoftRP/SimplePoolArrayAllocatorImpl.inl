#ifndef SOFTRP_SIMPLE_POOL_ARRAY_ALLOCATOR_IMPL_INL_
#define SOFTRP_SIMPLE_POOL_ARRAY_ALLOCATOR_IMPL_INL_
#include "SimplePoolArrayAllocator.h"
namespace SoftRP {

	inline void* SimpleAllocator::allocate(size_t size, size_t alignment) {
		return new unsigned char[size];
	}

	inline void SimpleAllocator::deallocate(void* ptr) {
		delete[] static_cast<unsigned char*>(ptr);
	}
}
#endif
