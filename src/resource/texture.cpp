#include "resource/texture.h"

// creates and returns a sprite with the texture, and origin centered

::sprite texture::sprite(const std::string& name) const {
	const auto& tex = resources.at(name);
	const auto size = static_cast<sf::Vector2f>(tex.getSize());
	::sprite spr{ &tex, size / 2.f, size };
	return spr;
}

sf::Texture texture::load_from_file(const std::filesystem::path& path) const {
	sf::Texture texture{};
	texture.loadFromFile(path.string());
	texture.setSmooth(true);
	return texture;
}
