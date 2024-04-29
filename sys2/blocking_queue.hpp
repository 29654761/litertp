/**
 * @file blocking_queue.hpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include <mutex>
#include <condition_variable>
#include <deque>

namespace sys {

template <typename T>
class blocking_queue
{
private:
    std::mutex    m_mutex;
    std::condition_variable m_condition;
    std::deque<T>           m_queue;
    bool m_cancel = false;
    size_t m_max_size=10000;
public:
    blocking_queue() {

    }
    blocking_queue(size_t max_size) {
        m_max_size = max_size;
    }
    ~blocking_queue() {
        close();
    }
    bool push(T const& value) 
    {
        size_t c = 0;
        {
            std::unique_lock<std::mutex> lock(this->m_mutex);
            c = m_queue.size();
            if (c >= m_max_size) {
                return false;
            }
            m_queue.push_front(value);
        }
        if (c == 0) {
            this->m_condition.notify_one();
        }
        return true;
    }
    bool pop(T& t) {
        std::unique_lock<std::mutex> lock(this->m_mutex);
        this->m_condition.wait(lock, [=] {
            if (m_cancel) {
                return true;
            }
            return !this->m_queue.empty();
            });

        if (m_cancel || m_queue.empty()) {
            return false;
        }

        t = std::move(this->m_queue.back());
        this->m_queue.pop_back();
        return true;
    }
    size_t size() {
        std::unique_lock<std::mutex> lock(this->m_mutex);
        return m_queue.size();
    }
    void reset() {
        m_cancel = false;
    }
    void clear() {
        std::unique_lock<std::mutex> lock(this->m_mutex);
        while (!m_queue.empty()) {
            m_queue.pop_back();
        }
    }
    void close()
    {
        m_cancel = true;
        this->m_condition.notify_one();
    }
};

}
