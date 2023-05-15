module;
#include <numbers>
#include <SFML/System/Vector2.hpp>
#include <utility>

constexpr float pi = std::numbers::pi_v<float>;

export module angle;
import math_utils;

export struct angle {
	constexpr static angle deg(const float deg) {
		return angle{}.set_deg(deg);
	}
	constexpr static angle rad(const float deg) {
		return angle{}.set_deg(deg);
	}

	constexpr float deg() const {
		return radians * 180.f / pi;
	}

	constexpr float rad() const {
		return radians;
	}

	constexpr angle& set_deg(const float deg) {
		radians = deg * pi / 180.f;
		return *this;
	}

	constexpr angle& set_rad(const float radians) {
		this->radians = radians;
		return *this;
	}

	sf::Vector2f vec() const {
		return {
			cosf(radians),
			sinf(radians)
		};
	}
	static angle from_vec(const sf::Vector2f& vec) {
		return angle{}.set_rad(atan2(vec.y, vec.x));
	}

	// https://stackoverflow.com/a/7869457
	angle diff(const angle to) const {
		const float diff{ to.radians - radians };
		return angle::rad(fmod_pos(diff + pi, pi * 2.f) - pi);
	}

	[[nodiscard]]
	angle rotate_towards(const angle target, const angle by) const;

	constexpr angle operator-() const {
		return angle{}.set_rad(-radians);
	}

	constexpr angle& operator+=(const angle& rhs) {
		radians += rhs.radians;
		return *this;
	}
	constexpr angle& operator-=(const angle& rhs) {
		radians -= rhs.radians;
		return *this;
	}
	constexpr angle& operator*=(const float scalar) {
		radians *= scalar;
		return *this;
	}
	constexpr angle& operator/=(const float scalar) {
		radians /= scalar;
		return *this;
	}
private:
	float radians{};
};

export constexpr bool operator<(const angle& lhs, const angle& rhs) {
	return lhs.rad() < rhs.rad();
}
export constexpr bool operator>(const angle& lhs, const angle& rhs) {
	return lhs.rad() > rhs.rad();
}
export constexpr bool operator<=(const angle& lhs, const angle& rhs) {
	return lhs.rad() <= rhs.rad();
}
export constexpr bool operator>=(const angle& lhs, const angle& rhs) {
	return lhs.rad() >= rhs.rad();
}
export constexpr bool operator==(const angle& lhs, const angle& rhs) {
	return lhs.rad() == rhs.rad();
}
export constexpr bool operator!=(const angle& lhs, const angle& rhs) {
	return lhs.rad() != rhs.rad();
}

export constexpr angle operator+(angle lhs, const angle& rhs) {
	return lhs += rhs;
}
export constexpr angle operator-(angle lhs, const angle& rhs) {
	return lhs -= rhs;
}
export constexpr angle operator*(angle lhs, const float scalar) {
	return lhs *= scalar;
}
export constexpr angle operator/(angle lhs, const float scalar) {
	return lhs /= scalar;
}

[[nodiscard]]
inline angle angle::rotate_towards(const angle target, const angle by) const {
	auto out = *this;
	const auto diff = target.diff(out);

	if (diff.rad() < 0.f) {
		out += std::max(diff, by);
	}
	else {
		out += std::min(diff, -by);
	}
	return out;
}
