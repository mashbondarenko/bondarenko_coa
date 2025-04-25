cmake_minimum_required(VERSION 3.10)
project(statservice)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(gRPC CONFIG REQUIRED)
link_directories(/usr/local/lib)

set(GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../proto/generated")
include_directories(${GENERATED_DIR})
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(/opt/homebrew/include)
include_directories(/opt/homebrew/Cellar/libpq/17.4_1/include)
link_directories(/opt/homebrew/Cellar/libpq/17.4_1/lib)
link_directories(/opt/homebrew/lib)

set(SOURCES
main.cpp
StatServiceImpl.cpp 
    ${GENERATED_DIR}/stat.pb.cc
    ${GENERATED_DIR}/stat.grpc.pb.cc
)

add_executable(statservice ${SOURCES})

target_link_libraries(statservice
gRPC::grpc++            
    protobuf::libprotobuf
    Threads::Threads
    pqxx
    pq
    Threads::Threads
)