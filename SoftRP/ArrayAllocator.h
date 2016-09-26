#ifndef SOFTRP_ARRAY_ALLOCATOR_H_
#define SOFTRP_ARRAY_ALLOCATOR_H_
#include "SoftRPDefs.h"
#include <vector>
#include <map>
#include <unordered_set>
#include<deque>
#include <mutex>
namespace SoftRP {

	/*
	Concrete data type which represents an allocator of T-typed fixed size arrays.
	The size is specified at construction time and fixed. 
	Besides single allocations, arrays of arrays can be allocated too. The former allocations
	are managed by objects of type AllocationDesc, while the latter are managed by objects of 
	type ArrayAllocationDesc.
	*/
	template<typename T, typename AllocationDesc, typename ArrayAllocationDesc>
	class ArrayAllocator {
	public:

		//ctor. constructs an ArrayAllocator which allocates arrays of size allocStride
		ArrayAllocator(size_t allocStride, size_t allocAlignment);
		//dtor
		~ArrayAllocator();
		//copy
		ArrayAllocator(const ArrayAllocator&) = delete;
		ArrayAllocator& operator=(const ArrayAllocator&) = delete;
		//move
		ArrayAllocator(ArrayAllocator&& va);		
		ArrayAllocator& operator=(ArrayAllocator&& va);

		//allocate a single array
		T* allocate();
		//deallocate a single array
		void deallocate(T* data);
		//allocate an array of count arrays
		T* allocateArray(size_t count);
		//deallocate an array of arrays
		void deallocateArray(T* dataArray);

	private:
		size_t m_allocStride;
		size_t m_allocAlignment;
		std::vector<AllocationDesc> m_allocations{};
		std::map<size_t, ArrayAllocationDesc> m_arrayAllocations{};
#ifdef SOFTRP_MULTI_THREAD
		std::mutex m_mutex{};
#endif
	};
	
	/*
	Concrete data type which represents a pool of T-typed fixed size arrays.
	The size of the pool is implementation-dependent and it is allocated entirely at 
	construction time.
	The type is a valid candidate for the ArrayAllocator's first template argument.
	*/
	template<typename T, typename Allocator>
	class PoolAllocDescBase {
	public:
		//ctor. constructs a PoolAllocDescBase which allocates arrays of size allocStride
		PoolAllocDescBase(size_t allocStride, size_t allocAlignment);
		//dtor
		~PoolAllocDescBase();
		//copy
		PoolAllocDescBase(const PoolAllocDescBase&) = delete;		
		PoolAllocDescBase& operator=(const PoolAllocDescBase&) = delete;
		//move
		PoolAllocDescBase(PoolAllocDescBase&&);
		PoolAllocDescBase& operator=(PoolAllocDescBase&&);

		//allocate a single array
		T* allocate();
		//deallocate a single array
		bool deallocate(T* ptr);
		//are other allocations possible?
		bool canAllocate() const;		
		//are there any allocation that has not been paired by a deallocation?
		bool hasAllocations()const;

	private:
		static constexpr size_t BLOCK_SIZE = 64;
		uint64_t m_allocationState{ 0 };
		size_t m_allocStride;
		size_t m_nextIndex{ 0 };
		//order of declaration is important here...look at the constructor
		T* m_data;
		T* m_end;
		Allocator m_allocator;
	};

	/*
	Concrete data type which represents a pool of arrays of T-typed fixed size arrays.
	The size of the pool is unbounded.
	The type is a valid candidate for the ArrayAllocator's second template argument.
	*/
	template<typename T, typename Allocator>
	class PoolArrayAllocDescBase {
	public:		
		/*
		ctor. constructs a PoolArrayAllocDescBase which allocates count-sized arrays of 
		arrays of size allocStride
		*/
		PoolArrayAllocDescBase(size_t allocStride, size_t count, size_t allocAlignment);
		//dtor
		~PoolArrayAllocDescBase();
		//copy
		PoolArrayAllocDescBase(const PoolArrayAllocDescBase&) = delete;		
		PoolArrayAllocDescBase& operator=(const PoolArrayAllocDescBase&) = delete;
		//move
		PoolArrayAllocDescBase(PoolArrayAllocDescBase&&) = default;
		PoolArrayAllocDescBase& operator=(PoolArrayAllocDescBase&&) = default;

		//allocate an array of arrays
		T* allocate();
		//deallocate an array of arrays
		bool deallocate(T* ptr);
		//are there any allocation that has not been paired by a deallocation?
		bool hasAllocations()const;

	private:
		size_t m_elementSize;
		size_t m_allocSize;
		size_t m_allocAlignment;
		std::unordered_set<T*> m_allocated{};
		std::deque<T*> m_free{};
		Allocator m_allocator;
	};


	/*
	ArrayAllocator specialization which uses PoolAllocDescBase and PoolArrayAllocDescBase to manage 
	allocations.
	*/
	template<typename T, typename Allocator>
	using PoolArrayAllocator = ArrayAllocator<T, PoolAllocDescBase<T, Allocator>, PoolArrayAllocDescBase<T, Allocator>>;

}
#include "ArrayAllocatorImpl.inl"
#endif
