#pragma once
#include <SFML/System/Vector2.hpp>

inline constexpr int fixed_framerate{ 60 };
inline constexpr float fixed_timestep{ 1.f / fixed_framerate };

extern float diff; // 0.5f = easy, 1.f = normal, 1.5f = hard, 2.f = lunatic

extern const sf::Vector2f gameplay_size;
