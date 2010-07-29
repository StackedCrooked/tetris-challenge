#ifndef PARSER_H
#define PARSER_H


#include "Block.h"
#include <string>
#include <vector>


namespace Tetris
{

    class Parser
    {
    public:
        bool parse(const std::string & inFile, std::vector<Block> & outBlockIDs);

        static BlockType convertCharToBlockType(char charType);

        Block makeBlockIdentifierFromString(const std::string & inString);
    };

} // namespace Tetris


#endif // PARSER_H