#include "ai_player.h"
#include <algorithm>
#include <iostream>
#include <cmath>

AIPlayer::AIPlayer(AIDifficulty difficulty, PlayerColor color)
        : difficulty_(difficulty),
          color_(color),
          rng_(std::random_device()()),
          move_randomness_(3) {
    // Настройка параметров в зависимости от указанной сложности
    setDifficulty(difficulty);
}

void AIPlayer::setDifficulty(AIDifficulty difficulty) {
    difficulty_ = difficulty;
    // Настраиваем параметры ИИ (скорость и случайность) в зависимости от сложности
    switch (difficulty) {
        case AIDifficulty::EASY:
            move_randomness_ = 5;  // Выбирает случайный ход из топ-5 лучших
            break;
        case AIDifficulty::MEDIUM:
            move_randomness_ = 3;
            break;
        case AIDifficulty::HARD:
            move_randomness_ = 2;
            break;
        case AIDifficulty::EXPERT:
            move_randomness_ = 1;  // Всегда выбирает лучший ход без случайности
            break;
    }
}

std::optional<Move> AIPlayer::getBestMove(const Game& game) {
    // В режиме Racing Chess ИИ может делать ход в любой момент,
    // но только своими фигурами и только если они не на кулдауне.
    auto moves = evaluateAllMoves(game);

    if (moves.empty()) {
        std::cout << "DEBUG: AI не нашел доступных ходов" << std::endl;
        return std::nullopt;
    }

    // Сортируем все возможные ходы по убыванию их оценки (наиболее предпочтительные первые)
    std::sort(moves.begin(), moves.end(), [](const MoveScore& a, const MoveScore& b) {
        return a.score > b.score;
    });

    // Выбираем случайный ход из топ-N лучших ходов, где N зависит от сложности (move_randomness_)
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

    // Получаем все фигуры текущего игрока (ИИ)
    auto pieces = board.getPlayerPieces(color_, false);
    MoveValidator validator;
    for (const auto& piece : pieces) {
        // Пропускаем фигуры, которые пока не могут ходить (находятся на кулдауне)
        if (piece.cooldown_ticks_remaining > 0) {
            continue;
        }
        // Получаем все допустимые ходы для данной фигуры
        auto valid_moves = validator.getValidMoves(board, piece.id);
        // Оцениваем каждый возможный ход этой фигуры
        for (const auto& target : valid_moves) {
            MoveScore move_score;
            move_score.move.piece_id = piece.id;
            move_score.move.from = piece.position;
            move_score.move.to = target;
            move_score.move.timestamp = 0;  // Временная метка (не используется в этой реализации)
            // Вычисляем эвристическую оценку данного хода
            move_score.score = evaluateMove(game, piece, target);
            moves.push_back(move_score);
        }
    }
    return moves;
}

double AIPlayer::evaluateMove(const Game& game, const Piece& piece, Position target) {
    double score = 0.0;
    // Базовая эвристика: поощрение пешек за продвижение вперед
    if (piece.type == PieceType::PAWN) {
        score += getRowScore(piece, target);
    }
    // Поощрение коней и слонов за продвижение к центру доски
    if (piece.type == PieceType::KNIGHT || piece.type == PieceType::BISHOP) {
        score += getColScore(piece, target);
    }
    // Большой бонус за взятие вражеской фигуры
    score += getCaptureScore(game, piece, target) * 8.0;
    // Бонус за создание угроз вражеским фигурам (атакуемые фигуры противника)
    score += getPressureScore(game, piece, target) * 1.5;
    // Штраф за то, что данная наша фигура может быть сразу побита врагом после хода
    score -= getVulnerabilityScore(game, piece, target) * 1.8;
    // Бонус за защиту своих фигур (если ход помогает прикрыть свои фигуры)
    score += getProtectionScore(game, piece, target) * 1.2;
    // Бонус за рокировку (улучшает защиту короля и активирует ладью)
    if (piece.type == PieceType::KING && std::abs(target.col - piece.position.col) == 2) {
        score += 3.0;
    }
    // Бонус за превращение пешки в ферзя (достижение последней горизонтали)
    if (piece.type == PieceType::PAWN &&
        ((piece.color == PlayerColor::WHITE && target.row == 7) ||
         (piece.color == PlayerColor::BLACK && target.row == 0))) {
        score += 9.0;
    }
    // Штраф, если после этого хода наш король окажется под угрозой взятия
    double king_threat = getKingThreatScore(game, piece, target);
    score -= king_threat * 1.8;
    return score;
}

double AIPlayer::getRowScore(const Piece& piece, Position target) {
    // Эвристика для пешки: продвижение вперед
    if (piece.type != PieceType::PAWN) {
        return 0.0;
    }
    int direction = (piece.color == PlayerColor::WHITE) ? 1 : -1;
    int progress = (target.row - piece.position.row) * direction;
    // Небольшой базовый бонус за любой ход вперед
    double base_score = progress * 0.1;
    // Дополнительный бонус, чем ближе пешка к последней горизонтали (промоции)
    int distance_to_promotion = (piece.color == PlayerColor::WHITE) ? (7 - target.row) : target.row;
    double promotion_score = (7 - distance_to_promotion) * 0.05;
    return base_score + promotion_score;
}

double AIPlayer::getColScore(const Piece& piece, Position target) {
    // Эвристика: поощрение за приближение к центру доски (важно для коней и слонов)
    double col_center = std::abs(3.5 - target.col);
    double row_center = std::abs(3.5 - target.row);
    // Чем ближе фигура окажется к центру (ряда 3 или 4 и колонке 3 или 4), тем выше оценка
    return 0.1 * (4 - col_center) + 0.1 * (4 - row_center);
}

double AIPlayer::getCaptureScore(const Game& game, const Piece& piece, Position target) {
    const Board& board = game.getBoard();
    auto target_piece = board.getPieceAt(target);
    if (!target_piece) {
        return 0.0; // На этой клетке нет вражеской фигуры, захвата нет
    }
    // Ценности фигур для оценки захвата (чем ценнее фигура противника, тем лучше захват)
    double piece_values[] = {
            1.0,   // Пешка
            3.0,   // Конь
            3.2,   // Слон
            5.0,   // Ладья
            9.0,   // Ферзь
            100.0  // Король (очень высокая ценность, захват короля выигрывает игру)
    };
    return piece_values[static_cast<int>(target_piece->type)];
}

double AIPlayer::getPressureScore(const Game& game, const Piece& piece, Position target) {
    // Эвристика давления: сколько вражеских фигур будет находиться под ударом после нашего хода
    Board temp_board = game.getBoard();
    temp_board.movePiece(piece.id, target);
    // Берем фигуру после перемещения на новой доске
    auto moved_piece = temp_board.getPieceById(piece.id);
    if (!moved_piece) {
        return 0.0;
    }
    MoveValidator validator;
    auto new_moves = validator.getValidMoves(temp_board, moved_piece->id);
    double pressure_score = 0.0;
    for (const auto& pos : new_moves) {
        auto threatened_piece = temp_board.getPieceAt(pos);
        if (threatened_piece && threatened_piece->color != piece.color) {
            // Если с новой позиции мы угрожаем вражеской фигуре, добавляем ее ценность
            switch (threatened_piece->type) {
                case PieceType::PAWN:   pressure_score += 1.0; break;
                case PieceType::KNIGHT: // Намеренно объединены с BISHOP для одной оценки
                case PieceType::BISHOP: pressure_score += 3.0; break;
                case PieceType::ROOK:   pressure_score += 5.0; break;
                case PieceType::QUEEN:  pressure_score += 9.0; break;
                case PieceType::KING:   pressure_score += 12.0; break;
            }
        }
    }
    // Возвращаем суммарную "ценность давления" с небольшим коэффициентом
    return pressure_score * 0.1;
}

double AIPlayer::getVulnerabilityScore(const Game& game, const Piece& piece, Position target) {
    // Проверяем, не станет ли перемещенная фигура сама под ударом врага
    Board temp_board = game.getBoard();
    temp_board.movePiece(piece.id, target);
    PlayerColor enemy_color = (piece.color == PlayerColor::WHITE) ? PlayerColor::BLACK : PlayerColor::WHITE;
    auto enemy_pieces = temp_board.getPlayerPieces(enemy_color, false);
    // Оценка ценности перемещаемой фигуры (для штрафа, если она будет под ударом)
    double piece_value = 0.0;
    switch (piece.type) {
        case PieceType::PAWN:   piece_value = 1.0; break;
        case PieceType::KNIGHT:
        case PieceType::BISHOP: piece_value = 3.0; break;
        case PieceType::ROOK:   piece_value = 5.0; break;
        case PieceType::QUEEN:  piece_value = 9.0; break;
        case PieceType::KING:   piece_value = 100.0; break;
    }
    // Проверяем все вражеские фигуры: могут ли они пойти на позицию target (где будет наша фигура)
    MoveValidator validator;
    bool is_threatened = false;
    for (const auto& enemy : enemy_pieces) {
        if (enemy.cooldown_ticks_remaining > 0) {
            continue; // Фигура врага не может ходить, если она на кулдауне
        }
        auto enemy_moves = validator.getValidMoves(temp_board, enemy.id);
        for (const auto& enemy_move : enemy_moves) {
            if (enemy_move == target) {
                // Найдена вражеская фигура, которая может побить нашу перемещенную фигуру
                is_threatened = true;
                break;
            }
        }
        if (is_threatened) break;
    }
    return is_threatened ? piece_value : 0.0;
}

double AIPlayer::getProtectionScore(const Game& game, const Piece& piece, Position target) {
    // Проверяем, защищает ли этот ход какие-либо наши фигуры
    Board temp_board = game.getBoard();
    temp_board.movePiece(piece.id, target);
    // Получаем указатель на перемещенную фигуру на новой доске
    auto moved_piece = temp_board.getPieceById(piece.id);
    if (!moved_piece) {
        return 0.0;
    }
    // Собираем все наши фигуры после совершения хода
    auto friendly_pieces = temp_board.getPlayerPieces(color_, false);
    MoveValidator validator;
    auto new_moves = validator.getValidMoves(temp_board, moved_piece->id);
    double protection_score = 0.0;
    for (const auto& friendly : friendly_pieces) {
        if (friendly.id == piece.id) {
            continue; // пропускаем саму перемещенную фигуру
        }
        // Считаем, что фигура защищена, если с новой позиции наша перемещенная фигура может сходить на клетку рядом с этой фигурой
        for (const auto& move_pos : new_moves) {
            int row_diff = std::abs(move_pos.row - friendly.position.row);
            int col_diff = std::abs(move_pos.col - friendly.position.col);
            if (row_diff <= 1 && col_diff <= 1) {
                // Чем ценнее защищаемая фигура, тем больший бонус
                switch (friendly.type) {
                    case PieceType::PAWN:   protection_score += 0.5; break;
                    case PieceType::KNIGHT:
                    case PieceType::BISHOP: protection_score += 1.5; break;
                    case PieceType::ROOK:   protection_score += 2.5; break;
                    case PieceType::QUEEN:  protection_score += 4.5; break;
                    case PieceType::KING:   protection_score += 5.0; break;
                }
                break; // засчитываем защиту этой фигуры один раз
            }
        }
    }
    return protection_score * 0.1;
}

double AIPlayer::getKingThreatScore(const Game& game, const Piece& piece, Position target) {
    // Проверяем, не станет ли наш король уязвимым после этого хода
    // Если мы двигаем самого короля, его уязвимость учтена в getVulnerabilityScore
    if (piece.type == PieceType::KING) {
        return 0.0;
    }
    // Создаем копию доски и совершаем ход
    Board temp_board = game.getBoard();
    temp_board.movePiece(piece.id, target);
    // Находим положение нашего короля на временной доске
    PlayerColor friendly_color = piece.color;
    Position king_pos;
    bool king_found = false;
    auto friendly_pieces = temp_board.getPlayerPieces(friendly_color, false);
    for (const auto& p : friendly_pieces) {
        if (p.type == PieceType::KING) {
            king_pos = p.position;
            king_found = true;
            break;
        }
    }
    if (!king_found) {
        return 0.0;
    }
    // Проверяем все фигуры противника: может ли какая-либо из них сходить на клетку короля
    PlayerColor enemy_color = (piece.color == PlayerColor::WHITE) ? PlayerColor::BLACK : PlayerColor::WHITE;
    auto enemy_pieces = temp_board.getPlayerPieces(enemy_color, false);
    MoveValidator validator;
    for (const auto& enemy : enemy_pieces) {
        if (enemy.cooldown_ticks_remaining > 0) {
            continue; // враг не может сразу пойти этой фигурой
        }
        auto enemy_moves = validator.getValidMoves(temp_board, enemy.id);
        for (const auto& move_pos : enemy_moves) {
            if (move_pos == king_pos) {
                // После нашего хода король может быть атакован этой фигурой
                return 100.0; // возвращаем высокую ценность угрозы королю
            }
        }
    }
    return 0.0;
}

void AIPlayer::setMoveProbability(int ticks) {
    // Установить интервал (в тиках) между ходами ИИ вручную
    ticks_per_move_ = ticks;
}
