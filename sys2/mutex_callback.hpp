
/**
 * @file mutex_callback.hpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <algorithm>

namespace sys {

template <class TFun>
class mutex_callback
{
public:
	struct callback_item_st
	{
		TFun fun;
		void* ctx;
	};
private:
	std::shared_mutex mutex_;
	std::vector<callback_item_st> items_;
public:

	bool add(TFun fun, void* ctx) {
		std::unique_lock<std::shared_mutex> g(mutex_);
		auto iter = std::find_if(items_.begin(), items_.end(), [fun, ctx](const callback_item_st& it) {
			return it.fun == fun && it.ctx == ctx;
			});

		if (iter != items_.end()) {
			return false;
		}

		callback_item_st it;
		it.fun = fun;
		it.ctx = ctx;


		items_.push_back(it);
		return true;
	}

	void remove(TFun fun, void* ctx) {
		std::unique_lock<std::shared_mutex> g(mutex_);
		auto iter = std::find_if(items_.begin(), items_.end(), [fun, ctx](const callback_item_st& it) {
			return it.fun == fun && it.ctx == ctx;
			});

		if (iter != items_.end()) {
			items_.erase(iter);
		}
	}

	void clear() {
		std::unique_lock<std::shared_mutex> g(mutex_);
		items_.clear();
	}

	void clone(std::vector<callback_item_st>& items) {
		std::shared_lock<std::shared_mutex> g(mutex_);
		items = items_;
	}

	template<class ...Args>
	void invoke(Args... args) {
		std::shared_lock<std::shared_mutex> g(mutex_);
		
		for (auto itr = items_.begin(); itr != items_.end(); itr++) {
			itr->fun(itr->ctx, args...);
		}
	}
};

}
