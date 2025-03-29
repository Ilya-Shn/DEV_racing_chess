#include "game.h"
#include <iostream>

Game::Game(std::function<void(GameState)> state_change_callback)
        : state_(GameState::NOT_STARTED),
          state_change_callback_(state_change_callback),
          white_cooldown_(10),
          black_cooldown_(10),
          against_ai_(false) {

    // Инициализация игры
}

void Game::applySettings(const GameSettings& settings) {
    white_cooldown_ = settings.white_cooldown_ticks;
    black_cooldown_ = settings.black_cooldown_ticks;
    timer_.setTickRate(settings.tick_rate_ms);
    against_ai_ = settings.against_ai;

    // Настройка доски
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

        // Запускаем таймер
        timer_.start([this]() { this->tick(); });

        if (state_change_callback_) {
            state_change_callback_(state_);
        }
    }
}

void Game::pause() {
    if (state_ == GameState::ACTIVE) {
        state_ = GameState::PAUSED;

        // Останавливаем таймер
        timer_.stop();

        if (state_change_callback_) {
            state_change_callback_(state_);
        }
    }
}

void Game::resume() {
    if (state_ == GameState::PAUSED) {
        state_ = GameState::ACTIVE;

        // Возобновляем таймер
        timer_.start([this]() { this->tick(); });

        if (state_change_callback_) {
            state_change_callback_(state_);
        }
    }
}

void Game::reset() {
    // Останавливаем таймер
    timer_.stop();

    // Сбрасываем доску
    if (board_.getAllPieces().empty()) {
        board_.setupStandardPosition();
    }

    // Сбрасываем состояние
    state_ = GameState::WAITING_FOR_SETTINGS;

    if (state_change_callback_) {
        state_change_callback_(state_);
    }
}

bool Game::makeMove(uint32_t piece_id, Position target) {
    if (state_ != GameState::ACTIVE) {
        return false;
    }

    // Проверяем, существует ли фигура
    auto piece_opt = board_.getPieceById(piece_id);
    if (!piece_opt) {
        return false;
    }

    // Проверяем, находится ли фигура на кулдауне
    if (piece_opt->cooldown_ticks_remaining > 0) {
        std::cout << "DEBUG: Фигура на кулдауне: " << piece_opt->cooldown_ticks_remaining << " тиков" << std::endl;
        return false;
    }

    // Проверяем, является ли ход допустимым
    if (!validator_.isValidMove(board_, piece_id, target)) {
        return false;
    }

    if (isCastlingMove(*piece_opt, target)) {
        handleCastling(piece_id, target);
        return true;
    }

    // Выполняем ход
    if (board_.movePiece(piece_id, target)) {
        // Проверяем и выполняем превращение пешки, если необходимо
        checkPawnPromotion(piece_id, target);

        // Получаем фигуру после перемещения и применяем кулдаун
        applyCooldown(piece_id);

        // Проверяем, не закончилась ли игра
        updateGameState();

        return true;
    }

    return false;
}

// Новый метод для проверки и выполнения превращения пешки
void Game::checkPawnPromotion(uint32_t piece_id, Position target) {
    auto piece_opt = board_.getPieceById(piece_id);
    if (!piece_opt || piece_opt->type != PieceType::PAWN) {
        return;
    }

    // Проверяем, достигла ли пешка последней горизонтали
    bool is_last_rank = (piece_opt->color == PlayerColor::WHITE && target.row == 7) ||
                        (piece_opt->color == PlayerColor::BLACK && target.row == 0);

    if (is_last_rank) {
        // Превращаем пешку в ферзя
        board_.promotePawn(piece_id, PieceType::QUEEN);
        std::cout << "DEBUG: Пешка превращена в ферзя" << std::endl;
    }
}

// В файле game.cpp добавить реализацию методов

// Проверяет, является ли движение короля рокировкой
bool Game::isCastlingMove(const Piece& king, Position target) const {
    // Проверяем, что король не ходил (для стандартной рокировки)
    if (king.moved) {
        return false;
    }

    // Проверяем, что король двигается на две клетки по горизонтали
    int col_diff = std::abs(target.col - king.position.col);
    return col_diff == 2 && king.position.row == target.row;
}

// Обрабатывает рокировку
bool Game::handleCastling(uint32_t king_id, Position target) {
    auto king_opt = board_.getPieceById(king_id);
    if (!king_opt || king_opt->moved) {
        return false;
    }

    const Piece& king = king_opt.value();

    // Проверяем, является ли это движение рокировкой
    if (!isCastlingMove(king, target)) {
        return false;
    }

    // Определяем направление рокировки (влево или вправо)
    bool is_kingside = target.col > king.position.col;
    int rook_col = is_kingside ? 7 : 0;

    // Находим ладью
    Position rook_pos = {king.position.row, rook_col};
    auto rook_opt = board_.getPieceAt(rook_pos);

    if (!rook_opt || rook_opt->type != PieceType::ROOK ||
        rook_opt->color != king.color || rook_opt->moved) {
        return false;
    }

    // Проверяем, свободен ли путь для рокировки
    int step = is_kingside ? 1 : -1;
    for (int col = king.position.col + step; col != rook_col; col += step) {
        Position pos = {king.position.row, col};
        if (board_.getPieceAt(pos)) {
            return false;
        }
    }

    // Выполняем рокировку (перемещаем короля и ладью)
    uint32_t rook_id = rook_opt->id;
    Position rook_target = {king.position.row, king.position.col + step};

    // Перемещаем короля
    if (!board_.movePiece(king_id, target)) {
        return false;
    }

    // Перемещаем ладью
    if (!board_.movePiece(rook_id, rook_target)) {
        // Если не удалось переместить ладью, возвращаем короля назад
        board_.movePiece(king_id, king.position);
        return false;
    }

    // Применяем кулдаун к обеим фигурам
    applyCooldown(king_id);
    applyCooldown(rook_id);

    std::cout << "DEBUG: Выполнена рокировка " << (is_kingside ? "на королевский фланг" : "на ферзевый фланг") << std::endl;
    return true;
}

std::vector<Position> Game::getValidMoves(uint32_t piece_id) const {
    return validator_.getValidMoves(board_, piece_id);
}

void Game::tick() {
    static int tick_counter = 0;
    tick_counter++;

    // Уменьшаем кулдауны
    board_.decrementCooldowns();

    // Проверяем, не закончилась ли игра
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
    // В простейшей реализации, игра заканчивается, когда король захвачен
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

// ИСПРАВЛЕНО: Теперь принимаем piece_id вместо ссылки на фигуру
void Game::applyCooldown(uint32_t piece_id) {
    // Получаем фигуру по ID
    auto piece_opt = board_.getPieceById(piece_id);
    if (!piece_opt) {
        return;
    }

    // Определяем значение кулдауна в зависимости от цвета
    int cooldown = (piece_opt->color == PlayerColor::WHITE) ? white_cooldown_ : black_cooldown_;

    // Обновляем значение cooldown_ticks_remaining напрямую в контейнере фигур
    board_.setPieceCooldown(piece_id, cooldown);

    std::cout << "DEBUG: Установлен cooldown " << cooldown << " на фигуру ID " << piece_id << std::endl;
}

void Game::checkGameOver() {
    updateGameState();
}