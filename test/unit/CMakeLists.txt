setup_conan(FILE conanfile.txt)

add_executable(oregano_test
    BrokerConfigurationTest.cpp
    EventInterfaceTest.cpp
    main.cpp
    MessageBrokerWrapperTest.cpp
    MessageHandlerManagerTest.cpp
    PublisherTest.cpp
    RequestHandlerTest.cpp
    RequestSenderTest.cpp
    ResponsePromiseTest.cpp
    SubscriberTest.cpp
    TransactionInterfaceTest.cpp
)

target_link_libraries(oregano_test
    PRIVATE project_options
    PRIVATE project_warnings
    PRIVATE oregano
    PRIVATE CONAN_PKG::gtest
    PRIVATE CONAN_PKG::cppcodec
    PRIVATE CONAN_PKG::nlohmann_json
)

include(GoogleTest)
gtest_discover_tests(oregano_test)

add_test(NAME oregano_test COMMAND oregano_test)
