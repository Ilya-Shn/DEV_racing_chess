#include <gtest/gtest.h>
#include "../board.h"
#include "../fen_parser.h"

class BoardTest : public ::testing::Test {
protected:
    void SetUp() override {
        board.setupStandardPosition();
    }

    Board board;
};

TEST_F(BoardTest, InitialSetupCorrect) {
    
    auto pieces = board.getAllPieces();
    ASSERT_EQ(32, pieces.size());

    
    EXPECT_EQ(1, board.countKings(PlayerColor::WHITE));
    EXPECT_EQ(1, board.countKings(PlayerColor::BLACK));

    auto white_king = board.getPieceAt({0, 4});
    ASSERT_TRUE(white_king.has_value());
    EXPECT_EQ(PieceType::KING, white_king->type);
    EXPECT_EQ(PlayerColor::WHITE, white_king->color);

    auto black_king = board.getPieceAt({7, 4});
    ASSERT_TRUE(black_king.has_value());
    EXPECT_EQ(PieceType::KING, black_king->type);
    EXPECT_EQ(PlayerColor::BLACK, black_king->color);
}

TEST_F(BoardTest, MovePieceWorks) {
    
    auto pawn = board.getPieceAt({1, 0});
    ASSERT_TRUE(pawn.has_value());
    uint32_t pawn_id = pawn->id;

    bool move_result = board.movePiece(pawn_id, {3, 0});
    EXPECT_TRUE(move_result);

    
    auto moved_pawn = board.getPieceById(pawn_id);
    ASSERT_TRUE(moved_pawn.has_value());
    EXPECT_EQ(3, moved_pawn->position.row);
    EXPECT_EQ(0, moved_pawn->position.col);

    
    EXPECT_FALSE(board.getPieceAt({1, 0}).has_value());
}

TEST_F(BoardTest, CaptureWorks) {
    
    auto white_pawn = board.getPieceAt({1, 0});
    ASSERT_TRUE(white_pawn.has_value());
    board.movePiece(white_pawn->id, {5, 0}); 

    auto black_pawn = board.getPieceAt({6, 1});
    ASSERT_TRUE(black_pawn.has_value());

    
    board.movePiece(black_pawn->id, {5, 0});

    
    auto captured_piece = board.getPieceById(white_pawn->id);
    ASSERT_TRUE(captured_piece.has_value());
    EXPECT_TRUE(captured_piece->captured);

    
    auto piece_at_pos = board.getPieceAt({5, 0});
    ASSERT_TRUE(piece_at_pos.has_value());
    EXPECT_EQ(black_pawn->id, piece_at_pos->id);
}

TEST_F(BoardTest, FENParsingAndGenerationWorks) {
    
    board.setupFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    
    FENParser parser;
    std::string generated_fen = parser.boardToFEN(board);
    EXPECT_EQ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", generated_fen);
}