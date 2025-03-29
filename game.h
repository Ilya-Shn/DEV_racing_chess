#pragma once
#include "board.h"
#include "move_validator.h"
#include "timer.h"
#include <functional>

class Game {
public:
    Game(std::function<void(GameState)> state_change_callback = nullptr);

    void applySettings(const GameSettings& settings);
    void start();
    void pause();
    void resume();
    void reset();

    bool makeMove(uint32_t piece_id, Position target);
    std::vector<Position> getValidMoves(uint32_t piece_id) const;

    GameState getState() const;
    const Board& getBoard() const;
    Board& getBoard();

    int getWhiteCooldown() const;
    int getBlackCooldown() const;

private:
    void tick();
    void checkGameOver();
    void updateGameState();
    void applyCooldown(uint32_t piece_id); // ИСПРАВЛЕНО: Принимаем piece_id

    void checkPawnPromotion(uint32_t piece_id, Position target);
    bool handleCastling(uint32_t king_id, Position target);
    bool isCastlingMove(const Piece& king, Position target) const;

    Board board_;
    MoveValidator validator_;
    Timer timer_;

    GameState state_;
    std::function<void(GameState)> state_change_callback_;

    int white_cooldown_;
    int black_cooldown_;
    bool against_ai_;
};