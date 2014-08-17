cmake_minimum_required(VERSION 3.0.0)

project(UTLauncher)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")
include(MingwResource)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(PkgConfig)

pkg_check_modules(LIBTORRENT libtorrent-rasterbar)

message(STATUS "${LIBTORRENT_CFLAGS}")
message(STATUS "${LIBTORRENT_LIBRARIES}")

qt5_add_resources(resources files.qrc)

string (REPLACE ";" " " LIBTORRENT_CFLAGS_STR "${LIBTORRENT_CFLAGS} -DBOOST_THREAD_USE_LIB")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y ${LIBTORRENT_CFLAGS_STR}")

message(STATUS "${CMAKE_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-arcs -ftest-coverage")

if(CMAKE_BUILD_TYPE MATCHES Release)
add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

if(CMAKE_SYSTEM_NAME MATCHES Windows)
add_win32_resource(rc_srcs application.rc)
endif(CMAKE_SYSTEM_NAME MATCHES Windows)

add_executable(UTLauncher WIN32 ${resources} ${rc_srcs} main.cpp utlauncher.cpp download.cpp torrentdownloader.cpp utsplash.cpp bootstrap.cpp serverbrowser.cpp)

add_definitions(-DNO_DOWNLOAD)

if(STATIC_BUILD_WIN32)
	add_definitions(-DSTATIC_PLUGIN_WINDOWS)
	target_link_libraries(UTLauncher Qt5::Widgets Qt5::QWindowsIntegrationPlugin Qt5::Network ${LIBTORRENT_LIBRARIES} z harfbuzz opengl32 jpeg png pcre16 sicuuc glib-2.0 sicudt intl iconv ws2_32 winmm imm32 freetype bz2 ssl crypto boost_system-mt )
else()
	target_link_libraries(UTLauncher Qt5::Widgets Qt5::Network ${LIBTORRENT_LIBRARIES} ssl crypto boost_system-mt )
endif()
message(STATUS ${STATIC_BUILD_WIN32_LIBRARIES})
