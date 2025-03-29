#include "move_validator.h"
#include <cmath>
#include <algorithm>
#include <vector>

MoveValidator::MoveValidator() {
    // Конструктор
}

bool MoveValidator::isValidMove(const Board& board, uint32_t piece_id, Position target) const {
    // Проверяем, существует ли фигура
    auto piece_opt = board.getPieceById(piece_id);
    if (!piece_opt) {
        return false;
    }

    const Piece& piece = piece_opt.value();

    // Проверяем, не захвачена ли фигура
    if (piece.captured) {
        return false;
    }

    // Проверяем, не находится ли фигура на кулдауне
    if (piece.cooldown_ticks_remaining > 0) {
        return false;
    }

    // Не пытаемся ли мы остаться на месте
    if (piece.position == target) {
        return false;
    }

    // Целевая позиция должна быть пустой или содержать вражескую фигуру
    if (!isTargetEmptyOrEnemy(board, piece, target)) {
        return false;
    }

    // Проверяем правила движения для конкретного типа фигуры
    switch (piece.type) {
        case PieceType::PAWN:
            return isValidPawnMove(board, piece, target);
        case PieceType::KNIGHT:
            return isValidKnightMove(board, piece, target);
        case PieceType::BISHOP:
            return isValidBishopMove(board, piece, target);
        case PieceType::ROOK:
            return isValidRookMove(board, piece, target);
        case PieceType::QUEEN:
            return isValidQueenMove(board, piece, target);
        case PieceType::KING:
            return isValidKingMove(board, piece, target);
        default:
            return false;
    }
}

std::vector<Position> MoveValidator::getValidMoves(const Board& board, uint32_t piece_id) const {
    std::vector<Position> valid_moves;

    auto piece_opt = board.getPieceById(piece_id);
    if (!piece_opt || piece_opt->captured || piece_opt->cooldown_ticks_remaining > 0) {
        return valid_moves;
    }

    // Перебираем все возможные позиции на доске
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position target = {row, col};
            if (isValidMove(board, piece_id, target)) {
                valid_moves.push_back(target);
            }
        }
    }

    return valid_moves;
}

bool MoveValidator::isValidPawnMove(const Board& board, const Piece& piece, Position target) const {
    int direction = (piece.color == PlayerColor::WHITE) ? 1 : -1;
    int forward_one = piece.position.row + direction;
    int start_row = (piece.color == PlayerColor::WHITE) ? 1 : 6;

    // Движение вперед на одну клетку
    if (target.col == piece.position.col && target.row == forward_one) {
        return !board.getPieceAt(target).has_value(); // Должно быть пусто
    }

    // Движение вперед на две клетки с начальной позиции
    if (piece.position.row == start_row &&
        target.col == piece.position.col &&
        target.row == piece.position.row + 2 * direction) {

        Position intermediate = {piece.position.row + direction, piece.position.col};
        return !board.getPieceAt(intermediate).has_value() && // Промежуточная клетка должна быть пуста
               !board.getPieceAt(target).has_value() && // Целевая клетка должна быть пуста
               !piece.moved; // Пешка не должна была двигаться
    }

    // Взятие по диагонали
    if (target.row == forward_one &&
        (target.col == piece.position.col - 1 || target.col == piece.position.col + 1)) {

        auto target_piece = board.getPieceAt(target);
        return target_piece && target_piece->color != piece.color; // Должна быть вражеская фигура
    }

    return false;
}

bool MoveValidator::isValidKnightMove(const Board& board, const Piece& piece, Position target) const {
    int row_diff = std::abs(target.row - piece.position.row);
    int col_diff = std::abs(target.col - piece.position.col);

    // Конь ходит буквой "Г" - 2 клетки в одном направлении и 1 в другом
    return (row_diff == 2 && col_diff == 1) || (row_diff == 1 && col_diff == 2);
}

bool MoveValidator::isValidBishopMove(const Board& board, const Piece& piece, Position target) const {
    int row_diff = std::abs(target.row - piece.position.row);
    int col_diff = std::abs(target.col - piece.position.col);

    // Слон ходит только по диагоналям
    if (row_diff != col_diff) {
        return false;
    }

    // Проверяем, что путь свободен
    return isPathClear(board, piece.position, target);
}

bool MoveValidator::isValidRookMove(const Board& board, const Piece& piece, Position target) const {
    // Ладья ходит только по вертикалям или горизонталям
    if (piece.position.row != target.row && piece.position.col != target.col) {
        return false;
    }

    // Проверяем, что путь свободен
    return isPathClear(board, piece.position, target);
}

bool MoveValidator::isValidQueenMove(const Board& board, const Piece& piece, Position target) const {
    // Ферзь имеет ходы и ладьи, и слона
    return isValidRookMove(board, piece, target) || isValidBishopMove(board, piece, target);
}

// В файле move_validator.cpp обновить метод isValidKingMove

bool MoveValidator::isValidKingMove(const Board& board, const Piece& piece, Position target) const {
    int row_diff = std::abs(target.row - piece.position.row);
    int col_diff = std::abs(target.col - piece.position.col);

    // Стандартный ход короля - на одну клетку в любом направлении
    if (row_diff <= 1 && col_diff <= 1) {
        return true;
    }

    // Проверка на рокировку
    if (!piece.moved && row_diff == 0 && col_diff == 2) {
        // Определяем направление рокировки
        bool is_kingside = target.col > piece.position.col;
        int rook_col = is_kingside ? 7 : 0;

        // Проверяем наличие ладьи на нужной позиции
        Position rook_pos = {piece.position.row, rook_col};
        auto rook_opt = board.getPieceAt(rook_pos);

        if (!rook_opt || rook_opt->type != PieceType::ROOK ||
            rook_opt->color != piece.color || rook_opt->moved) {
            return false;
        }

        // Проверяем, свободен ли путь между королем и ладьей
        int step = is_kingside ? 1 : -1;
        for (int col = piece.position.col + step; col != rook_col; col += step) {
            Position pos = {piece.position.row, col};
            if (board.getPieceAt(pos)) {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool MoveValidator::isPathClear(const Board& board, Position from, Position to) const {
    int row_dir = 0;
    if (from.row < to.row) row_dir = 1;
    else if (from.row > to.row) row_dir = -1;

    int col_dir = 0;
    if (from.col < to.col) col_dir = 1;
    else if (from.col > to.col) col_dir = -1;

    Position current = {from.row + row_dir, from.col + col_dir};
    while (current != to) {
        if (board.getPieceAt(current).has_value()) {
            return false; // Путь заблокирован
        }
        current.row += row_dir;
        current.col += col_dir;
    }

    return true;
}

bool MoveValidator::isTargetEmptyOrEnemy(const Board& board, const Piece& piece, Position target) const {
    auto target_piece = board.getPieceAt(target);

    // Либо клетка пустая, либо там стоит вражеская фигура
    return !target_piece || target_piece->color != piece.color;
}