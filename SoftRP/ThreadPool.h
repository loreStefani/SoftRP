#ifndef SOFTRP_THREAD_POOL_H_
#define SOFTRP_THREAD_POOL_H_
#include "SoftRPDefs.h"
#include "TaskConsumer.h"
#include <functional>
#include <mutex>
#include <unordered_map>
namespace SoftRP {
	
	/*
	Concrete data type which represents a manager of a set of TaskConsumers to whom the tasks
	submitted are distributed.
	A fence mechanism is also offered to the clients. This can be used to keep track of the 
	progress of the TaskConsumers.	
	*/

	class ThreadPool {
	public:
				
		using Fence = uint64_t;
		using TaskType = TaskConsumer::TaskType;

		ThreadPool(size_t maxTaskConsumerCount = 64);		
		~ThreadPool();

		//copy
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		//move
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;

		//submit a task for execution which will be handed to a TaskConsumer
		void addTask(TaskType task);
		
		//are there any pending task in any TaskConsumer?
		bool hasTasks();
		
		//block the calling thread until all TaskConsumers has executed all the pending tasks
		void waitForPendingTasks();		

		/*
		increment the current Fence value, the value returned can be used to wait until all
		tasks submitted up to this point have been executed
		*/
		Fence addFence();

		//block the calling thread until the fence has been reached by all TaskConsumers
		void waitForFence(Fence f);

		//combine in an unique operation the submission of a task and the increment of the Fence
		Fence addTaskAndFence(TaskType task);	
		
		//get the value of the last fence added, which may not have been reached yet
		Fence currFence();		
		
	private:

		Fence addFenceUnsafe();
		void decrementFence(Fence f);

		size_t m_nextTaskConsumer{ 0 };		
		const size_t m_maxTaskConsumerCount;
		Fence m_fenceValue{ 0 };		
		std::unique_ptr<TaskConsumer[]> m_taskConsumers;
		std::mutex m_mtx{};
		
		struct FenceCounter {
			FenceCounter(uint64_t _remaining);
			uint64_t remaining;
			uint64_t waiting{ 0 };
			std::condition_variable fenceReached{};
		};

		std::unordered_map<Fence, FenceCounter> m_fenceCounters{};		
	};	
}
#include "ThreadPoolImpl.inl"
#endif
