#include "fen_parser.h"
#include <sstream>
#include <algorithm>

bool FENParser::isValidFEN(const std::string& fen) {
    std::string::size_type pos = fen.find(' ');
    std::string board_part = (pos != std::string::npos) ? fen.substr(0, pos) : fen;

    return validateBoardPart(board_part);
}

std::string FENParser::getDefaultFEN() {
    return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
}

std::string FENParser::boardToFEN(const Board& board) {
    std::stringstream fen;
    auto pieces = board.getAllPieces(false);

    char board_array[8][8] = {0};

    for (const auto& piece : pieces) {
        if (piece.position.row < 0 || piece.position.row > 7 ||
            piece.position.col < 0 || piece.position.col > 7) {
            continue;
        }

        char piece_char;

        switch (piece.type) {
            case PieceType::PAWN:   piece_char = 'p'; break;
            case PieceType::KNIGHT: piece_char = 'n'; break;
            case PieceType::BISHOP: piece_char = 'b'; break;
            case PieceType::ROOK:   piece_char = 'r'; break;
            case PieceType::QUEEN:  piece_char = 'q'; break;
            case PieceType::KING:   piece_char = 'k'; break;
            default: continue;
        }

        if (piece.color == PlayerColor::WHITE) {
            piece_char = toupper(piece_char);
        }

        board_array[piece.position.row][piece.position.col] = piece_char;
    }

    for (int row = 7; row >= 0; row--) {
        int empty_count = 0;

        for (int col = 0; col < 8; col++) {
            if (board_array[row][col] == 0) {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    fen << empty_count;
                    empty_count = 0;
                }
                fen << board_array[row][col];
            }
        }

        if (empty_count > 0) {
            fen << empty_count;
        }

        if (row > 0) {
            fen << '/';
        }
    }

    return fen.str();
}

bool FENParser::parseFEN(const std::string& fen, Board& board) {
    return board.setupFromFEN(fen);
}

bool FENParser::validateBoardPart(const std::string& board_part) {
    int row = 7;
    int col = 0;
    int white_kings = 0;
    int black_kings = 0;

    for (char ch : board_part) {
        if (ch == '/') {
            if (col != 8) return false;

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

            switch (ch) {
                case 'K': white_kings++; break;
                case 'k': black_kings++; break;
                case 'P': case 'N': case 'B': case 'R': case 'Q':
                case 'p': case 'n': case 'b': case 'r': case 'q':
                    break;
                default:
                    return false;
            }

            col++;
        }
    }

    return row == 0 && col == 8 && white_kings == 1 && black_kings == 1;
}
