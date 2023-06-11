module;
#include <SFML/Audio.hpp>
#include <unordered_set>
#include <list>

export module audio;
import resource;

constexpr std::size_t max_sounds{ 32 };
// [0, 100]
constexpr float default_volume{ 5.f };

export class audio : public resource<sf::SoundBuffer> {
public:
	void play(const std::string& name) {
		// limit number of sounds playing at once
		if (playing.size() >= max_sounds) {
			return;
		}
		// don't play the same sound twice in one frame
		if (sounds_played_this_frame.contains(name)) {
			return;
		}
		// create sound and start playing it
		auto& sound = playing.emplace_back(resources.at(name));
		sound.setVolume(default_volume);
		sound.play();
		sounds_played_this_frame.insert(name);
	}

	void update() {
		// stop sounds that have finished playing
		for (auto it = playing.begin(); it != playing.end();) {
			if (it->getStatus() == sf::Sound::Status::Stopped) {
				it = playing.erase(it);
			}
			else {
				++it;
			}
		}
		// clear sounds played this frame
		sounds_played_this_frame.clear();
	}
private:
	sf::SoundBuffer load_from_file(
		const std::filesystem::path& path
	) const override {
		sf::SoundBuffer sound_buffer{};
		sound_buffer.loadFromFile(path.string());
		return sound_buffer;
	}

	std::unordered_set<std::string> sounds_played_this_frame{};
	std::list<sf::Sound> playing{};
};
