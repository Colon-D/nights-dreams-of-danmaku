#pragma once
#include <SFML/Audio.hpp>
#include <unordered_set>
#include <list>
#include "resource/resource.h"

class audio : public resource<sf::SoundBuffer> {
public:
	void play(const std::string& name);

	void update();
private:
	sf::SoundBuffer load_from_file(
		const std::filesystem::path& path
	) const override;

	std::unordered_set<std::string> sounds_played_this_frame{};
	std::list<sf::Sound> playing{};
};
