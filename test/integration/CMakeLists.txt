setup_conan(FILE conanfile.txt)

add_executable(oregano_integration_test
    main.cpp
    PubSubTest.cpp
    TransactionTest.cpp
)

target_link_libraries(oregano_integration_test
    PRIVATE project_options
    PRIVATE project_warnings
    PRIVATE oregano
    PRIVATE CONAN_PKG::gtest
    PRIVATE CONAN_PKG::cppcodec
    PRIVATE CONAN_PKG::nlohmann_json
)

include(GoogleTest)
gtest_discover_tests(oregano_integration_test)

add_test(NAME oregano_integration_test COMMAND oregano_integration_test)
