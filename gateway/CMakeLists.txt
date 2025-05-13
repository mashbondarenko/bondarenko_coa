cmake_minimum_required(VERSION 3.10)
project(gateway)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "arm64")

find_package(Threads REQUIRED)
find_package(absl REQUIRED)
find_package(gRPC CONFIG REQUIRED)
link_directories(/usr/local/lib)

set(GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../proto/generated")
include_directories(${GENERATED_DIR})
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(/opt/homebrew/include)
include_directories(/opt/homebrew/opt/librdkafka/include)
link_directories(/opt/homebrew/opt/librdkafka/lib)
link_directories(/opt/homebrew/lib)


set(SOURCES
${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthServiceProxyImpl.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/PromoServiceProxyImpl.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/StatService.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/KafkaProd.cpp
    ${GENERATED_DIR}/promo.pb.cc
    ${GENERATED_DIR}/promo.grpc.pb.cc
    ${GENERATED_DIR}/auth.pb.cc
    ${GENERATED_DIR}/auth.grpc.pb.cc
    ${GENERATED_DIR}/stat.pb.cc
    ${GENERATED_DIR}/stat.grpc.pb.cc
)

add_executable(gateway ${SOURCES})
add_subdirectory(tests)

target_link_libraries(gateway
rdkafka++
gRPC::grpc++            
    protobuf::libprotobuf
    Threads::Threads
    absl::synchronization
    pthread
)