cmake_minimum_required(VERSION 4.1)
project(KlondikeSolver)

set(CMAKE_CXX_STANDARD 20)

add_executable(KlondikeSolver main.cpp
        constant.h
        meow.h
        src/card.h
        src/card.cpp
        src/poker.h
        src/poker.cpp
        mode/base_mode.h
        mode/test_mode.h
        mode/test_mode.cpp
        helper/cxxopts.h
        helper/xxhash.h
        helper/helper.h
        helper/helper.cpp
        src/action.h
        src/action.cpp
        src/action_type.h
        src/bucket.h
        src/bucket.cpp
        src/card_ext.h
        src/card_ext.cpp
        src/estimation.h
        src/estimation.cpp
        src/motion.h
        src/motion.cpp
        src/motion_index.h
        src/motion_node.h
        src/motion_node.cpp
        src/pile.h
        src/pile.cpp
        src/solver.h
        src/solver.cpp
        src/state.h
        src/state.cpp
        src/state_key.h
        src/state_map.h
        src/state_map.cpp
        src/tableau.h
        src/tableau.cpp
        src/talon_helper.h
        src/talon_helper.cpp
        src/solve_result.h
        src/solve_result.cpp
        src/hint.cpp
        src/analyzer.h
        src/analyzer.cpp
        json_struct/json.hpp
        json_struct/level_json_data.h
        io/json_reader.h
        io/json_reader.cpp
        csv_struct/export_data.hpp
        csv_struct/export_data.cpp
)
