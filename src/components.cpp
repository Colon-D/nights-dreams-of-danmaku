#include "components.h"

sf::Transform transform::get_transform() const {
	return sf::Transform{}
	.translate(pos)
		.rotate(rot.deg())
		.scale(scale);
}

sf::Sprite sprite::get_sprite(const transform& transform) const {
	sf::Sprite sf_spr{ *texture };
	sf_spr.setColor(color);
	sf_spr.setOrigin(origin);
	sf_spr.setPosition(transform.pos);
	sf_spr.setRotation(transform.rot.deg());
	const auto& tex_size = texture->getSize();
	sf_spr.setScale({
		transform.scale.x * size.x / tex_size.x,
		transform.scale.y * size.y / tex_size.y
	});
	return sf_spr;
}

transform lerp(const transform& a, const transform& b, const float ratio) {
	return {
		lerp(a.pos, b.pos, ratio),
		lerp(a.rot, b.rot, ratio),
		lerp(a.scale, b.scale, ratio)
	};
}

position_history::position_history(const std::size_t max_size) {
	history.resize(max_size);
}

void position_history::push(const sf::Vector2f& pos) {
	if (length < history.size()) {
		++length;
	}
	else {
		begin = (begin + 1) % history.size();
	}
	const std::size_t end{ (begin + length - 1) % history.size() };
	history[end] = pos;
}

sf::Vector2f position_history::pop() {
	assert(length > 0);
	const std::size_t end{ (begin + length - 1) % history.size() };
	const sf::Vector2f value{ history[end] };
	--length;
	return value;
}

sf::Vector2f& position_history::operator[](const std::size_t index) {
	assert(index < length);
	return history[(begin + index) % history.size()];
}

const sf::Vector2f& position_history::operator[](
	const std::size_t index
) const {
	assert(index < length);
	return history[(begin + index) % history.size()];
}

std::size_t position_history::size() const {
	return length;
}

sf::Vector2f position_history::front() const {
	return (*this)[0];
}

sf::Vector2f position_history::back() const {
	return (*this)[length - 1];
}

void position_history::clear() {
	begin = 0;
	length = 0;
}
