#include "Parser.h"
#include <assert.h>


namespace Tetris
{

	bool Parser::parse(const std::string & inFile, std::vector<Block> & outBlockIDs)
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
			Block bi(makeBlockIdentifierFromString(str));
			outBlockIDs.push_back(bi);
		}
		fclose(fp);

		return true;
	}

	
	BlockType Parser::convertCharToBlockType(char charType)
	{
		switch (charType)
		{
			case 'I':
			{
				return I_BLOCK;
			}
			case 'J':
			{
				return J_BLOCK;
			}
			case 'L':
			{
				return L_BLOCK;
			}
			case 'O':
			{
				return O_BLOCK;
			}
			case 'S':
			{
				return S_BLOCK;
			}
			case 'T':
			{
				return T_BLOCK;
			}
			case 'Z':
			{
				return Z_BLOCK;
			}
			default:
			{
				assert(!"Parser error");
			}
		}
		return NO_BLOCK;
	}


	Block Parser::makeBlockIdentifierFromString(const std::string & inString)
	{
		char charId, charType;
		int rotation = 0;
		sscanf(inString.c_str(), "%c %c %d", &charId, &charType, &rotation);
		return Block(charId, convertCharToBlockType(charType), rotation);
	}

} // namespace Tetris