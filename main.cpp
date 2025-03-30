#include "game.h"
#include "game_ui.h"
#include "fen_parser.h"
#include "ai_player.h"
#include <iostream>
#include <string>
#include <functional>

int main() {
    // Настройки по умолчанию
    GameSettings settings;
    settings.tick_rate_ms = 100; // Устанавливаем стандартный тик в 100мс (0.1 секунды)
    settings.against_ai = false;
    settings.fen_string = FENParser::getDefaultFEN();

    AIDifficulty ai_difficulty = AIDifficulty::MEDIUM;

    // Ввод режима игры
    std::cout << "Select game mode:" << std::endl;
    std::cout << "1. Human vs Human" << std::endl;
    std::cout << "2. Human vs AI" << std::endl;
    std::cout << "Your choice (1-2): ";

    int game_mode;
    std::cin >> game_mode;
    settings.against_ai = (game_mode == 2);

    if (settings.against_ai) {
        std::cout << "\nSelect AI difficulty:" << std::endl;
        std::cout << "1. Easy" << std::endl;
        std::cout << "2. Medium" << std::endl;
        std::cout << "3. Hard" << std::endl;
        std::cout << "4. Expert" << std::endl;
        std::cout << "Your choice (1-4): ";

        int difficulty;
        std::cin >> difficulty;

        switch (difficulty) {
            case 1: ai_difficulty = AIDifficulty::EASY; break;
            case 2: ai_difficulty = AIDifficulty::MEDIUM; break;
            case 3: ai_difficulty = AIDifficulty::HARD; break;
            case 4: ai_difficulty = AIDifficulty::EXPERT; break;
            default: ai_difficulty = AIDifficulty::MEDIUM; break;
        }
    }

    // Ввод кулдаунов в секундах (вместо тиков)
    double white_cooldown_seconds;
    std::cout << "\nEnter cooldown for White pieces (in seconds): ";
    std::cin >> white_cooldown_seconds;
    // Конвертируем секунды в тики (1 тик = 0.1 сек)
    settings.white_cooldown_ticks = static_cast<int>(white_cooldown_seconds * 10);

    double black_cooldown_seconds;
    std::cout << "Enter cooldown for Black pieces (in seconds): ";
    std::cin >> black_cooldown_seconds;
    settings.black_cooldown_ticks = static_cast<int>(black_cooldown_seconds * 10);

    // Ввод начальной позиции
    std::cout << "\nSelect initial position:" << std::endl;
    std::cout << "1. Standard chess position" << std::endl;
    std::cout << "2. Custom position (FEN notation)" << std::endl;
    std::cout << "Your choice (1-2): ";

    int position_choice;
    std::cin >> position_choice;

    if (position_choice == 2) {
        std::cout << "\nEnter FEN notation (or 'standard' for default position): ";
        std::cin.ignore(); // Очищаем буфер ввода
        std::string fen;
        std::getline(std::cin, fen);

        if (FENParser::isValidFEN(fen)) {
            settings.fen_string = fen;
        } else {
            std::cout << "Invalid FEN notation. Using standard position." << std::endl;
            settings.fen_string = FENParser::getDefaultFEN();
        }
    }

    // Создание и настройка игры
    Game game([](GameState state) {
        // Callback для обработки изменения состояния игры
        if (state == GameState::WHITE_WIN) {
            std::cout << "Game over - White wins!" << std::endl;
        } else if (state == GameState::BLACK_WIN) {
            std::cout << "Game over - Black wins!" << std::endl;
        }
    });

    // Применяем настройки и запускаем игру
    game.applySettings(settings);
    game.start();

    // Создание и запуск UI
    GameUI ui(game, settings.against_ai);
    ui.run();

    return 0;
}