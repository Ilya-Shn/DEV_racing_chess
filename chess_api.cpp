#include "chess_api.h"
#include <iostream>
#include <iomanip>

ChessAPI::ChessAPI(bool against_ai, AIDifficulty difficulty,
                   double white_cooldown, double black_cooldown,
                   const std::string& fen)
        : against_ai_(against_ai),
          game_([](GameState) {}) {

    GameSettings settings;
    settings.against_ai = against_ai;
    settings.ai_difficulty = against_ai ? std::optional<AIDifficulty>(difficulty) : std::nullopt;
    settings.white_cooldown_ticks = static_cast<int>(white_cooldown * 10);
    settings.black_cooldown_ticks = static_cast<int>(black_cooldown * 10);
    settings.tick_rate_ms = 100;
    settings.fen_string = fen.empty() ? FENParser::getDefaultFEN() : fen;

    game_.applySettings(settings);
    game_.start();

    if (against_ai) {
        ai_player_ = std::make_unique<AIPlayer>(difficulty, PlayerColor::BLACK);
    }
}

bool ChessAPI::makeMove(int fromRow, int fromCol, int toRow, int toCol) {
    if (fromRow < 0 || fromRow > 7 || fromCol < 0 || fromCol > 7 ||
        toRow < 0 || toRow > 7 || toCol < 0 || toCol > 7) {
        return false;
    }

    auto piece = game_.getBoard().getPieceAt({fromRow, fromCol});
    if (!piece) {
        return false;
    }

    return game_.makeMove(piece->id, {toRow, toCol});
}

std::optional<Piece> ChessAPI::getPieceAt(int row, int col) const {
    if (row < 0 || row > 7 || col < 0 || col > 7) {
        return std::nullopt;
    }
    return game_.getBoard().getPieceAt({row, col});
}

void ChessAPI::printBoard() const {
    const Board& board = game_.getBoard();

    std::cout << "  +------------------------+\n";
    for (int row = 7; row >= 0; --row) {
        std::cout << (row + 1) << " | ";
        for (int col = 0; col < 8; ++col) {
            auto piece = board.getPieceAt({row, col});
            char symbol = '.';

            if (piece) {
                switch (piece->type) {
                    case PieceType::PAWN:   symbol = (piece->color == PlayerColor::WHITE) ? 'P' : 'p'; break;
                    case PieceType::KNIGHT: symbol = (piece->color == PlayerColor::WHITE) ? 'N' : 'n'; break;
                    case PieceType::BISHOP: symbol = (piece->color == PlayerColor::WHITE) ? 'B' : 'b'; break;
                    case PieceType::ROOK:   symbol = (piece->color == PlayerColor::WHITE) ? 'R' : 'r'; break;
                    case PieceType::QUEEN:  symbol = (piece->color == PlayerColor::WHITE) ? 'Q' : 'q'; break;
                    case PieceType::KING:   symbol = (piece->color == PlayerColor::WHITE) ? 'K' : 'k'; break;
                }

                if (piece->cooldown_ticks_remaining > 0) {
                    std::cout << "\033[2m" << symbol << "\033[0m ";
                } else {
                    std::cout << symbol << " ";
                }
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "|\n";
    }
    std::cout << "  +------------------------+\n";
    std::cout << "    a b c d e f g h\n";

    std::cout << "Game status: ";
    switch (game_.getState()) {
        case GameState::ACTIVE: std::cout << "Active"; break;
        case GameState::WHITE_WIN: std::cout << "White wins"; break;
        case GameState::BLACK_WIN: std::cout << "Black wins"; break;
        default: std::cout << "Other"; break;
    }
    std::cout << std::endl;
}

GameState ChessAPI::getGameState() const {
    return game_.getState();
}

bool ChessAPI::isGameOver() const {
    GameState state = game_.getState();
    return state == GameState::WHITE_WIN || state == GameState::BLACK_WIN;
}

void ChessAPI::reset(const std::string& fen) {
    GameSettings settings;
    settings.against_ai = against_ai_;
    if (against_ai_) {
        settings.ai_difficulty = ai_player_->getDifficulty();
    }
    settings.white_cooldown_ticks = game_.getWhiteCooldown();
    settings.black_cooldown_ticks = game_.getBlackCooldown();
    settings.tick_rate_ms = 100;
    settings.fen_string = fen.empty() ? FENParser::getDefaultFEN() : fen;

    game_.reset();
    game_.applySettings(settings);
    game_.start();
}

bool ChessAPI::makeAIMove() {
    if (!against_ai_ || !ai_player_) {
        return false;
    }

    auto move = ai_player_->getBestMove(game_);
    if (move) {
        return game_.makeMove(move->piece_id, move->to);
    }
    return false;
}