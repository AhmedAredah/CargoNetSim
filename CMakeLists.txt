cmake_minimum_required(VERSION 3.16)
project(CargoNetSim VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to enable or disable tests
option(CARGONET_BUILD_TESTS "Build the CargoNetSim test suite" ON)

# Include our custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Enable automatic handling of Qt MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find required packages
include(cmake/FindDependencies.cmake)

# Define version information
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/Version.h
)

# Add subdirectories
add_subdirectory(src)

# Conditionally add tests directory
if(CARGONET_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Install configuration
install(DIRECTORY config/ DESTINATION etc/CargoNetSim)
install(DIRECTORY media/ DESTINATION share/CargoNetSim/media)

# Package configuration
include(CPack)
