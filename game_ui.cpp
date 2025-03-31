#include "game_ui.h"
#include "fen_parser.h"
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>


GameUI::GameUI(Game &game, GameSettings settings, unsigned width, unsigned height)
        : window_(sf::VideoMode(width, height), "Speed Chess"),
          state_(UIState::GAME_ACTIVE),
          game_(game),
          against_ai_(settings.against_ai),
          selected_piece_id_(std::nullopt),
          is_dragging_(false),
          board_size_(512),
          board_offset_x_((width - 512) / 2),
          board_offset_y_((height - 512) / 2),
          square_size_(512 / 8) {

    if (settings.against_ai) {
        if (settings.ai_difficulty.has_value()) {
            ai_player_ = std::make_unique<AIPlayer>(settings.ai_difficulty.value(), PlayerColor::BLACK);
        } else {
            ai_player_ = std::make_unique<AIPlayer>(AIDifficulty::MEDIUM, PlayerColor::BLACK);
        }
    }

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

    if (!font_.loadFromFile("data/fonts/HSESans-Regular.otf")) {
        std::cerr << "Failed to load font! Make sure data/fonts/HSESans-Regular.otf exists." << std::endl;
    }



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

    light_square_color_ = sf::Color(240, 217, 181);
    dark_square_color_ = sf::Color(181, 136, 99);
    highlight_color_ = sf::Color(124, 192, 214, 200);
    cooldown_color_ = sf::Color(100, 100, 100, 180);
}

void GameUI::handleEvents() {
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        } else if (event.type == sf::Event::KeyPressed) {
            handleKeyPress(event.key.code);
        } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            if (state_ == UIState::GAME_ACTIVE) {
                handleMouseButtonPressed(event.mouseButton.x, event.mouseButton.y);
            }
        } else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            if (state_ == UIState::GAME_ACTIVE && is_dragging_) {
                handleMouseButtonReleased(event.mouseButton.x, event.mouseButton.y);
            }
        } else if (event.type == sf::Event::MouseMoved) {
            handleMouseMoved(event.mouseMove.x, event.mouseMove.y);
        }
    }
}

void GameUI::update() {

    if (game_.getState() == GameState::WHITE_WIN || game_.getState() == GameState::BLACK_WIN) {
        state_ = UIState::GAME_OVER;
    }


    static sf::Clock ai_clock;
    if (state_ == UIState::GAME_ACTIVE && against_ai_ && ai_player_ &&
        game_.getState() == GameState::ACTIVE) {


        static float delay = 4.0f;
        switch (ai_player_->getDifficulty()) {
            case AIDifficulty::EASY:
                delay = 7.0f;
                break;
            case AIDifficulty::MEDIUM:
                delay = 4.0f;
                break;
            case AIDifficulty::HARD:
                delay = 2.5f;
                break;
            case AIDifficulty::EXPERT:
                delay = 1.0f;
                break;
            default:
                delay = 4.0f;
                break;
        }

        if (ai_clock.getElapsedTime().asSeconds() >= delay) {
            auto move = ai_player_->getBestMove(game_);
            if (move) {
                game_.makeMove(move->piece_id, move->to);
            }
            ai_clock.restart();
        }
    }
}

void GameUI::render() {
    window_.clear(sf::Color(50, 50, 50));


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


    if (game_.getState() == GameState::PAUSED) {
        drawPauseScreen();
    }
}

void GameUI::handleGameOverScreen() {

    drawBoard();
    drawPieces();


    sf::RectangleShape overlay(sf::Vector2f(window_.getSize().x, window_.getSize().y));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window_.draw(overlay);


    std::string winner_text;
    if (game_.getState() == GameState::WHITE_WIN) {
        winner_text = "White Wins!";
    } else {
        winner_text = "Black Wins!";
    }

    sf::Text text(winner_text, font_, 40);
    text.setFillColor(sf::Color::White);


    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    text.setPosition(window_.getSize().x / 2.0f, window_.getSize().y / 2.0f - 30);

    window_.draw(text);


    sf::Text hint("Press 'R' to restart or 'Esc' to quit", font_, 20);
    hint.setFillColor(sf::Color::White);


    textRect = hint.getLocalBounds();
    hint.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    hint.setPosition(window_.getSize().x / 2.0f, window_.getSize().y / 2.0f + 30);

    window_.draw(hint);
}

void GameUI::drawBoard() {

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            sf::RectangleShape square(sf::Vector2f(square_size_, square_size_));
            square.setPosition(board_offset_x_ + col * square_size_, board_offset_y_ + (7 - row) * square_size_);


            if ((row + col) % 2 == 0) {
                square.setFillColor(light_square_color_);
            } else {
                square.setFillColor(dark_square_color_);
            }

            window_.draw(square);


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


    for (int i = 0; i < 8; ++i) {
        sf::Text rowText(std::to_string(i + 1), font_, 15);
        rowText.setFillColor(sf::Color::White);
        rowText.setPosition(board_offset_x_ - 20, board_offset_y_ + (7 - i) * square_size_ + square_size_ / 2 - 8);
        window_.draw(rowText);

        sf::Text colText(static_cast<char>('a' + i), font_, 15);
        colText.setFillColor(sf::Color::White);
        colText.setPosition(board_offset_x_ + i * square_size_ + square_size_ / 2 - 5,
                            board_offset_y_ + board_size_ + 5);
        window_.draw(colText);
    }
}

void GameUI::drawPieces() {

    auto pieces = game_.getBoard().getAllPieces(false);


    for (const auto &piece: pieces) {

        if (is_dragging_ && selected_piece_id_.has_value() && piece.id == selected_piece_id_.value()) {
            continue;
        }

        drawPieceWithCooldown(piece);
    }


    if (is_dragging_ && selected_piece_id_.has_value()) {
        auto piece_opt = game_.getBoard().getPieceById(selected_piece_id_.value());
        if (piece_opt) {
            std::string key = getPieceKey(piece_opt->type, piece_opt->color);

            if (textures_.find(key) != textures_.end()) {
                sf::Sprite sprite;
                sprite.setTexture(textures_[key]);


                float scale = static_cast<float>(square_size_) / sprite.getTexture()->getSize().x;
                sprite.setScale(scale, scale);


                sprite.setPosition(
                        mouse_x_ - drag_offset_x_,
                        mouse_y_ - drag_offset_y_
                );

                window_.draw(sprite);
            }
        }
    }
}

void GameUI::drawPieceWithCooldown(const Piece &piece) {

    std::string key = getPieceKey(piece.type, piece.color);

    if (textures_.find(key) != textures_.end()) {
        sf::Sprite sprite;
        sprite.setTexture(textures_[key]);


        float scale = static_cast<float>(square_size_) / sprite.getTexture()->getSize().x;
        sprite.setScale(scale, scale);


        sprite.setPosition(
                board_offset_x_ + piece.position.col * square_size_,
                board_offset_y_ + (7 - piece.position.row) * square_size_
        );


        window_.draw(sprite);


        if (piece.cooldown_ticks_remaining > 0) {

            int cooldown;
            if (piece.color == PlayerColor::WHITE) {
                cooldown = game_.getWhiteCooldown();
            } else {
                cooldown = game_.getBlackCooldown();
            }


            float percent = static_cast<float>(piece.cooldown_ticks_remaining) / cooldown;


            sf::CircleShape cooldown_shape(square_size_ / 2);


            sf::Color color = sf::Color(50, 50, 200, 180);
            cooldown_shape.setFillColor(color);


            cooldown_shape.setOrigin(square_size_ / 2, square_size_ / 2);
            cooldown_shape.setPosition(
                    board_offset_x_ + piece.position.col * square_size_ + square_size_ / 2,
                    board_offset_y_ + (7 - piece.position.row) * square_size_ + square_size_ / 2
            );

            window_.draw(cooldown_shape);


            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << static_cast<float>(piece.cooldown_ticks_remaining) / 10.0;
            sf::Text cooldown_text(ss.str(), font_, 20);
            cooldown_text.setFillColor(sf::Color::White);
            cooldown_text.setOutlineColor(sf::Color::Black);
            cooldown_text.setOutlineThickness(1.0f);


            sf::FloatRect textRect = cooldown_text.getLocalBounds();
            cooldown_text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            cooldown_text.setPosition(
                    board_offset_x_ + piece.position.col * square_size_ + square_size_ / 2,
                    board_offset_y_ + (7 - piece.position.row) * square_size_ + square_size_ / 2
            );

            window_.draw(cooldown_text);
        }
    }
}

void GameUI::drawCooldowns() {

    drawPieces();
}

void GameUI::drawPauseScreen() {

    sf::RectangleShape overlay(sf::Vector2f(window_.getSize().x, window_.getSize().y));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window_.draw(overlay);


    sf::Text text("Game Paused", font_, 40);
    text.setFillColor(sf::Color::White);


    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    text.setPosition(window_.getSize().x / 2.0f, window_.getSize().y / 2.0f - 30);

    window_.draw(text);


    sf::Text hint("Press 'Space' to resume or 'Esc' to quit", font_, 20);
    hint.setFillColor(sf::Color::White);


    textRect = hint.getLocalBounds();
    hint.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    hint.setPosition(window_.getSize().x / 2.0f, window_.getSize().y / 2.0f + 30);

    window_.draw(hint);
}

Position GameUI::boardPositionFromMouse(sf::Vector2i mouse_pos) {
    int col = (mouse_pos.x - board_offset_x_) / square_size_;
    int row = 7 - (mouse_pos.y - board_offset_y_) / square_size_;


    if (col < 0 || col > 7 || row < 0 || row > 7) {
        return {-1, -1};
    }

    return {row, col};
}

void GameUI::handleKeyPress(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Escape:
            window_.close();
            break;

        case sf::Keyboard::Space:

            if (game_.getState() == GameState::ACTIVE) {
                game_.pause();
            } else if (game_.getState() == GameState::PAUSED) {
                game_.resume();
            }
            break;

        case sf::Keyboard::R:
            if (state_ == UIState::GAME_OVER) {

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

    if (x < board_offset_x_ || x >= board_offset_x_ + board_size_ ||
        y < board_offset_y_ || y >= board_offset_y_ + board_size_) {
        return;
    }

    Position board_pos = boardPositionFromMouse(sf::Vector2i(x, y));
    if (board_pos.row == -1) return;


    auto piece = game_.getBoard().getPieceAt(board_pos);
    if (piece) {

        if (against_ai_ && piece->color == PlayerColor::BLACK) {
            return;
        }

        if (piece->cooldown_ticks_remaining > 0) {
            return;
        }


        selected_piece_id_ = piece->id;
        drag_offset_x_ = x - (board_offset_x_ + board_pos.col * square_size_ + square_size_ / 2);
        drag_offset_y_ = y - (board_offset_y_ + (7 - board_pos.row) * square_size_ + square_size_ / 2);
        is_dragging_ = true;


        selected_piece_position_ = board_pos;
    }
}

void GameUI::handleMouseButtonReleased(int x, int y) {

    if (!is_dragging_ || !selected_piece_id_.has_value()) {
        return;
    }

    Position target = boardPositionFromMouse(sf::Vector2i(x, y));


    if (target.row != -1) {

        bool success = game_.makeMove(selected_piece_id_.value(), target);
    }


    is_dragging_ = false;
    selected_piece_id_ = std::nullopt;
}

void GameUI::handleMouseMoved(int x, int y) {

    mouse_x_ = x;
    mouse_y_ = y;
}

std::string GameUI::getPieceKey(PieceType type, PlayerColor color) {
    std::string key;


    switch (type) {
        case PieceType::PAWN:
            key = "pawn_";
            break;
        case PieceType::KNIGHT:
            key = "knight_";
            break;
        case PieceType::BISHOP:
            key = "bishop_";
            break;
        case PieceType::ROOK:
            key = "rook_";
            break;
        case PieceType::QUEEN:
            key = "queen_";
            break;
        case PieceType::KING:
            key = "king_";
            break;
        default:
            key = "unknown_";
            break;
    }


    key += (color == PlayerColor::WHITE) ? "white" : "black";

    return key;
}
