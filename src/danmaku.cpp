#include "danmaku.h"
#include "services.h"

entt::entity create_danmaku(
	entt::registry& ecs,
	const sf::Vector2f& pos,
	const velocity& vel,
	const sf::Color& col
) {
	const auto e = ecs.create();
	ecs.emplace<danmaku>(e);
	ecs.emplace<rotate_towards_velocity>(e);

	auto spr = serv->texture.sprite("danmaku");
	spr.color = col;
	ecs.emplace<sprite>(e, spr);
	const transform trans{ pos };
	ecs.emplace<transform>(e, trans);
	ecs.emplace<prev<transform>>(e, trans);

	ecs.emplace<velocity>(e, vel);

	ecs.emplace<sf::Shader*>(e, serv->shaders.map.at("danmaku").get());

	ecs.emplace<hitbox>(e, 5.f);
	ecs.emplace<painful>(e);

	return e;
}
