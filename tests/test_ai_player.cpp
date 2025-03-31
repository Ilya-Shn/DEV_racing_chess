#include <gtest/gtest.h>
#include "../ai_player.h"
#include "../game.h"
#include "../fen_parser.h"

class AIPlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        game = std::make_unique<Game>([](GameState){});
        game->applySettings(getDefaultSettings());
        game->start();

        ai = std::make_unique<AIPlayer>(AIDifficulty::MEDIUM, PlayerColor::BLACK);
    }

    GameSettings getDefaultSettings() {
        GameSettings settings;
        settings.white_cooldown_ticks = 10;
        settings.black_cooldown_ticks = 10;
        settings.tick_rate_ms = 100;
        settings.against_ai = true;
        settings.ai_difficulty = AIDifficulty::MEDIUM;
        settings.fen_string = FENParser::getDefaultFEN();
        return settings;
    }

    std::unique_ptr<Game> game;
    std::unique_ptr<AIPlayer> ai;
};

TEST_F(AIPlayerTest, AIGeneratesValidMoves) {
    
    auto move = ai->getBestMove(*game);

    
    ASSERT_TRUE(move.has_value());

    
    auto piece = game->getBoard().getPieceById(move->piece_id);
    ASSERT_TRUE(piece.has_value());

    
    EXPECT_EQ(PlayerColor::BLACK, piece->color);

    
    MoveValidator validator;
    EXPECT_TRUE(validator.isValidMove(game->getBoard(), move->piece_id, move->to));
}

TEST_F(AIPlayerTest, AIDifficultySettings) {
    
    ai->setDifficulty(AIDifficulty::EASY);
    EXPECT_EQ(AIDifficulty::EASY, ai->getDifficulty());

    ai->setDifficulty(AIDifficulty::HARD);
    EXPECT_EQ(AIDifficulty::HARD, ai->getDifficulty());

    ai->setDifficulty(AIDifficulty::EXPERT);
    EXPECT_EQ(AIDifficulty::EXPERT, ai->getDifficulty());
}

TEST_F(AIPlayerTest, AIEvaluatesPositionsCorrectly) {
    
    Board board;
    board.setupFromFEN("rnbqkb1r/ppp2ppp/5n2/3p4/3P4/2N2N2/PPP1PPPP/R1BQKB1R");

    Game custom_game([](GameState){});
    GameSettings settings = getDefaultSettings();
    settings.fen_string = "rnbqkb1r/ppp2ppp/5n2/3p4/3P4/2N2N2/PPP1PPPP/R1BQKB1R";
    custom_game.applySettings(settings);
    custom_game.start();

    
    AIPlayer custom_ai(AIDifficulty::EXPERT, PlayerColor::BLACK);
    auto move = custom_ai.getBestMove(custom_game);

    
    ASSERT_TRUE(move.has_value());

    
    
}