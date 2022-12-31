cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0079 NEW)

project(ImmersiveAssassination CXX)


set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SRC_FILES
	CONFIGURE_DEPENDS
	Src/*.cpp
	Src/*.c
	Src/*.hpp
	Src/*.h
)

find_package(Threads REQUIRED)
find_package(directxmath CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)
add_definitions(-D_WEBSOCKETPP_CPP11_THREAD_)

add_subdirectory(External/buttplug-cpp)
target_link_libraries(buttplug-cpp PRIVATE
	websocketpp::websocketpp
)

set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_LIST_DIR}/External/ZHMModSDK/_install/x64-Release-Mingw")
find_package(ZHMModSDK REQUIRED)

add_library(ImmersiveAssassination SHARED
	${SRC_FILES}
)

target_include_directories(ImmersiveAssassination PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/Src
	${CMAKE_CURRENT_SOURCE_DIR}/_inc
)

set_target_properties(ImmersiveAssassination PROPERTIES LINK_FLAGS_RELEASE -s)

target_link_libraries(ImmersiveAssassination PRIVATE
	imgui::imgui
	Microsoft::DirectXMath
	ZHMModSDK::ZHMModSDK
	buttplug-cpp
	ws2_32
)
