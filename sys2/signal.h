/**
 * @file signal.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */



#pragma once
#include <mutex>
#include <condition_variable>

namespace sys {

class signal
{
private:
	std::mutex m_mutex;
	std::condition_variable m_cv;
	bool m_predicate;

public:
	signal();
	~signal();

	void wait();
	bool wait(int msec);
	void notify();

	void reset() {
		m_predicate = false;
	}
private:
	bool predicate() {
		return m_predicate;
	}

};

}
