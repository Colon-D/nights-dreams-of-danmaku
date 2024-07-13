#include "services.h"

services::services() {
	texture.load(".png");
	audio.load(".ogg");
}

std::unique_ptr<services> serv{};
