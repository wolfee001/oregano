add_executable(publisher
    publisher/main.cpp
)

target_link_libraries(publisher
    PRIVATE project_options
    PRIVATE project_warnings
    PRIVATE oregano
    PRIVATE CONAN_PKG::sole
    PRIVATE CONAN_PKG::nlohmann_json
)

add_executable(subscriber
    subscriber/main.cpp
)

target_link_libraries(subscriber
    PRIVATE project_options
    PRIVATE project_warnings
    PRIVATE oregano
    PRIVATE CONAN_PKG::nlohmann_json
)
