#pragma once
#include <string>
#include "../core/board.h"

class FENParser {
public:
    static bool isValidFEN(const std::string& fen);
    static std::string getDefaultFEN();
    static std::string boardToFEN(const Board& board);
    static bool parseFEN(const std::string& fen, Board& board);

private:
    static bool validateBoardPart(const std::string& board_part);
};
