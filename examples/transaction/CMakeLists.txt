add_executable(producer
    producer/main.cpp
)

target_link_libraries(producer
    PRIVATE project_options
    PRIVATE project_warnings
    PRIVATE oregano
    PRIVATE CONAN_PKG::sole
    PRIVATE CONAN_PKG::nlohmann_json
)

add_executable(consumer
    consumer/main.cpp
)

target_link_libraries(consumer
    PRIVATE project_options
    PRIVATE project_warnings
    PRIVATE oregano
    PRIVATE CONAN_PKG::sole
    PRIVATE CONAN_PKG::nlohmann_json
)
