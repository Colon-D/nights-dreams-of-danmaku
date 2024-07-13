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
#include "constants.h"
#include "danmaku.h"
#include "bosses/boss.h"
#include "bosses/gillwing.h"
#include "services.h"
#include "player.h"
#include <format>

entt::entity create_player(entt::registry& ecs, const sf::Vector2f& pos, const player& player, const sprite& spr) {
	const auto player_e = ecs.create();
	ecs.emplace<::player>(player_e, player);

	ecs.emplace<transform>(player_e, pos);
	ecs.emplace<prev<transform>>(player_e, pos);
	ecs.emplace<sprite>(player_e, spr);

	ecs.emplace<velocity>(player_e);
	ecs.emplace<facing>(player_e, angle::deg(-90.f));

	ecs.emplace<hitbox>(player_e, 5.f);

	ecs.emplace<position_history>(player_e, player::max_paraloop_points);

	return player_e;
}

int main(int argc, char* argv[]) {
	if (!std::filesystem::exists("res/nights.png")) {
		std::cerr << "ERROR: \"./res/nights.png\" can not be found.\n";
		std::cerr << "Run executable in directory \"./\".";
		return EXIT_FAILURE;
	}

	serv = std::make_unique<services>();

	auto danmaku_spr = serv->texture.sprite("danmaku");
	danmaku_spr.origin += { 4.f, -0.5f };
	danmaku_spr.color = sf::Color::Red;

	scene_services scn{};

	sf::RenderWindow window{ { 1024, 720 }, "NiGHTS: Dreams of Danmaku" };
	sf::View gameplay_view{ {}, gameplay_size };

	std::size_t fps_counter{};
	std::size_t fps_display{};
	float last_fps_display{ 1.f };

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

	auto& mut_ecs = scn.registry;
	const auto& ecs = mut_ecs;

	create_player(mut_ecs, { -100.f, 0.f }, { input::default_keyboard }, serv->texture.sprite("nights"));
	create_player(mut_ecs, {  100.f, 0.f }, { input::default_gamepad() }, serv->texture.sprite("reala"));

	create_gillwing(mut_ecs);

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
			auto logo_sprite = serv->texture.sprite("logo");
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

		++fps_counter;
		last_fps_display -= var_dt;
		if (last_fps_display < 0.f) {
			last_fps_display = 1.f;
			fps_display = fps_counter;
			fps_counter = 0;
		}

		accumulator += var_dt;
		constexpr float dt{ fixed_timestep };
;		if (accumulator > dt) {
			accumulator = std::fmod(accumulator, dt);

			// FIXED TIMESTEP
			prev_system.update_previous();

			// update position history
			for (const auto& [e, t, _] : ecs.view<transform, position_history>().each()) {
				mut_ecs.patch<position_history>(e, [&](position_history& ph) {
					ph.push(t.pos);
				});
			}

			serv->audio.update();

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
							std::cout << acceleration << '\n';
							std::cout << vel.y << '\n';
						}
					}
				});
			}

			attack_system::update_sigh.publish(scn);

			// apply standard gravity to entities
			for (const auto& [e, _] : ecs.view<gravity, velocity>().each()) {
				mut_ecs.patch<velocity>(e, [&](velocity& vel) {
					constexpr float gravity{ 256.f };
					vel.y += dt * gravity;
				});
			}

			// apply acceleration to entities
			for (const auto& [e, accel, _] : ecs.view<acceleration, velocity>().each()) {
				mut_ecs.patch<velocity>(e, [&](velocity& vel) {
					vel += dt * accel.val;
				});
			}

			// apply velocity to sprite's position
			for (const auto& [e, _, vel] : ecs.view<transform, velocity>().each()) {
				mut_ecs.patch<transform>(e, [&](transform& t) {
					t.pos += dt * vel;
				});
			}

			// update gillwing's segments
			gillwing_update(scn);

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

			// update player's paraloop points
			for (const auto& [e, _, t, ph] : ecs.view<player, transform, position_history>().each()) {
				// check to see if line formed by last two points intersects any other two points
				if (ph.size() > 5) {
					line end{ ph[ph.size() - 2], ph.back() };
					for (std::size_t i{ 0 }; i < ph.size() - 4; ++i) {
						line other{ ph[i], ph[i + 1] };
						if (intersects(other, end)) {
							// PARALOOP OCCURED
							std::vector<sf::Vector2f> paraloop{};
							paraloop.resize(ph.size() - i);
							for (std::size_t j{ 0 }; j < paraloop.size(); ++j) {
								paraloop[j] = ph[i + j];
							}

							// find all danmaku inside the paraloop and destroy them,
							// also damage the boss
							float damage{};
							for (const auto& [d_e, d_t] : ecs.view<danmaku, transform>().each()) {
								if (intersects(d_t.pos, paraloop)) {
									damage += 1.f;
									mut_ecs.destroy(d_e);
								}
							}
							for (const auto b_e : ecs.view<boss>()) {
								mut_ecs.patch<boss>(b_e, [&](boss& boss) {
									boss.set_health({ mut_ecs, b_e }, boss.get_health() - damage);
								});
							}

							// clear position history
							// (hopefully this won't break anything if I need player's position history later)
							mut_ecs.patch<position_history>(e, [&](position_history& ph) {
								ph.clear();
							});

							break;
						}
					}
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
								mut_ecs.emplace<sf::Shader*>(p_e, serv->shaders.map.at("disintegrate").get());
								mut_ecs.remove<hitbox>(p_e);
								p.input = {};
								serv->audio.play("scream");
							}
							else {
								serv->audio.play("hurt");
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

		serv->shaders.map.at("danmaku")->setUniform("time_elapsed", elapsed);

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

			ImGui::Text("FPS: %d", fps_display);

			ImGui::Checkbox("visible hitboxes", &debug_visible_hitboxes);
			if (ImGui::Button("funny scream")) {
				serv->audio.play("scream");
			}
			for (const auto& [e, b] : ecs.view<boss>().each()) {
				ImGui::Text("Boss(e: %d) Health: %d", e, static_cast<int>(b.get_health()));
				if (ImGui::Button(std::format("Damage Boss(e: {}) by 32", static_cast<int>(e)).c_str())) {
					mut_ecs.patch<boss>(e, [&](boss& b) {
						b.set_health({ mut_ecs, e }, b.get_health() - 32.f);
					});
				}
			}

			ImGui::SliderFloat("Difficulty", &diff, 0.25f, 2.f);
			if (ImGui::Button("Easy")) {
				diff = 0.5f;
			}
			ImGui::SameLine();
			if (ImGui::Button("Normal")) {
				diff = 1.f;
			}
			ImGui::SameLine();
			if (ImGui::Button("Hard")) {
				diff = 1.5f;
			}
			ImGui::SameLine();
			if (ImGui::Button("Lunatic")) {
				diff = 2.f;
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
				auto ideya_spr = serv->texture.sprite("ideya");
				ideya_spr.color = sf::Color{ 0xFF3F3FFF };
				const float ideya_size{ 2.f * hit.radius };
				ideya_spr.size = { ideya_size, ideya_size };
				transform ideya_trans{ t.pos };
				target.draw(ideya_spr.get_sprite(ideya_trans), serv->shaders.map.at("ideya").get());

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
					target.draw(ideya_spr.get_sprite(ideya_trans), serv->shaders.map.at("ideya").get());
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

			// render player's paraloop lines (debugging)
			for (const auto& [e, _, ph] : ecs.view<player, position_history>().each()) {
				sf::VertexArray va{ sf::PrimitiveType::LineStrip };
				va.resize(ph.size());
				for (std::size_t i{ 0 }; i < ph.size(); ++i) {
					va[i].position = ph[i];
				}
				target.draw(va);
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
