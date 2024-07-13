#pragma once
#include "attacks/attack.h"
#include "resource/texture.h"

struct wave_movement : basic_attack<wave_movement> {
	using basic_attack::basic_attack;
	
	bool moving_right{ true };
	float elapsed{ 0.f };

	static void update(scene_services& scn);
};

struct wave_danmaku : basic_attack<wave_danmaku> {
	using basic_attack::basic_attack;
	float reload{ 0.f };
	bool alt_color{ false };

	static void update(scene_services& scn);
};
