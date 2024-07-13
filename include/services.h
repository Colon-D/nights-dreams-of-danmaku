#pragma once
#include <random>
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include "resource/audio.h"
#include "resource/texture.h"
#include "resource/shaders.h"
#include <memory>

// eww yucky globals yikes cringe what the heck?!

// available everywhere, will exist and be used by everything
class services {
public:
	services();

	texture texture{};
	audio audio{};
	shaders shaders{};
};

extern std::unique_ptr<services> serv;

// only available in a scene, passed into objects that need it
struct scene_services {
	std::default_random_engine rng{ std::random_device{}() };
	entt::registry registry{};
};
