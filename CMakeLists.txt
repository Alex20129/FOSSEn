CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
PROJECT(FOSSEn)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
SET(CMAKE_AUTOMOC ON)

IF(CMAKE_BUILD_TYPE STREQUAL "Release")
	ADD_COMPILE_OPTIONS(-O3)
ELSE()
	ADD_COMPILE_OPTIONS(-O0)
ENDIF()

ADD_SUBDIRECTORY(phantomjs)
INCLUDE_DIRECTORIES(phantomjs/src)
LINK_LIBRARIES(phantomjs_static)

FIND_PACKAGE(OpenSSL REQUIRED)
MESSAGE("OpenSSL version: ${OPENSSL_VERSION}")
LINK_LIBRARIES("${OPENSSL_LIBRARIES}")

FIND_PACKAGE(Qt5 COMPONENTS Core Network WebKitWidgets Sql REQUIRED)
MESSAGE("Qt version ${Qt5Core_VERSION}")
IF(Qt5Core_VERSION VERSION_LESS 5.15)
	MESSAGE(FATAL_ERROR "This version of Qt is not supported. Please use Qt 5.15 or later")
ENDIF()
LINK_LIBRARIES(Qt5::Core Qt5::Network Qt5::WebKitWidgets Qt5::Sql)

FIND_PACKAGE(Threads REQUIRED)
LINK_LIBRARIES(Threads::Threads)
LINK_LIBRARIES(jansson)

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}
	main.hpp
	main.cpp
	crawler.hpp
	crawler.cpp
	indexer.hpp
	indexer.cpp
	simple_hash_func.hpp
	simple_hash_func.cpp
	phantom_wrapper.hpp
	phantom_wrapper.cpp)
