cmake_minimum_required(VERSION 4.1)
project(KlondikeSolver)

set(CMAKE_CXX_STANDARD 20)

add_executable(KlondikeSolver main.cpp
        constant.h
        src/card.h
        src/card.cpp
        src/history_item.h
        src/history_item.cpp
        src/poker.h
        src/poker.cpp
        src/state.h
        src/state.cpp
        src/solver.h
        src/solver.cpp
        helper/helper.h
        helper/helper.cpp
)
