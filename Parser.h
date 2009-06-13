#ifndef PARSER_H
#define PARSER_H


#include "Block.h"
#include <string>
#include <vector>


class Parser
{
public:
	bool parse(const std::string & inFile, std::vector<BlockIdentifier> & outBlockIDs);

	BlockIdentifier makeBlockIdentifierFromString(const std::string & inString);
};


#endif // PARSER_H