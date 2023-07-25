#pragma once
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "input.h"
#include "angle.h"
#include "constants.h"

struct velocity : sf::Vector2f {
	velocity(const sf::Vector2f& vel = {}) : sf::Vector2f{ vel } {}
	using sf::Vector2f::Vector2f;
	using sf::Vector2f::operator=;
	operator sf::Vector2f() const {
		return *this;
	}
};

struct player {
	input input{};
	int ideya{ 5 };
	float last_hit{};
	// store 2.5s of paraloop points
	std::vector<sf::Vector2f> paraloop_points{};
	constexpr static std::size_t max_paraloop_points{
		static_cast<std::size_t>(fixed_framerate * 2.5)
	};
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

struct transform {
	sf::Vector2f pos{};
	angle rot{};
	sf::Vector2f scale{ 1.f, 1.f };

	sf::Transform get_transform() const;
};

transform lerp(const transform& a, const transform& b, float ratio);

struct sprite {
	const sf::Texture* texture{};
	sf::Vector2f origin{};
	sf::Vector2f size{};
	sf::Color color{ sf::Color::White };

	sf::Sprite get_sprite(const transform& transform) const;
};
