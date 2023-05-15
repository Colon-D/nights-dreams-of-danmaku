module;
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <variant>
#include <vector>

export module input;
import math_utils;

struct key_in {
	sf::Keyboard::Key key{};
};

struct button_in {
	unsigned joystick_index{};
	unsigned button_index{};
};

enum class polarity {
	negative,
	positive
};

struct half_axis_in {
	unsigned joystick_index{};
	sf::Joystick::Axis axis{};
	polarity polarity{};
};

using single_input = std::variant<key_in, button_in, half_axis_in>;
using inputs = std::vector<single_input>;

template<class... types>
struct overloaded : types... { using types::operator()...; };

float total(const inputs& inputs) {
	float total{ 0.f };

	for (const auto& input : inputs) {
		total += std::visit(overloaded{
			[](const key_in& key) {
				return sf::Keyboard::isKeyPressed(key.key) ? 1.f : 0.f;
			},
			[](const button_in& button) {
				return sf::Joystick::isButtonPressed(
					button.joystick_index, button.button_index
				) ? 1.f : 0.f;
			},
			[](const half_axis_in& half_axis) {
				float value{ sf::Joystick::getAxisPosition(
					half_axis.joystick_index, half_axis.axis
				) / 100.f };
				if (half_axis.polarity == polarity::negative) {
					value = -value;
				}

				// this results in a + shaped deadzone
				// if this not wanted, apply deadzone in movement()
				constexpr float deadzone{ 0.075f };
				if (value < deadzone) {
					return 0.f;
				}

				return value;
			}
		}, input);
	}

	return std::min(total, 1.f);
}

export struct input {
	sf::Vector2f movement() const {
		sf::Vector2f movement{
			total(move_right_inputs) - total(move_left_inputs),
			total(move_down_inputs) - total(move_up_inputs),
		};
		// don't allow digital input to move faster diagonally
		if (mag(movement) > 1.f) {
			movement = normalized(movement);
		}
		return movement;
	}
	bool boost() const {
		return total(boost_inputs);
	}

	inputs move_up_inputs{};
	inputs move_left_inputs{};
	inputs move_down_inputs{};
	inputs move_right_inputs{};
	inputs boost_inputs{};

	static const input default_keyboard;
	constexpr static input default_gamepad(const unsigned index = 0) {
		constexpr unsigned a_button{ 0 };
		constexpr unsigned dpad_up_button{ 9 };
		constexpr unsigned dpad_down_button{ 10 };
		constexpr unsigned dpad_left_button{ 11 };
		constexpr unsigned dpad_right_button{ 12 };
		return {
			{ half_axis_in{ index, sf::Joystick::Y, polarity::negative }, half_axis_in{ index, sf::Joystick::PovY, polarity::positive } },
			{ half_axis_in{ index, sf::Joystick::X, polarity::negative }, half_axis_in{ index, sf::Joystick::PovX, polarity::negative } },
			{ half_axis_in{ index, sf::Joystick::Y, polarity::positive }, half_axis_in{ index, sf::Joystick::PovY, polarity::negative } },
			{ half_axis_in{ index, sf::Joystick::X, polarity::positive }, half_axis_in{ index, sf::Joystick::PovX, polarity::positive } },
			{ button_in{ index, a_button } }
		};
	}
};

const input input::default_keyboard{
	{ key_in{ sf::Keyboard::Key::Up    }, key_in{sf::Keyboard::Key::W } },
	{ key_in{ sf::Keyboard::Key::Left  }, key_in{sf::Keyboard::Key::A } },
	{ key_in{ sf::Keyboard::Key::Down  }, key_in{sf::Keyboard::Key::S } },
	{ key_in{ sf::Keyboard::Key::Right }, key_in{sf::Keyboard::Key::D } },
	{ key_in{ sf::Keyboard::Key::Space }, key_in{sf::Keyboard::Key::Z } }
};
