#ifndef SOFTRP_OBJECT_POOL_IMPL_INL_
#define SOFTRP_OBJECT_POOL_IMPL_INL_
#include "ObjectPool.h"
#include<utility>
namespace SoftRP {

	/* ObjectPool implementation */

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	inline ObjectPool<T, C, A, Deact, Destr>::ObjectPool(const C& creationPolicy, const A& activationPolicy,
														 const Deact& deactivationPolicy, const Destr& destructionPolicy)		
		:m_creationPolicy{ creationPolicy }, m_activationPolicy{ activationPolicy },
		m_deactivationPolicy{ deactivationPolicy }, m_destructionPolicy{ destructionPolicy } {
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	inline ObjectPool<T, C, A, Deact, Destr>::~ObjectPool() {
		makeEmpty();
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	inline ObjectPool<T, C, A, Deact, Destr>::ObjectPool(ObjectPool&& objPool) {
		std::lock_guard<std::mutex> lock{ objPool.m_mutex };
		move(std::move(objPool));
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	inline ObjectPool<T, C, A, Deact, Destr>& ObjectPool<T, C, A, Deact, Destr>::operator=(ObjectPool&& objPool) {
		if (&objPool == this)
			return *this;

		makeEmpty();

		std::unique_lock<std::mutex> lock1{ m_mutex, std::defer_lock };
		std::unique_lock<std::mutex> lock2{ objPool.m_mutex, std::defer_lock };
		std::lock(lock1, lock2);
		move(std::move(objPool));		
		return *this;
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	inline void ObjectPool<T, C, A, Deact, Destr>::move(ObjectPool&& objPool) {
		m_creationPolicy = std::move(objPool.m_creationPolicy);
		m_activationPolicy = std::move(objPool.m_activationPolicy);
		m_deactivationPolicy = std::move(objPool.m_deactivationPolicy);
		m_destructionPolicy = std::move(objPool.m_destructionPolicy);
		m_pool = std::move(objPool.m_pool);
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	template<typename... Args>
	inline T ObjectPool<T, C, A, Deact, Destr>::takeOne(Args... args) {
#ifdef SOFTRP_MULTI_THREAD
		std::lock_guard<std::mutex> lock{ m_mutex };
#endif
		return takeUnsafe(std::forward<Args>(args)...);
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	template<typename... Args>
	inline void ObjectPool<T, C, A, Deact, Destr>::putOne(T&& o, Args... args) {
#ifdef SOFTRP_MULTI_THREAD
		std::lock_guard<std::mutex> lock{ m_mutex };
#endif
		putUnsafe(std::forward<T>(o), std::forward<Args>(args)...);
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	inline void ObjectPool<T, C, A, Deact, Destr>::acquire() {
#ifdef SOFTRP_MULTI_THREAD
		m_mutex.lock();
#endif
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	inline void ObjectPool<T, C, A, Deact, Destr>::release() {
#ifdef SOFTRP_MULTI_THREAD
		m_mutex.unlock();
#endif
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	template<typename... Args>
	inline T ObjectPool<T, C, A, Deact, Destr>::takeOneAcquired(Args... args) {
		return takeUnsafe(std::forward<Args>(args)...);
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	template<typename... Args>
	inline void ObjectPool<T, C, A, Deact, Destr>::putOneAcquired(T&& o, Args... args) {
		return putUnsafe(std::forward<T>(o), std::forward<Args>(args)...);
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	template<typename... Args>
	inline void ObjectPool<T, C, A, Deact, Destr>::makeEmpty(Args... args) {
#ifdef SOFTRP_MULTI_THREAD
		std::lock_guard<std::mutex> lock{ m_mutex };
#endif			
		size_t count = m_pool.size();
		for (size_t i = 0; i < count; i++)
			m_destructionPolicy.destroy(std::move(m_pool[i]), std::forward<Args>(args)...);
		m_pool.clear();
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	template<typename... Args>
	inline T ObjectPool<T, C, A, Deact, Destr>::takeUnsafe(Args... args) {
		if (m_pool.size() > 0) {
			auto res = std::move(m_pool.back());
			m_pool.pop_back();
			m_activationPolicy.activate(res, std::forward<Args>(args)...);
			return res;
		} else
			return m_creationPolicy.create(std::forward<Args>(args)...);
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	template<typename... Args>
	inline void ObjectPool<T, C, A, Deact, Destr>::putUnsafe(T&& o, Args... args) {
		m_deactivationPolicy.deactivate(o, std::forward<Args>(args)...);
		m_pool.push_back(std::forward<T>(o));
	}

	template<typename T, typename C, typename A, typename Deact, typename Destr>
	template<typename... Args>
	inline void ObjectPool<T, C, A, Deact, Destr>::warm(size_t size, Args... args) {
		const size_t currSize = m_pool.size();
		if (currSize >= size)
			return;
		m_pool.reserve(size);
		for (size_t i = currSize; i < size; i++)
			m_pool.push_back(std::move(m_creationPolicy.create(std::forward<Args>(args)...)));
	}

	/* ConstructCreationPolicy implementation */

	template<typename T>
	template<typename... Args>
	inline T ConstructCreationPolicy<T>::create(Args... args) {
		return T{ std::forward<Args>(args)... };
	}
	
	/* EmptyActivationPolicy implementation */

	template<typename T>
	template<typename... Args>
	inline void EmptyActivationPolicy<T>::activate(T& o, Args... args) {
	}
	
	/* EmptyDeactivationPolicy implementation */

	template<typename T>
	template<typename... Args>
	inline void EmptyDeactivationPolicy<T>::deactivate(T& o, Args... args) {
	}

	/* EmptyDestructionPolicy implementation */

	template<typename T>
	template<typename... Args>
	inline void EmptyDestructionPolicy<T>::destroy(T&& o, Args... args) {
	}

}
#endif
