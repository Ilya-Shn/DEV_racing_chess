#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct Position {
    int row;
    int col;

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

enum class PlayerColor {
    WHITE,
    BLACK
};

enum class PieceType {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum class GameState {
    NOT_STARTED,
    WAITING_FOR_SETTINGS,
    ACTIVE,
    PAUSED,
    WHITE_WIN,
    BLACK_WIN
};

enum class AIDifficulty {
    EASY,
    MEDIUM,
    HARD,
    EXPERT
};

struct Piece {
    uint32_t id;
    PieceType type;
    PlayerColor color;
    Position position;
    bool captured;
    bool moved;
    int cooldown_ticks_remaining;
};

struct Move {
    uint32_t piece_id;
    Position from;
    Position to;
    uint64_t timestamp;
};

struct GameSettings {
    int white_cooldown_ticks;
    int black_cooldown_ticks;
    int tick_rate_ms;
    bool against_ai;
    std::optional<AIDifficulty> ai_difficulty;
    std::string fen_string;
};
