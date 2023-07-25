#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <variant>
#include <vector>

template<class... types>
struct overloaded : types... { using types::operator()...; };

struct input {
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

	sf::Vector2f movement() const;
	bool boost() const;

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
private:
	float total(const inputs& inputs) const;
};
