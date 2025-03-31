#include "game.h"
#include <iostream>

Game::Game(std::function<void(GameState)> state_change_callback)
        : state_(GameState::NOT_STARTED),
          state_change_callback_(state_change_callback),
          white_cooldown_(10),
          black_cooldown_(10),
          against_ai_(false) {
}

void Game::applySettings(const GameSettings& settings) {
    white_cooldown_ = settings.white_cooldown_ticks;
    black_cooldown_ = settings.black_cooldown_ticks;
    timer_.setTickRate(settings.tick_rate_ms);
    against_ai_ = settings.against_ai;

    if (settings.fen_string.empty() || settings.fen_string == "standard") {
        board_.setupStandardPosition();
    } else {
        if (!board_.setupFromFEN(settings.fen_string)) {
            std::cerr << "Invalid FEN string, using standard position" << std::endl;
            board_.setupStandardPosition();
        }
    }

    state_ = GameState::WAITING_FOR_SETTINGS;

    if (state_change_callback_) {
        state_change_callback_(state_);
    }
}

void Game::start() {
    if (state_ == GameState::WAITING_FOR_SETTINGS || state_ == GameState::NOT_STARTED) {
        state_ = GameState::ACTIVE;

        timer_.start([this]() { this->tick(); });

        if (state_change_callback_) {
            state_change_callback_(state_);
        }
    }
}

void Game::pause() {
    if (state_ == GameState::ACTIVE) {
        state_ = GameState::PAUSED;

        timer_.stop();

        if (state_change_callback_) {
            state_change_callback_(state_);
        }
    }
}

void Game::resume() {
    if (state_ == GameState::PAUSED) {
        state_ = GameState::ACTIVE;

        timer_.start([this]() { this->tick(); });

        if (state_change_callback_) {
            state_change_callback_(state_);
        }
    }
}

void Game::reset() {
    timer_.stop();

    if (board_.getAllPieces().empty()) {
        board_.setupStandardPosition();
    }

    state_ = GameState::WAITING_FOR_SETTINGS;

    if (state_change_callback_) {
        state_change_callback_(state_);
    }
}

bool Game::makeMove(uint32_t piece_id, Position target) {
    if (state_ != GameState::ACTIVE) {
        return false;
    }

    auto piece_opt = board_.getPieceById(piece_id);
    if (!piece_opt) {
        return false;
    }

    if (piece_opt->cooldown_ticks_remaining > 0) {
        return false;
    }

    if (!validator_.isValidMove(board_, piece_id, target)) {
        return false;
    }

    if (isCastlingMove(*piece_opt, target)) {
        handleCastling(piece_id, target);
        return true;
    }

    if (board_.movePiece(piece_id, target)) {
        checkPawnPromotion(piece_id, target);
        applyCooldown(piece_id);
        updateGameState();
        return true;
    }

    return false;
}

void Game::checkPawnPromotion(uint32_t piece_id, Position target) {
    auto piece_opt = board_.getPieceById(piece_id);
    if (!piece_opt || piece_opt->type != PieceType::PAWN) {
        return;
    }

    bool is_last_rank = (piece_opt->color == PlayerColor::WHITE && target.row == 7) ||
                        (piece_opt->color == PlayerColor::BLACK && target.row == 0);

    if (is_last_rank) {
        board_.promotePawn(piece_id, PieceType::QUEEN);
    }
}

bool Game::isCastlingMove(const Piece& king, Position target) const {
    if (king.moved) {
        return false;
    }

    int col_diff = std::abs(target.col - king.position.col);
    return col_diff == 2 && king.position.row == target.row;
}

bool Game::handleCastling(uint32_t king_id, Position target) {
    auto king_opt = board_.getPieceById(king_id);
    if (!king_opt || king_opt->moved) {
        return false;
    }

    const Piece& king = king_opt.value();

    if (!isCastlingMove(king, target)) {
        return false;
    }

    bool is_kingside = target.col > king.position.col;
    int rook_col = is_kingside ? 7 : 0;

    Position rook_pos = {king.position.row, rook_col};
    auto rook_opt = board_.getPieceAt(rook_pos);

    if (!rook_opt || rook_opt->type != PieceType::ROOK ||
        rook_opt->color != king.color || rook_opt->moved) {
        return false;
    }

    int step = is_kingside ? 1 : -1;
    for (int col = king.position.col + step; col != rook_col; col += step) {
        Position pos = {king.position.row, col};
        if (board_.getPieceAt(pos)) {
            return false;
        }
    }

    uint32_t rook_id = rook_opt->id;
    Position rook_target = {king.position.row, king.position.col + step};

    if (!board_.movePiece(king_id, target)) {
        return false;
    }

    if (!board_.movePiece(rook_id, rook_target)) {
        board_.movePiece(king_id, king.position);
        return false;
    }

    applyCooldown(king_id);
    applyCooldown(rook_id);

    return true;
}

std::vector<Position> Game::getValidMoves(uint32_t piece_id) const {
    return validator_.getValidMoves(board_, piece_id);
}

void Game::tick() {
    static int tick_counter = 0;
    tick_counter++;

    board_.decrementCooldowns();
    updateGameState();
}

Board& Game::getBoard() {
    return board_;
}

const Board& Game::getBoard() const {
    return board_;
}

GameState Game::getState() const {
    return state_;
}

int Game::getWhiteCooldown() const {
    return white_cooldown_;
}

int Game::getBlackCooldown() const {
    return black_cooldown_;
}

void Game::updateGameState() {
    if (board_.countKings(PlayerColor::WHITE) == 0) {
        state_ = GameState::BLACK_WIN;
        timer_.stop();
        if (state_change_callback_) {
            state_change_callback_(state_);
        }
    }
    else if (board_.countKings(PlayerColor::BLACK) == 0) {
        state_ = GameState::WHITE_WIN;
        timer_.stop();
        if (state_change_callback_) {
            state_change_callback_(state_);
        }
    }
}

void Game::applyCooldown(uint32_t piece_id) {
    auto piece_opt = board_.getPieceById(piece_id);
    if (!piece_opt) {
        return;
    }

    int cooldown = (piece_opt->color == PlayerColor::WHITE) ? white_cooldown_ : black_cooldown_;
    board_.setPieceCooldown(piece_id, cooldown);
}

void Game::checkGameOver() {
    updateGameState();
}
