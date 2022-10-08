project("speedwire_test")

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)

if (MSVC)
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
set (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};C:/Ralfs/VisualC++/googletest/out/install/x64-Debug) # adapt path to your setup
else()
set (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};C:/Ralfs/VisualC++/googletest/out/install/x64-Release) # adapt path to your setup
endif()
message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
endif()

find_package(GTest REQUIRED)

add_executable (${PROJECT_NAME} EXCLUDE_FROM_ALL
    speedwire_test.cpp
    RingBufferTest.cpp
    SpeedwireTimeTest.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC GTest::gtest speedwire)

if (MSVC)
target_link_libraries(${PROJECT_NAME} PUBLIC GTest::gtest speedwire ws2_32.lib Iphlpapi.lib)
else()
target_link_libraries(${PROJECT_NAME} PUBLIC GTest::gtest speedwire)
endif()
