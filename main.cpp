#include "game.h"
#include "game_ui.h"
#include "fen_parser.h"
#include "ai_player.h"
#include <iostream>
#include <string>
#include <functional>

int main() {
    GameSettings settings;
    settings.tick_rate_ms = 100;
    settings.against_ai = false;
    settings.fen_string = FENParser::getDefaultFEN();


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
            case 1: settings.ai_difficulty = AIDifficulty::EASY; break;
            case 2: settings.ai_difficulty = AIDifficulty::MEDIUM; break;
            case 3: settings.ai_difficulty = AIDifficulty::HARD; break;
            case 4: settings.ai_difficulty = AIDifficulty::EXPERT; break;
            default: settings.ai_difficulty = AIDifficulty::MEDIUM; break;
        }
    }

    double white_cooldown_seconds;
    std::cout << "\nEnter cooldown for White pieces (in seconds): ";
    std::cin >> white_cooldown_seconds;
    settings.white_cooldown_ticks = static_cast<int>(white_cooldown_seconds * 10);

    double black_cooldown_seconds;
    std::cout << "Enter cooldown for Black pieces (in seconds): ";
    std::cin >> black_cooldown_seconds;
    settings.black_cooldown_ticks = static_cast<int>(black_cooldown_seconds * 10);

    std::cout << "\nSelect initial position:" << std::endl;
    std::cout << "1. Standard chess position" << std::endl;
    std::cout << "2. Custom position (FEN notation)" << std::endl;
    std::cout << "Your choice (1-2): ";

    int position_choice;
    std::cin >> position_choice;

    if (position_choice == 2) {
        std::cout << "\nEnter FEN notation (or 'standard' for default position): ";
        std::cin.ignore();
        std::string fen;
        std::getline(std::cin, fen);

        if (FENParser::isValidFEN(fen)) {
            settings.fen_string = fen;
        } else {
            std::cout << "Invalid FEN notation. Using standard position." << std::endl;
            settings.fen_string = FENParser::getDefaultFEN();
        }
    }

    Game game([](GameState state) {
        if (state == GameState::WHITE_WIN) {
            std::cout << "Game over - White wins!" << std::endl;
        } else if (state == GameState::BLACK_WIN) {
            std::cout << "Game over - Black wins!" << std::endl;
        }
    });

    game.applySettings(settings);
    game.start();

    GameUI ui(game, settings);
    ui.run();

    return 0;
}
