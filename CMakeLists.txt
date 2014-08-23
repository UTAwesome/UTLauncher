cmake_minimum_required(VERSION 3.0.0)

project(UTLauncher)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")
include(MingwResource)
include(Version)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(PkgConfig)
find_package(Boost COMPONENTS system REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#pkg_check_modules(LIBTORRENT libtorrent-rasterbar)
#string (REPLACE ";" " " LIBTORRENT_CFLAGS_STR "${LIBTORRENT_CFLAGS} -DBOOST_THREAD_USE_LIB")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBTORRENT_CFLAGS_STR}")

if(CMAKE_BUILD_TYPE MATCHES Release)
add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

add_subdirectory(3rdparty)
include_directories(${3rdparty_INCLUDE_DIRS})

qt5_add_resources(resources files.qrc flags.qrc)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
add_win32_resource(rc_srcs application.rc)
endif(CMAKE_SYSTEM_NAME MATCHES Windows)

if(CMAKE_SYSTEM_NAME MATCHES Windows AND CMAKE_BUILD_TYPE MATCHES Release)
	set(BUILD_TYPE WIN32)
else()
	set(BUILD_TYPE "")
endif()

set(UTLauncher_VERSION "0.2.6" CACHE STRING "Version number" FORCE INTERNAL)
set_version_number(UTLauncher)

add_definitions(-DNO_DOWNLOAD -DVERSION_MAJOR=${UTLauncher_VERSION_MAJOR} -DVERSION_MINOR=${UTLauncher_VERSION_MINOR} -DVERSION_PATCH=${UTLauncher_VERSION_PATCH})
add_executable(UTLauncher ${BUILD_TYPE} ${resources} ${rc_srcs} main.cpp utlauncher.cpp download.cpp utsplash.cpp bootstrap.cpp serverbrowser.cpp configdialog.cpp $<TARGET_OBJECTS:QtAwesome>)


if(STATIC_BUILD_WIN32)
	pkg_check_modules(QT5WIDGETS Qt5Widgets)
	add_definitions(-DSTATIC_PLUGIN_WINDOWS)
	target_link_libraries(UTLauncher Qt5::Widgets Qt5::QWindowsIntegrationPlugin Qt5::Network ${QT5WIDGETS_LDFLAGS})
else()
	target_link_libraries(UTLauncher Qt5::Widgets Qt5::Network ${Boost_SYSTEM_LIBRARY})
endif()

install(TARGETS UTLauncher RUNTIME DESTINATION bin)