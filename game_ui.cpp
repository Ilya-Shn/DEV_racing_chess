#include "game_ui.h"
#include "fen_parser.h"
#include <iostream>
#include <cmath>

GameUI::GameUI(Game& game, bool against_ai, unsigned width, unsigned height)
        : window_(sf::VideoMode(width, height), "Speed Chess"),
          state_(UIState::GAME_ACTIVE),
          game_(game),
          against_ai_(against_ai),
          selected_piece_id_(std::nullopt),
          is_dragging_(false),
          board_size_(512),
          board_offset_x_((width - 512) / 2),
          board_offset_y_((height - 512) / 2),
          square_size_(512 / 8) {

    // Создаем AI, если нужно
    if (against_ai_) {
        ai_player_ = std::make_unique<AIPlayer>(AIDifficulty::MEDIUM, PlayerColor::BLACK);
        std::cout << "DEBUG: AI создан с цветом BLACK" << std::endl;
    }

    // Загрузка ресурсов и настройка доски
    loadResources();
    setupBoard();
}

void GameUI::run() {
    sf::Clock clock;

    while (window_.isOpen()) {
        sf::Time deltaTime = clock.restart();

        handleEvents();
        update();
        render();
    }
}

void GameUI::loadResources() {
    // Загрузка шрифта
    if (!font_.loadFromFile("data/fonts/HSESans-Regular.otf")) {
        std::cerr << "Failed to load font! Make sure data/fonts/HSESans-Regular.otf exists." << std::endl;
    }

    // Загрузка текстур фигур
    // Белые фигуры
    if (!textures_["pawn_white"].loadFromFile("data/images/white_pawn.png")) {
        std::cerr << "Failed to load white pawn texture!" << std::endl;
    }

    if (!textures_["knight_white"].loadFromFile("data/images/white_knight.png")) {
        std::cerr << "Failed to load white knight texture!" << std::endl;
    }

    if (!textures_["bishop_white"].loadFromFile("data/images/white_bishop.png")) {
        std::cerr << "Failed to load white bishop texture!" << std::endl;
    }

    if (!textures_["rook_white"].loadFromFile("data/images/white_rook.png")) {
        std::cerr << "Failed to load white rook texture!" << std::endl;
    }

    if (!textures_["queen_white"].loadFromFile("data/images/white_queen.png")) {
        std::cerr << "Failed to load white queen texture!" << std::endl;
    }

    if (!textures_["king_white"].loadFromFile("data/images/white_king.png")) {
        std::cerr << "Failed to load white king texture!" << std::endl;
    }

    // Черные фигуры
    if (!textures_["pawn_black"].loadFromFile("data/images/black_pawn.png")) {
        std::cerr << "Failed to load black pawn texture!" << std::endl;
    }

    if (!textures_["knight_black"].loadFromFile("data/images/black_knight.png")) {
        std::cerr << "Failed to load black knight texture!" << std::endl;
    }

    if (!textures_["bishop_black"].loadFromFile("data/images/black_bishop.png")) {
        std::cerr << "Failed to load black bishop texture!" << std::endl;
    }

    if (!textures_["rook_black"].loadFromFile("data/images/black_rook.png")) {
        std::cerr << "Failed to load black rook texture!" << std::endl;
    }

    if (!textures_["queen_black"].loadFromFile("data/images/black_queen.png")) {
        std::cerr << "Failed to load black queen texture!" << std::endl;
    }

    if (!textures_["king_black"].loadFromFile("data/images/black_king.png")) {
        std::cerr << "Failed to load black king texture!" << std::endl;
    }
}

void GameUI::setupBoard() {
    // Настройка цветов доски
    light_square_color_ = sf::Color(240, 217, 181); // Светло-бежевый
    dark_square_color_ = sf::Color(181, 136, 99);   // Темно-коричневый
    highlight_color_ = sf::Color(124, 192, 214, 200); // Голубой полупрозрачный
    cooldown_color_ = sf::Color(100, 100, 100, 180); // Серый полупрозрачный
}

void GameUI::handleEvents() {
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        }
        else if (event.type == sf::Event::KeyPressed) {
            handleKeyPress(event.key.code);
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            if (state_ == UIState::GAME_ACTIVE) {
                handleMouseButtonPressed(event.mouseButton.x, event.mouseButton.y);
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            if (state_ == UIState::GAME_ACTIVE && is_dragging_) {
                handleMouseButtonReleased(event.mouseButton.x, event.mouseButton.y);
            }
        }
        else if (event.type == sf::Event::MouseMoved) {
            handleMouseMoved(event.mouseMove.x, event.mouseMove.y);
        }
    }
}

void GameUI::update() {
    // Проверка текущего состояния игры
    if (game_.getState() == GameState::WHITE_WIN || game_.getState() == GameState::BLACK_WIN) {
        state_ = UIState::GAME_OVER;
    }

    // Логика для AI с адаптивным таймером на основе сложности
    static sf::Clock ai_clock;
    if (state_ == UIState::GAME_ACTIVE && against_ai_ && ai_player_ &&
        game_.getState() == GameState::ACTIVE) {

        // ИСПРАВЛЕНО: Динамический таймер на основе сложности AI
        float delay = 0.0f;
        switch (ai_player_->getDifficulty()) {
            case AIDifficulty::EASY:
                delay = 1.5f;  // Самый медленный (1.5 сек)
                break;
            case AIDifficulty::MEDIUM:
                delay = 1.0f;  // Средний (1 сек)
                break;
            case AIDifficulty::HARD:
                delay = 0.7f;  // Быстрее (0.7 сек)
                break;
            case AIDifficulty::EXPERT:
                delay = 0.4f;  // Самый быстрый (0.4 сек)
                break;
            default:
                delay = 1.0f;
                break;
        }

        if (ai_clock.getElapsedTime().asSeconds() >= delay) {
            std::cout << "DEBUG: AI's turn, getting best move..." << std::endl;
            auto move = ai_player_->getBestMove(game_);
            if (move) {
                std::cout << "DEBUG: AI делает ход: piece_id=" << move->piece_id << std::endl;
                game_.makeMove(move->piece_id, move->to);
            } else {
                std::cout << "DEBUG: AI не смог найти подходящий ход" << std::endl;
            }
            ai_clock.restart();
        }
    }
}

void GameUI::render() {
    window_.clear(sf::Color(50, 50, 50)); // Тёмно-серый фон

    // Отрисовка в зависимости от состояния
    if (state_ == UIState::GAME_ACTIVE) {
        handleGameScreen();
    } else if (state_ == UIState::GAME_OVER) {
        handleGameOverScreen();
    }

    window_.display();
}

void GameUI::handleGameScreen() {
    drawBoard();
    drawPieces();

    // Отображение паузы, если игра приостановлена
    if (game_.getState() == GameState::PAUSED) {
        drawPauseScreen();
    }
}

void GameUI::handleGameOverScreen() {
    // Сначала рисуем игровую доску
    drawBoard();
    drawPieces();

    // Затем накладываем экран окончания игры
    sf::RectangleShape overlay(sf::Vector2f(window_.getSize().x, window_.getSize().y));
    overlay.setFillColor(sf::Color(0, 0, 0, 180)); // Черный полупрозрачный
    window_.draw(overlay);

    // Текст победителя
    std::string winner_text;
    if (game_.getState() == GameState::WHITE_WIN) {
        winner_text = "White Wins!";
    } else {
        winner_text = "Black Wins!";
    }

    sf::Text text(winner_text, font_, 40);
    text.setFillColor(sf::Color::White);

    // Центрируем текст
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    text.setPosition(window_.getSize().x/2.0f, window_.getSize().y/2.0f - 30);

    window_.draw(text);

    // Текст с подсказкой
    sf::Text hint("Press 'R' to restart or 'Esc' to quit", font_, 20);
    hint.setFillColor(sf::Color::White);

    // Центрируем подсказку
    textRect = hint.getLocalBounds();
    hint.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    hint.setPosition(window_.getSize().x/2.0f, window_.getSize().y/2.0f + 30);

    window_.draw(hint);
}

void GameUI::drawBoard() {
    // Отрисовка доски
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            sf::RectangleShape square(sf::Vector2f(square_size_, square_size_));
            square.setPosition(board_offset_x_ + col * square_size_, board_offset_y_ + (7 - row) * square_size_);

            // Чередуем цвета клеток
            if ((row + col) % 2 == 0) {
                square.setFillColor(light_square_color_);
            } else {
                square.setFillColor(dark_square_color_);
            }

            window_.draw(square);

            // Если это выбранная клетка, подсвечиваем её
            if (selected_piece_id_.has_value() &&
                selected_piece_position_.row == row &&
                selected_piece_position_.col == col) {

                sf::RectangleShape highlight(sf::Vector2f(square_size_, square_size_));
                highlight.setPosition(board_offset_x_ + col * square_size_, board_offset_y_ + (7 - row) * square_size_);
                highlight.setFillColor(highlight_color_);
                window_.draw(highlight);
            }
        }
    }

    // Отрисовка координат
    for (int i = 0; i < 8; ++i) {
        sf::Text rowText(std::to_string(i + 1), font_, 15);
        rowText.setFillColor(sf::Color::White);
        rowText.setPosition(board_offset_x_ - 20, board_offset_y_ + (7 - i) * square_size_ + square_size_/2 - 8);
        window_.draw(rowText);

        sf::Text colText(static_cast<char>('a' + i), font_, 15);
        colText.setFillColor(sf::Color::White);
        colText.setPosition(board_offset_x_ + i * square_size_ + square_size_/2 - 5, board_offset_y_ + board_size_ + 5);
        window_.draw(colText);
    }
}

void GameUI::drawPieces() {
    // Получаем все фигуры с доски
    auto pieces = game_.getBoard().getAllPieces(false); // Без захваченных фигур

    // Отрисовываем фигуры
    for (const auto& piece : pieces) {
        // Пропускаем перетаскиваемую фигуру
        if (is_dragging_ && selected_piece_id_.has_value() && piece.id == selected_piece_id_.value()) {
            continue;
        }

        drawPieceWithCooldown(piece);
    }

    // Отрисовываем перетаскиваемую фигуру поверх всех остальных
    if (is_dragging_ && selected_piece_id_.has_value()) {
        auto piece_opt = game_.getBoard().getPieceById(selected_piece_id_.value());
        if (piece_opt) {
            std::string key = getPieceKey(piece_opt->type, piece_opt->color);

            if (textures_.find(key) != textures_.end()) {
                sf::Sprite sprite;
                sprite.setTexture(textures_[key]);

                // Масштабируем спрайт
                float scale = static_cast<float>(square_size_) / sprite.getTexture()->getSize().x;
                sprite.setScale(scale, scale);

                // Позиционируем спрайт под курсором с учетом смещения
                sprite.setPosition(
                        mouse_x_ - drag_offset_x_,
                        mouse_y_ - drag_offset_y_
                );

                window_.draw(sprite);
            }
        }
    }
}

void GameUI::drawPieceWithCooldown(const Piece& piece) {
    // Получаем ключ текстуры
    std::string key = getPieceKey(piece.type, piece.color);

    if (textures_.find(key) != textures_.end()) {
        sf::Sprite sprite;
        sprite.setTexture(textures_[key]);

        // Масштабируем спрайт чтобы он поместился в клетку
        float scale = static_cast<float>(square_size_) / sprite.getTexture()->getSize().x;
        sprite.setScale(scale, scale);

        // Позиционируем спрайт на доске
        sprite.setPosition(
                board_offset_x_ + piece.position.col * square_size_,
                board_offset_y_ + (7 - piece.position.row) * square_size_
        );

        // Рисуем фигуру
        window_.draw(sprite);

        // Если фигура на кулдауне, рисуем затемнение
        if (piece.cooldown_ticks_remaining > 0) {
            // ИСПРАВЛЕНО: Более заметное отображение кулдауна
            int cooldown;
            if (piece.color == PlayerColor::WHITE) {
                cooldown = game_.getWhiteCooldown();
            } else {
                cooldown = game_.getBlackCooldown();
            }

            // Процент оставшегося времени кулдауна
            float percent = static_cast<float>(piece.cooldown_ticks_remaining) / cooldown;

            // Создаем полупрозрачный круг для отображения кулдауна
            sf::CircleShape cooldown_shape(square_size_ / 2);

            // Улучшенные цвета для кулдауна
            sf::Color color = sf::Color(50, 50, 200, 180); // Более заметный синий цвет с повышенной непрозрачностью
            cooldown_shape.setFillColor(color);

            // Позиционируем в центре клетки
            cooldown_shape.setOrigin(square_size_ / 2, square_size_ / 2);
            cooldown_shape.setPosition(
                    board_offset_x_ + piece.position.col * square_size_ + square_size_ / 2,
                    board_offset_y_ + (7 - piece.position.row) * square_size_ + square_size_ / 2
            );

            window_.draw(cooldown_shape);

            // Отображаем числовой счетчик кулдауна
            sf::Text cooldown_text(std::to_string(piece.cooldown_ticks_remaining), font_, 20); // Увеличен размер текста
            cooldown_text.setFillColor(sf::Color::White);
            cooldown_text.setOutlineColor(sf::Color::Black);
            cooldown_text.setOutlineThickness(1.0f); // Добавлена обводка для лучшей читаемости

            // Центрируем текст на фигуре
            sf::FloatRect textRect = cooldown_text.getLocalBounds();
            cooldown_text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
            cooldown_text.setPosition(
                    board_offset_x_ + piece.position.col * square_size_ + square_size_ / 2,
                    board_offset_y_ + (7 - piece.position.row) * square_size_ + square_size_ / 2
            );

            window_.draw(cooldown_text);
        }
    }
}

void GameUI::drawCooldowns() {
    // Просто вызываем отрисовку фигур, так как cooldown отображается там же
    drawPieces();
}

void GameUI::drawPauseScreen() {
    // Полупрозрачный фон
    sf::RectangleShape overlay(sf::Vector2f(window_.getSize().x, window_.getSize().y));
    overlay.setFillColor(sf::Color(0, 0, 0, 150)); // Черный полупрозрачный
    window_.draw(overlay);

    // Текст паузы
    sf::Text text("Game Paused", font_, 40);
    text.setFillColor(sf::Color::White);

    // Центрируем текст
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    text.setPosition(window_.getSize().x/2.0f, window_.getSize().y/2.0f - 30);

    window_.draw(text);

    // Текст с подсказкой
    sf::Text hint("Press 'Space' to resume or 'Esc' to quit", font_, 20);
    hint.setFillColor(sf::Color::White);

    // Центрируем подсказку
    textRect = hint.getLocalBounds();
    hint.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    hint.setPosition(window_.getSize().x/2.0f, window_.getSize().y/2.0f + 30);

    window_.draw(hint);
}

Position GameUI::boardPositionFromMouse(sf::Vector2i mouse_pos) {
    int col = (mouse_pos.x - board_offset_x_) / square_size_;
    int row = 7 - (mouse_pos.y - board_offset_y_) / square_size_; // Инвертируем строки

    // Проверка границ доски
    if (col < 0 || col > 7 || row < 0 || row > 7) {
        return {-1, -1}; // Недопустимая позиция
    }

    return {row, col};
}

void GameUI::handleKeyPress(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Escape:
            window_.close();
            break;

        case sf::Keyboard::Space:
            // Пауза/возобновление игры
            if (game_.getState() == GameState::ACTIVE) {
                game_.pause();
            }
            else if (game_.getState() == GameState::PAUSED) {
                game_.resume();
            }
            break;

        case sf::Keyboard::R:
            if (state_ == UIState::GAME_OVER) {
                // Перезапуск игры
                game_.reset();
                game_.start();
                state_ = UIState::GAME_ACTIVE;
            }
            break;

        default:
            break;
    }
}

void GameUI::handleMouseButtonPressed(int x, int y) {
    // Проверяем, что нажатие произошло на доске
    if (x < board_offset_x_ || x >= board_offset_x_ + board_size_ ||
        y < board_offset_y_ || y >= board_offset_y_ + board_size_) {
        return;
    }

    Position board_pos = boardPositionFromMouse(sf::Vector2i(x, y));
    if (board_pos.row == -1) return; // Недопустимая позиция

    // Проверяем, есть ли фигура на этой клетке
    auto piece = game_.getBoard().getPieceAt(board_pos);
    if (piece) {
        // Проверяем, не на кулдауне ли фигура
        if (piece->cooldown_ticks_remaining > 0) {
            std::cout << "DEBUG: Фигура на кулдауне" << std::endl;
            return;
        }

        // Запоминаем выбранную фигуру и начинаем перетаскивание
        selected_piece_id_ = piece->id;
        drag_offset_x_ = x - (board_offset_x_ + board_pos.col * square_size_ + square_size_ / 2);
        drag_offset_y_ = y - (board_offset_y_ + (7 - board_pos.row) * square_size_ + square_size_ / 2);
        is_dragging_ = true;

        // Запоминаем начальную позицию для отрисовки подсветки
        selected_piece_position_ = board_pos;

        std::cout << "DEBUG: Выбрана фигура ID " << piece->id << std::endl;
    }
}

void GameUI::handleMouseButtonReleased(int x, int y) {
    // Если не перетаскиваем фигуру, ничего не делаем
    if (!is_dragging_ || !selected_piece_id_.has_value()) {
        return;
    }

    Position target = boardPositionFromMouse(sf::Vector2i(x, y));

    // Проверяем, что координаты в пределах доски
    if (target.row != -1) {
        // Пытаемся сделать ход
        bool success = game_.makeMove(selected_piece_id_.value(), target);
        std::cout << "DEBUG: Попытка хода " << (success ? "успешна" : "не удалась") << std::endl;
    }

    // Сбрасываем перетаскивание
    is_dragging_ = false;
    selected_piece_id_ = std::nullopt;
}

void GameUI::handleMouseMoved(int x, int y) {
    // Обновляем позицию курсора для перетаскивания
    mouse_x_ = x;
    mouse_y_ = y;
}

std::string GameUI::getPieceKey(PieceType type, PlayerColor color) {
    std::string key;

    // Преобразуем тип фигуры в строку
    switch (type) {
        case PieceType::PAWN:   key = "pawn_"; break;
        case PieceType::KNIGHT: key = "knight_"; break;
        case PieceType::BISHOP: key = "bishop_"; break;
        case PieceType::ROOK:   key = "rook_"; break;
        case PieceType::QUEEN:  key = "queen_"; break;
        case PieceType::KING:   key = "king_"; break;
        default:                key = "unknown_"; break;
    }

    // Добавляем цвет
    key += (color == PlayerColor::WHITE) ? "white" : "black";

    return key;
}