cmake_minimum_required(VERSION 3.10)
project(SpeedChess VERSION 1.0)

# Требуемый стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем опции компиляции
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Находим SFML
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

# Исходные файлы проекта
set(SOURCES
        main.cpp
        game.cpp
        board.cpp
        move_validator.cpp
        ai_player.cpp
        timer.cpp
        fen_parser.cpp
        game_ui.cpp
)

# Заголовочные файлы
set(HEADERS
        game.h
        board.h
        move_validator.h
        ai_player.h
        timer.h
        fen_parser.h
        game_ui.h
        chess_types.h
)

# Создаем исполняемый файл
add_executable(SpeedChess ${SOURCES} ${HEADERS})

# Подключаем SFML
target_link_libraries(SpeedChess PRIVATE sfml-system sfml-window sfml-graphics)

# Копирование ресурсов в директорию сборки
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Опционально: установка
install(TARGETS SpeedChess DESTINATION bin)
install(DIRECTORY data DESTINATION bin)