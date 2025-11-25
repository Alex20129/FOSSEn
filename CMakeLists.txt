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

FIND_PACKAGE(OpenSSL REQUIRED)
MESSAGE("OpenSSL version: ${OPENSSL_VERSION}")
LINK_LIBRARIES("${OPENSSL_LIBRARIES}")

FIND_PACKAGE(Qt6 COMPONENTS Core Network Sql WebEngineCore REQUIRED)
LINK_LIBRARIES(Qt6::Core Qt6::Network Qt6::Sql Qt6::WebEngineCore)

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
	web_page_processor.hpp
	web_page_processor.cpp
	util.hpp
	util.cpp)
