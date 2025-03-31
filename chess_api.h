#pragma once

#include "game.h"
#include "ai_player.h"
#include "fen_parser.h"
#include <string>
#include <optional>

class ChessAPI {
public:
    // Создание игры с настройками
    ChessAPI(bool against_ai = false,
             AIDifficulty difficulty = AIDifficulty::MEDIUM,
             double white_cooldown = 1.0,
             double black_cooldown = 1.0,
             const std::string& fen = "");

    // Методы для взаимодействия
    bool makeMove(int fromRow, int fromCol, int toRow, int toCol);
    std::optional<Piece> getPieceAt(int row, int col) const;
    void printBoard() const;

    // Дополнительные полезные методы
    GameState getGameState() const;
    bool isGameOver() const;
    void reset(const std::string& fen = "");

    // Метод для выполнения хода ИИ (если игра с ботом)
    bool makeAIMove();

private:
    Game game_;
    std::unique_ptr<AIPlayer> ai_player_;
    bool against_ai_;
};