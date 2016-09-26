#ifndef SOFTRP_ARRAY_ALLOCATOR_IMPL_INL_
#define SOFTRP_ARRAY_ALLOCATOR_IMPL_INL_
#include "ArrayAllocator.h"
#include <utility>
#include <cassert>
namespace SoftRP {

	/* ArrayAllocator implementation */

	template<typename T, typename AllocationDesc, typename ArrayAllocationDesc>
	inline ArrayAllocator<T, AllocationDesc, ArrayAllocationDesc>::
		ArrayAllocator(size_t allocStride, size_t allocAlignment) 
		: m_allocStride{ allocStride }, m_allocAlignment{ allocAlignment }
	{
	}

	template<typename T, typename AllocationDesc, typename ArrayAllocationDesc>
	inline ArrayAllocator<T, AllocationDesc, ArrayAllocationDesc>::
		ArrayAllocator(ArrayAllocator&& va) {

#ifdef SOFTRP_MULTI_THREAD
		std::lock_guard<std::mutex> vaLock{ va.m_mutex };
#endif			
		m_allocStride = va.m_allocStride;
		m_allocAlignment = va.m_allocAlignment;
		m_allocations = std::move(va.m_allocations);
		m_arrayAllocations = std::move(va.m_arrayAllocations);
	}

	template<typename T, typename AllocationDesc, typename ArrayAllocationDesc>
	inline ArrayAllocator<T, AllocationDesc, ArrayAllocationDesc>::~ArrayAllocator() {
#ifdef _DEBUG
		for (auto& allocDesc : m_allocations)
			assert(!allocDesc.hasAllocations());

		for (auto& p : m_arrayAllocations)
			assert(!p.second.hasAllocations());
#endif
	}

	template<typename T, typename AllocationDesc, typename ArrayAllocationDesc>
	inline ArrayAllocator<T, AllocationDesc, ArrayAllocationDesc>& ArrayAllocator<T, AllocationDesc, ArrayAllocationDesc>::operator=(ArrayAllocator&& va) {
		if (&va == this)
			return *this;
#ifdef SOFTRP_MULTI_THREAD
		std::unique_lock<std::mutex> thisLock{ m_mutex, std::defer_lock };
		std::unique_lock<std::mutex> vaLock{ va.m_mutex, std::defer_lock };
		std::lock(thisLock, vaLock);
#endif	
		m_allocStride = va.m_allocStride;
		m_allocAlignment = va.m_allocAlignment;
		m_allocations = std::move(va.m_allocations);
		m_arrayAllocations = std::move(va.m_arrayAllocations);
		return *this;
	}

	template<typename T, typename AllocationDesc, typename ArrayAllocationDesc>
	inline T* ArrayAllocator<T, AllocationDesc, ArrayAllocationDesc>::allocate() {
#ifdef SOFTRP_MULTI_THREAD
		std::lock_guard<std::mutex> lock{ m_mutex };
#endif
		for (auto& allocDesc : m_allocations) {
			if (allocDesc.canAllocate()) {
				return allocDesc.allocate();
			}
		}

		m_allocations.push_back(AllocationDesc{ m_allocStride, m_allocAlignment });
		return m_allocations.back().allocate();
	}

	template<typename T, typename AllocationDesc, typename ArrayAllocationDesc>
	inline void ArrayAllocator<T, AllocationDesc, ArrayAllocationDesc>::deallocate(T* data) {
#ifdef SOFTRP_MULTI_THREAD
		std::lock_guard<std::mutex> lock{ m_mutex };
#endif
		for (auto& allocDesc : m_allocations) {
			if (allocDesc.deallocate(data))
				return;
		}
		throw std::runtime_error{ "Deallocation requested to the wrong allocator" };
	}

	template<typename T, typename AllocationDesc, typename ArrayAllocationDesc>
	inline T* ArrayAllocator<T, AllocationDesc, ArrayAllocationDesc>::allocateArray(size_t count) {
#ifdef SOFTRP_MULTI_THREAD
		std::lock_guard<std::mutex> lock{ m_mutex };
#endif
		auto it = m_arrayAllocations.find(count);
		if (it != m_arrayAllocations.end()) {
			return it->second.allocate();
		} else {
			ArrayAllocationDesc arrayAllocDesc{ m_allocStride, count, m_allocAlignment };
			T* ptr = arrayAllocDesc.allocate();
			m_arrayAllocations.emplace(count, std::move(arrayAllocDesc));
			return ptr;
		}
	}

	template<typename T, typename AllocationDesc, typename ArrayAllocationDesc>
	inline void ArrayAllocator<T, AllocationDesc, ArrayAllocationDesc>::deallocateArray(T* dataArray) {
#ifdef SOFTRP_MULTI_THREAD
		std::lock_guard<std::mutex> lock{ m_mutex };
#endif
		for (auto& pair : m_arrayAllocations) {
			auto& arrayAllocDesc = pair.second;
			if (arrayAllocDesc.deallocate(dataArray))
				return;
		}
		throw std::runtime_error{ "Deallocation requested to the wrong allocator" };
	}

	/* PoolAllocDescBase implementation */

	template<typename T, typename Allocator>
	inline PoolAllocDescBase<T, Allocator>::PoolAllocDescBase(size_t allocStride, size_t allocAlignment) {
		const size_t elementSize = allocStride*sizeof(T);
		const size_t allocSize = BLOCK_SIZE*elementSize;
		m_data = static_cast<T*>(m_allocator.allocate(allocSize, allocAlignment));
		m_end = m_data + allocStride*BLOCK_SIZE;
		m_allocStride = allocStride;
	}

	template<typename T, typename Allocator>
	inline PoolAllocDescBase<T, Allocator>::~PoolAllocDescBase() {
		if (m_data) {
			m_allocator.deallocate(m_data);
			m_data = nullptr;
		}
	}

	template<typename T, typename Allocator>
	inline PoolAllocDescBase<T, Allocator>::PoolAllocDescBase(PoolAllocDescBase&& aad)
		:m_allocationState{ aad.m_allocationState }, m_allocStride{ aad.m_allocStride }, m_data{ aad.m_data }, m_end{ aad.m_end } {
		aad.m_data = nullptr;
	}

	template<typename T, typename Allocator>
	inline PoolAllocDescBase<T, Allocator>& PoolAllocDescBase<T, Allocator>::operator=(PoolAllocDescBase&& aad) {
		m_allocationState = aad.m_allocationState;
		m_allocStride = aad.m_allocStride;
		m_data = aad.m_data;
		m_end = aad.m_end;
		aad.m_data = nullptr;
	}

	template<typename T, typename Allocator>
	inline T* PoolAllocDescBase<T, Allocator>::allocate() {
		size_t index = m_nextIndex;

		uint64_t currAlloc = static_cast<uint64_t>(1) << index;
		for (size_t i = index; i < BLOCK_SIZE; i++) {
			if ((m_allocationState & currAlloc) == 0) {
				m_allocationState |= currAlloc;
				m_nextIndex = i + 1;
				return m_data + i*m_allocStride;
			}

			currAlloc <<= 1;
		}

		currAlloc = 1;
		for (size_t i = 0; i < index; i++) {
			if ((m_allocationState & currAlloc) == 0) {
				m_allocationState |= currAlloc;
				m_nextIndex = i + 1;
				return m_data + i*m_allocStride;
			}

			currAlloc <<= 1;
		}

		return nullptr;
	}

	template<typename T, typename Allocator>
	inline bool PoolAllocDescBase<T, Allocator>::canAllocate() const {
		return m_allocationState != 0xFFFFFFFFFFFFFFFF;
	}

	template<typename T, typename Allocator>
	inline bool PoolAllocDescBase<T, Allocator>::hasAllocations() const {
		return m_allocationState != 0x0;
	}

	template<typename T, typename Allocator>
	inline bool PoolAllocDescBase<T, Allocator>::deallocate(T* ptr) {

		if (ptr < m_data || ptr >= m_end)
			return false;

		size_t index = (ptr - m_data) / m_allocStride;
#ifdef _DEBUG

		if ((ptr - m_data) != index*m_allocStride)
			throw std::runtime_error{ "Deallocating from the wrong address!" };

		if ((m_allocationState & (static_cast<uint64_t>(1) << index)) == 0)
			throw std::runtime_error{ "Deallocating from the wrong allocator!" };
#endif
		m_allocationState &= ~(static_cast<uint64_t>(1) << index);
		m_nextIndex = index;

		return true;
	}

	/* PoolArrayAllocDescBase implementation */

	template<typename T, typename Allocator>
	inline PoolArrayAllocDescBase<T, Allocator>::PoolArrayAllocDescBase(size_t allocStride, size_t count, size_t allocAlignment)
		: m_elementSize{ allocStride*sizeof(T) }, m_allocSize{ count*m_elementSize }, 
		m_allocAlignment{ allocAlignment }
	{
	}

	template<typename T, typename Allocator>
	inline PoolArrayAllocDescBase<T, Allocator>::~PoolArrayAllocDescBase() {
		for (const auto& p : m_allocated)
			m_allocator.deallocate(p);
		for (const auto& p : m_free)
			m_allocator.deallocate(p);
	}

	template<typename T, typename Allocator>
	inline T* PoolArrayAllocDescBase<T, Allocator>::allocate() {
		T* ptr{ nullptr };
		if (m_free.size() == 0)
			ptr = static_cast<T*>(m_allocator.allocate(m_allocSize, m_allocAlignment));
		else {
			ptr = m_free.front();
			m_free.pop_front();
		}
		m_allocated.emplace(ptr);
		return ptr;
	}

	template<typename T, typename Allocator>
	inline bool PoolArrayAllocDescBase<T, Allocator>::hasAllocations()const {
		return m_allocated.size() != 0;
	}

	template<typename T, typename Allocator>
	inline bool PoolArrayAllocDescBase<T, Allocator>::deallocate(T* ptr) {
		auto it = m_allocated.find(ptr);

		if (it == m_allocated.end())
			return false;

		m_allocated.erase(it);
		m_free.emplace_back(ptr);

		return true;
	}

}
#endif