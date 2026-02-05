cmake_minimum_required (VERSION 3.5)

project (CladeChat VERSION 0.8.0 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 98)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Concurrent)

find_package(OpenSSL)

#qt_standard_project_setup()

qt_wrap_cpp(moc_sources
	actions.hh
	addaccount.hh
	chatwindow.hh
	chatworker.hh
	contact.hh
	net.h
	userworker.hh
	window.hh
)

add_executable(CladeChat
	client.cc
	addaccount.cc
	chatwindow.cc
	chatworker.cc
	userworker.cc
	window.cc
	${moc_sources}
)

target_link_libraries(CladeChat PRIVATE Qt5::Widgets Qt5::Concurrent)

target_link_libraries(CladeChat PRIVATE OpenSSL::SSL)
