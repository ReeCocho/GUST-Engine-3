#include "Debugging.hpp"
#include "Threading.hpp"

namespace gust
{
	SimulationThread::SimulationThread() : m_running(false), m_stopping(false)
	{
		// Spawn thread
		m_thread = std::thread
		(
			[this]()
			{
				// While we aren't stopping...
				while (!m_stopping)
				{
					// Wait until we need to run the function
					{
						std::unique_lock<std::mutex> lock(m_running_mutex);
						m_running_condition.wait(lock, [this] { return m_running; });
					}

					// Run it.
					m_function();

					{
						std::unique_lock<std::mutex> lock(m_running_mutex);
						m_running = false;
						m_running_condition.notify_one();
					}
				}
			}
		);
	}

	SimulationThread::SimulationThread(std::function<void()> func) : SimulationThread()
	{
		m_function = std::move(func);
	}

	SimulationThread::~SimulationThread()
	{
		m_stopping = true;
		m_thread.join();
	}

	void SimulationThread::start()
	{
		wait();

		std::unique_lock<std::mutex> lock(m_running_mutex);
		m_running = true;
		m_running_condition.notify_one();
	}

	void SimulationThread::wait()
	{
		std::unique_lock<std::mutex> lock(m_running_mutex);
		m_running_condition.wait(lock, [this]() { return !m_running; });
	}



	ThreadPool::ThreadPool(size_t threadCount)
	{
		m_workers.resize(threadCount);
		for (size_t i = 0; i < threadCount; i++)
			m_workers[i] = std::make_unique<WorkerThread>(this);
	}

	ThreadPool::~ThreadPool()
	{
		wait();
	}

	ThreadPool::WorkerThread::WorkerThread(ThreadPool* pool) : m_threadPool(pool), m_stopping(false)
	{
		m_thread = std::thread
		([this]()
		{
			// While we aren't stopping the worker...
			while (!m_stopping)
			{
				// Find a job in the job queue
				std::function<void(void)> job;
				{
					// Wait until the job queue isn't empty
					std::unique_lock<std::mutex> lock(m_threadPool->m_jobs_mutex);
					m_threadPool->m_condition.wait(lock, [this] { return !m_threadPool->m_jobQueue.empty() || m_stopping; });

					// Make sure we can stop mid loop
					if (m_stopping)
						break;

					// Take a job out of the queue
					job = m_threadPool->m_jobQueue.front();
					m_threadPool->m_jobQueue.pop();
					m_threadPool->m_condition.notify_one();
				}

				// Run the job
				job();
			}
		});
	}

	ThreadPool::WorkerThread::~WorkerThread()
	{
		{
			std::unique_lock<std::mutex> lock(m_threadPool->m_jobs_mutex);
			m_stopping = true;
			m_threadPool->m_condition.notify_one();
		}

		m_thread.join();
	}
}