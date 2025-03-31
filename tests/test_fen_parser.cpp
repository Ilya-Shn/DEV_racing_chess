#include <gtest/gtest.h>
#include "../utility/fen_parser.h"
#include "../core/board.h"

TEST(FenParserTest, ValidFENStrings) {
    
    EXPECT_TRUE(FENParser::isValidFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"));

    
    EXPECT_TRUE(FENParser::isValidFEN("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR"));
}

TEST(FenParserTest, InvalidFENStrings) {
    
    EXPECT_FALSE(FENParser::isValidFEN("rnbq1bnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"));

    
    EXPECT_FALSE(FENParser::isValidFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/8"));

    
    EXPECT_FALSE(FENParser::isValidFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN?"));
}

TEST(FenParserTest, BoardToFenConversion) {
    Board board;
    board.setupStandardPosition();

    std::string fen = FENParser::boardToFEN(board);
    EXPECT_EQ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", fen);

    
    auto pawn = board.getPieceAt({1, 4}); 
    ASSERT_TRUE(pawn.has_value());
    board.movePiece(pawn->id, {3, 4}); 

    fen = FENParser::boardToFEN(board);
    EXPECT_EQ("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR", fen);
}