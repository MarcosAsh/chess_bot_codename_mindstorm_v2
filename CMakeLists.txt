cmake_minimum_required(VERSION 3.29)
project(chess_bot)

set(CMAKE_CXX_STANDARD 20)

# Add SFML
find_package(SFML REQUIRED COMPONENTS graphics window system)

add_executable(chess_bot
        src/main.cpp
        src/gui/gui.cpp
        src/engine/board.cpp
        src/engine/ai.cpp
        src/engine/evaluation.cpp
        src/engine/utils.cpp
        src/constants.h
        src/o_main.cpp
        src/engine/gameloops.cpp
        src/engine/gameloops.h
)

target_link_libraries(chess_bot sfml-graphics sfml-window sfml-system)