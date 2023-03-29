add_executable(oregano_test
    main.cpp
)

target_link_libraries(oregano_test
    PRIVATE project_options
    PRIVATE project_warnings
    PRIVATE oregano
    PRIVATE CONAN_PKG::gtest
)

include(GoogleTest)
gtest_discover_tests(oregano_test)

add_test(NAME oregano_test COMMAND oregano_test)
