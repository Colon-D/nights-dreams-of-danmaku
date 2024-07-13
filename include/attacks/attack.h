#pragma once
#include <entt/entt.hpp>
#include "bosses/boss.h"
#include "services.h"

struct attack_system {
	// non copyable/moveable (update_sigh shouldn't be moved)
	attack_system(const attack_system&) = delete;
	attack_system(attack_system&&) = delete;
	attack_system& operator=(const attack_system&) = delete;
	attack_system& operator=(attack_system&&) = delete;

	static entt::sigh<void(scene_services&)> update_sigh;
	static entt::sink<decltype(update_sigh)> on_update;
};

template<typename attack_type>
struct basic_attack {
	basic_attack(entt::handle h) {
		attack_system::on_update.connect<&attack_type::update>();

		if (auto* const b = h.try_get<boss>()) {
			b->on_phase_change.sink.connect<&attack_type::erase>(this);
		}
		h.registry()->on_destroy<attack_type>().connect<&attack_type::disconnect>(this);
	}

	void erase(const entt::handle h) {
		h.erase<attack_type>();
	}

	void disconnect(entt::registry& ecs, const entt::entity e) {
		if (auto* const b = ecs.try_get<boss>(e)) {
			b->on_phase_change.sink.disconnect<&attack_type::erase>(this);
		}
		ecs.on_destroy<attack_type>().disconnect<&attack_type::disconnect>(this);
	}
};
