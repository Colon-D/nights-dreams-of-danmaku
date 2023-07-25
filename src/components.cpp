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
