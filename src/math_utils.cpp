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

float cross(const sf::Vector2f& lhs, const sf::Vector2f& rhs) {
	return lhs.x * rhs.y - lhs.y * rhs.x;
}

float dot(const sf::Vector2f& lhs, const sf::Vector2f& rhs) {
	return lhs.x * rhs.x + lhs.y * rhs.y;
}

// https://stackoverflow.com/a/565282/2562287
// used for paralooping, does NOT allow collinear collisions, and lines that
// have an equal point are NOT considered to intersect
bool intersects(const line& lhs, const line& rhs) {
	const auto p = lhs.begin;
	const auto r = lhs.end - lhs.begin;
	const auto q = rhs.begin;
	const auto s = rhs.end - rhs.begin;

	const auto qps = cross((q - p), s);
	const auto qpr = cross((q - p), r);
	const auto rs = cross(r, s);

	const auto t = qps / rs;
	const auto u = qpr / rs;

	// collinear
	if (rs == 0.f && qpr == 0.f) {
		// nothing collinear is allowed
		return false;

		// collinear is allowed, below is ignored
		const auto t0 = dot((q - p), r) / dot(r, r);
		const auto sr = dot(s, r);
		const auto t1 = t0 + sr / dot(r, r);
		// collinear and overlapping
		if (sr >= 0.f && t0 <= 1.f && t1 >= 0.f) {
			return true;
		}
		else if (sr < 0.f && t0 >= 0.f && t1 <= 1.f) {
			return true;
		}
		// collinear but disjoint
		return false;
	}
	// parallel and non-intersecting
	else if (rs == 0.f && qpr != 0.f) {
		return false;
	}
	// meeting
	else if (rs != 0.f && 0.f < t && t < 1.f && 0.f < u && u < 1.f) {
		return true;
	}
	// not parallel but do not intersect
	return false;
}

bool intersects(const sf::Vector2f& point, const std::vector<sf::Vector2f> polygon) {
	bool inside{ false };
	for (std::size_t i{ 0 }; i < polygon.size(); ++i) {
		const auto j = (i + 1) % polygon.size();
		if (
			((polygon[i].y > point.y) != (polygon[j].y > point.y))
			&& (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x)
		) {
			inside = !inside;
		}
	}
	return inside;
}
