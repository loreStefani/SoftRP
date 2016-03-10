#ifndef SOFTRP_TASK_CONSUMER_IMPL_INL_
#define SOFTRP_TASK_CONSUMER_IMPL_INL_
#include "TaskConsumer.h"
namespace SoftRP {

	inline TaskConsumer::~TaskConsumer() {
		terminate();
		t.join();
	}

	inline TaskConsumer::TaskConsumer(TaskConsumer&& tc) {
		std::lock_guard<std::mutex> lock{ tc.m_mtx };
		move(std::move(tc));
	}

	inline TaskConsumer& TaskConsumer::operator=(TaskConsumer&& tc) {
		if (&tc == this)
			return *this;

		std::unique_lock<std::mutex> thisLock{ m_mtx, std::defer_lock };
		std::unique_lock<std::mutex> tcLock{ tc.m_mtx, std::defer_lock };
		std::lock(thisLock, tcLock);
		move(std::move(tc));

		return *this;
	}

	inline void TaskConsumer::addTask(TaskType task) {
		{
			std::lock_guard<std::mutex> lock{ m_mtx };
			if (!m_terminate)
				m_tasks.push(std::move(task));
		}
		m_availableTasks.notify_one();
	}

	inline bool TaskConsumer::hasTasks() {
		std::lock_guard<std::mutex> lock{ m_mtx };
		return m_tasks.size() != 0 || m_executing;
	}

	inline void TaskConsumer::waitForConsumation() {
		std::unique_lock<std::mutex> lock{ m_mtx };
		while (m_tasks.size() != 0 || m_executing)
			m_consumedTask.wait(lock);
	}

	inline void TaskConsumer::terminate() {
		{
			std::lock_guard<std::mutex> lock{ m_mtx };
			m_terminate = true;
		}
		m_availableTasks.notify_one();
	}

	inline void TaskConsumer::operator() () {

		std::unique_lock<std::mutex> lock{ m_mtx };

		while (!m_terminate) {

			while (m_tasks.size() == 0) {
				m_availableTasks.wait(lock);
				if (m_terminate)
					return;
			}
			/*
			moving the task is important; keeping a reference
			might cause problems (like in the previous versions).
			Because the lock is released during execution, if a new task is added (by a new thread)
			the tasks could be moved in an other memory area by m_tasks. This
			causes the concurrent execution of the task and its destructor.
			*/
			TaskType task{ std::move(m_tasks.front()) };
			m_tasks.pop();
			m_executing = true;
			lock.unlock();
			task();
			lock.lock();
			m_executing = false;
			if (m_tasks.size() == 0) {
				lock.unlock();
				m_consumedTask.notify_all();
				lock.lock();
			}
		}
	}

	inline void TaskConsumer::move(TaskConsumer&& tc) {
		m_tasks = std::move(tc.m_tasks);
		m_terminate = tc.m_terminate;
	}

}
#endif