#include "input.h"
#include "math_utils.h"

sf::Vector2f input::movement() const
{
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

bool input::boost() const {
	return total(boost_inputs);
}

float input::total(const inputs& inputs) const {
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

const input input::default_keyboard{
	{ key_in{ sf::Keyboard::Key::Up    }, key_in{ sf::Keyboard::Key::W } },
	{ key_in{ sf::Keyboard::Key::Left  }, key_in{ sf::Keyboard::Key::A } },
	{ key_in{ sf::Keyboard::Key::Down  }, key_in{ sf::Keyboard::Key::S } },
	{ key_in{ sf::Keyboard::Key::Right }, key_in{ sf::Keyboard::Key::D } },
	{ key_in{ sf::Keyboard::Key::Space }, key_in{ sf::Keyboard::Key::Z } }
};
