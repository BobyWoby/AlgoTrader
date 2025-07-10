cmake_minimum_required(VERSION 4.0)
project(AlgoTrader)

find_package(http REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread system asio)
find_package(OpenSSL REQUIRED)
find_package(libpqxx REQUIRED)
find_package(RapidJSON  REQUIRED)


add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE http::http)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::headers Boost::thread
                                              ${OPENSSL_LIBRARIES} pqxx)
