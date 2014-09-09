project(3rdparty)

qt5_add_resources(faresources QtAwesome/QtAwesome/QtAwesome.qrc)

include_directories(QtAwesome/QtAwesome ${Qt5Widgets_INCLUDE_DIRS})

add_library(QtAwesome OBJECT QtAwesome/QtAwesome/QtAwesome.cpp ${faresources})


set_target_properties(QtAwesome PROPERTIES COMPILE_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")





include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/quazip/quazip
  ${CMAKE_CURRENT_BINARY_DIR}/quazip/quazip
  ${ZLIB_INCLUDE_DIR}
)

file(GLOB SRCS "quazip/quazip/*.c" "quazip/quazip/*.cpp")
file(GLOB PUBLIC_HEADERS "quazip/quazip/*.h")

# Must be added to enable export macro
ADD_DEFINITIONS(-DQUAZIP_BUILD)

add_library(quazip OBJECT ${SRCS})
#target_link_libraries(quazip Qt5::Core ${ZLIB_LIBRARY})

set(3rdparty_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/QtAwesome/QtAwesome CACHE INTERNAL "")

#target_link_libraries(QtAwesome Qt5::Widgets)
