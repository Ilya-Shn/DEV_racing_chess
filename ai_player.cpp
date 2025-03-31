#include "ai_player.h"
#include <algorithm>
#include <iostream>
#include <cmath>

AIPlayer::AIPlayer(AIDifficulty difficulty, PlayerColor color)
        : difficulty_(difficulty),
          color_(color),
          rng_(std::random_device()()),
          move_randomness_(3) {
    setDifficulty(difficulty);
}

void AIPlayer::setDifficulty(AIDifficulty difficulty) {
    difficulty_ = difficulty;
    switch (difficulty) {
        case AIDifficulty::EASY:
            move_randomness_ = 5;
            break;
        case AIDifficulty::MEDIUM:
            move_randomness_ = 3;
            break;
        case AIDifficulty::HARD:
            move_randomness_ = 2;
            break;
        case AIDifficulty::EXPERT:
            move_randomness_ = 1;
            break;
    }
}

std::optional<Move> AIPlayer::getBestMove(const Game &game) {
    auto moves = evaluateAllMoves(game);

    if (moves.empty()) {
        return std::nullopt;
    }

    std::sort(moves.begin(), moves.end(), [](const MoveScore &a, const MoveScore &b) {
        return a.score > b.score;
    });

    int top_n = std::min(move_randomness_, static_cast<int>(moves.size()));
    std::uniform_int_distribution<> dist(0, top_n - 1);
    int selected_index = dist(rng_);

    Move best_move = moves[selected_index].move;
    return best_move;
}

std::vector<AIPlayer::MoveScore> AIPlayer::evaluateAllMoves(const Game &game) {
    std::vector<MoveScore> moves;
    const Board &board = game.getBoard();

    auto pieces = board.getPlayerPieces(color_, false);
    MoveValidator validator;
    for (const auto &piece: pieces) {
        if (piece.cooldown_ticks_remaining > 0) {
            continue;
        }
        auto valid_moves = validator.getValidMoves(board, piece.id);
        for (const auto &target: valid_moves) {
            MoveScore move_score;
            move_score.move.piece_id = piece.id;
            move_score.move.from = piece.position;
            move_score.move.to = target;
            move_score.move.timestamp = 0;
            move_score.score = evaluateMove(game, piece, target);
            moves.push_back(move_score);
        }
    }
    return moves;
}

double AIPlayer::evaluateMove(const Game &game, const Piece &piece, Position target) {
    double score = 0.0;
    if (piece.type == PieceType::PAWN) {
        score += getRowScore(piece, target);
    }
    if (piece.type == PieceType::KNIGHT || piece.type == PieceType::BISHOP) {
        score += getColScore(piece, target);
    }
    score += getCaptureScore(game, piece, target) * 8.0;
    score += getPressureScore(game, piece, target) * 1.5;
    score -= getVulnerabilityScore(game, piece, target) * 1.8;
    score += getProtectionScore(game, piece, target) * 1.2;
    if (piece.type == PieceType::KING && std::abs(target.col - piece.position.col) == 2) {
        score += 3.0;
    }
    if (piece.type == PieceType::PAWN &&
        ((piece.color == PlayerColor::WHITE && target.row == 7) ||
         (piece.color == PlayerColor::BLACK && target.row == 0))) {
        score += 9.0;
    }
    double king_threat = getKingThreatScore(game, piece, target);
    score -= king_threat * 1.8;
    return score;
}

double AIPlayer::getRowScore(const Piece &piece, Position target) {
    if (piece.type != PieceType::PAWN) {
        return 0.0;
    }
    int direction = (piece.color == PlayerColor::WHITE) ? 1 : -1;
    int progress = (target.row - piece.position.row) * direction;
    double base_score = progress * 0.1;
    int distance_to_promotion = (piece.color == PlayerColor::WHITE) ? (7 - target.row) : target.row;
    double promotion_score = (7 - distance_to_promotion) * 0.05;
    return base_score + promotion_score;
}

double AIPlayer::getColScore(const Piece &piece, Position target) {
    double col_center = std::abs(3.5 - target.col);
    double row_center = std::abs(3.5 - target.row);
    return 0.1 * (4 - col_center) + 0.1 * (4 - row_center);
}

double AIPlayer::getCaptureScore(const Game &game, const Piece &piece, Position target) {
    const Board &board = game.getBoard();
    auto target_piece = board.getPieceAt(target);
    if (!target_piece) {
        return 0.0;
    }
    double piece_values[] = {
            1.0,
            3.0,
            3.2,
            5.0,
            9.0,
            100.0
    };
    return piece_values[static_cast<int>(target_piece->type)];
}

double AIPlayer::getPressureScore(const Game &game, const Piece &piece, Position target) {
    Board temp_board = game.getBoard();
    temp_board.movePiece(piece.id, target);
    auto moved_piece = temp_board.getPieceById(piece.id);
    if (!moved_piece) {
        return 0.0;
    }
    MoveValidator validator;
    auto new_moves = validator.getValidMoves(temp_board, moved_piece->id);
    double pressure_score = 0.0;
    for (const auto &pos: new_moves) {
        auto threatened_piece = temp_board.getPieceAt(pos);
        if (threatened_piece && threatened_piece->color != piece.color) {
            switch (threatened_piece->type) {
                case PieceType::PAWN:
                    pressure_score += 1.0;
                    break;
                case PieceType::KNIGHT:
                case PieceType::BISHOP:
                    pressure_score += 3.0;
                    break;
                case PieceType::ROOK:
                    pressure_score += 5.0;
                    break;
                case PieceType::QUEEN:
                    pressure_score += 9.0;
                    break;
                case PieceType::KING:
                    pressure_score += 12.0;
                    break;
            }
        }
    }
    return pressure_score * 0.1;
}

double AIPlayer::getVulnerabilityScore(const Game &game, const Piece &piece, Position target) {
    Board temp_board = game.getBoard();
    temp_board.movePiece(piece.id, target);
    PlayerColor enemy_color = (piece.color == PlayerColor::WHITE) ? PlayerColor::BLACK : PlayerColor::WHITE;
    auto enemy_pieces = temp_board.getPlayerPieces(enemy_color, false);
    double piece_value = 0.0;
    switch (piece.type) {
        case PieceType::PAWN:
            piece_value = 1.0;
            break;
        case PieceType::KNIGHT:
        case PieceType::BISHOP:
            piece_value = 3.0;
            break;
        case PieceType::ROOK:
            piece_value = 5.0;
            break;
        case PieceType::QUEEN:
            piece_value = 9.0;
            break;
        case PieceType::KING:
            piece_value = 100.0;
            break;
    }
    MoveValidator validator;
    bool is_threatened = false;
    for (const auto &enemy: enemy_pieces) {
        if (enemy.cooldown_ticks_remaining > 0) {
            continue;
        }
        auto enemy_moves = validator.getValidMoves(temp_board, enemy.id);
        for (const auto &enemy_move: enemy_moves) {
            if (enemy_move == target) {
                is_threatened = true;
                break;
            }
        }
        if (is_threatened) break;
    }
    return is_threatened ? piece_value : 0.0;
}

double AIPlayer::getProtectionScore(const Game &game, const Piece &piece, Position target) {
    Board temp_board = game.getBoard();
    temp_board.movePiece(piece.id, target);
    auto moved_piece = temp_board.getPieceById(piece.id);
    if (!moved_piece) {
        return 0.0;
    }
    auto friendly_pieces = temp_board.getPlayerPieces(color_, false);
    MoveValidator validator;
    auto new_moves = validator.getValidMoves(temp_board, moved_piece->id);
    double protection_score = 0.0;
    for (const auto &friendly: friendly_pieces) {
        if (friendly.id == piece.id) {
            continue;
        }
        for (const auto &move_pos: new_moves) {
            int row_diff = std::abs(move_pos.row - friendly.position.row);
            int col_diff = std::abs(move_pos.col - friendly.position.col);
            if (row_diff <= 1 && col_diff <= 1) {
                switch (friendly.type) {
                    case PieceType::PAWN:
                        protection_score += 0.5;
                        break;
                    case PieceType::KNIGHT:
                    case PieceType::BISHOP:
                        protection_score += 1.5;
                        break;
                    case PieceType::ROOK:
                        protection_score += 2.5;
                        break;
                    case PieceType::QUEEN:
                        protection_score += 4.5;
                        break;
                    case PieceType::KING:
                        protection_score += 5.0;
                        break;
                }
                break;
            }
        }
    }
    return protection_score * 0.1;
}

double AIPlayer::getKingThreatScore(const Game &game, const Piece &piece, Position target) {
    if (piece.type == PieceType::KING) {
        return 0.0;
    }
    Board temp_board = game.getBoard();
    temp_board.movePiece(piece.id, target);
    PlayerColor friendly_color = piece.color;
    Position king_pos;
    bool king_found = false;
    auto friendly_pieces = temp_board.getPlayerPieces(friendly_color, false);
    for (const auto &p: friendly_pieces) {
        if (p.type == PieceType::KING) {
            king_pos = p.position;
            king_found = true;
            break;
        }
    }
    if (!king_found) {
        return 0.0;
    }
    PlayerColor enemy_color = (piece.color == PlayerColor::WHITE) ? PlayerColor::BLACK : PlayerColor::WHITE;
    auto enemy_pieces = temp_board.getPlayerPieces(enemy_color, false);
    MoveValidator validator;
    for (const auto &enemy: enemy_pieces) {
        if (enemy.cooldown_ticks_remaining > 0) {
            continue;
        }
        auto enemy_moves = validator.getValidMoves(temp_board, enemy.id);
        for (const auto &move_pos: enemy_moves) {
            if (move_pos == king_pos) {
                return 100.0;
            }
        }
    }
    return 0.0;
}
