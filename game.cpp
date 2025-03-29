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

    // Выполняем ход
    if (board_.movePiece(piece_id, target)) {
        // Получаем фигуру после перемещения и применяем кулдаун
        applyCooldown(piece_id);

        // Проверяем, не закончилась ли игра
        updateGameState();

        return true;
    }

    return false;
}

std::vector<Position> Game::getValidMoves(uint32_t piece_id) const {
    return validator_.getValidMoves(board_, piece_id);
}

void Game::tick() {
    static int tick_counter = 0;
    tick_counter++;
    std::cout << "DEBUG: Game tick " << tick_counter << std::endl;

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