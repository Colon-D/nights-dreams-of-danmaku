module;
#include "assert_with_message.h"
#include <SFML/System/Vector2.hpp>
#include <cmath>

export module math_utils;

export template <typename type>
type lerp(const type& a, const type& b, const float t) {
	return a + t * (b - a);
}

export float mag(const sf::Vector2f& vec) {
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

/// asserts that division by 0 does not happen, on release returns {}
export [[nodiscard]]
sf::Vector2f normalized(const sf::Vector2f& vec) {
	const float magnitude{ mag(vec) };
	// stop division by 0
	if (magnitude == 0.f) {
		assert_with_message(false, "normalized() called on vector with magnitude 0");
		return {};
	}
	return vec / magnitude;
}

// https://stackoverflow.com/a/7869457
export template<typename dividend_t, typename divisor_t>
constexpr dividend_t fmod_pos(dividend_t dividend, divisor_t divisor) {
	return std::fmod((std::fmod(dividend, divisor) + divisor), divisor);
}
