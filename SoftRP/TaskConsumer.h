#ifndef SOFTRP_TASK_CONSUMER_H_
#define SOFTRP_TASK_CONSUMER_H_
#include<queue>
#include<mutex>
#include<thread>
#include<functional>
namespace SoftRP {

	/*
	Concrete data type which represents a thread that executes tasks submitted in 
	a common producer-consumer fashion.
	*/

	class TaskConsumer {
	public:
		using TaskType = std::function<void(void)>;

		TaskConsumer() = default;		
		~TaskConsumer();
		
		//copy
		TaskConsumer(const TaskConsumer&) = delete;
		TaskConsumer& operator=(const TaskConsumer&) = delete;
		//move
		TaskConsumer(TaskConsumer&& tc);		
		TaskConsumer& operator=(TaskConsumer&& tc);

		//submit a new task for execution
		void addTask(TaskType task);
		//are there pending tasks?
		bool hasTasks();
		//block the calling thread until all task have been executed
		void waitForConsumation();
		//terminate the thread when all the pending tasks have been executed
		void terminate();
		
	private:

		void operator()();
		void move(TaskConsumer&& tc);

		bool m_terminate{false};
		bool m_executing{ false };
		std::mutex m_mtx{};
		std::condition_variable m_availableTasks{};
		std::condition_variable m_consumedTask{};		
		std::queue<TaskType> m_tasks{};
		std::thread t{ &TaskConsumer::operator(), this };
	};
}
#include "TaskConsumerImpl.inl"
#endif
