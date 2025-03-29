#pragma once
#include <SFML/Graphics.hpp>
#include "game.h"
#include "ai_player.h"

enum class UIState {
    GAME_ACTIVE,
    GAME_OVER
};

class GameUI {
public:
    GameUI(Game& game, bool against_ai = false, unsigned width = 800, unsigned height = 600);
    void run();

private:
    sf::RenderWindow window_;
    UIState state_;
    Game& game_;
    std::unique_ptr<AIPlayer> ai_player_;
    bool against_ai_;

    // UI элементы
    sf::Font font_;
    std::map<std::string, sf::Texture> textures_;

    // Выбранная фигура
    std::optional<uint32_t> selected_piece_id_;
    Position selected_piece_position_;
    bool is_dragging_;
    int mouse_x_, mouse_y_;
    int drag_offset_x_, drag_offset_y_;

    // Параметры доски
    int board_size_;
    int board_offset_x_;
    int board_offset_y_;
    int square_size_;
    sf::Color light_square_color_;
    sf::Color dark_square_color_;
    sf::Color highlight_color_;
    sf::Color cooldown_color_;

    // Методы UI
    void loadResources();
    void handleEvents();
    void update();
    void render();

    // Обработчики разных состояний
    void handleGameScreen();
    void handleGameOverScreen();

    // Вспомогательные методы для UI
    void drawBoard();
    void drawPieces();
    void drawCooldowns();
    void drawPieceWithCooldown(const Piece& piece);
    void drawPauseScreen();

    // Обработка ввода
    Position boardPositionFromMouse(sf::Vector2i mouse_pos);
    void handleKeyPress(sf::Keyboard::Key key);
    void handleMouseButtonPressed(int x, int y);
    void handleMouseButtonReleased(int x, int y);
    void handleMouseMoved(int x, int y);

    // Вспомогательные методы
    std::string getPieceKey(PieceType type, PlayerColor color);
    void setupBoard();
};