cmake_minimum_required(VERSION 3.14)
include(FetchContent)

FetchContent_Declare(
  "sfml"
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG        2.6.x
)
FetchContent_MakeAvailable("sfml")
