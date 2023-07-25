#pragma once
#include <SFML/Graphics.hpp>
#include "components.h"
#include "resource/resource.h"

class texture : public resource<sf::Texture> {
public:
	// creates and returns a sprite with the texture, and origin centered
	::sprite sprite(const std::string& name) const;
private:
	sf::Texture load_from_file(
		const std::filesystem::path& path
	) const override;
};
