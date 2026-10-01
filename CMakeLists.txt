cmake_minimum_required(VERSION 3.28)
project(DrivingRobot C CXX ASM)

include(${CMAKE_SOURCE_DIR}/cmake/configuration.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(${CORE_FLAGS})
add_link_options(${CORE_FLAGS})

add_subdirectory(Drivers)
add_subdirectory(Peripherals)
add_subdirectory(Scheduler)
add_subdirectory(Devices)
add_subdirectory(Core)
