#include "board.h"
#include <iostream>
#include <vector>
#include <algorithm>

Board::Board() : next_id_(1) {
}

void Board::setupStandardPosition() {
    pieces_.clear();
    next_id_ = 1;

    for (int col = 0; col < 8; col++) {
        Piece pawn;
        pawn.id = next_id_++;
        pawn.type = PieceType::PAWN;
        pawn.color = PlayerColor::WHITE;
        pawn.position = {1, col};
        pawn.captured = false;
        pawn.moved = false;
        pawn.cooldown_ticks_remaining = 0;
        pieces_[pawn.id] = pawn;
    }

    PieceType white_pieces[8] = {
            PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP, PieceType::QUEEN,
            PieceType::KING, PieceType::BISHOP, PieceType::KNIGHT, PieceType::ROOK
    };

    for (int col = 0; col < 8; col++) {
        Piece piece;
        piece.id = next_id_++;
        piece.type = white_pieces[col];
        piece.color = PlayerColor::WHITE;
        piece.position = {0, col};
        piece.captured = false;
        piece.moved = false;
        piece.cooldown_ticks_remaining = 0;
        pieces_[piece.id] = piece;
    }

    for (int col = 0; col < 8; col++) {
        Piece pawn;
        pawn.id = next_id_++;
        pawn.type = PieceType::PAWN;
        pawn.color = PlayerColor::BLACK;
        pawn.position = {6, col};
        pawn.captured = false;
        pawn.moved = false;
        pawn.cooldown_ticks_remaining = 0;
        pieces_[pawn.id] = pawn;
    }

    PieceType black_pieces[8] = {
            PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP, PieceType::QUEEN,
            PieceType::KING, PieceType::BISHOP, PieceType::KNIGHT, PieceType::ROOK
    };

    for (int col = 0; col < 8; col++) {
        Piece piece;
        piece.id = next_id_++;
        piece.type = black_pieces[col];
        piece.color = PlayerColor::BLACK;
        piece.position = {7, col};
        piece.captured = false;
        piece.moved = false;
        piece.cooldown_ticks_remaining = 0;
        pieces_[piece.id] = piece;
    }
}

bool Board::setupFromFEN(const std::string& fen) {
    pieces_.clear();
    next_id_ = 1;

    int row = 7;
    int col = 0;

    std::string::size_type pos = fen.find(' ');
    std::string board_part = (pos != std::string::npos) ? fen.substr(0, pos) : fen;

    for (char ch : board_part) {
        if (ch == '/') {
            row--;
            col = 0;
            if (row < 0) return false;
        }
        else if (isdigit(ch)) {
            col += (ch - '0');
            if (col > 8) return false;
        }
        else {
            if (col >= 8) return false;

            Piece piece;
            piece.id = next_id_++;
            piece.position = {row, col};
            piece.captured = false;
            piece.moved = false;
            piece.cooldown_ticks_remaining = 0;

            switch (ch) {
                case 'P': piece.type = PieceType::PAWN; piece.color = PlayerColor::WHITE; break;
                case 'N': piece.type = PieceType::KNIGHT; piece.color = PlayerColor::WHITE; break;
                case 'B': piece.type = PieceType::BISHOP; piece.color = PlayerColor::WHITE; break;
                case 'R': piece.type = PieceType::ROOK; piece.color = PlayerColor::WHITE; break;
                case 'Q': piece.type = PieceType::QUEEN; piece.color = PlayerColor::WHITE; break;
                case 'K': piece.type = PieceType::KING; piece.color = PlayerColor::WHITE; break;
                case 'p': piece.type = PieceType::PAWN; piece.color = PlayerColor::BLACK; break;
                case 'n': piece.type = PieceType::KNIGHT; piece.color = PlayerColor::BLACK; break;
                case 'b': piece.type = PieceType::BISHOP; piece.color = PlayerColor::BLACK; break;
                case 'r': piece.type = PieceType::ROOK; piece.color = PlayerColor::BLACK; break;
                case 'q': piece.type = PieceType::QUEEN; piece.color = PlayerColor::BLACK; break;
                case 'k': piece.type = PieceType::KING; piece.color = PlayerColor::BLACK; break;
                default: return false;
            }

            pieces_[piece.id] = piece;
            col++;
        }
    }

    if (countKings(PlayerColor::WHITE) != 1 || countKings(PlayerColor::BLACK) != 1) {
        return false;
    }

    return true;
}

std::optional<Piece> Board::getPieceAt(Position position) const {
    for (const auto& [id, piece] : pieces_) {
        if (!piece.captured && piece.position == position) {
            return piece;
        }
    }
    return std::nullopt;
}

std::optional<Piece> Board::getPieceById(uint32_t id) const {
    auto it = pieces_.find(id);
    if (it != pieces_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool Board::movePiece(uint32_t id, Position to) {
    auto it = pieces_.find(id);
    if (it == pieces_.end() || it->second.captured) {
        return false;
    }

    Position old_pos = it->second.position;

    auto target_piece = getPieceAt(to);
    if (target_piece && target_piece->color != it->second.color) {
        capturePieceAt(to);
    }

    it->second.position = to;
    it->second.moved = true;

    return true;
}

bool Board::capturePiece(uint32_t id) {
    auto it = pieces_.find(id);
    if (it == pieces_.end() || it->second.captured) {
        return false;
    }

    it->second.captured = true;
    return true;
}

void Board::capturePieceAt(Position pos) {
    for (auto& [id, piece] : pieces_) {
        if (!piece.captured && piece.position == pos) {
            piece.captured = true;
            break;
        }
    }
}

bool Board::setPieceCooldown(uint32_t id, int cooldown) {
    auto it = pieces_.find(id);
    if (it == pieces_.end() || it->second.captured) {
        return false;
    }

    it->second.cooldown_ticks_remaining = cooldown;
    return true;
}

std::vector<Piece> Board::getAllPieces(bool include_captured) const {
    std::vector<Piece> result;
    result.reserve(pieces_.size());

    for (const auto& [id, piece] : pieces_) {
        if (include_captured || !piece.captured) {
            result.push_back(piece);
        }
    }

    return result;
}

std::vector<Piece> Board::getPlayerPieces(PlayerColor color, bool include_captured) const {
    std::vector<Piece> result;

    for (const auto& [id, piece] : pieces_) {
        if (piece.color == color && (include_captured || !piece.captured)) {
            result.push_back(piece);
        }
    }

    return result;
}

void Board::decrementCooldowns() {
    int updated = 0;

    for (auto& [id, piece] : pieces_) {
        if (piece.cooldown_ticks_remaining > 0) {
            piece.cooldown_ticks_remaining--;
            updated++;
        }
    }
}

int Board::countKings(PlayerColor color) const {
    int count = 0;
    for (const auto& [id, piece] : pieces_) {
        if (!piece.captured && piece.type == PieceType::KING && piece.color == color) {
            count++;
        }
    }
    return count;
}

bool Board::promotePawn(uint32_t id, PieceType new_type) {
    auto it = pieces_.find(id);
    if (it == pieces_.end() || it->second.captured) {
        return false;
    }

    if (it->second.type != PieceType::PAWN) {
        return false;
    }

    it->second.type = new_type;
    return true;
}
