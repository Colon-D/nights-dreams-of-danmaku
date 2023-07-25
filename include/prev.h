#pragma once
#include "math_utils.h"
#include <entt/entt.hpp>

constexpr bool debug_disable_previous_lerp{ false };
constexpr bool debug_connect_debug_lerp{ true };

template<typename component>
struct prev {
	component value{};
	operator component() const {
		return value;
	}
};

class basic_previous_system {
public:
	basic_previous_system(entt::registry& registry);
	/// if prev<component> exists:
	///   returns lerp() of previous<component> and component
	/// else:
	///   returns component
	template<typename component>
	component try_lerp(
		const entt::entity entity, const float alpha
	) const {
		const auto& current = registry.get<component>(entity);
		if constexpr (debug_disable_previous_lerp) {
			return current;
		}
		if (
			const auto previous =
			registry.try_get<const ::prev<component>>(entity)
		) {
			return lerp(previous->value, current, alpha);
		}
		return current;
	}
protected:
	/// sets previous<component> to component for all entities
	template<typename component>
	void update_previous() {
		for (
			const auto e :
			registry.view<prev<component>, const component>()
		) {
			update_previous_static<component>(registry, e);
		}
	}

	/// calls update_previous<component>() on construct if previous<component>
	template<typename component>
	void update_previous_on_construct() {
		registry.on_construct<prev<component>>().connect<
			&basic_previous_system::update_previous_static<component>
		>();
	}
private:
	/// sets previous<component> to component for entity, this exists for
	/// update_previous_on_construct(), else use non-static function
	template<typename component>
	static void update_previous_static(
		entt::registry& registry, const entt::entity entity
	) {
		registry.replace<prev<component>>(
			entity, registry.get<const component>(entity)
		);
	}

	/// asserts that previous<transform> exists for a transform that moves
	static void debug_lerp(
		entt::registry& registry, const entt::entity entity
	);

	entt::registry& registry;
};

template<typename... components>
class previous_system : public basic_previous_system {
public:
	previous_system(entt::registry& registry) :
		basic_previous_system{ registry } {
		(update_previous_on_construct<components>(), ...);
	}

	/// sets previous<component> to component for all entities and components
	void update_previous() {
		(basic_previous_system::update_previous<components>(), ...);
	}
};
