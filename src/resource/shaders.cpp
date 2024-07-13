#include "resource/shaders.h"
#include <filesystem>

shaders::shaders() {
	// recursively load all files in the resources folder
	for (
		const auto path :
		std::filesystem::recursive_directory_iterator("res")
	) {
		if (
			path.path().extension() != ".frag"
			&& path.path().extension() != ".vert"
		) {
			continue;
		}
		const auto name = path.path().stem().string();
		if (map.contains(name)) {
			continue;
		}

		std::string frag_path_str{};
		std::string vert_path_str{};
		auto shdr = std::make_unique<sf::Shader>();
		if (path.path().extension() == ".frag") {
			frag_path_str = path.path().string();
			if (
				auto vert_path = path.path();
				vert_path.replace_extension(".vert"),
				std::filesystem::exists(vert_path)
			) {
				vert_path_str = vert_path.string();
			}
			else {
				shdr->loadFromFile(frag_path_str, sf::Shader::Type::Fragment);
			}
		}
		else {
			vert_path_str = path.path().string();
			if (
				auto frag_path = path.path();
				frag_path.replace_extension(".frag"),
				std::filesystem::exists(frag_path)
			) {
				frag_path_str = frag_path.string();
			}
			else {
				shdr->loadFromFile(vert_path_str, sf::Shader::Type::Vertex);
			}
		}
		if (!vert_path_str.empty() && !frag_path_str.empty()) {
			shdr->loadFromFile(vert_path_str, frag_path_str);
		}

		map.insert({ name, std::move(shdr) });
	}
}
