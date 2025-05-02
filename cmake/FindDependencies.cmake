# Find Qt6
if(CARGONET_BUILD_TESTS)
    # Include Test component when tests are enabled
    find_package(Qt6 COMPONENTS Core Gui Widgets Network Xml Test REQUIRED)
else()
    # Standard components otherwise
    find_package(Qt6 COMPONENTS Core Gui Widgets Network Xml REQUIRED)
endif()

# Platform-specific paths
if(WIN32)
    # Windows-specific paths
    set(CONTAINER_SEARCH_PATHS "C:/Program Files/Container/cmake" CACHE PATH "Default path to container's library")
    set(RABBITMQ_CMAKE_DIR "C:/Program Files/rabbitmq-c/lib/cmake/rabbitmq-c" CACHE PATH "Default path to RabbitMQ-C library on Windows")
    set(KDREPORTS_DIR "C:/Program Files/KDAB/KDReports/cmake" CACHE PATH "Path to KDReports CMake directory")
elseif(APPLE)
    # macOS-specific paths
    set(CONTAINER_SEARCH_PATHS "/usr/local/lib/cmake/Container" CACHE PATH "Default path to container's library on macOS")
    set(RABBITMQ_CMAKE_DIR "/usr/local/lib/rabbitmq-c/cmake" CACHE PATH "Default path to RabbitMQ-C library on macOS")
    set(KDREPORTS_DIR "/usr/local/lib/cmake/KDReports-qt6" CACHE PATH "Path to KDReports CMake directory")
elseif(UNIX)
    # Linux-specific paths
    set(CONTAINER_SEARCH_PATHS "/usr/local/lib/cmake/Container" CACHE PATH "Default path to container's library on Linux")
    set(RABBITMQ_CMAKE_DIR "/usr/local/lib/cmake/rabbitmq-c" CACHE PATH "Default path to RabbitMQ-C library on Linux")
    set(KDREPORTS_DIR "/usr/local/KDAB/KDReports-2.3.95/lib/cmake/KDReports-qt6" CACHE PATH "Path to KDReports CMake directory")
else()
    message(FATAL_ERROR "Unsupported platform. Please set paths for CONTAINER_CMAKE_DIR and RABBITMQ_CMAKE_DIR manually.")
endif()

# Find the installed Container library
set(CONTAINER_CMAKE_DIR "${CONTAINER_SEARCH_PATHS}" CACHE PATH "Path to Container library's CMake files")

find_package(Container REQUIRED PATHS ${CONTAINER_CMAKE_DIR} NO_DEFAULT_PATH)

# Check if the directory exists
if(NOT EXISTS "${CONTAINER_LIB_DIR}")
    message(FATAL_ERROR "The specified CONTAINER_LIB_DIR does not exist: ${CONTAINER_LIB_DIR}")
endif()

find_package(RabbitMQ-C REQUIRED CONFIG PATHS ${RABBITMQ_CMAKE_DIR})

if (NOT RabbitMQ-C_FOUND)
    message(FATAL_ERROR "RabbitMQ-C not found. Please specify the correct path to the RabbitMQ-C cmake installation.")
endif()

# Use the KDReports CMake package
find_package(KDReports-qt6 REQUIRED CONFIG PATHS ${KDREPORTS_DIR})

if (NOT KDReports-qt6_FOUND)
    message(FATAL_ERROR "KDReports-qt6 not found. Please specify the correct path to the KDReports cmake installation.")
endif()

# Set and cache the path to the RabbitMQ bin directory using RABBITMQ_CMAKE_DIR
if(WIN32)
    # For Windows, use /bin
    set(RABBITMQ_SHRD_LIB_DIR "${RABBITMQ_CMAKE_DIR}/../../../bin" CACHE PATH "Path to the RabbitMQ-C library's bin directory")
elseif(UNIX AND NOT APPLE)
    # For Linux, use /lib or /lib64 (adjust based on your setup)
    set(RABBITMQ_SHRD_LIB_DIR "${RABBITMQ_CMAKE_DIR}/../../" CACHE PATH "Path to the RabbitMQ-C library's bin directory")
elseif(APPLE)
    # For macOS, use /lib or /lib64 (adjust based on your setup)
    set(RABBITMQ_SHRD_LIB_DIR "${RABBITMQ_CMAKE_DIR}/../../" CACHE PATH "Path to the RabbitMQ-C library's bin directory")
endif()

