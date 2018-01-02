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
				while (true)
				{
					// Wait until we need to run the function
					{
						std::unique_lock<std::mutex> lock(m_running_mutex);
						m_running_condition.wait(lock, [this] { return m_running || m_stopping; });

						if (m_stopping)
							break;
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
		{
			std::unique_lock<std::mutex> lock(m_running_mutex);
			m_stopping = true;
			m_running_condition.notify_one();
		}

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



	WorkerThread::WorkerThread()
	{
		m_thread = std::thread(&WorkerThread::work, this);
	}

	WorkerThread::~WorkerThread()
	{
		if (m_thread.joinable())
		{
			wait();

			{
				std::lock_guard<std::mutex> lock(m_jobs_mutex);
				m_destroying = true;
				m_condition.notify_one();
			}

			m_thread.join();
		}
	}

	void WorkerThread::addJob(std::function<void(void)> job)
	{
		std::lock_guard<std::mutex> lock(m_jobs_mutex);
		m_jobs.push(move(job));
		m_condition.notify_one();
	}

	void WorkerThread::wait()
	{
		std::unique_lock<std::mutex> lock(m_jobs_mutex);
		m_condition.wait(lock, [this]() { return m_jobs.empty(); });
	}

	void WorkerThread::work()
	{
		while (true)
		{
			std::function<void(void)> job;
			{
				std::unique_lock<std::mutex> lock(m_jobs_mutex);
				m_condition.wait(lock, [this] { return !m_jobs.empty() || m_destroying; });

				if (m_destroying)
					break;

				job = m_jobs.front();
			}

			job();

			{
				std::lock_guard<std::mutex> lock(m_jobs_mutex);
				m_jobs.pop();
				m_condition.notify_one();
			}
		}
	}



	ThreadPool::ThreadPool()
	{
		
	}

	ThreadPool::ThreadPool(size_t threadCount)
	{
		workers.resize(threadCount);
		for (size_t i = 0; i < threadCount; i++)
			workers[i] = new WorkerThread();
	}

	ThreadPool::~ThreadPool()
	{
		wait();

		for (auto worker : workers)
			delete worker;
	}

	void ThreadPool::wait()
	{
		for (auto worker : workers)
			worker->wait();
	}
}