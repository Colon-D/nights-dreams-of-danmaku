module;
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <functional>

export module ui;

struct ui {
	virtual ~ui() = default;
	virtual sf::Vector2f min_size() const = 0;

	virtual void update(sf::FloatRect viewport_area, float scale) {
	}

	virtual void draw(sf::RenderTarget& target, sf::FloatRect viewport_area, float scale) const {
		sf::View view{ viewport_area };
		view.setViewport({
			viewport_area.left   / target.getSize().x,
			viewport_area.top    / target.getSize().y,
			viewport_area.width  / target.getSize().x,
			viewport_area.height / target.getSize().y
		});
		target.setView(view);
		sf::RectangleShape test{ viewport_area.getSize() - sf::Vector2f{ 4.f, 4.f } };
		test.setPosition(viewport_area.getPosition() + sf::Vector2f{ 2.f, 2.f });
		test.setFillColor(sf::Color::Blue);
		test.setOutlineColor(sf::Color::Red);
		test.setOutlineThickness(2.f);
		target.draw(test);
		test.setSize(min_size() - sf::Vector2f{ 2.f, 2.f });
		test.setOutlineColor(sf::Color::Green);
		test.setOutlineThickness(1.f);
		target.draw(test);
	};
};

export struct leaf_ui : ui {
	sf::Vector2f min_size() const override {
		return size;
	}

	void draw(sf::RenderTarget& target, sf::FloatRect viewport_area, float scale) const override {
		ui::draw(target, viewport_area, scale);
		draw_fn(target);
	}

	sf::Vector2f size{};
	std::function<void(sf::FloatRect viewport_area, float scale)> update_fn{};
	std::function<void(sf::RenderTarget& target)> draw_fn{};
};

export struct margin_ui : ui {
	sf::Vector2f min_size() const override {
		return
			child->min_size()
			+ sf::Vector2f{ margin, margin } * 2.f;
	}

	void draw(sf::RenderTarget& target, sf::FloatRect viewport_area, float scale) const override {
		ui::draw(target, viewport_area, scale);

		viewport_area.left   += scale * margin;
		viewport_area.top    += scale * margin;
		viewport_area.width  -= scale * margin * 2.f;
		viewport_area.height -= scale * margin * 2.f;
		child->draw(target, viewport_area, scale);
	}

	std::unique_ptr<ui> child{};
	float margin{};
};

export struct row_ui : ui {
	sf::Vector2f min_size() const override {
		sf::Vector2f size{};
		for (const auto& child : children) {
			size.x += child->min_size().x;
			size.y = std::max(size.y, child->min_size().y);
		}
		return size;
	}

	void draw(sf::RenderTarget& target, sf::FloatRect viewport_area, float scale) const override {
		ui::draw(target, viewport_area, scale);

		for (const auto& child : children) {
			viewport_area.width = child->min_size().x * scale;
			viewport_area.height = child->min_size().y * scale;
			child->draw(target, viewport_area, scale);
			viewport_area.left += viewport_area.width;
		}
	}

	std::vector<std::unique_ptr<ui>> children{};
};

export struct center_and_scale_ui : ui {
	sf::Vector2f min_size() const override {
		return child->min_size();
	}

	void draw(sf::RenderTarget& target, sf::FloatRect viewport_area, float scale) const override {
		ui::draw(target, viewport_area, scale);

		// resize viewport area to resize and fit child in center
		const auto child_size = child->min_size();
		const auto child_ratio = child_size.x / child_size.y;
		const auto viewport_ratio = viewport_area.width / viewport_area.height;
		if (viewport_ratio > child_ratio) {
			const auto old_width = viewport_area.width;
			viewport_area.width = viewport_area.height * child_ratio;
			viewport_area.left += (old_width - viewport_area.width) / 2.f;
			scale = viewport_area.height / child_size.y;
		}
		else {
			const auto old_height = viewport_area.height;
			viewport_area.height = viewport_area.width / child_ratio;
			viewport_area.top += (old_height - viewport_area.height) / 2.f;
			scale = viewport_area.width / child_size.x;
		}

		child->draw(target, viewport_area, scale);
	}

	std::unique_ptr<ui> child{};
};
