add_subdirectory(GUI)
add_subdirectory(Backend)

# Define the resources variable
set(RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/CargoNetSim.qrc
)

qt6_add_resources(RESOURCE_SOURCES ${RESOURCES})

set(SOURCES
    main.cpp    # Main file

    ${RESOURCE_SOURCES}  # Include the compiled resources
)

add_executable(CargoNetSim ${SOURCES})

target_link_libraries(CargoNetSim
    PRIVATE
    CargoNetSimBackend
    CargoNetSimGUI
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
    Container::Container
    rabbitmq::rabbitmq
)

# Installation
install(TARGETS CargoNetSim 
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Copy required DLLs on Windows
if(WIN32)
    # Add commands to copy DLLs to build directory
    add_custom_command(TARGET CargoNetSim POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${RABBITMQ_SHRD_LIB_DIR}"
        $<TARGET_FILE_DIR:CargoNetSim>
    )
    
    # Install the DLLs too
    install(DIRECTORY ${RABBITMQ_SHRD_LIB_DIR}/
        DESTINATION bin
        FILES_MATCHING PATTERN "*.dll"
    )
endif()
