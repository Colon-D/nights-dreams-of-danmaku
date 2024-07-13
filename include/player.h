#pragma once
#include <entt/entt.hpp>
#include "input.h"
#include "constants.h"
#include <SFML/System/Vector2.hpp>
#include <vector>

struct player {
	input input{};
	int ideya{ 5 };
	float last_hit{};
	// store 2.25s of paraloop points
	constexpr static std::size_t max_paraloop_points{
		static_cast<std::size_t>(fixed_framerate * 2.25)
	};
};

entt::entity get_closest_player(
	const entt::registry& ecs, const sf::Vector2f& pos
);

entt::entity get_furthest_player(
	const entt::registry& ecs, const sf::Vector2f& pos
);
