cmake_minimum_required(VERSION 3.10)
project(SpeedChess VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

find_package(GTest REQUIRED)
enable_testing()

set(SOURCES
        main.cpp
        core/game.cpp
        core/board.cpp
        core/move_validator.cpp
        bot/ai_player.cpp
        utility/timer.cpp
        utility/fen_parser.cpp
        ui/game_ui.cpp
)

set(HEADERS
        core/game.h
        core/board.h
        core/move_validator.h
        bot/ai_player.h
        utility/timer.h
        utility/fen_parser.h
        ui/game_ui.h
        core/chess_types.h
)

add_executable(SpeedChess ${SOURCES} ${HEADERS})

target_link_libraries(SpeedChess PRIVATE sfml-system sfml-window sfml-graphics)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()
add_subdirectory(tests)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})



install(TARGETS SpeedChess DESTINATION bin)
install(DIRECTORY data DESTINATION bin)
