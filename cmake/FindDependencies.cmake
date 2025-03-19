# Find Qt6
find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)

# Platform-specific paths
if(WIN32)
    # Windows-specific paths
    set(CONTAINER_SEARCH_PATHS "C:/Program Files/Container/cmake" CACHE PATH "Default path to container's library")
    set(RABBITMQ_CMAKE_DIR "C:/Program Files/rabbitmq-c/lib/cmake/rabbitmq-c" CACHE PATH "Default path to RabbitMQ-C library on Windows")
elseif(APPLE)
    # macOS-specific paths
    set(CONTAINER_SEARCH_PATHS "/usr/local/lib/cmake/Container" CACHE PATH "Default path to container's library on macOS")
    set(RABBITMQ_CMAKE_DIR "/usr/local/lib/rabbitmq-c/cmake" CACHE PATH "Default path to RabbitMQ-C library on macOS")
elseif(UNIX)
    # Linux-specific paths
    set(CONTAINER_SEARCH_PATHS "/usr/local/lib/cmake/Container" CACHE PATH "Default path to container's library on Linux")
    set(RABBITMQ_CMAKE_DIR "/usr/local/lib/cmake/rabbitmq-c" CACHE PATH "Default path to RabbitMQ-C library on Linux")
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
