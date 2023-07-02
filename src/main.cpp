#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <entt/entt.hpp>
#include <filesystem>
#include <format>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>
#include <memory>
#include <random>
#include <array>
import angle;
import input;
import math_utils;
import audio;
import texture;
import ui;

struct velocity : sf::Vector2f {
	velocity(const sf::Vector2f& vel = {}) : sf::Vector2f{ vel } {}
	using sf::Vector2f::Vector2f;
	using sf::Vector2f::operator=;
};

struct player {
	input input{};
	int ideya{ 5 };
	float last_hit{};
};

struct facing {
	angle angle{};
};

struct gillwing {
	float timer{ 0.f };
	bool heading_right{ true };
};

// erased offscreen
struct danmaku {
};

// hurts the player
struct painful {
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

entt::entity create_player(entt::registry& ecs, const sf::Vector2f& pos, const player& player, sf::Sprite spr) {
	const auto player_e = ecs.create();
	ecs.emplace<::player>(player_e, player);

	spr.setPosition(pos);
	ecs.emplace<sf::Sprite>(player_e, spr);

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
	ecs.emplace<painful>(e);

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

int main(int argc, char* argv[]) {
	if (!std::filesystem::exists("res/nights.png")) {
		std::cerr << "ERROR: \"./res/nights.png\" can not be found.\n";
		std::cerr << "Run executable in directory \"./\".";
		return EXIT_FAILURE;
	}

	texture texture{};
	texture.load(".png");

	audio audio{};
	audio.load(".ogg");

	auto danmaku_spr = texture.sprite("danmaku");
	danmaku_spr.setOrigin({ 68.f, 63.5f });
	danmaku_spr.setColor(sf::Color::Red);

	sf::Shader danmaku_shdr{};
	danmaku_shdr.loadFromFile("res/shaders/danmaku.frag", sf::Shader::Type::Fragment);
	sf::Shader disintegrate_shdr{};
	disintegrate_shdr.loadFromFile("res/shaders/disintegrate.frag", sf::Shader::Type::Fragment);
	sf::Shader ideya_shdr{};
	ideya_shdr.loadFromFile("res/shaders/ideya.frag", sf::Shader::Type::Fragment);

	std::default_random_engine rng{ std::random_device{}() };

	sf::RenderWindow window{ { 1280, 720 }, "NiGHTS: Dreams of Danmaku" };
	sf::View gameplay_view{ {}, { 640.f, 480.f } };

	ImGui::SFML::Init(window);
	auto& imgui_io = ImGui::GetIO();
	imgui_io.IniFilename = nullptr;
	// default style (not reference or mutable as copy is used for scaling)
	{
		auto& style = ImGui::GetStyle();
		auto& colors = style.Colors;
		colors[ImGuiCol_WindowBg]       = sf::Color::Transparent;
		colors[ImGuiCol_Button]         = sf::Color{ 0xFF31B5FF };
		colors[ImGuiCol_ButtonHovered]  = sf::Color{ 0xFF31B57F };
		colors[ImGuiCol_ButtonActive]   = sf::Color{ 0xFF31B53F };
		colors[ImGuiCol_CheckMark]      = sf::Color{ 0xFFFFFFFF };
		colors[ImGuiCol_FrameBg]        = sf::Color{ 0xFF31B5FF };
		colors[ImGuiCol_FrameBgHovered] = sf::Color{ 0xFF31B57F };
		colors[ImGuiCol_FrameBgActive]  = sf::Color{ 0xFF31B53F };
		style.WindowBorderSize = 0.f;
	}

	const auto imgui_style = ImGui::GetStyle();

	entt::registry mut_ecs{};
	const entt::registry& ecs = mut_ecs;

	create_player(mut_ecs, { -100.f, 0.f }, { input::default_keyboard }, texture.sprite("nights"));
	create_player(mut_ecs, {  100.f, 0.f }, { input::default_gamepad() }, texture.sprite("reala"));

	// create gillwing
	const auto gillwing_e = mut_ecs.create();

	mut_ecs.emplace<gillwing>(gillwing_e);

	sf::Sprite gillwing_spr{ texture.sprite("gillwing") };
	gillwing_spr.setOrigin({ 750.f, 300.f });
	constexpr float gillwing_scale{ 0.25f };
	gillwing_spr.setScale({ gillwing_scale, gillwing_scale });
	gillwing_spr.setPosition({ -500.f, -150.f });
	mut_ecs.emplace<sf::Sprite>(gillwing_e, gillwing_spr);

	mut_ecs.emplace<velocity>(gillwing_e);
	mut_ecs.emplace<rotate_towards_velocity>(gillwing_e, true);

	mut_ecs.emplace<hitbox>(gillwing_e, 52.f);
	mut_ecs.emplace<painful>(gillwing_e);

	sf::Clock clock{};
	float accumulator{ 0.f };

	float elapsed{ 0.f };

	bool debug_visible_hitboxes{ false };

	// create ui
	center_and_scale_ui ui{};
	leaf_ui* gameplay_ui{};
	leaf_ui* imgui_ui{};
	{
		auto gameplay_ui_owned = std::make_unique<leaf_ui>();
		gameplay_ui = gameplay_ui_owned.get();
		gameplay_ui->size = gameplay_view.getSize();
		gameplay_ui->draw_fn = [&](sf::RenderTarget& target) {
			// replaced later
		};
		auto gameplay_margin_ui = std::make_unique<margin_ui>();
		gameplay_margin_ui->child = std::move(gameplay_ui_owned);
		gameplay_margin_ui->margin = 8.f;

		auto imgui_ui_owned = std::make_unique<leaf_ui>();
		imgui_ui = imgui_ui_owned.get();
		imgui_ui->size = { 192.f, gameplay_view.getSize().y };
		imgui_ui->draw_fn = [&](sf::RenderTarget& target) {
			auto logo_sprite = texture.sprite("logo");
			const auto& sprite_size = logo_sprite.getTexture()->getSize();
			const auto scale_down = imgui_ui->size.x / sprite_size.x;
			logo_sprite.scale({ scale_down, scale_down });
			logo_sprite.setPosition({
				0.f,
				imgui_ui->size.y / 2.f - scale_down * sprite_size.y / 2.f
			});
			target.draw(logo_sprite);
			// the rest of the ui is drawn by imgui
		};
		auto imgui_margin_ui = std::make_unique<margin_ui>();
		imgui_margin_ui->child = std::move(imgui_ui_owned);
		imgui_margin_ui->margin = 8.f;

		auto row_ui = std::make_unique<::row_ui>();
		row_ui->children.push_back(std::move(gameplay_margin_ui));
		row_ui->children.push_back(std::move(imgui_margin_ui));

		ui.child = std::move(row_ui);

	}

	const auto resized_window = [&]() {
		// resize ui
		ui.viewport_area = {
			{ 0.f, 0.f }, static_cast<sf::Vector2f>(window.getSize())
		};
		ui.update();
		// resize imgui
		auto imgui_new_style = imgui_style;
		imgui_new_style.ScaleAllSizes(imgui_ui->scale);
		ImGui::GetStyle() = imgui_new_style;
		imgui_io.Fonts->Clear();
		imgui_io.Fonts->AddFontFromFileTTF(
			"res/FiraSans-Regular.ttf",
			16.f * imgui_ui->scale
		);
		ImGui::SFML::UpdateFontTexture();
	};
	resized_window();

	while (window.isOpen()) {
		sf::Event event {};
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(window, event);
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::Resized:
				resized_window();
				break;
			}
		}

		const auto dt_time = clock.restart();
		ImGui::SFML::Update(window, dt_time);
		const auto var_dt = dt_time.asSeconds();
		elapsed += var_dt;

		accumulator += var_dt;
		constexpr float dt = 1.f / 20.f;
		if (accumulator > dt) {
			accumulator = std::fmod(accumulator, dt);

			// FIXED TIMESTEP
			audio.update();

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
					float spd{ curr_spd };
					if (curr_spd < target_spd) {
						spd = std::min(curr_spd + acceleration * dt, target_spd);
					}
					else if (curr_spd > target_spd) {
						spd = std::max(curr_spd - acceleration * dt, target_spd);
					}
					vel = spd * (curr_spd
						? normalized(vel)
						: sf::Vector2f{ 0.f, -1.f });

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
						if (spr.getPosition().x > gameplay_view.getSize().x / 2.f) {
							gw.heading_right = false;
						}
					}
					else {
						if (spr.getPosition().x < -gameplay_view.getSize().x / 2.f) {
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

			// clamp player in bounds by sliding/bouncing off walls,
			// recalculating their position, and often their velocity and facing
			for (
				const auto& [e, hit, spr, vel, facing, player] :
				ecs.view<hitbox, sf::Sprite, velocity, facing, player>().each()
			) {
				auto pos = spr.getPosition();

				const auto boost = player.input.boost();
				const auto movement = player.input.movement() != sf::Vector2f{};
				auto new_vel = vel;
				auto new_facing = facing;
				const auto rot_vel = angle::deg(800.f) * dt;
				auto rotate_towards_closest_tangent = [](
					const angle& angle,
					const ::angle& tangent,
					const ::angle& rot_vel
					) {
						const auto tangent2 = tangent + angle::deg(180.f);
						const auto closest_tangent =
							angle.diff(tangent).abs() < angle.diff(tangent2).abs()
							? tangent
							: tangent2;
						return angle.rotate_towards(closest_tangent, rot_vel);
					};
				bool oob{ false };
				// if oob, slide/bounce off wall
				if (pos.x - hit.radius < -gameplay_view.getSize().x / 2.f) {
					pos.x = -gameplay_view.getSize().x / 2.f + hit.radius;
					if (boost) {
						new_vel.x = -vel.x;
						new_facing.angle = angle::deg(180.f) - new_facing.angle;
					}
					else {
						new_facing.angle = rotate_towards_closest_tangent(
							facing.angle, angle::deg(90.f), rot_vel
						);
					}
					oob = true;
				}
				else if (pos.x + hit.radius > gameplay_view.getSize().x / 2.f) {
					pos.x = gameplay_view.getSize().x / 2.f - hit.radius;
					if (boost) {
						new_vel.x = -vel.x;
						new_facing.angle = angle::deg(180.f) - new_facing.angle;
					}
					else {
						new_facing.angle = rotate_towards_closest_tangent(
							facing.angle, angle::deg(90.f), rot_vel
						);
					}
					oob = true;
				}
				if (pos.y - hit.radius < -gameplay_view.getSize().y / 2.f) {
					pos.y = -gameplay_view.getSize().y / 2.f + hit.radius;
					if (boost) {
						new_vel.y = -vel.y;
						new_facing.angle = -new_facing.angle;
					}
					else {
						new_facing.angle = rotate_towards_closest_tangent(
							facing.angle, angle::deg(0.f), rot_vel
						);
					}
					oob = true;
				}
				else if (pos.y + hit.radius > gameplay_view.getSize().y / 2.f) {
					pos.y = gameplay_view.getSize().y / 2.f - hit.radius;
					if (boost) {
						new_vel.y = -vel.y;
						new_facing.angle = -new_facing.angle;
					}
					else {
						new_facing.angle = rotate_towards_closest_tangent(
							facing.angle, angle::deg(0.f), rot_vel
						);
					}
					oob = true;
				}
				// if not oob, don't update components
				if (!oob) {
					continue;
				}
				mut_ecs.patch<sf::Sprite>(e, [&](sf::Sprite& spr) {
					spr.setPosition(pos);
				});
				// movement flag needed else nights will lie flat on the ground
				if (boost || movement) {
					mut_ecs.replace<velocity>(e, new_vel);
					mut_ecs.replace<::facing>(e, new_facing);
				}
			}

			// erase any danmaku that are off-screen
			for (const auto& [e, spr] : ecs.view<danmaku, sf::Sprite>().each()) {
				const auto& pos = spr.getPosition();
				const auto& spr_size = spr.getScale();
				const auto tex_size =
					static_cast<sf::Vector2f>(spr.getTexture()->getSize());
				const sf::Vector2f size{
					spr_size.x * tex_size.x, spr_size.y * tex_size.y
				};
				const auto& view_pos = gameplay_view.getCenter();
				const auto& view_size = gameplay_view.getSize();
				if (
					pos.x + size.x < view_pos.x - view_size.x / 2.f
					|| pos.x - size.x > view_pos.x + view_size.x / 2.f
					|| pos.y + size.y < view_pos.y - view_size.y / 2.f
					|| pos.y - size.y > view_pos.y + view_size.y / 2.f
				) {
					mut_ecs.destroy(e);
				}
			}

			// check for collisions between painful hitboxes and players
			for (const auto& [d_e, d_spr, d_hit] : ecs.view<painful, sf::Sprite, hitbox>().each()) {
				// player loop is inner so they can't collide twice in one frame
				for (const auto& [p_e, p_p, p_spr, p_hit] : ecs.view<player, sf::Sprite, hitbox>().each()) {
					constexpr float invuln_time = 1.f;
					// if player is invuln
					if (p_p.last_hit + invuln_time > elapsed) {
						continue;
					}
					// compare radius of hitboxes alongside position from sprite
					const auto& d_pos = d_spr.getPosition();
					const auto& p_pos = p_spr.getPosition();
					const auto dist = mag(d_pos - p_pos);
					if (dist < d_hit.radius + p_hit.radius) {
						// collision detected, damage player (reduce ideya)
						mut_ecs.patch<player>(p_e, [&](player& p) {
							--p.ideya;
							// if no more ideya (dead)
							if (p.ideya <= 0) {
								mut_ecs.emplace<disintegrate>(p_e, elapsed);
								mut_ecs.emplace<sf::Shader*>(p_e, &disintegrate_shdr);
								mut_ecs.remove<hitbox>(p_e);
								p.input = {};
								audio.play("scream");
							}
							else {
								audio.play("hurt");
								p.last_hit = elapsed;
							}
						});
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
		}

		// VARIABLE TIMESTEP

		danmaku_shdr.setUniform("time_elapsed", elapsed);

		if (ImGui::Begin(
			"debug",
			nullptr,
			ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_::ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_::ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse
		)) {
			ImGui::SetWindowPos(imgui_ui->viewport_area.getPosition());
			ImGui::SetWindowSize(imgui_ui->viewport_area.getSize());

			ImGui::Checkbox("visible hitboxes", &debug_visible_hitboxes);
			if (ImGui::Button("funny scream")) {
				audio.play("scream");
			}
			ImGui::End();
		}

		ImGui::EndFrame();

		const auto gameplay_draw_fn = [&](sf::RenderTarget& target) {
			// set view
			auto view = target.getView();
			view.setCenter(gameplay_view.getCenter());
			view.setSize(gameplay_view.getSize());
			target.setView(view);

			// set bg color
			sf::RectangleShape bg{ gameplay_view.getSize() };
			bg.setFillColor(sf::Color::Black);
			bg.setOrigin(gameplay_view.getSize() / 2.f);
			target.draw(bg);

			// render sprites
			for (const auto& [e, spr] : ecs.view<sf::Sprite>().each()) {
				if (const auto shdr = ecs.try_get<sf::Shader*>(e)) {
					if (const auto dis = ecs.try_get<disintegrate>(e)) {
						mut_ecs.patch<sf::Shader*>(e, [&](sf::Shader* shdr) {
							shdr->setUniform("disintegration", elapsed - dis->begin);
						});
					}
					target.draw(spr, *shdr);
				}
				else {
					target.draw(spr);
				}
			}

			// render player ideya
			for (const auto& [_, spr, player, hit] : ecs.view<sf::Sprite, player, hitbox>().each()) {
				// draw hitbox ideya
				auto ideya_spr = texture.sprite("ideya");
				ideya_spr.setColor(sf::Color{ 0xFF3F3FFF });
				const auto& ideya_scale = 2.f * hit.radius / ideya_spr.getTexture()->getSize().x;
				ideya_spr.setScale({ ideya_scale, ideya_scale });
				ideya_spr.setPosition(spr.getPosition());
				target.draw(ideya_spr, &ideya_shdr);

				// draw orbitting ideya
				const std::array<sf::Color, 4> orbit_colors{
					sf::Color{ 0xFFFFFFFF },
					sf::Color{ 0x3FFF3FFF },
					sf::Color{ 0xFFFF3FFF },
					sf::Color{ 0x3F3FFFFF }
				};
				const auto orbit_radius = 24.f;
				const auto orbit_speed = 120.f;
				const auto orbit_init = elapsed * orbit_speed;
				for (std::size_t ideya_idx{ 0 }; ideya_idx < player.ideya - 1; ++ideya_idx) {
					ideya_spr.setColor(orbit_colors[ideya_idx]);
					// use lerp
					const auto orbit_angle = angle::deg(
						orbit_init + 360.f * ideya_idx / (player.ideya - 1)
					);
					ideya_spr.setPosition(
						spr.getPosition() +
						orbit_radius * orbit_angle.vec()
					);
					target.draw(ideya_spr, &ideya_shdr);
				}
			}

			// render hitboxes
			if (debug_visible_hitboxes) {
				for (const auto& [e, spr, hit] : ecs.view<sf::Sprite, hitbox>().each()) {
					sf::CircleShape circle{ hit.radius };
					circle.setOrigin({ hit.radius, hit.radius });
					circle.setPosition(spr.getPosition());
					circle.setFillColor(
						ecs.any_of<painful>(e) ? sf::Color::Red : sf::Color::Cyan
					);
					target.draw(circle);
				}
			}
		};
		gameplay_ui->draw_fn = gameplay_draw_fn;

		window.clear(sf::Color{ 0x8C00ADFF });
		ui.draw(window);
		ImGui::SFML::Render(window);

		window.display();
	}

	ImGui::SFML::Shutdown();

	return EXIT_SUCCESS;
}
