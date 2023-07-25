#pragma once
#include <numbers>
#include <SFML/System/Vector2.hpp>
#include <utility>
#include <iostream>
#include "math_utils.h"

constexpr float pi = std::numbers::pi_v<float>;

struct angle {
	constexpr static angle deg(const float deg) {
		return angle{}.set_deg(deg);
	}
	constexpr static angle rad(const float deg) {
		return angle{}.set_rad(deg);
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

	sf::Vector2f vec() const;
	static angle from_vec(const sf::Vector2f& vec);

	// https://stackoverflow.com/a/7869457
	constexpr angle diff(const angle to) const {
		const float diff{ to.radians - radians };
		return angle::rad(fmod_pos(diff + pi, pi * 2.f) - pi);
	}

	constexpr angle abs() const {
		return angle::rad(std::abs(radians));
	}

	// https://stackoverflow.com/a/7869457
	// returns value normalized to [-pi, pi]
	[[nodiscard]]
	constexpr angle normalize() const {
		return angle::rad(fmod_pos(radians + pi, pi * 2.f) - pi);
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

constexpr bool operator<(const angle& lhs, const angle& rhs) {
	return lhs.rad() < rhs.rad();
}
constexpr bool operator>(const angle& lhs, const angle& rhs) {
	return lhs.rad() > rhs.rad();
}
constexpr bool operator<=(const angle& lhs, const angle& rhs) {
	return lhs.rad() <= rhs.rad();
}
constexpr bool operator>=(const angle& lhs, const angle& rhs) {
	return lhs.rad() >= rhs.rad();
}
constexpr bool operator==(const angle& lhs, const angle& rhs) {
	return lhs.rad() == rhs.rad();
}
constexpr bool operator!=(const angle& lhs, const angle& rhs) {
	return lhs.rad() != rhs.rad();
}

constexpr angle operator+(angle lhs, const angle& rhs) {
	return lhs += rhs;
}
constexpr angle operator-(angle lhs, const angle& rhs) {
	return lhs -= rhs;
}
constexpr angle operator*(angle lhs, const float scalar) {
	return lhs *= scalar;
}
constexpr angle operator*(const float scalar, angle rhs) {
	return rhs *= scalar;
}
constexpr angle operator/(angle lhs, const float scalar) {
	return lhs /= scalar;
}

constexpr angle lerp(const angle& a, const angle& b, const float t) {
	return angle::rad(lerp(a.rad(), (a + b.diff(a)).rad(), t));
}

