cmake_minimum_required(VERSION 3.15)
project(ArbitragePlatform VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/market_data/polymarket_client.cpp
    src/arbitrage/arbitrage_engine.cpp
)

add_executable(arbitrage-platform ${SOURCES})
target_link_libraries(arbitrage-platform pthread)
