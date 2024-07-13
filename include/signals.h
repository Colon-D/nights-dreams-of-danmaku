#pragma once
#include <entt/entt.hpp>

// signal handler and sink, with move semantics
template<typename signature>
class signal {
public:
	signal() = default;
	signal(const signal& signal) = delete;
	signal(signal&& signal) {
		sink = sigh;
	}
	signal& operator=(const signal& signal) = delete;
	signal& operator=(signal&& signal) {
		sink = sigh;
		return *this;
	}

	entt::sigh<signature> sigh{};
	entt::sink<decltype(sigh)> sink{ sigh };
};
