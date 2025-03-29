#include "ai_player.h"
#include <algorithm>
#include <iostream>
#include <cmath>

AIPlayer::AIPlayer(AIDifficulty difficulty, PlayerColor color)
        : difficulty_(difficulty),
          color_(color),
          rng_(std::random_device()()),
          ticks_per_move_(20),
          ticks_counter_(0),
          move_randomness_(3) {

    // Настройка сложности
    setDifficulty(difficulty);
}

void AIPlayer::setDifficulty(AIDifficulty difficulty) {
    difficulty_ = difficulty;

    // Настраиваем параметры в зависимости от сложности
    switch (difficulty) {
        case AIDifficulty::EASY:
            ticks_per_move_ = 40;  // Долго думает между ходами
            move_randomness_ = 5;  // Выбирает из топ-5 ходов (больше случайности)
            break;

        case AIDifficulty::MEDIUM:
            ticks_per_move_ = 20;
            move_randomness_ = 3;
            break;

        case AIDifficulty::HARD:
            ticks_per_move_ = 10;
            move_randomness_ = 2;
            break;

        case AIDifficulty::EXPERT:
            ticks_per_move_ = 5;   // Быстро делает ходы
            move_randomness_ = 1;  // Всегда выбирает лучший ход
            break;
    }
}

std::optional<Move> AIPlayer::getBestMove(const Game& game) {
    // В режиме Racing Chess AI может ходить в любое время,
    // но только своими фигурами и только если они не на кулдауне

    auto moves = evaluateAllMoves(game);

    if (moves.empty()) {
        std::cout << "DEBUG: AI не нашел доступных ходов" << std::endl;
        return std::nullopt;
    }

    // Сортируем ходы по убыванию оценки
    std::sort(moves.begin(), moves.end(), [](const MoveScore& a, const MoveScore& b) {
        return a.score > b.score;
    });

    // Выбираем случайный ход из топ-N лучших ходов
    int top_n = std::min(move_randomness_, static_cast<int>(moves.size()));
    std::uniform_int_distribution<> dist(0, top_n - 1);
    int selected_index = dist(rng_);

    Move best_move = moves[selected_index].move;

    std::cout << "DEBUG: AI выбрал ход фигурой ID " << best_move.piece_id
              << " с оценкой " << moves[selected_index].score << std::endl;

    return best_move;
}

std::vector<AIPlayer::MoveScore> AIPlayer::evaluateAllMoves(const Game& game) {
    std::vector<MoveScore> moves;
    const Board& board = game.getBoard();

    // Получаем все фигуры AI
    auto pieces = board.getPlayerPieces(color_, false);

    // Для каждой фигуры находим все возможные ходы и оцениваем их
    MoveValidator validator;
    for (const auto& piece : pieces) {
        // Пропускаем фигуры на кулдауне
        if (piece.cooldown_ticks_remaining > 0) {
            continue;
        }

        // Получаем все возможные ходы для фигуры
        auto valid_moves = validator.getValidMoves(board, piece.id);

        // Оцениваем каждый ход
        for (const auto& target : valid_moves) {
            MoveScore move_score;
            move_score.move.piece_id = piece.id;
            move_score.move.from = piece.position;
            move_score.move.to = target;
            move_score.move.timestamp = 0;  // Временная метка не используется в упрощенной версии

            // Оцениваем ход
            move_score.score = evaluateMove(game, piece, target);

            moves.push_back(move_score);
        }
    }

    return moves;
}

double AIPlayer::evaluateMove(const Game& game, const Piece& piece, Position target) {
    double score = 0;

    // Базовая оценка - продвижение вперед (для пешек)
    if (piece.type == PieceType::PAWN) {
        score += getRowScore(piece, target);
    }

    // Центрирование (для коней и слонов)
    if (piece.type == PieceType::KNIGHT || piece.type == PieceType::BISHOP) {
        score += getColScore(piece, target);
    }

    // Бонус за взятие фигуры
    score += getCaptureScore(game, piece, target) * 8.0;  // Наибольший вес

    // Бонус за оказание давления на вражеские фигуры
    score += getPressureScore(game, piece, target) * 1.5;

    // Штраф за подвергание фигуры опасности
    score -= getVulnerabilityScore(game, piece, target) * 1.8;

    // Бонус за защиту своих фигур
    score += getProtectionScore(game, piece, target) * 1.2;

    return score;
}

double AIPlayer::getRowScore(const Piece& piece, Position target) {
    if (piece.type != PieceType::PAWN) return 0;

    int direction = (piece.color == PlayerColor::WHITE) ? 1 : -1;
    int progress = (target.row - piece.position.row) * direction;

    // Базовая оценка за продвижение вперед
    double base_score = progress * 0.1;

    // Бонус за приближение к последней горизонтали
    int distance_to_promotion = (piece.color == PlayerColor::WHITE) ? (7 - target.row) : target.row;
    double promotion_score = (7 - distance_to_promotion) * 0.05;

    return base_score + promotion_score;
}

double AIPlayer::getColScore(const Piece& piece, Position target) {
    // Бонус за приближение к центру доски
    double col_center = std::abs(3.5 - target.col);
    double row_center = std::abs(3.5 - target.row);

    // Чем ближе к центру, тем выше оценка
    return 0.1 * (4 - col_center) + 0.1 * (4 - row_center);
}

double AIPlayer::getCaptureScore(const Game& game, const Piece& piece, Position target) {
    const Board& board = game.getBoard();
    auto target_piece = board.getPieceAt(target);

    if (!target_piece) {
        return 0; // Нет взятия
    }

    // Оценки типов фигур
    double piece_values[] = {
            1.0,  // Пешка
            3.0,  // Конь
            3.2,  // Слон
            5.0,  // Ладья
            9.0,  // Ферзь
            100.0   // Король (не должны взять короля в нашей реализации)
    };

    return piece_values[static_cast<int>(target_piece->type)];
}

double AIPlayer::getPressureScore(const Game& game, const Piece& piece, Position target) {
    // Создаем временную доску для анализа
    Board temp_board = game.getBoard();

    // Временно перемещаем фигуру
    temp_board.movePiece(piece.id, target);

    // Получаем фигуру после перемещения
    auto moved_piece = temp_board.getPieceById(piece.id);
    if (!moved_piece) {
        return 0;
    }

    // Анализируем, сколько вражеских фигур атакует наша фигура с новой позиции
    MoveValidator validator;
    auto new_moves = validator.getValidMoves(temp_board, moved_piece->id);

    int pressure_count = 0;
    for (const auto& pos : new_moves) {
        auto threatened_piece = temp_board.getPieceAt(pos);
        if (threatened_piece && threatened_piece->color != piece.color) {
            // Оценка зависит от ценности фигуры под ударом
            double piece_value = 0;
            switch (threatened_piece->type) {
                case PieceType::PAWN: piece_value = 1.0; break;
                case PieceType::KNIGHT: case PieceType::BISHOP: piece_value = 3.0; break;
                case PieceType::ROOK: piece_value = 5.0; break;
                case PieceType::QUEEN: piece_value = 9.0; break;
                case PieceType::KING: piece_value = 12.0; break;
            }
            pressure_count += piece_value;
        }
    }

    return pressure_count * 0.1;
}

double AIPlayer::getVulnerabilityScore(const Game& game, const Piece& piece, Position target) {
    // Создаем временную доску для анализа
    Board temp_board = game.getBoard();

    // Временно перемещаем фигуру
    temp_board.movePiece(piece.id, target);

    PlayerColor enemy_color = (piece.color == PlayerColor::WHITE) ? PlayerColor::BLACK : PlayerColor::WHITE;
    auto enemy_pieces = temp_board.getPlayerPieces(enemy_color, false);

    // Оценка фигуры
    double piece_value = 0;
    switch (piece.type) {
        case PieceType::PAWN: piece_value = 1.0; break;
        case PieceType::KNIGHT: case PieceType::BISHOP: piece_value = 3.0; break;
        case PieceType::ROOK: piece_value = 5.0; break;
        case PieceType::QUEEN: piece_value = 9.0; break;
        case PieceType::KING: piece_value = 10.0; break;
    }

    // Проверяем, может ли вражеская фигура атаковать нашу фигуру
    MoveValidator validator;
    bool is_threatened = false;
    for (const auto& enemy : enemy_pieces) {
        if (enemy.cooldown_ticks_remaining > 0) continue; // Пропускаем фигуры на кулдауне

        auto enemy_moves = validator.getValidMoves(temp_board, enemy.id);
        for (const auto& enemy_move : enemy_moves) {
            if (enemy_move == target) {
                is_threatened = true;
                break;
            }
        }
        if (is_threatened) break;
    }

    return is_threatened ? piece_value : 0;
}

double AIPlayer::getProtectionScore(const Game& game, const Piece& piece, Position target) {
    // Создаем временную доску для анализа
    Board temp_board = game.getBoard();

    // Временно перемещаем фигуру
    temp_board.movePiece(piece.id, target);

    // Получаем фигуру после перемещения
    auto moved_piece = temp_board.getPieceById(piece.id);
    if (!moved_piece) {
        return 0;
    }

    // Получаем все свои фигуры
    auto friendly_pieces = temp_board.getPlayerPieces(color_, false);

    // Анализируем, сколько своих фигур защищает наша фигура
    MoveValidator validator;
    auto new_moves = validator.getValidMoves(temp_board, moved_piece->id);

    double protection_score = 0;
    for (const auto& friendly : friendly_pieces) {
        if (friendly.id == piece.id) continue; // Пропускаем саму фигуру

        // Считаем фигуру защищенной, если мы можем сделать ход на клетку рядом с ней
        for (const auto& move_pos : new_moves) {
            // Примерная проверка близости (упрощенно)
            int row_diff = std::abs(move_pos.row - friendly.position.row);
            int col_diff = std::abs(move_pos.col - friendly.position.col);
            if (row_diff <= 1 && col_diff <= 1) {
                // Оценка зависит от ценности защищаемой фигуры
                double friendly_value = 0;
                switch (friendly.type) {
                    case PieceType::PAWN: friendly_value = 0.5; break;
                    case PieceType::KNIGHT: case PieceType::BISHOP: friendly_value = 1.5; break;
                    case PieceType::ROOK: friendly_value = 2.5; break;
                    case PieceType::QUEEN: friendly_value = 4.5; break;
                    case PieceType::KING: friendly_value = 5.0; break;
                }
                protection_score += friendly_value;
                break;  // Считаем фигуру один раз
            }
        }
    }

    return protection_score * 0.1;
}

void AIPlayer::setMoveProbability(int ticks) {
    ticks_per_move_ = ticks;
}