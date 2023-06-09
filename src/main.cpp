#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <entt/entt.hpp>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <random>
import angle;
import input;
import math_utils;

struct velocity : sf::Vector2f {
	velocity(const sf::Vector2f& vel = {}) : sf::Vector2f{ vel } {}
	using sf::Vector2f::Vector2f;
	using sf::Vector2f::operator=;
};

struct player {
	input input{};
};

struct facing {
	angle angle{};
};

struct gillwing {
	float timer{ 0.f };
	bool heading_right{ true };
};

struct danmaku {
};

struct rotate_towards_velocity {
	bool flip{ false };
};

struct gravity {
};

struct hitbox {
	float radius{};
};

struct disintegrate {
	float begin{};
};

entt::entity create_player(entt::registry& ecs, const sf::Vector2f& pos, const player& player, const sf::Texture& texture) {
	const auto player_e = ecs.create();
	ecs.emplace<::player>(player_e, player);

	sf::Sprite player_spr{ texture };
	player_spr.setOrigin(static_cast<sf::Vector2f>(texture.getSize()) / 2.f);
	player_spr.setPosition(pos);
	ecs.emplace<sf::Sprite>(player_e, player_spr);

	ecs.emplace<velocity>(player_e);
	ecs.emplace<facing>(player_e, angle::deg(-90.f));

	ecs.emplace<hitbox>(player_e, 5.f);

	return player_e;
}

entt::entity create_danmaku(entt::registry& ecs, sf::Shader& shdr, sf::Sprite spr, const sf::Vector2f& pos, const velocity& vel, const sf::Color& col) {
	const auto e = ecs.create();
	ecs.emplace<danmaku>(e);
	ecs.emplace<rotate_towards_velocity>(e);

	spr.setPosition(pos);
	spr.setColor(col);
	ecs.emplace<sf::Sprite>(e, spr);

	ecs.emplace<velocity>(e, vel);

	ecs.emplace<sf::Shader*>(e, &shdr);

	ecs.emplace<hitbox>(e, 5.f);

	return e;
}

// get closest player
entt::entity get_closest_player(
	const entt::registry& ecs, const sf::Vector2f& pos
) {
	entt::entity closest_player{ entt::null };
	for (const auto& [p, p_spr, _] : ecs.view<sf::Sprite, player>().each()) {
		if (closest_player == entt::null) {
			closest_player = p;
			continue;
		}
		const auto& closest_p_spr = ecs.get<sf::Sprite>(closest_player);
		const auto dist = pos - p_spr.getPosition();
		const auto closest_dist = pos - closest_p_spr.getPosition();
		if (mag(dist) < mag(closest_dist)) {
			closest_player = p;
		}
	}
	return closest_player;
}

// function to resize view
void resize_view(sf::RenderWindow& window, const sf::Vector2f& target_size) {
	const auto window_size =
		static_cast<sf::Vector2f>(window.getSize());
	const float window_ar{ window_size.x / window_size.y };
	const float target_ar{ target_size.x / target_size.y };
	sf::Vector2f size{};
	if (window_ar < target_ar) {
		size = {
			target_size.x,
			target_size.x / window_ar
		};
	}
	else {
		size = {
			target_size.y * window_ar,
			target_size.y
		};
	}
	window.setView(sf::View{ {}, size });
}

int main(int argc, char* argv[]) {
	if (!std::filesystem::exists("res/nights.png")) {
		std::cerr << "ERROR: \"./res/nights.png\" can not be found.\n";
		std::cerr << "Run executable in directory \"./\".";
		return EXIT_FAILURE;
	}

	sf::Texture nights_tex{};
	nights_tex.loadFromFile("res/nights.png");
	sf::Texture reala_tex{};
	reala_tex.loadFromFile("res/reala.png");
	sf::Texture danmaku_tex{};
	danmaku_tex.loadFromFile("res/danmaku.png");
	danmaku_tex.setSmooth(true);
	sf::Sprite danmaku_spr{ danmaku_tex };
	danmaku_spr.setOrigin({ 68.f, 63.5f });
	danmaku_spr.setColor(sf::Color::Red);
	sf::Texture gillwing_tex{};
	gillwing_tex.loadFromFile("res/gillwing.png");
	gillwing_tex.setSmooth(true);

	sf::Shader danmaku_shdr{};
	danmaku_shdr.loadFromFile("res/shaders/danmaku.frag", sf::Shader::Type::Fragment);

	sf::Shader disintegrate_shdr{};
	disintegrate_shdr.loadFromFile("res/shaders/disintegrate.frag", sf::Shader::Type::Fragment);

	sf::SoundBuffer scream_sfx{};
	scream_sfx.loadFromFile("res/scream.ogg");
	// todo: sound player
	sf::Sound sound{};
	sound.setBuffer(scream_sfx);
	sound.setVolume(4.f);

	std::default_random_engine rng{ std::random_device{}() };

	sf::RenderWindow window{ { 1280, 960 }, "NiGHTS: Dreams of Danmaku" };
	const sf::Vector2f target_size{ 640.f, 480.f };
	resize_view(window, target_size);

	entt::registry mut_ecs{};
	const entt::registry& ecs = mut_ecs;

	create_player(mut_ecs, { -100.f, 0.f }, { input::default_keyboard }, nights_tex);
	create_player(mut_ecs, {  100.f, 0.f }, { input::default_gamepad() }, reala_tex);

	// create gillwing
	const auto gillwing_e = mut_ecs.create();

	mut_ecs.emplace<gillwing>(gillwing_e);

	sf::Sprite gillwing_spr{ gillwing_tex };
	gillwing_spr.setOrigin({ 750.f, 300.f });
	constexpr float gillwing_scale{ 0.25f };
	gillwing_spr.setScale({ gillwing_scale, gillwing_scale });
	gillwing_spr.setPosition({ -500.f, -150.f });
	mut_ecs.emplace<sf::Sprite>(gillwing_e, gillwing_spr);

	mut_ecs.emplace<velocity>(gillwing_e);
	mut_ecs.emplace<rotate_towards_velocity>(gillwing_e, true);

	sf::Clock clock{};

	float elapsed{ 0.f };

	while (window.isOpen()) {
		sf::Event event{};
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::Resized:
				resize_view(window, target_size);
				break;
			}
		}

		const auto dt = clock.restart().asSeconds();
		elapsed += dt;
		danmaku_shdr.setUniform("time_elapsed", elapsed);

		// input

		// - face towards input

		for (const auto& [e, _, player] : ecs.view<facing, player>().each()) {
			mut_ecs.patch<facing>(e, [&](facing& facing) {
				const auto movement = player.input.movement();
				// if pushing the movement stick in a direction
				if (movement != sf::Vector2f{}) {
					// change angle to go towards stick direction
					const auto rot_vel = angle::deg(
						player.input.boost() ? 100.f : 400.f
					);
					const auto target_angle = angle::from_vec(player.input.movement());
					facing.angle = facing.angle.rotate_towards(target_angle, rot_vel * dt);
				}
				// if not pushing the movement stick in a direction
				else {
					// if boosting then continue for loop
					if (player.input.boost()) {
						return;
					}
					constexpr auto rot_vel = angle::deg(90.f);
					constexpr auto target_angle = angle::deg(-90.f);
					// turn upwards
					const auto diff = target_angle.diff(facing.angle);
					facing.angle = facing.angle.rotate_towards(target_angle, rot_vel * dt);
				}
			});
		}

		// - change speed with input

		for (const auto& [e, _, facing, player] : ecs.view<velocity, facing, player>().each()) {
			const auto movement_input = player.input.movement();
			const bool apply_movement_input{ movement_input != sf::Vector2f{} };

			float target_spd{ 0.f };
			float acceleration{ 350.f };
			if (player.input.boost()) {
				target_spd = 700.f;
				acceleration = 700.f;
			}
			else if (apply_movement_input) {
				target_spd = 350.f;
				constexpr bool analog_precision{ true };
				if constexpr (analog_precision) {
					target_spd *= mag(movement_input);
				}
			}

			mut_ecs.patch<velocity>(e, [&](velocity& vel) {
				// change vel's speed to go towards target_spd
				const float curr_spd{ mag(vel) };
				float spd{ curr_spd  };
				if (curr_spd < target_spd) {
					spd = std::min(curr_spd + acceleration * dt, target_spd);
				}
				else if (curr_spd > target_spd) {
					spd = std::max(curr_spd - acceleration * dt, target_spd);
				}
				vel = spd * normalized(vel);

				// set the velocity to be in direction that player is facing
				if (apply_movement_input || player.input.boost()) {
					vel = facing.angle.vec() * mag(vel);
				}
				// apply gravity
				else {
					const float gravity{ acceleration + 6.f };
					constexpr float terminal_velocity{ 12.f };
					if (vel.y < terminal_velocity) {
						vel.y = std::min(vel.y + gravity * dt, terminal_velocity);
					}
				}
			});
		}

		// gillwing
		for (const auto& [e, vel, spr, gw] : ecs.view<velocity, sf::Sprite, gillwing>().each()) {
			//// get closest player
			//const auto closest_player = get_closest_player(ecs, spr.getPosition());
			//if (closest_player == entt::null) {
			//	continue;
			//}
			//const auto& target_pos =
			//	ecs.get<sf::Sprite>(closest_player).getPosition();

			//// rotate velocity towards closest player
			//mut_ecs.patch<velocity>(e, [&](velocity& vel) {
			//	constexpr float speed{ 150.f };
			//	constexpr auto rot_vel = angle::deg(90.f);
			//	const auto target_angle =
			//		angle::from_vec(target_pos - spr.getPosition());
			//	auto angle = angle::from_vec(vel);
			//	angle = angle.rotate_towards(target_angle, rot_vel * dt);
			//	vel = speed * angle.vec();
			//});

			// change direction if on edge of view
			mut_ecs.patch<gillwing>(e, [&](gillwing& gw) {
				if (gw.heading_right) {
					if (spr.getPosition().x > target_size.x / 2.f) {
						gw.heading_right = false;
					}
				}
				else {
					if (spr.getPosition().x < -target_size.x / 2.f) {
						gw.heading_right = true;
					}
				}
			});

			// move in a sine wave pattern across the screen
			mut_ecs.patch<velocity>(e, [&](velocity& vel) {
				float horizontal_speed{ 200.f };
				constexpr float amplitude{ 50.f };
				constexpr float frequency{ 2.f };
				if (!gw.heading_right) {
					horizontal_speed = -horizontal_speed;
				}
				vel = sf::Vector2f{
					horizontal_speed,
					amplitude * std::cos(elapsed * frequency),
				};
			});

			// reduce gillwing timer and spawn danmaku
			mut_ecs.patch<gillwing>(e, [&](gillwing& gillwing) {
				constexpr float frequency{ 0.1f };
				gillwing.timer -= dt;
				if (gillwing.timer <= 0.f) {
					gillwing.timer += frequency;

					// shoot danmaku out in a random direction
					constexpr float square_radius{ 100.f };
					std::uniform_real_distribution dist{
						-square_radius, square_radius
					};
					std::uniform_int_distribution blue_dist{ 0, 255 };
					const velocity danmaku_vel{
						velocity{ dist(rng), dist(rng) }
					};
					const sf::Color danmaku_col{
						0, 255, static_cast<std::uint8_t>(blue_dist(rng))
					};
					const auto danmaku = create_danmaku(mut_ecs, danmaku_shdr, danmaku_spr, spr.getPosition(), danmaku_vel, danmaku_col);
					mut_ecs.emplace<gravity>(danmaku);
				}
			});
		}

		// apply standard gravity to entities
		for (const auto& [e, _] : ecs.view<gravity, velocity>().each()) {
			mut_ecs.patch<velocity>(e, [&](velocity& vel) {
				constexpr float gravity{ 256.f };
				vel.y += dt * gravity;
			});
		}

		// apply velocity to sprite's position
		for (const auto& [e, _, vel] : ecs.view<sf::Sprite, velocity>().each()) {
			mut_ecs.patch<sf::Sprite>(e, [&](sf::Sprite& spr) {
				spr.move(dt * vel);
			});
		}

		// erase any danmaku that are off-screen
		for (const auto& [e, spr] : ecs.view<danmaku, sf::Sprite>().each()) {
			const auto& pos = spr.getPosition();
			const auto& spr_size = spr.getScale();
			const auto& tex_size = static_cast<sf::Vector2f>(spr.getTexture()->getSize());
			const sf::Vector2f size{ spr_size.x * tex_size.x, spr_size.y * tex_size.y };
			const auto& view_pos = window.getView().getCenter();
			if (
				pos.x + size.x < view_pos.x - target_size.x / 2.f
				|| pos.x - size.x > view_pos.x + target_size.x / 2.f
				|| pos.y + size.y < view_pos.y - target_size.y / 2.f
				|| pos.y - size.y > view_pos.y + target_size.y / 2.f
			) {
				mut_ecs.destroy(e);
			}
		}

		// check for collisions between danmaku and players
		for (const auto& [d_e, d_spr, d_hit] : ecs.view<danmaku, sf::Sprite, hitbox>().each()) {
			for (const auto& [p_e, _, p_spr, p_hit] : ecs.view<player, sf::Sprite, hitbox>().each()) {
				// compare radius of hitboxes alongside position from sprite
				const auto& d_pos = d_spr.getPosition();
				const auto& p_pos = p_spr.getPosition();
				const auto dist = mag(d_pos - p_pos);
				if (dist < d_hit.radius + p_hit.radius) {
					// collision detected, disintegrate player
					mut_ecs.emplace<disintegrate>(p_e, elapsed);
					mut_ecs.emplace<sf::Shader*>(p_e, &disintegrate_shdr);
					mut_ecs.remove<hitbox>(p_e);
					mut_ecs.patch<player>(p_e, [&](player& p) { p.input = {}; });
					sound.play();
				}
			}
		}

		// set player sprite's direction to entity's direction
		for (const auto& [e, player, facing, _] : ecs.view<player, facing, sf::Sprite>().each()) {
			mut_ecs.patch<sf::Sprite>(e, [&](sf::Sprite& spr) {
				spr.setRotation(facing.angle.deg());
			});
		}

		// "flip" sprite if boosting
		for (const auto& [e, player, _] : ecs.view<player, sf::Sprite>().each()) {
			mut_ecs.patch<sf::Sprite>(e, [&](sf::Sprite& spr) {
				if (player.input.boost()) {
					spr.setScale({ 1.f, sin(elapsed * 32.f) });
				}
				else {
					spr.setScale({ 1.f, 1.f });
				}
			});
		}

		// rotate sprite to match velocity
		for (
			const auto& [e, rtv, vel, _] :
			ecs.view<rotate_towards_velocity, velocity, sf::Sprite>().each()
		) {
			mut_ecs.patch<sf::Sprite>(e, [&](sf::Sprite& spr) {
				const auto deg = fmod_pos(angle::from_vec(vel).deg(), 360.f);
				spr.setRotation(deg);
				if (rtv.flip) {
					const auto scale = spr.getScale();
					if (deg > 90.f && deg <= 270.f) {
						spr.setScale({ scale.x, -std::abs(scale.y) });
					}
					else {
						spr.setScale({ scale.x, std::abs(scale.y) });
					}
				}
			});
		}

		// delete after disintegration
		for (const auto& [e, dis] : ecs.view<disintegrate>().each()) {
			if (elapsed - dis.begin > 1.f) {
				mut_ecs.destroy(e);
			}
		}

		// render sprites
		window.clear();
		for (const auto& [e, spr] : ecs.view<sf::Sprite>().each()) {
			if (const auto shdr = ecs.try_get<sf::Shader*>(e)) {
				if (const auto dis = ecs.try_get<disintegrate>(e)) {
					mut_ecs.patch<sf::Shader*>(e, [&](sf::Shader* shdr) {
						shdr->setUniform("disintegration", elapsed - dis->begin);
					});
				}
				window.draw(spr, *shdr);
			}
			else {
				window.draw(spr);
			}
		}
		window.display();

	}

	return EXIT_SUCCESS;
}
