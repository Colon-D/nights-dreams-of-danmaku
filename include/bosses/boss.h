#pragma once
#include <entt/entt.hpp>
#include <queue>
#include <functional>
#include "signals.h"

struct boss {
	boss(const std::queue<std::function<void()>>& phase_queue);

	void change_phase(const entt::handle handle);

	// changes between phases (ie construct new attacks, old should deconstruct self through signal)
	// alternatively, each boss could connect to the phase change signal and it could have the phase number as an argument?
	std::queue<std::function<void()>> phase_queue{};

	signal<void(entt::handle)> on_phase_change{};

	float get_health() const;
	/// calls on_phase_change if health <= 0.f
	// todo: should this be static?
	void set_health(entt::handle handle, float health);
private:
	// called in ctor
	void change_phase_no_event();

	float health{ 100.f };
};
