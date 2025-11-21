cmake_minimum_required (VERSION 3.17.0)

project (CladeChat VERSION 0.8.0 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set(MOC_SOURCE
	actions.hh
	addaccount.hh
	chatwindow.hh
	chatworker.hh
	contact.hh
	net.h
	userworker.hh
	window.hh
)

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

find_package(OpenSSL)

qt_standard_project_setup()

qt_add_executable(CladeChat
	client.cc
	addaccount.cc
	chatwindow.cc
	chatworker.cc
	userworker.cc
	window.cc
)

target_link_libraries(CladeChat PRIVATE Qt6::Widgets Qt6::Concurrent)

target_link_libraries(CladeChat PRIVATE OpenSSL::SSL)

install (TARGETS CladeChat DESTINATION bin)
