#pragma once
#include "../../common.hpp"
#include <source_location>

namespace VX {

	constexpr auto MAX_POOL_SIZE = 32u;

	struct thread_pool_job
	{
		std::string name;
		std::function<void()> m_func;
		std::source_location m_source_location;
	};

	class thread_pool
	{

	public:
		bool initiliaze();
		void cleanup();

	public:
		std::atomic<bool> m_accept_jobs = true;
		std::condition_variable m_data_condition;

		std::stack<thread_pool_job> m_job_stack;
		std::mutex m_lock;
		std::vector<std::thread> m_thread_pool;

		std::atomic<size_t> m_allocated_thread_count = 4;
		std::atomic<size_t> m_busy_threads = 0;

	public:
		void push(std::string name, std::function<void()> func, std::source_location location = std::source_location::current());

		std::pair<size_t, size_t> Usage() const
		{
			return { m_busy_threads, m_allocated_thread_count };
		}

	private:
		void run();
		void rescale_thread_pool();
	};

	thread_pool* get_thread_pool();
}