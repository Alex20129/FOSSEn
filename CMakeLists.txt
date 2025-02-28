CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(FOSSEn)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(CMAKE_CXX_FLAGS "-O0 -Wall -Wextra -Wpedantic")

ADD_SUBDIRECTORY(phantomjs)
INCLUDE_DIRECTORIES(phantomjs/src)
LINK_LIBRARIES(phantomjs_static)

FIND_PACKAGE(Qt5 COMPONENTS Core Network WebKitWidgets REQUIRED)
MESSAGE("Using Qt version ${Qt5Core_VERSION}")
IF(Qt5Core_VERSION VERSION_LESS 5.15)
	MESSAGE(FATAL_ERROR "This version of Qt is not supported. Please use Qt 5.15 or later")
ENDIF()
LINK_LIBRARIES(Qt5::Core Qt5::Network Qt5::WebKitWidgets)

SET(CMAKE_AUTOMOC ON)

FIND_PACKAGE(Threads REQUIRED)
LINK_LIBRARIES(Threads::Threads)

LINK_LIBRARIES(jansson curl htmlcxx ssl crypto)

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}
	main.hpp
	main.cpp
	phantom_wrapper.hpp
	phantom_wrapper.cpp)
