/**
 * @file callback.hpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */




#pragma once

#include <vector>
#include <algorithm>

namespace sys {

template <class TFun>
class callback
{
public:
	struct callback_item_st
	{
		TFun fun;
		void* ctx;
	};
private:
	std::vector<callback_item_st> items_;
public:

	bool add(TFun fun, void* ctx) {
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
		auto iter = std::find_if(items_.begin(), items_.end(), [fun, ctx](const callback_item_st& it) {
			return it.fun == fun && it.ctx == ctx;
			});

		if (iter != items_.end()) {
			items_.erase(iter);
		}
	}

	void clear() {
		items_.clear();
	}


	template<class ...Args>
	void invoke(Args... args) {
		for (auto itr = items_.begin(); itr != items_.end(); itr++) {
			itr->fun(itr->ctx, args...);
		}
	}
};

}
