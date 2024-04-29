/**
 * @file signal.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "signal.h"

#include <functional>

namespace sys {
signal::signal()
{
	m_predicate = false;
}
signal::~signal()
{

}

void signal::wait()
{
	std::unique_lock<std::mutex> lck(m_mutex);
	m_cv.wait(lck, std::bind(&signal::predicate, this));
	m_predicate = false;
}

bool signal::wait(int msec)
{
	std::unique_lock<std::mutex> lck(m_mutex);
	auto r = m_cv.wait_for(lck, std::chrono::milliseconds(msec), std::bind(&signal::predicate, this));
	m_predicate = false;
	if (!r) {
		return false;
	}
	return true;
}


void signal::notify()
{
	std::unique_lock<std::mutex> lck(m_mutex);
	m_predicate = true;
	m_cv.notify_all();
}


}
