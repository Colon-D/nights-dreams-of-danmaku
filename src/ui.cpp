#include "ui.h"

void ui::update() {
}

void ui::draw(sf::RenderTarget& target) const {
}

sf::Vector2f leaf_ui::min_size() const {
	return size;
}

void leaf_ui::draw(sf::RenderTarget& target) const {
	ui::draw(target);
	sf::View view{ {}, size };
	view.setViewport({
		viewport_area.left   / target.getSize().x,
		viewport_area.top    / target.getSize().y,
		viewport_area.width  / target.getSize().x,
		viewport_area.height / target.getSize().y
	});
	target.setView(view);
	draw_fn(target);
}

sf::Vector2f margin_ui::min_size() const {
	return child->min_size() + sf::Vector2f{ margin, margin } * 2.f;
}

void margin_ui::update() {
	ui::update();

	child->viewport_area = viewport_area;
	child->viewport_area.left += scale * margin;
	child->viewport_area.top += scale * margin;
	child->viewport_area.width -= scale * margin * 2.f;
	child->viewport_area.height -= scale * margin * 2.f;
	child->scale = scale;

	child->update();
}

void margin_ui::draw(sf::RenderTarget& target) const {
	ui::draw(target);
	child->draw(target);
}

sf::Vector2f row_ui::min_size() const {
	sf::Vector2f size{};
	for (const auto& child : children) {
		size.x += child->min_size().x;
		size.y = std::max(size.y, child->min_size().y);
	}
	return size;
}

void row_ui::update() {
	ui::update();
	auto child_viewport_area = viewport_area;
	for (const auto& child : children) {
		child_viewport_area.width = child->min_size().x * scale;
		child_viewport_area.height = child->min_size().y * scale;
		child->viewport_area = child_viewport_area;
		child->scale = scale;
		child->update();
		child_viewport_area.left += child_viewport_area.width;
	}
}

void row_ui::draw(sf::RenderTarget& target) const {
	ui::draw(target);

	for (const auto& child : children) {
		child->draw(target);
	}
}

sf::Vector2f center_and_scale_ui::min_size() const {
	return child->min_size();
}

void center_and_scale_ui::update() {
	ui::update();

	child->viewport_area = viewport_area;
	const auto child_size = child->min_size();
	const auto child_ratio = child_size.x / child_size.y;
	const auto viewport_ratio = viewport_area.width / viewport_area.height;
	if (viewport_ratio > child_ratio) {
		child->viewport_area.width = viewport_area.height * child_ratio;
		child->viewport_area.left += (viewport_area.width - child->viewport_area.width) / 2.f;
		scale = viewport_area.height / child_size.y;
		child->scale = scale;
	}
	else {
		child->viewport_area.height = viewport_area.width / child_ratio;
		child->viewport_area.top += (viewport_area.height - child->viewport_area.height) / 2.f;
		scale = viewport_area.width / child_size.x;
		child->scale = scale;
	}

	child->update();
}

void center_and_scale_ui::draw(sf::RenderTarget& target) const {
	ui::draw(target);
	child->draw(target);
}
