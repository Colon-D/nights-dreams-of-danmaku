module;
#include <SFML/Audio.hpp>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <unordered_set>
#include <list>

export module audio;

constexpr std::size_t max_sounds{ 32 };
// [0, 100]
constexpr float default_volume{ 5.f };

export class audio {
public:
	audio() {
		// recursively load all .ogg files in the resources folder
		for (
			const auto path :
			std::filesystem::recursive_directory_iterator("res")
		) {
			if (path.path().extension() != ".ogg") {
				continue;
			}
			const auto name = path.path().stem().string();
			sf::SoundBuffer sample{};
			sample.loadFromFile(path.path().string());
			samples.insert({ name, sample });
		}
	}

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
		auto& sound = playing.emplace_back(samples.at(name));
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

	std::unordered_map<std::string, sf::SoundBuffer> samples{};
	std::unordered_set<std::string> sounds_played_this_frame{};
	std::list<sf::Sound> playing{};
};
