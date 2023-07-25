#pragma once
#include "assert_with_message.h"
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <vector>

template <typename type>
constexpr type lerp(const type& a, const type& b, const float t) {
	return a + t * (b - a);
}

float mag(const sf::Vector2f& vec);

/// asserts that division by 0 does not happen, on release returns {}
[[nodiscard]]
sf::Vector2f normalized(const sf::Vector2f& vec);

// https://stackoverflow.com/a/7869457
template<typename dividend_t, typename divisor_t>
constexpr dividend_t fmod_pos(dividend_t dividend, divisor_t divisor) {
	return std::fmod((std::fmod(dividend, divisor) + divisor), divisor);
}

struct line {
	sf::Vector2f begin{};
	sf::Vector2f end{};
};

// https://gist.github.com/TimSC/47203a0f5f15293d2099507ba5da44e6
/// Calculate intersection of two lines.
///\return true if found, false if not found or error
bool intersects(
	const line& lhs, const line& rhs
);

// https://stackoverflow.com/a/2922778/2562287
bool intersects(
	const sf::Vector2f& point, const std::vector<sf::Vector2f> polygon
);
