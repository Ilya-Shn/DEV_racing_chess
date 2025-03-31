add_library(SpeedChessLib STATIC
        ${CMAKE_SOURCE_DIR}/game.cpp
        ${CMAKE_SOURCE_DIR}/board.cpp
        ${CMAKE_SOURCE_DIR}/move_validator.cpp
        ${CMAKE_SOURCE_DIR}/ai_player.cpp
        ${CMAKE_SOURCE_DIR}/timer.cpp
        ${CMAKE_SOURCE_DIR}/fen_parser.cpp
)

set(TEST_FILES
        test_board.cpp
        test_move_validator.cpp
        test_fen_parser.cpp
        test_ai_player.cpp
)

add_executable(SpeedChessTests test_main.cpp ${TEST_FILES})
target_link_libraries(SpeedChessTests
        PRIVATE
        SpeedChessLib
        GTest::GTest
        GTest::Main
)

add_test(NAME SpeedChessAllTests COMMAND SpeedChessTests)