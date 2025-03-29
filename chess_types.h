#pragma once

#include <cstdint>
#include <optional>
#include <string>

// Позиция на доске
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

// Цвет игрока
enum class PlayerColor {
    WHITE,
    BLACK
};

// Тип фигуры
enum class PieceType {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

// Состояние игры
enum class GameState {
    NOT_STARTED,
    WAITING_FOR_SETTINGS,
    ACTIVE,
    PAUSED,
    WHITE_WIN,
    BLACK_WIN
};

// Сложность ИИ
enum class AIDifficulty {
    EASY,
    MEDIUM,
    HARD,
    EXPERT
};

// Шахматная фигура
struct Piece {
    uint32_t id;
    PieceType type;
    PlayerColor color;
    Position position;
    bool captured;
    bool moved;
    int cooldown_ticks_remaining;
};

// Ход фигуры
struct Move {
    uint32_t piece_id;
    Position from;
    Position to;
    uint64_t timestamp;
};

// Настройки игры
struct GameSettings {
    int white_cooldown_ticks;
    int black_cooldown_ticks;
    int tick_rate_ms;
    bool against_ai;
    std::string fen_string;
};