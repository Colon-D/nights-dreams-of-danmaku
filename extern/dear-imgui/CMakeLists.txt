cmake_minimum_required(VERSION 3.14)
include(FetchContent)

FetchContent_Declare(
  dear-imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.89.5
)
FetchContent_MakeAvailable("dear-imgui")
# needed for imgui-sfml
set(IMGUI_DIR "${dear-imgui_SOURCE_DIR}" PARENT_SCOPE)
