#pragma once

#include "chess_types.h"
#include "game.h"
#include <vector>
#include <optional>
#include <random>

class AIPlayer {
public:
    AIPlayer(AIDifficulty difficulty, PlayerColor color);

    std::optional<Move> getBestMove(const Game& game);
    void setDifficulty(AIDifficulty difficulty);
    void setMoveProbability(int ticks);

    // ДОБАВЛЕНО: Геттер для получения сложности
    AIDifficulty getDifficulty() const { return difficulty_; }

private:
    AIDifficulty difficulty_;
    PlayerColor color_;
    std::mt19937 rng_;
    int ticks_per_move_;    // Количество тиков между ходами
    int ticks_counter_;     // Счетчик тиков
    int move_randomness_;   // Случайность выбора хода (1 = всегда лучший, >1 = выбор из топ-N)

    struct MoveScore {
        Move move;
        double score;
    };

    std::vector<MoveScore> evaluateAllMoves(const Game& game);
    double evaluateMove(const Game& game, const Piece& piece, Position target);

    // Компоненты оценки хода
    double getRowScore(const Piece& piece, Position target);
    double getColScore(const Piece& piece, Position target);
    double getCaptureScore(const Game& game, const Piece& piece, Position target);
    double getPressureScore(const Game& game, const Piece& piece, Position target);
    double getVulnerabilityScore(const Game& game, const Piece& piece, Position target);
    double getProtectionScore(const Game& game, const Piece& piece, Position target);
};