#pragma once

/**
 * @file Threading.hpp
 * @brief Threading header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace gust
{
	/**
	 * @class SimulationThread
	 * @brief A thread that repeats the same function when told to.
	 */
	class SimulationThread
	{
	public:

		/**
		 * @brief Constructor.
		 */
		SimulationThread();

		/**
		 * @brief Constructor.
		 * @param Function to run.
		 */
		SimulationThread(std::function<void()> func);

		/**
		 * @brief Destructor.
		 */
		~SimulationThread();

		/**
		 * @brief Start simulation.
		 */
		void start();

		/**
		 * @brief Wait for the simulation to finish.
		 */
		void wait();

	private:

		/** Thread. */
		std::thread m_thread;

		/** Function to run. */
		std::function<void()> m_function;

		/** Is the thread running? */
		bool m_running;
		std::mutex m_running_mutex;
		std::condition_variable m_running_condition;

		/** Is the thread stopping? */
		std::atomic_bool m_stopping;
	};

	/**
	 * @class ThreadPool
	 * @brief A group of threads that take jobs from a primary queue.
	 */
	class ThreadPool
	{
		friend class WorkerThread;

		/**
		 * @class WorkerThread
		 * @brief A thread in a thread pool.
		 */
		class WorkerThread
		{
		public:

			/**
			 * @brief Constructor.
			 * @param Thread pool the worker is in.
			 */
			WorkerThread(ThreadPool* pool);

			/**
			 * @brief Destructor.
			 */
			~WorkerThread();

		private:

			/** Thread pool the worker is in. */
			ThreadPool* m_threadPool;

			/** Should the worker be stopping? */
			std::atomic_bool m_stopping;

			/** Thread used by the worker. */
			std::thread m_thread;
		};

	public:

		/**
		 * @brief Default constructor.
		 */
		ThreadPool() = default;

		/**
		 * @brief Constructor.
		 * @param Thread count.
		 */
		ThreadPool(size_t threadCount);

		/**
		 * @brief Destructor.
		 */
		~ThreadPool();

		/**
		 * @brief Add a job.
		 * @param Job to add.
		 */
		inline void addJob(std::function<void()> job)
		{
			std::lock_guard<std::mutex> lock(m_jobs_mutex);
			m_jobQueue.push(std::move(job));
			m_condition.notify_one();
		}

		/**
		 * @brief Wait for the workers to finish working.
		 */
		void wait()
		{
			std::unique_lock<std::mutex> lock(m_jobs_mutex);
			m_condition.wait(lock, [this]() { return m_jobQueue.empty(); });
		}

	private:

		/** Worker threads. */
		std::vector<std::unique_ptr<WorkerThread>> m_workers;

		/** List of jobs in the job queue */
		std::queue<std::function<void()>> m_jobQueue;

		/** Job queue mutex. */
		std::mutex m_jobs_mutex;

		/** Condition to wait for. */
		std::condition_variable m_condition;
	};
}