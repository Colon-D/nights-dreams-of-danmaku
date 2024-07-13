#pragma once
#include <entt/entt.hpp>
#include "components.h"
#include "prev.h"
#include "resource/texture.h"
#include "attacks/attack.h"
#include "attacks/gillwing/wave.h"
#include "bosses/boss.h"

//struct gillwing {
//	float timer{ 0.f };
//	bool heading_right{ true };
//};

struct gillwing_segment {
	// next segment moves towards the tail
	entt::entity next_segment{ entt::null };
};

entt::entity create_gillwing(entt::registry& ecs);

void gillwing_update(scene_services& scn);
