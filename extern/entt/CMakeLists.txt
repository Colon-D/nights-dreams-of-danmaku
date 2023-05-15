cmake_minimum_required(VERSION 3.14)
include(FetchContent)

FetchContent_Declare(
  "entt"
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG        v3.11.x
)
FetchContent_MakeAvailable("entt")
