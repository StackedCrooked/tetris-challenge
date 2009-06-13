#include "Parser.h"
#include <windows.h> // TODO: REMOVE!!!
#include <assert.h>


bool Parser::parse(const std::string & inFile, std::vector<BlockIdentifier> & outBlockIDs)
{
	char str[200];
	FILE *fp;
	fp = fopen(inFile.c_str(), "r");
	if(!fp)
	{
		return false;
	}
	
	while (fgets(str, sizeof(str), fp) != NULL)
	{
		// strip trailing '\n' if it exists
		int len = strlen(str)-1;
		if(str[len] == '\n')
		{
			str[len] = 0;
		}
		BlockIdentifier bi(makeBlockIdentifierFromString(str));
		outBlockIDs.push_back(bi);
	}
	fclose(fp);

	return true;
}


BlockIdentifier Parser::makeBlockIdentifierFromString(const std::string & inString)
{
	char index, charType;
	int rotation = 0;
	sscanf(inString.c_str(), "%c %c %d", &index, &charType, &rotation);

	Block::Type type = Block::I_BLOCK;
	switch (charType)
	{
		case 'I':
		{
			type = Block::I_BLOCK;
			break;
		}
		case 'J':
		{
			type = Block::J_BLOCK;
			break;
		}
		case 'L':
		{
			type = Block::L_BLOCK;
			break;
		}
		case 'O':
		{
			type = Block::O_BLOCK;
			break;
		}
		case 'S':
		{
			type = Block::S_BLOCK;
			break;
		}
		case 'T':
		{
			type = Block::T_BLOCK;
			break;
		}
		case 'Z':
		{
			type = Block::Z_BLOCK;
			break;
		}
		default:
		{
			assert(!"Parser error");
			break;
		}
	}

	return BlockIdentifier(type, rotation);
}