cmake_minimum_required(VERSION 2.8.12)

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
find_package(ZLIB)
#find_package(Boost COMPONENTS system REQUIRED)

# appindicator --------------------------------------------
if(WITH_APPINDICATOR)
	pkg_check_modules(PC_APPINDICATOR REQUIRED appindicator-0.1)
	pkg_check_modules(GTK2 REQUIRED gtk+-2.0)

	find_path(APPINDICATOR_INCLUDE_DIR NAMES libappindicator/app-indicator.h
		HINTS ${PC_APPINDICATOR_INCLUDEDIR} ${PC_APPINDICATOR_INCLUDE_DIRS}
		PATH_SUFFIXES libappindicator-0.1)

	find_library(APPINDICATOR_LIBRARY NAMES appindicator)

	include(FindPackageHandleStandardArgs)

	find_package_handle_standard_args(APPINDICATOR DEFAULT_MSG APPINDICATOR_LIBRARY APPINDICATOR_INCLUDE_DIR)

	if(APPINDICATOR_FOUND)
		set(APPINDICATOR_LIBRARIES ${APPINDICATOR_LIBRARY})
		set(APPINDICATOR_INCLUDE_DIRS ${APPINDICATOR_INCLUDE_DIR})
	endif()

	mark_as_advanced(APPINDICATOR_INCLUDE_DIR APPINDICATOR_LIBRARY)
endif()
# ------------------------------------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

#pkg_check_modules(LIBTORRENT libtorrent-rasterbar)
#string (REPLACE ";" " " LIBTORRENT_CFLAGS_STR "${LIBTORRENT_CFLAGS} -DBOOST_THREAD_USE_LIB")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBTORRENT_CFLAGS_STR}")

if(CMAKE_BUILD_TYPE MATCHES Release)
#add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

add_subdirectory(3rdparty)
if(APPINDICATOR_FOUND)
	include_directories(${3rdparty_INCLUDE_DIRS} ${GTK2_INCLUDE_DIRS} ${APPINDICATOR_INCLUDE_DIR})
	link_directories(${GTK2_LIBRARY_DIRS} ${APPINDICATOR_LIBRARY})
else()
	include_directories(${3rdparty_INCLUDE_DIRS})
endif()

qt5_add_resources(resources files.qrc flags.qrc)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
add_win32_resource(rc_srcs application.rc)
endif(CMAKE_SYSTEM_NAME MATCHES Windows)

add_definitions(-DLAUNCH_WITH_UE4)
if(CMAKE_SYSTEM_NAME MATCHES Windows AND CMAKE_BUILD_TYPE MATCHES Release)
	set(BUILD_TYPE WIN32)
	#add_definitions(-DLAUNCH_WITH_UE4)
else()
	set(BUILD_TYPE "")
endif()

set(UTLauncher_VERSION "0.3.6" CACHE STRING "Version number" FORCE INTERNAL)
set_version_number(UTLauncher)

add_definitions(-DNO_DOWNLOAD -DVERSION_MAJOR=${UTLauncher_VERSION_MAJOR} -DVERSION_MINOR=${UTLauncher_VERSION_MINOR} -DVERSION_PATCH=${UTLauncher_VERSION_PATCH})
add_executable(UTLauncher ${BUILD_TYPE} ${resources} ${rc_srcs} main.cpp utlauncher.cpp download.cpp utsplash.cpp bootstrap.cpp serverbrowser.cpp configdialog.cpp $<TARGET_OBJECTS:QtAwesome> $<TARGET_OBJECTS:quazip>)


if(STATIC_BUILD_WIN32)
	pkg_check_modules(QT5WIDGETS Qt5Widgets)
	add_definitions(-DSTATIC_PLUGIN_WINDOWS)
	target_link_libraries(UTLauncher Qt5::Widgets Qt5::QWindowsIntegrationPlugin Qt5::Network ${QT5WIDGETS_LDFLAGS} ${ZLIB_LIBRARY})
else()
	if(APPINDICATOR_FOUND)
		add_definitions(-DAPPINDICATOR)
		target_link_libraries(UTLauncher Qt5::Widgets Qt5::Network ${Boost_SYSTEM_LIBRARY} ${ZLIB_LIBRARY} ${GTK2_LIBRARIES} ${APPINDICATOR_LIBRARY})
	else()
		target_link_libraries(UTLauncher Qt5::Widgets Qt5::Network ${Boost_SYSTEM_LIBRARY} ${ZLIB_LIBRARY})
	endif()
endif()

install(TARGETS UTLauncher RUNTIME DESTINATION bin)
