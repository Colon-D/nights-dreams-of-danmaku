#include "prev.h"
#include "assert_with_message.h"
#include "components.h"

basic_previous_system::basic_previous_system(entt::registry& registry) :
	registry{ registry } {
	if constexpr (debug_connect_debug_lerp) {
		registry.on_update<transform>().connect<&debug_lerp>();
	}
}

void basic_previous_system::debug_lerp(entt::registry& registry, const entt::entity entity) {
	if (!registry.all_of<prev<transform>>(entity)) {
		assert_with_message(
			false, "transform moved without prev<transform>"
		);
	}
}
