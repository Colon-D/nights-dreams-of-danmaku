#pragma once
#include "prev.h"
#include "components.h"
#include "resource/texture.h"
#include <entt/entt.hpp>

// erased offscreen
struct danmaku {
};

entt::entity create_danmaku(
	entt::registry& ecs,
	const sf::Vector2f& pos,
	const velocity& vel,
	const sf::Color& col = sf::Color::White
);
