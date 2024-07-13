#pragma once
#include <string>
#include <unordered_map>
#include <SFML/Graphics/Shader.hpp>
#include <memory>

// loads .vert and .frag shader files from the res folder
class shaders {
public:
	shaders();

	std::unordered_map<std::string, std::unique_ptr<sf::Shader>> map{};
};
