#include "attacks/gillwing/wave.h"
#include "components.h"
#include "constants.h"
#include "services.h"
#include "danmaku.h"
#include "player.h"

void wave_movement::update(scene_services& scn) {
	auto& mut_ecs = scn.registry;
	const auto& ecs = mut_ecs;

	for (const auto& [e, w, t] : ecs.view<wave_movement, transform>().each()) {
		mut_ecs.patch<wave_movement>(e, [&](wave_movement& w) {
			w.elapsed += fixed_timestep;
			
			// if offscreen, appear at player y
			if (!w.moving_right) {
				if (t.pos.x < -gameplay_size.x - 256.f) {
					mut_ecs.patch<transform>(e, [&](transform& t) {
						t.pos.x = -gameplay_size.x - 256.f;
						const auto furthest = get_furthest_player(ecs, t.pos);
						if (furthest != entt::null) {
							t.pos.y = ecs.get<transform>(furthest).pos.y;
						}
					});
					w.moving_right = true;
				}
			}
			else {
				if (t.pos.x > gameplay_size.x + 256.f) {
					mut_ecs.patch<transform>(e, [&](transform& t) {
						t.pos.x = gameplay_size.x + 256.f;
						const auto furthest = get_furthest_player(ecs, t.pos);
						if (furthest != entt::null) {
							t.pos.y = ecs.get<transform>(furthest).pos.y;
						}
					});
					w.moving_right = false;
				}
			}
		});

		// move in a sine wave pattern across the screen
		mut_ecs.patch<velocity>(e, [&](velocity& vel) {
			// move faster the further offscreen you are
			const float offscreen = std::max(0.f, std::abs(t.pos.x) - gameplay_size.x / 2.f);
			float horizontal_speed{ (offscreen + 196.f) * diff };
			const float amplitude{ 64.f * diff };
			const float frequency{ 2.4f * diff };
			if (!w.moving_right) {
				horizontal_speed = -horizontal_speed;
			}
			vel = sf::Vector2f{
				horizontal_speed,
				amplitude * std::cos(w.elapsed * frequency),
			};
		});
	}
}

void wave_danmaku::update(scene_services& scn) {
	auto& mut_ecs = scn.registry;
	const auto& ecs = mut_ecs;

	for (const auto& [e, w, v, t] : ecs.view<wave_danmaku, velocity, transform>().each()) {
		mut_ecs.patch<wave_danmaku>(e, [&](wave_danmaku& w) {
			w.reload -= fixed_timestep;
			if (w.reload < 0.f) {
				w.reload += 0.2f / (diff * diff);

				// shoot a danmaku out perpendicular to velocity

				// get perpendicular vector
				sf::Vector2f perp{ -v.y, v.x };
				// normalize it
				perp = normalized(perp);
				// scale it
				perp *= 32.f + 16.f * diff;

				const auto color = w.alt_color ? sf::Color{ 0x004A7FFF } : sf::Color{0x007F0EFF};
				w.alt_color = !w.alt_color;
				const auto danmaku_e = create_danmaku(mut_ecs, t.pos, -perp / 4.f, color);
				mut_ecs.emplace<acceleration>(danmaku_e, perp);
				const auto danmaku_e2 = create_danmaku(mut_ecs, t.pos, perp / 4.f, color);
				mut_ecs.emplace<acceleration>(danmaku_e2, -perp);
			}
		});
	}
}
