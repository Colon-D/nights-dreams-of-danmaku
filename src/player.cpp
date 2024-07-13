#include "player.h"
#include "services.h"

entt::entity get_closest_player(
	const entt::registry& ecs, const sf::Vector2f& pos
) {
	entt::entity closest_player{ entt::null };
	for (const auto& [p, p_t, _] : ecs.view<transform, player>().each()) {
		if (closest_player == entt::null) {
			closest_player = p;
			continue;
		}
		const auto& closest_p_t = ecs.get<transform>(closest_player);
		const auto dist = pos - p_t.pos;
		const auto closest_dist = pos - closest_p_t.pos;
		if (mag(dist) < mag(closest_dist)) {
			closest_player = p;
		}
	}
	return closest_player;
}

entt::entity get_furthest_player(
	const entt::registry& ecs, const sf::Vector2f& pos
) {
	entt::entity furthest_player{ entt::null };
	for (const auto& [p, p_t, _] : ecs.view<transform, player>().each()) {
		if (furthest_player == entt::null) {
			furthest_player = p;
			continue;
		}
		const auto& furthest_p_t = ecs.get<transform>(furthest_player);
		const auto dist = pos - p_t.pos;
		const auto furthest_dist = pos - furthest_p_t.pos;
		if (mag(dist) > mag(furthest_dist)) {
			furthest_player = p;
		}
	}
	return furthest_player;
}
