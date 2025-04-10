# # Tests directory for CargoNetSim using Qt6 Test framework

# # Enable automatic handling of Qt MOC, UIC, and RCC for tests
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTORCC ON)

# # Set include directories for tests
# set(TEST_INCLUDE_DIRS
#     ${CMAKE_SOURCE_DIR}/src
#     ${CMAKE_BINARY_DIR}/src
# )

# # Define all test files - add new test files here
# set(TEST_FILES
#     TerminalClientTest.cpp
#     # Add additional test files below:
#     # ContainerTest.cpp
#     # PathTest.cpp
#     # TerminalTest.cpp
# )

# # Create an executable that runs all tests
# add_executable(all_tests ${TEST_FILES})

# # Set include directories
# target_include_directories(all_tests PRIVATE ${TEST_INCLUDE_DIRS})

# # Link with required libraries
# target_link_libraries(all_tests PRIVATE
#     Qt6::Core
#     Qt6::Test
#     Qt6::Network
#     CargoNetSimBackend
#     Container::Container
#     rabbitmq::rabbitmq
# )

# # Add the test to the project
# add_test(NAME all_tests COMMAND all_tests)

# # Set properties for the test
# set_tests_properties(all_tests PROPERTIES
#     ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
# )

# # Optional: Individual test executables if needed
# option(BUILD_INDIVIDUAL_TESTS "Build separate executables for each test file" OFF)

# if(BUILD_INDIVIDUAL_TESTS)
#     foreach(TEST_SOURCE ${TEST_FILES})
#         # Get the filename without extension to use as target name
#         get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
        
#         # Add executable
#         add_executable(${TEST_NAME} ${TEST_SOURCE})
        
#         # Add include directories
#         target_include_directories(${TEST_NAME} PRIVATE ${TEST_INCLUDE_DIRS})
        
#         # Link against the Qt6 test framework and other required libraries
#         target_link_libraries(${TEST_NAME} PRIVATE
#             Qt6::Core
#             Qt6::Test
#             Qt6::Network
#             CargoNetSimBackend
#             Container::Container
#             rabbitmq::rabbitmq
#         )
        
#         # Add the test to the project
#         add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        
#         # Set properties for the test
#         set_tests_properties(${TEST_NAME} PROPERTIES
#             ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
#         )
#     endforeach()
# endif()

# message(STATUS "CargoNetSim tests configured with Qt6 Test framework")

