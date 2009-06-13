#include "Block.h"
#include <assert.h>


namespace Tetris
{

	Block::Cache Block::sCache;


	const Block & Block::Get(BlockType inType, int inRotation)
	{
		BlockIdentifier id(inType, inRotation);
		Cache::iterator it = sCache.find(id);
		if (it == sCache.end())
		{
			sCache.insert(std::make_pair(id, Block(inType, inRotation)));
			it = sCache.find(id);
		}
		return it->second;
	}


	int Block::NumRotations(BlockType inType)
	{
		switch (inType)
		{
			case I_BLOCK:
			{
				return 2;
			}
			case J_BLOCK:
			{
				return 4;
			}
			case L_BLOCK:
			{
				return 4;
			}
			case O_BLOCK:
			{
				return 1;
			}
			case S_BLOCK:
			{
				return 2;
			}
			case T_BLOCK:
			{
				return 4;
			}
			case Z_BLOCK:
			{
				return 2;
			}
			default:
			{
				assert(!"No valid block type given.");
				return 1;
			}
		}
	}


	Block::Block(BlockType inType, int inRotation) :
		mType(inType),
		mRotation(inRotation),
		mGrid(makeBlock(inType, inRotation))
	{
	}
		

	BlockType Block::type() const
	{
		return mType;
	}


	int Block::rotation() const
	{
		return mRotation;
	}


	const Grid & Block::grid() const
	{
		return mGrid;
	}


	BlockIdentifier::BlockIdentifier(BlockType inType, int inRotation) :
		type(inType),
		rotation(inRotation)
	{
	}

		
	bool BlockIdentifier::operator ==(const BlockIdentifier & rhs)
	{
		return type == rhs.type && rotation == rhs.rotation;
	}


	bool operator< (const BlockIdentifier & lhs, const BlockIdentifier & rhs)
	{
		if (lhs.type != rhs.type)
		{
			return lhs.type < rhs.type;
		}
		else
		{
			return lhs.rotation < rhs.rotation;
		}
	}


	Grid Block::makeBlock(int inType, int rotation)
	{
		if (inType == I_BLOCK)
		{
			return makeIBlock(rotation);
		}
		else if (inType == J_BLOCK)
		{
			return makeJBlock(rotation);
		}
		else if (inType == L_BLOCK)
		{
			return makeLBlock(rotation);
		}
		else if (inType == O_BLOCK)
		{
			return makeOBlock(rotation);
		}
		else if (inType == S_BLOCK)
		{
			return makeSBlock(rotation);
		}
		else if (inType == T_BLOCK)
		{
			return makeTBlock(rotation);
		}
		else if (inType == Z_BLOCK)
		{
			return makeZBlock(rotation);
		}
		else
		{
			assert(!"No valid block type given!");
		}
		
		// if we get here something is wrong, however we have to return something
		static Grid failBlock(4, 4, NO_BLOCK);
		return failBlock;
	}


	Grid Block::makeIBlock(int rotation)
	{
		if (rotation%2 == 0)
		{
			Grid block(1, 4, NO_BLOCK);
			block.set(0, 0, I_BLOCK);
			block.set(0, 1, I_BLOCK);
			block.set(0, 2, I_BLOCK);
			block.set(0, 3, I_BLOCK);
			return block;	
		}
		else
		{
			Grid block(4, 1, NO_BLOCK);
			block.set(0, 0, I_BLOCK);
			block.set(1, 0, I_BLOCK);
			block.set(2, 0, I_BLOCK);
			block.set(3, 0, I_BLOCK);
			return block;
		}
	}

	Grid Block::makeJBlock(int rotation)
	{
		if (rotation%4 == 0)
		{
			Grid block(3, 2, NO_BLOCK);
			block.set(0, 1, J_BLOCK);
			block.set(1, 1, J_BLOCK);
			block.set(2, 0, J_BLOCK);
			block.set(2, 1, J_BLOCK);
			return block;
		}
		else if (rotation%4 == 1)
		{
			Grid block(2, 3, NO_BLOCK);
			block.set(0, 0, J_BLOCK);
			block.set(1, 0, J_BLOCK);
			block.set(1, 1, J_BLOCK);
			block.set(1, 2, J_BLOCK);
			return block;
		}
		else if (rotation%4 == 2)
		{
			Grid block(3, 2, NO_BLOCK);
			block.set(0, 0, J_BLOCK);
			block.set(0, 1, J_BLOCK);
			block.set(1, 0, J_BLOCK);
			block.set(2, 0, J_BLOCK);
			return block;
		}
		else
		{
			Grid block(2, 3, NO_BLOCK);
			block.set(0, 0, J_BLOCK);
			block.set(0, 1, J_BLOCK);
			block.set(0, 2, J_BLOCK);
			block.set(1, 2, J_BLOCK);
			return block;
		}
	}

	Grid Block::makeLBlock(int rotation)
	{
		if (rotation%4 == 0)
		{
			Grid block(3, 2, NO_BLOCK);
			block.set(0, 0, L_BLOCK);
			block.set(1, 0, L_BLOCK);
			block.set(2, 0, L_BLOCK);
			block.set(2, 1, L_BLOCK);
			return block;
		}
		else if (rotation%4 == 1)
		{
			Grid block(2, 3, NO_BLOCK);
			block.set(0, 0, L_BLOCK);
			block.set(0, 1, L_BLOCK);
			block.set(0, 2, L_BLOCK);
			block.set(1, 0, L_BLOCK);
			return block;
		}
		else if (rotation%4 == 2)
		{
			Grid block(3, 2, NO_BLOCK);
			block.set(0, 0, L_BLOCK);
			block.set(0, 1, L_BLOCK);
			block.set(1, 1, L_BLOCK);
			block.set(2, 1, L_BLOCK);
			return block;
		}
		else
		{
			Grid block(2, 3, NO_BLOCK);
			block.set(0, 2, L_BLOCK);
			block.set(1, 0, L_BLOCK);
			block.set(1, 1, L_BLOCK);
			block.set(1, 2, L_BLOCK);
			return block;
		}
	}

	Grid Block::makeOBlock(int rotation)
	{
		Grid block(2, 2, NO_BLOCK);
		block.set(0, 0, O_BLOCK);
		block.set(0, 1, O_BLOCK);
		block.set(1, 0, O_BLOCK);
		block.set(1, 1, O_BLOCK);
		return block;
	}

	Grid Block::makeSBlock(int rotation)
	{
		if (rotation%2 == 0)
		{
			Grid block(2, 3, NO_BLOCK);
			block.set(0, 1, S_BLOCK);
			block.set(0, 2, S_BLOCK);
			block.set(1, 0, S_BLOCK);
			block.set(1, 1, S_BLOCK);
			return block;
		}
		else
		{
			Grid block(3, 2, NO_BLOCK);
			block.set(0, 0, S_BLOCK);
			block.set(1, 0, S_BLOCK);
			block.set(1, 1, S_BLOCK);
			block.set(2, 1, S_BLOCK);
			return block;
		}
	}

	Grid Block::makeTBlock(int rotation)
	{
		if (rotation%4 == 0)
		{
			Grid block(2, 3, NO_BLOCK);
			block.set(0, 1, T_BLOCK);
			block.set(1, 0, T_BLOCK);
			block.set(1, 1, T_BLOCK);
			block.set(1, 2, T_BLOCK);
			return block;
		}
		else if (rotation%4 == 1)
		{
			Grid block(3, 2, NO_BLOCK);
			block.set(0, 0, T_BLOCK);
			block.set(1, 0, T_BLOCK);
			block.set(1, 1, T_BLOCK);
			block.set(2, 0, T_BLOCK);
			return block;
		}
		else if (rotation%4 == 2)
		{
			Grid block(2, 3, NO_BLOCK);
			block.set(0, 0, T_BLOCK);
			block.set(0, 1, T_BLOCK);
			block.set(0, 2, T_BLOCK);
			block.set(1, 1, T_BLOCK);
			return block;
		}
		else
		{
			Grid block(3, 2, NO_BLOCK);
			block.set(0, 1, T_BLOCK);
			block.set(1, 0, T_BLOCK);
			block.set(1, 1, T_BLOCK);
			block.set(2, 1, T_BLOCK);
			return block;
		}
	}

	Grid Block::makeZBlock(int rotation)
	{
		if (rotation%2 == 0)
		{
			Grid block(2, 3, NO_BLOCK);
			block.set(0, 0, Z_BLOCK);
			block.set(0, 1, Z_BLOCK);
			block.set(1, 1, Z_BLOCK);
			block.set(1, 2, Z_BLOCK);
			return block;
		}
		else
		{
			Grid block(3, 2, NO_BLOCK);
			block.set(0, 1, Z_BLOCK);
			block.set(1, 0, Z_BLOCK);
			block.set(1, 1, Z_BLOCK);
			block.set(2, 0, Z_BLOCK);
			return block;
		}
	}

} // namespace Tetris