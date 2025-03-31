#include <gtest/gtest.h>
#include "../move_validator.h"
#include "../board.h"

class MoveValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        board.setupStandardPosition();
    }

    Board board;
    MoveValidator validator;
};

TEST_F(MoveValidatorTest, ValidPawnMoves) {
    
    auto pawn = board.getPieceAt({1, 0});
    ASSERT_TRUE(pawn.has_value());

    
    EXPECT_TRUE(validator.isValidMove(board, pawn->id, {2, 0}));

    
    EXPECT_TRUE(validator.isValidMove(board, pawn->id, {3, 0}));

    
    EXPECT_FALSE(validator.isValidMove(board, pawn->id, {0, 0}));

    
    EXPECT_FALSE(validator.isValidMove(board, pawn->id, {2, 1}));
}

TEST_F(MoveValidatorTest, ValidKnightMoves) {
    
    auto knight = board.getPieceAt({0, 1});
    ASSERT_TRUE(knight.has_value());

    
    EXPECT_TRUE(validator.isValidMove(board, knight->id, {2, 0}));
    EXPECT_TRUE(validator.isValidMove(board, knight->id, {2, 2}));

    
    EXPECT_FALSE(validator.isValidMove(board, knight->id, {1, 1}));
}

TEST_F(MoveValidatorTest, PawnCapture) {
    auto white_pawn = board.getPieceAt({1, 1});
    auto black_pawn = board.getPieceAt({6, 0});
    ASSERT_TRUE(white_pawn.has_value());
    ASSERT_TRUE(black_pawn.has_value());

    board.movePiece(white_pawn->id, {4, 1});

    board.movePiece(black_pawn->id, {5, 0});

    EXPECT_TRUE(validator.isValidMove(board, white_pawn->id, {5, 0}));
}

TEST_F(MoveValidatorTest, BlockedMove) {
    auto pawn = board.getPieceAt({1, 1});
    auto bishop = board.getPieceAt({0, 2});
    ASSERT_TRUE(pawn.has_value());
    ASSERT_TRUE(bishop.has_value());

    EXPECT_FALSE(validator.isValidMove(board, bishop->id, {2, 0}));

    board.movePiece(pawn->id, {3, 1});
    EXPECT_TRUE(validator.isValidMove(board, bishop->id, {2, 0}));
}