#include "bosses/gillwing.h"
#include "services.h"
#include "math_utils.h"
#include "angle.h"

entt::entity create_gillwing(entt::registry& ecs) {
	const auto gillwing_e = ecs.create();

	//ecs.emplace<gillwing>(gillwing_e);
	gillwing_segment* prev_seg = &ecs.emplace<gillwing_segment>(gillwing_e);

	auto gillwing_spr = serv->texture.sprite("gillwing");
	gillwing_spr.origin = { 750.f, 300.f };
	constexpr float gillwing_scale{ 0.25f };
	gillwing_spr.size *= gillwing_scale;
	ecs.emplace<sprite>(gillwing_e, gillwing_spr);
	const transform gillwing_transform{ { -500.f, -150.f } };
	ecs.emplace<transform>(gillwing_e, gillwing_transform);
	ecs.emplace<prev<transform>>(gillwing_e, gillwing_transform);

	ecs.emplace<velocity>(gillwing_e);
	ecs.emplace<rotate_towards_velocity>(gillwing_e, true);

	ecs.emplace<hitbox>(gillwing_e, 52.f);
	ecs.emplace<painful>(gillwing_e);

	// wait, but then this wouldn't be fixed length...
	// but alternative would staighten out...
	// ecs.emplace<position_history>(gillwing_e, fixed_framerate);

	ecs.emplace<boss>(gillwing_e,
		std::queue<std::function<void()>>{ {
			[&, gillwing_e] {
				ecs.emplace<wave_movement>(gillwing_e, wave_movement{ { ecs, gillwing_e } });
				ecs.emplace<wave_danmaku>(gillwing_e, wave_danmaku{ { ecs, gillwing_e } });
			}
		} }
	);

	// all segments
	constexpr int num_segments{ 20 };
	for (int i{ 0 }; i < num_segments; ++i) {
		const auto seg_e = ecs.create();
		prev_seg->next_segment = seg_e;

		prev_seg = &ecs.emplace<gillwing_segment>(seg_e);

		auto seg_spr = serv->texture.sprite(i == num_segments - 1 ? "gillwing_tail" : i % 2 ? "gillwing_odd" : "gillwing_even");
		const auto seg_scale = static_cast<float>(
			std::lerp(1.f, 0.5f, i / (num_segments - 1.f))
		);
		seg_spr.size.x *= gillwing_scale;
		seg_spr.size.y *= seg_scale * gillwing_scale;
		ecs.emplace<sprite>(seg_e, seg_spr);
		ecs.emplace<transform>(seg_e, gillwing_transform);
		ecs.emplace<prev<transform>>(seg_e, gillwing_transform);

		ecs.emplace<hitbox>(seg_e, 16.f * seg_scale);
		ecs.emplace<painful>(seg_e);
	}

	return gillwing_e;
}

void gillwing_update(scene_services& scn) {
	auto& mut_ecs = scn.registry;
	const auto& ecs = mut_ecs;

	for (const auto& [e, g, t, s] : ecs.view<gillwing_segment, transform, sprite>().each()) {
		// get next segment
		const auto next_seg = g.next_segment;
		// update next segment's position to be a set distance away
		if (next_seg != entt::null) {
			mut_ecs.patch<transform>(next_seg, [&](transform& next_t) {
				// get direction from this segment to next segment
				auto dir = next_t.pos - t.pos;
				if (dir.x == 0.f && dir.y == 0.f) {
					return;
				}
				// set magnitude to be a set distance
				constexpr float segment_distance{ 18.f };
				dir = segment_distance * normalized(dir);
				// set next segment's position
				next_t.pos = t.pos + dir;
				// rotate next segment to face this segment
				next_t.rot = angle::from_vec(dir);
			});
		}
	}
}
