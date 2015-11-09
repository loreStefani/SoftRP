#ifndef SOFTRP_ALIGNED_POOL_ARRAY_ALLOCATOR_IMPL_INL_
#define SOFTRP_ALIGNED_POOL_ARRAY_ALLOCATOR_IMPL_INL_
#include "AlignedPoolArrayAllocator.h"
namespace SoftRP {
	
	inline void* AlignedAllocator::allocate(size_t size, size_t alignment) {
		static_assert(sizeof(unsigned char) == sizeof(char) && sizeof(char) == 1, "Unexpected unsigned char size");
		assert((alignment & (alignment - 1)) == 0 && size > 0); //alignment must be a power of 2				

		unsigned char* address = new unsigned char[size + alignment];
		const size_t mask = alignment - 1;
		size_t misAlignment = reinterpret_cast<uintptr_t>(address) & mask;
		size_t adjustment = (alignment - misAlignment);
		//min, max(misAlignment) == (0,alignment - 1). min, max(adjustment) == (1, alignment)

		unsigned char* alignedAddress = address + adjustment;
		assert(adjustment < 256);
		alignedAddress[-1] = static_cast<unsigned char>(adjustment);
		return alignedAddress;
	}

	inline void AlignedAllocator::deallocate(void* alignedAddress) {
		size_t adjustment = static_cast<size_t>(reinterpret_cast<unsigned char*>(alignedAddress)[-1]);
		delete[](reinterpret_cast<unsigned char*>(alignedAddress) - adjustment);
	}
}
#endif
