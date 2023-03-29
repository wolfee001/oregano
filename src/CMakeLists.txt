setup_conan(FILE conanfile.txt)

add_library(oregano
    implementation/BrokerConfiguration.cpp
    implementation/EventInterface.cpp
    implementation/MessageBrokerFactory.cpp
    implementation/MessageBrokerWrapper.cpp
    implementation/MessageHandlerManager.cpp
    implementation/Publisher.cpp
    implementation/RedisMessageBroker.cpp
    implementation/ResponsePromise.cpp
    implementation/RequestHandler.cpp
    implementation/RequestSender.cpp
    implementation/Subscriber.cpp
    implementation/TransactionInterface.cpp
)

target_link_libraries(oregano
    PRIVATE project_options
    PRIVATE project_warnings
    PRIVATE CONAN_PKG::redis-plus-plus
    PRIVATE CONAN_PKG::sole
    PRIVATE CONAN_PKG::nlohmann_json
    PRIVATE CONAN_PKG::cppcodec
)

target_include_directories(oregano PUBLIC interface)
