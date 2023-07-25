#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <functional>

struct ui {
	virtual ~ui() = default;
	virtual sf::Vector2f min_size() const = 0;

	// todo: this could take viewport_area and scale as parameters
	virtual void update();

	virtual void draw(sf::RenderTarget& target) const;;

	/// post processed area
	sf::FloatRect viewport_area{};
	/// post processed scale
	float scale{};
};

struct leaf_ui : ui {
	sf::Vector2f min_size() const override;

	void draw(sf::RenderTarget& target) const override;

	sf::Vector2f size{};
	std::function<void(sf::RenderTarget& target)> draw_fn{};
};

struct margin_ui : ui {
	sf::Vector2f min_size() const override;

	void update();

	void draw(sf::RenderTarget& target) const override;

	std::unique_ptr<ui> child{};
	float margin{};
};

struct row_ui : ui {
	sf::Vector2f min_size() const override;

	void update();

	void draw(sf::RenderTarget& target) const override;

	std::vector<std::unique_ptr<ui>> children{};
};

struct center_and_scale_ui : ui {
	sf::Vector2f min_size() const override;
	
	void update();

	void draw(sf::RenderTarget& target) const override;

	std::unique_ptr<ui> child{};
};
