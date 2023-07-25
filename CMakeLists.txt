cmake_minimum_required(VERSION 3.26)
project("nights_dreams_of_danmaku" CXX)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(extern)

add_executable("${PROJECT_NAME}"
	"src/main.cpp"
	"src/resource/audio.cpp"
	"src/resource/texture.cpp"
	"src/ui.cpp"
	"src/input.cpp"
	"src/angle.cpp"
	"src/math_utils.cpp"
	"src/components.cpp"
	"src/prev.cpp"
)
target_compile_features("${PROJECT_NAME}" PUBLIC cxx_std_20)

set_target_properties(
	"${PROJECT_NAME}" PROPERTIES
	CXX_EXTENSIONS OFF
)

target_include_directories("${PROJECT_NAME}" PUBLIC
	"include"
)

target_link_libraries("${PROJECT_NAME}" PUBLIC
	"EnTT::EnTT"
	"sfml-graphics"
	"sfml-window"
	"sfml-system"
	"sfml-audio"
	"ImGui-SFML::ImGui-SFML"
)
