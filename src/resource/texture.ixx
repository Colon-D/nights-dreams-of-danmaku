module;
#include <SFML/Graphics.hpp>

export module texture;
import resource;

constexpr std::size_t max_sounds{ 32 };
// [0, 100]
constexpr float default_volume{ 5.f };

export class texture : public resource<sf::Texture> {
public:
	// creates and returns a sprite with the texture, and origin centered
	sf::Sprite sprite(const std::string& name) const {
		const auto& tex = resources.at(name);
		sf::Sprite spr{ tex };
		spr.setOrigin(static_cast<sf::Vector2f>(tex.getSize()) / 2.f);
		return spr;
	}
private:
	sf::Texture load_from_file(
		const std::filesystem::path& path
	) const override {
		sf::Texture texture{};
		texture.loadFromFile(path.string());
		texture.setSmooth(true);
		return texture;
	}
};
