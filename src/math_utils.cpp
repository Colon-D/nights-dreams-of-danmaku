#include "math_utils.h"

float mag(const sf::Vector2f& vec) {
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

[[nodiscard]]
sf::Vector2f normalized(const sf::Vector2f& vec) {
	const float magnitude{ mag(vec) };
	// stop division by 0
	if (magnitude == 0.f) {
		assert_with_message(false, "normalized() called on vector with magnitude 0");
		return {};
	}
	return vec / magnitude;
}
