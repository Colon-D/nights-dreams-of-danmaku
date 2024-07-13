#include "bosses/boss.h"

boss::boss(const std::queue<std::function<void()>>& phase_queue) :
	phase_queue{ phase_queue }
{
	change_phase_no_event();
}

void boss::change_phase(const entt::handle handle) {
	on_phase_change.sigh.publish(handle);
	change_phase_no_event();
}

float boss::get_health() const {
    return health;
}

void boss::set_health(const entt::handle handle, const float health) {
	this->health = health;
	if (this->health <= 0.f) {
		change_phase(handle);
	}
}

void boss::change_phase_no_event() {
	// change phase to next phase in queue
	if (phase_queue.empty()) {
		return;
	}
	phase_queue.front()();
	phase_queue.pop();
	// increase health
	health = 100.f;
}
