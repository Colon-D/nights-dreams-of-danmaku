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
#include "input.h"
#include "angle.h"
#include "math_utils.h"
#include "components.h"
#include "prev.h"
#include "ui.h"
#include "resource/audio.h"
#include "resource/texture.h"

entt::entity create_player(entt::registry& ecs, const sf::Vector2f& pos, const player& player, const sprite& spr) {
	const auto player_e = ecs.create();
	ecs.emplace<::player>(player_e, player);

	ecs.emplace<transform>(player_e, pos);
	ecs.emplace<prev<transform>>(player_e, pos);
	ecs.emplace<sprite>(player_e, spr);

	ecs.emplace<velocity>(player_e);
	ecs.emplace<facing>(player_e, angle::deg(-90.f));

	ecs.emplace<hitbox>(player_e, 5.f);

	return player_e;
}

entt::entity create_danmaku(entt::registry& ecs, sf::Shader& shdr, sprite spr, const sf::Vector2f& pos, const velocity& vel, const sf::Color& col) {
	const auto e = ecs.create();
	ecs.emplace<danmaku>(e);
	ecs.emplace<rotate_towards_velocity>(e);

	spr.color = col;
	ecs.emplace<sprite>(e, spr);
	const transform trans{ pos };
	ecs.emplace<transform>(e, trans);
	ecs.emplace<prev<transform>>(e, trans);

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
	danmaku_spr.origin += { 4.f, -0.5f };
	danmaku_spr.color = sf::Color::Red;

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

	auto gillwing_spr = texture.sprite("gillwing");
	gillwing_spr.origin = { 750.f, 300.f };
	constexpr float gillwing_scale{ 0.25f };
	gillwing_spr.size *= gillwing_scale;
	mut_ecs.emplace<sprite>(gillwing_e, gillwing_spr);
	const transform gillwing_transform{ { -500.f, -150.f } };
	mut_ecs.emplace<transform>(gillwing_e, gillwing_transform);
	mut_ecs.emplace<prev<transform>>(gillwing_e, gillwing_transform);

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
			const auto& sprite_size = logo_sprite.texture->getSize();
			const auto scale_down = imgui_ui->size.x / sprite_size.x;
			const transform logo_trans{
				{
					0.f,
					imgui_ui->size.y / 2.f - scale_down * sprite_size.y / 2.f
				},
				{},
				{ scale_down, scale_down }
			};
			target.draw(logo_sprite.get_sprite(logo_trans));
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

	previous_system<transform> prev_system{ mut_ecs };

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
		constexpr float dt{ 1.f / 60.f };
;		if (accumulator > dt) {
			accumulator = std::fmod(accumulator, dt);

			// FIXED TIMESTEP
			prev_system.update_previous();

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
			for (const auto& [e, vel, t, gw] : ecs.view<velocity, transform, gillwing>().each()) {
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
						if (t.pos.x > gameplay_view.getSize().x / 2.f) {
							gw.heading_right = false;
						}
					}
					else {
						if (t.pos.x < -gameplay_view.getSize().x / 2.f) {
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
							dist(rng), dist(rng)
						};
						const sf::Color danmaku_col{
							0, 255, static_cast<std::uint8_t>(blue_dist(rng))
						};
						const auto danmaku = create_danmaku(mut_ecs, danmaku_shdr, danmaku_spr, t.pos, danmaku_vel, danmaku_col);
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
			for (const auto& [e, _, vel] : ecs.view<transform, velocity>().each()) {
				mut_ecs.patch<transform>(e, [&](transform& t) {
					t.pos += dt * vel;
				});
			}

			// clamp player in bounds by sliding/bouncing off walls,
			// recalculating their position, and often their velocity and facing
			for (
				const auto& [e, hit, t, vel, facing, player] :
				ecs.view<hitbox, transform, velocity, facing, player>().each()
			) {
				auto pos = t.pos;

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
				mut_ecs.patch<transform>(e, [&](transform& t) {
					t.pos = pos;
				});
				// movement flag needed else nights will lie flat on the ground
				if (boost || movement) {
					mut_ecs.replace<velocity>(e, new_vel);
					mut_ecs.replace<::facing>(e, new_facing);
				}
			}

			// erase any danmaku that are off-screen
			for (const auto& [e, t, spr] : ecs.view<danmaku, transform, sprite>().each()) {
				const sf::Vector2f size{
					t.scale.x * spr.size.x, t.scale.y * spr.size.y
				};
				const auto& view_pos = gameplay_view.getCenter();
				const auto& view_size = gameplay_view.getSize();
				if (
					t.pos.x + size.x < view_pos.x - view_size.x / 2.f
					|| t.pos.x - size.x > view_pos.x + view_size.x / 2.f
					|| t.pos.y + size.y < view_pos.y - view_size.y / 2.f
					|| t.pos.y - size.y > view_pos.y + view_size.y / 2.f
				) {
					mut_ecs.destroy(e);
				}
			}

			// check for collisions between painful hitboxes and players
			for (const auto& [d_e, d_t, d_hit] : ecs.view<painful, transform, hitbox>().each()) {
				// player loop is inner so they can't collide twice in one frame
				for (const auto& [p_e, p_p, p_t, p_hit] : ecs.view<player, transform, hitbox>().each()) {
					constexpr float invuln_time = 1.f;
					// if player is invuln
					if (p_p.last_hit + invuln_time > elapsed) {
						continue;
					}
					// compare radius of hitboxes alongside position from sprite
					const auto dist = mag(d_t.pos - p_t.pos);
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
			for (const auto& [e, player, facing, t] : ecs.view<player, facing, transform>().each()) {
				mut_ecs.patch<transform>(e, [&](transform& t) {
					// todo: is facing.angle needed now that transform is seperate from sprite?
					t.rot = facing.angle;
				});
			}

			// "flip" sprite if boosting
			for (const auto& [e, player, _] : ecs.view<player, transform>().each()) {
				mut_ecs.patch<transform>(e, [&](transform& t) {
					if (player.input.boost()) {
						t.scale = { 1.f, sin(elapsed * 32.f) };
					}
					else {
						t.scale = { 1.f, 1.f };
					}
				});
			}

			// rotate sprite to match velocity
			for (
				const auto& [e, rtv, vel, _] :
				ecs.view<rotate_towards_velocity, velocity, transform>().each()
			) {
				mut_ecs.patch<transform>(e, [&](transform& t) {
					t.rot = angle::from_vec(static_cast<sf::Vector2f>(vel)).normalize();
					if (rtv.flip) {
						if (
							t.rot > angle::deg(90.f)
							|| t.rot <= angle::deg(-90.f)
						) {
							if (t.scale.y > 0.f) {
								t.scale.y = -std::abs(t.scale.y);
								// snap
								mut_ecs.replace<prev<transform>>(e, t);
							}
						}
						else {
							if (t.scale.y < 0.f) {
								t.scale.y = std::abs(t.scale.y);
								// snap
								mut_ecs.replace<prev<transform>>(e, t);
							}
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
		const float alpha{ accumulator / dt };

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
			for (const auto& [e, _, spr] : ecs.view<transform, sprite>().each()) {
				const auto t = prev_system.try_lerp<transform>(e, alpha);

				if (const auto shdr = ecs.try_get<sf::Shader*>(e)) {
					if (const auto dis = ecs.try_get<disintegrate>(e)) {
						mut_ecs.patch<sf::Shader*>(e, [&](sf::Shader* shdr) {
							shdr->setUniform("disintegration", elapsed - dis->begin);
						});
					}
					target.draw(spr.get_sprite(t), *shdr);
				}
				else {
					target.draw(spr.get_sprite(t));
				}
			}

			// render player ideya
			// todo: fix
			for (const auto& [e, _, player, hit] : ecs.view<transform, player, hitbox>().each()) {
				const auto t = prev_system.try_lerp<transform>(e, alpha);
				// draw hitbox ideya
				auto ideya_spr = texture.sprite("ideya");
				ideya_spr.color = sf::Color{ 0xFF3F3FFF };
				const float ideya_size{ 2.f * hit.radius };
				ideya_spr.size = { ideya_size, ideya_size };
				transform ideya_trans{ t.pos };
				target.draw(ideya_spr.get_sprite(ideya_trans), &ideya_shdr);

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
					ideya_spr.color = orbit_colors[ideya_idx];
					// use lerp
					const auto orbit_angle = angle::deg(
						orbit_init + 360.f * ideya_idx / (player.ideya - 1)
					);
					ideya_trans.pos = t.pos + orbit_radius * orbit_angle.vec();
					target.draw(ideya_spr.get_sprite(ideya_trans), &ideya_shdr);
				}
			}

			// render hitboxes
			// (not lerped, because then it is less accurate?)
			if (debug_visible_hitboxes) {
				for (const auto& [e, t, hit] : ecs.view<transform, hitbox>().each()) {
					sf::CircleShape circle{ hit.radius };
					circle.setOrigin({ hit.radius, hit.radius });
					circle.setPosition(t.pos);
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
