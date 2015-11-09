#ifndef SOFTRP_THREAD_POOL_IMPL_INL_
#define SOFTRP_THREAD_POOL_IMPL_INL_
#include "ThreadPool.h"
namespace SoftRP {

	inline ThreadPool::ThreadPool(size_t maxTaskConsumerCount)
		: m_taskConsumers{ new TaskConsumer[maxTaskConsumerCount] },
		m_maxTaskConsumerCount{ maxTaskConsumerCount } {}

	inline ThreadPool::~ThreadPool() {
		waitForPendingTasks();
	}

	inline void ThreadPool::addTask(TaskType task) {
		size_t taskIndex;
		{
			std::lock_guard<std::mutex> lock{ m_mtx };
			//round-robin policy
			taskIndex = m_nextTaskConsumer++;
			if (m_nextTaskConsumer >= m_maxTaskConsumerCount)
				m_nextTaskConsumer = 0;
		}
		m_taskConsumers[taskIndex].addTask(task);
	}

	inline ThreadPool::Fence ThreadPool::addTaskAndFence(TaskType task) {
		std::lock_guard<std::mutex> lock{ m_mtx };
		m_taskConsumers[m_nextTaskConsumer].addTask(task);
		m_nextTaskConsumer++;
		if (m_nextTaskConsumer >= m_maxTaskConsumerCount)
			m_nextTaskConsumer = 0;
		return addFenceUnsafe();
	}

	inline bool ThreadPool::hasTasks() {
		for (size_t i = 0; i < m_maxTaskConsumerCount; i++)
			if (m_taskConsumers[i].hasTasks())
				return true;
		return false;
	}

	inline void ThreadPool::waitForPendingTasks() {
		for (size_t i = 0; i < m_maxTaskConsumerCount; i++)
			m_taskConsumers[i].waitForConsumation();
	}

	inline ThreadPool::Fence ThreadPool::addFence() {
		std::lock_guard<std::mutex> lock{ m_mtx };
		return addFenceUnsafe();
	}

	inline ThreadPool::Fence ThreadPool::currFence() {
		std::lock_guard<std::mutex> lock{ m_mtx };
		return m_fenceValue;
	}

	inline void ThreadPool::waitForFence(Fence f) {
		std::unique_lock<std::mutex> lock{ m_mtx };
		auto it = m_fenceCounters.find(f);

		if (it == m_fenceCounters.end()) {
			//the fence has already been reached
			if (f <= m_fenceValue)
				return;
			//the fence has not been added yet; create the counter
			it = m_fenceCounters.emplace(f, m_maxTaskConsumerCount).first;
		}

		FenceCounter& fenceCounter = it->second;
		fenceCounter.waiting++;
		while (fenceCounter.remaining > 0)
			fenceCounter.fenceReached.wait(lock);

		/*
		allow more threads to wait on the same FenceCounter
		(i.e. avoid multiple erase() calls with potentially invalid iterators...it happend the first time)
		*/

		fenceCounter.waiting--;
		if (fenceCounter.waiting == 0)
			m_fenceCounters.erase(it);
	}

	inline ThreadPool::FenceCounter::FenceCounter(uint64_t _remaining) : remaining{ _remaining } {}

	inline ThreadPool::Fence ThreadPool::addFenceUnsafe() {
		Fence targetFence = ++m_fenceValue;
		if (m_fenceCounters.find(targetFence) == m_fenceCounters.end())
			m_fenceCounters.emplace(targetFence, m_maxTaskConsumerCount);
		auto decrTask = [this, targetFence]() {
			decrementFence(targetFence);
		};
		for (size_t i = 0; i < m_maxTaskConsumerCount; i++)
			m_taskConsumers[i].addTask(decrTask);
		return targetFence;
	}

	inline void ThreadPool::decrementFence(Fence f) {
		std::lock_guard<std::mutex> lock{ m_mtx };
		auto it = m_fenceCounters.find(f);
#ifdef _DEBUG
		if (it == m_fenceCounters.end())
			throw std::runtime_error{ "Unexpected error : decrementing removed fence" };
#endif
		FenceCounter& fenceCounter = it->second;
		fenceCounter.remaining--;

		if (fenceCounter.remaining == 0)
			fenceCounter.fenceReached.notify_all(); //TODO: this should be done without lock
	}
}
#endif
