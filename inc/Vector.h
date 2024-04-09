#pragma once
#include <numbers>
#include <SFML/System/Vector2.hpp>

inline float length(const sf::Vector2f& v)
{
	return std::hypot(v.x, v.y);
}

inline float dotProduct(const sf::Vector2f& a, const sf::Vector2f& b)
{
	return a.x * b.x + a.y * b.y;
}

inline float angleDegree(const sf::Vector2f& a, const sf::Vector2f& b, float max = 360.f)
{
	const float cos = dotProduct(a, b) / (length(a) * length(b));
	const float angle = std::acos(cos) * 180.f / std::numbers::pi_v<float>;
	const float remains = angle / max - std::trunc(angle / max);
	return remains * max;
}

inline sf::Vector2f rotate(const sf::Vector2f& v, float angleDegree)
{
	const float cos = std::cos(angleDegree * std::numbers::pi_v<float> / 180.f);
	const float sin = std::sin(angleDegree * std::numbers::pi_v<float> / 180.f);

	return sf::Vector2f{ v.x * cos - v.y * sin, v.x * sin + v.y * cos };
}