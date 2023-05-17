cmake_minimum_required(VERSION 3.25...3.26)
project("nights_dreams_of_danmaku" CXX)

# C++ Modules experimental support
if(${CMAKE_VERSION} VERSION_LESS "3.26")
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "3c375311-a3c9-4396-a187-3227ef642046")
else()
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
endif()

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(extern)

add_executable("${PROJECT_NAME}"
	"src/main.cpp"
 	"src/angle.ixx"
	"src/input.ixx"
	"src/math_utils.ixx"
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
