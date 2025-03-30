#pragma once
#include "chess_types.h"
#include <memory>
#include <vector>
#include <optional>
#include <map>

class Board {
public:
    Board();
    void setupStandardPosition();
    bool setupFromFEN(const std::string& fen);

    std::optional<Piece> getPieceAt(Position position) const;
    std::optional<Piece> getPieceById(uint32_t id) const;

    bool movePiece(uint32_t id, Position to);
    bool capturePiece(uint32_t id);
    void capturePieceAt(Position pos);

    bool setPieceCooldown(uint32_t id, int cooldown);

    std::vector<Piece> getAllPieces(bool include_captured = false) const;
    std::vector<Piece> getPlayerPieces(PlayerColor color, bool include_captured = false) const;

    void decrementCooldowns();
    int countKings(PlayerColor color) const;

    bool promotePawn(uint32_t id, PieceType new_type);

private:
    std::map<uint32_t, Piece> pieces_;
    uint32_t next_id_;
};
