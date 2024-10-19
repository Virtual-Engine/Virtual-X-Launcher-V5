#include "threads_pool.hpp"
#include <shellapi.h>

namespace VX {
	thread_pool g_thread_pool;

	bool thread_pool::initiliaze()
	{
		rescale_thread_pool();
		LOG(INFO) << "Threads Initialized";

		return true;
	}

	void thread_pool::rescale_thread_pool()
	{
		LOG(VERBOSE) << "Resizing thread pool from " << m_thread_pool.size() << " to " << m_allocated_thread_count;
		m_thread_pool.reserve(m_allocated_thread_count);

		if (m_thread_pool.size() < m_allocated_thread_count)
		{
			for (auto i = m_thread_pool.size(); i < m_allocated_thread_count; i++)
				m_thread_pool.emplace_back(std::thread(&thread_pool::run, this));
		}
	}

	void thread_pool::cleanup()
	{
		{
			std::unique_lock lock(m_lock);
			m_accept_jobs = false;
		}
		m_data_condition.notify_all();

		for (auto& thread : m_thread_pool)
			thread.join();

		m_thread_pool.clear();
	}

	void thread_pool::push(std::string name, std::function<void()> func, std::source_location location)
	{
		if (func)
		{
			{
				std::unique_lock lock(m_lock);
				m_job_stack.push({ name,  func, location });

				if (m_allocated_thread_count - m_busy_threads < m_job_stack.size()) [[unlikely]]
					{
						LOG(WARNING) << "Thread pool potentially starved, resizing to accommodate for load.";

						if (m_allocated_thread_count >= MAX_POOL_SIZE)
						{
							LOG(FATAL) << "The thread pool limit has been reached, whatever you did this should not occur in production.";
						}
						if (m_accept_jobs && m_allocated_thread_count + 1 <= MAX_POOL_SIZE)
						{
							++m_allocated_thread_count;
							rescale_thread_pool();
						}
					}
			}
			m_data_condition.notify_all();
		}
	}

	void thread_pool::run()
	{
		for (;;)
		{
			std::unique_lock lock(m_lock);
			m_data_condition.wait(lock, [this]() {
				return !m_job_stack.empty() || !m_accept_jobs;
				});

			if (!m_accept_jobs) [[unlikely]]
				break;
				if (m_job_stack.empty()) [[likely]]
					continue;

					thread_pool_job job = m_job_stack.top();
					m_job_stack.pop();
					lock.unlock();

					++m_busy_threads;

					try
					{
						std::invoke(job.m_func);
					}
					catch (const std::exception& e)
					{
						LOG(WARNING) << "Exception thrown while executing job in thread:" << std::endl << e.what();
					}

					--m_busy_threads;
		}

		LOG(VERBOSE) << "Thread " << std::this_thread::get_id() << " exiting...";
	}

	thread_pool* get_thread_pool() { return &g_thread_pool; }
}