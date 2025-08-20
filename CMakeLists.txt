cmake_minimum_required(VERSION 4.0)
project(AlgoTrader)

find_package(http REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread system asio)
find_package(Catch2 3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(RapidJSON REQUIRED)

file(GLOB SOURCES src/Trader.cpp)

add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE http::http)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::headers Boost::thread
                                              ${OPENSSL_LIBRARIES})

add_executable(tests src/tests.cpp ${SOURCES})
# target_link_libraries( tests PRIVATE Boost::headers Boost::thread
# ${OPENSSL_LIBRARIES})
target_link_libraries(tests PRIVATE http::http)
target_link_libraries(tests PRIVATE Boost::headers Boost::thread
                                    Catch2::Catch2WithMain ${OPENSSL_LIBRARIES})
