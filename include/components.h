#pragma once
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "input.h"
#include "angle.h"
#include "constants.h"

struct velocity : sf::Vector2f {
	velocity(const sf::Vector2f& vel = {}) : sf::Vector2f{ vel } {}
	operator sf::Vector2f() const {
		return *this;
	}
};

struct acceleration {
	sf::Vector2f val{};
};

struct facing {
	angle angle{};
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

class position_history {
public:
	position_history(const std::size_t max_size);

	void push(const sf::Vector2f& pos);

	sf::Vector2f pop();

	sf::Vector2f& operator[](std::size_t index);

	const sf::Vector2f& operator[](std::size_t index) const;

	std::size_t size() const;

	sf::Vector2f front() const;

	sf::Vector2f back() const;

	void clear();
private:
	std::size_t begin{};
	std::size_t length{};

	std::vector<sf::Vector2f> history{};
};
