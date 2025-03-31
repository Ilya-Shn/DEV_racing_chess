#pragma once

#include "../core/chess_types.h"
#include "../core/game.h"
#include <vector>
#include <optional>
#include <random>

class AIPlayer {
public:
    AIPlayer(AIDifficulty difficulty, PlayerColor color);

    std::optional<Move> getBestMove(const Game& game);
    void setDifficulty(AIDifficulty difficulty);
    void setMoveProbability(int ticks);

    AIDifficulty getDifficulty() const { return difficulty_; }

private:
    AIDifficulty difficulty_;
    PlayerColor color_;
    std::mt19937 rng_;
    int move_randomness_;

    struct MoveScore {
        Move move;
        double score;
    };

    std::vector<MoveScore> evaluateAllMoves(const Game& game);
    double evaluateMove(const Game& game, const Piece& piece, Position target);

    double getRowScore(const Piece& piece, Position target);
    double getColScore(const Piece& piece, Position target);
    double getCaptureScore(const Game& game, const Piece& piece, Position target);
    double getPressureScore(const Game& game, const Piece& piece, Position target);
    double getVulnerabilityScore(const Game& game, const Piece& piece, Position target);
    double getProtectionScore(const Game& game, const Piece& piece, Position target);
    double getKingThreatScore(const Game& game, const Piece& piece, Position target);
};
