#ifndef SOFTRP_OBJECT_POOL_H_
#define SOFTRP_OBJECT_POOL_H_
#include "SoftRPDefs.h"
#include <vector>
#ifdef SOFTRP_MULTI_THREAD
#include <mutex>
#endif
namespace SoftRP {
		
	/*
	Concrete data type which represents a pool of objects of type T.
	The initial state of the pool is empty. Objects are created using an object 
	of type CreationPolicy, only if there are none already in the pool. Otherwise, 
	an object of the pool is activated, using an object of type ActivationPolicy, before is given 
	to the client. In either case, no ownership on the object returned is retained. Because of that, 
	objects that are put in the pool are not constrained to have been created from the pool.
	Once an object is put in the pool, it is deactivated using an object of type DeactivationPolicy.
	Optionally, all the objects of the pool can be destructed. The destruction is done using an 
	object of type DestructionPolicy before using the T's destructor.
	*/

	//forward declarations default template parameters
	template<typename T>
	struct ConstructCreationPolicy;

	template<typename T>
	struct EmptyActivationPolicy;

	template<typename T>
	struct EmptyDeactivationPolicy;

	template<typename T>
	struct EmptyDestructionPolicy;

	template<typename T, 
		typename CreationPolicy = ConstructCreationPolicy<T>,
		typename ActivationPolicy = EmptyActivationPolicy<T>,
		typename DeactivationPolicy = EmptyDeactivationPolicy<T>,
		typename DestructionPolicy = EmptyDestructionPolicy<T>>
	class ObjectPool {
	public:
		
		/*
		ctor. construct an ObjectPool copy constructing the policy objects from the 
		arguments passed in
		*/		
		ObjectPool(const CreationPolicy& creationPolicy = CreationPolicy{},
				   const ActivationPolicy& activationPolicy = ActivationPolicy{},
				   const DeactivationPolicy& deactivationPolicy = DeactivationPolicy{},
				   const DestructionPolicy& destructionPolicy = DestructionPolicy{});

		//dtor
		~ObjectPool();

		//copy
		ObjectPool(const ObjectPool&) = delete;
		ObjectPool& operator=(const ObjectPool&) = delete;

		//move
		ObjectPool(ObjectPool&&);
		ObjectPool& operator=(ObjectPool&&);
		
		/*
		take one object from the pool, activating it with the given arguments,
		or construct one, with the given arguments, if non available
		*/
		template<typename... Args>
		T takeOne(Args... args);

		//put one object in the pool, deactivating it with the given arguments		
		template<typename... Args>
		void putOne(T&& o, Args... args);
		
		/*
		the calling thread acquires exclusive access to the pool.
		useful when multiple elements need to be taken from or put in the pool.
		The specialized methods (declared below) have to be used, or deadlock will occur
		*/
		void acquire();

		//release exclusive access to the pool
		void release();

		/*
		as takeOne, but the calling thread needs exclusive access, taken with acquire().
		if acquire() has not been called, undefined behavior will occur if multiple threads 
		access the pool
		*/
		template<typename... Args>
		T takeOneAcquired(Args... args);

		/*
		as putOne, but the calling thread needs exclusive access, taken with acquire().
		if acquire() has not been called, undefined behavior will occur if multiple threads
		access the pool
		*/
		template<typename... Args>
		void putOneAcquired(T&& o, Args... args);

		//create size objects in the pool, constructing them with the given arguments
		template<typename... Args>
		void warm(size_t size, Args... args);

		//make the pool empty, destructing all the objects with the given arguments
		template<typename... Args>
		void makeEmpty(Args... args);

	private:

		void move(ObjectPool&& objPool);

		template<typename... Args>
		T takeUnsafe(Args... args);

		template<typename... Args>
		void putUnsafe(T&& o, Args... args);

		CreationPolicy m_creationPolicy;
		ActivationPolicy m_activationPolicy;
		DeactivationPolicy m_deactivationPolicy;
		DestructionPolicy m_destructionPolicy;
#ifdef SOFTRP_MULTI_THREAD
		std::mutex m_mutex{};
#endif
		std::vector<T> m_pool{};
	};
	
	/*
	Default creation policy for ObjectPool which constructs object of type T.
	The factory method forwards passed in arguments to T's constructors
	*/
	template<typename T>
	struct ConstructCreationPolicy {
		template<typename... Args>
		T create(Args... args);
	};

	/*
	Default activation policy for ObjectPool which does nothing.
	*/
	template<typename T>
	struct EmptyActivationPolicy {
		template<typename... Args>
		void activate(T& o, Args... args);
	};

	/*
	Default deactivation policy for ObjectPool which does nothing.
	*/
	template<typename T>
	struct EmptyDeactivationPolicy {
		template<typename... Args>
		void deactivate(T& o, Args... args);
	};

	/*
	Default destruction policy for ObjectPool which does nothing.
	*/
	template<typename T>
	struct EmptyDestructionPolicy {
		template<typename... Args>
		void destroy(T&& o, Args... args);
	};	
}
#include "ObjectPoolImpl.inl"
#endif