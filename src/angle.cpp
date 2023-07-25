#include "angle.h"

sf::Vector2f angle::vec() const {
	return {
		cosf(radians),
		sinf(radians)
	};
}

angle angle::from_vec(const sf::Vector2f& vec) {
	return angle{}.set_rad(atan2(vec.y, vec.x));
}

[[nodiscard]]
angle angle::rotate_towards(const angle target, const angle by) const {
	auto out = *this;
	const auto diff = out.diff(target);
	if (diff.rad() < 0.f) {
		out += std::max(diff, -by);
	}
	else {
		out += std::min(diff, by);
	}
	return out;
}
