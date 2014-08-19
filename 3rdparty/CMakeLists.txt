project(3rdparty)

qt5_add_resources(faresources QtAwesome/QtAwesome/QtAwesome.qrc)

include_directories(QtAwesome/QtAwesome ${Qt5Widgets_INCLUDE_DIRS})

add_library(QtAwesome OBJECT QtAwesome/QtAwesome/QtAwesome.cpp ${faresources})


set_target_properties(QtAwesome PROPERTIES COMPILE_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

set(3rdparty_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/QtAwesome/QtAwesome CACHE INTERNAL "")

#target_link_libraries(QtAwesome Qt5::Widgets)
