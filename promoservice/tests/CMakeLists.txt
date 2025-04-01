cmake_minimum_required(VERSION 3.10)
project(promoservice_tests)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/../proto/generated)

set(GENERATED_DIR ${CMAKE_SOURCE_DIR}/../proto/generated)

set(TEST_SOURCES
    test_promoservice.cpp
    ../PromoServiceImpl.cpp
    ${GENERATED_DIR}/promo.pb.cc
    ${GENERATED_DIR}/promo.grpc.pb.cc
)

add_executable(promoservice_tests ${TEST_SOURCES})

target_include_directories(promoservice_tests PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/../../proto/generated
${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(promoservice_tests
    gRPC::grpc++
    protobuf::libprotobuf
    GTest::GTest
    GTest::Main
    Threads::Threads
    pq
    pqxx
)
