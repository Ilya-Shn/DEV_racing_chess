#pragma once
#include "board.h"
#include <vector>

class MoveValidator {
public:
    MoveValidator();

    bool isValidMove(const Board& board, uint32_t piece_id, Position target) const;
    std::vector<Position> getValidMoves(const Board& board, uint32_t piece_id) const;

private:
    bool isValidPawnMove(const Board& board, const Piece& piece, Position target) const;
    bool isValidKnightMove(const Board& board, const Piece& piece, Position target) const;
    bool isValidBishopMove(const Board& board, const Piece& piece, Position target) const;
    bool isValidRookMove(const Board& board, const Piece& piece, Position target) const;
    bool isValidQueenMove(const Board& board, const Piece& piece, Position target) const;
    bool isValidKingMove(const Board& board, const Piece& piece, Position target) const;

    bool isPathClear(const Board& board, Position from, Position to) const;
    bool isTargetEmptyOrEnemy(const Board& board, const Piece& piece, Position target) const;
};