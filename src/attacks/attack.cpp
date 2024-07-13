#include "attacks/attack.h"

entt::sigh<void(scene_services&)> attack_system::update_sigh;
entt::sink<decltype(attack_system::update_sigh)> attack_system::on_update{
	attack_system::update_sigh
};
