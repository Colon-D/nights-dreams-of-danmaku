cmake_minimum_required(VERSION 3.14)
include(FetchContent)

FetchContent_Declare(
  imgui-sfml
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
  GIT_TAG        ea890431d2c47e02f84b08d1467a881b88732102
)
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
option(IMGUI_SFML_IMGUI_DEMO "Build imgui_demo.cpp" ON)
FetchContent_MakeAvailable("imgui-sfml")
