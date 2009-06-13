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


	const GenericGrid<int> & Block::grid() const
	{
		return mGrid;
	}


	BlockIdentifier::BlockIdentifier(int inType, int inRotation) :
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


	GenericGrid<int> Block::makeBlock(int inType, int rotation)
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
		static GenericGrid<int> failBlock(4, 4, 1);
		return failBlock;
	}


	GenericGrid<int> Block::makeIBlock(int rotation)
	{
		if (rotation%2 == 0)
		{
			GenericGrid<int> block(1, 4, 0);
			block.set(0, 0, 1);
			block.set(0, 1, 1);
			block.set(0, 2, 1);
			block.set(0, 3, 1);
			return block;	
		}
		else
		{
			GenericGrid<int> block(4, 1, 0);
			block.set(0, 0, 1);
			block.set(1, 0, 1);
			block.set(2, 0, 1);
			block.set(3, 0, 1);
			return block;
		}
	}

	GenericGrid<int> Block::makeJBlock(int rotation)
	{
		if (rotation%4 == 0)
		{
			GenericGrid<int> block(3, 2, 0);
			block.set(0, 1, 1);
			block.set(1, 1, 1);
			block.set(2, 0, 1);
			block.set(2, 1, 1);
			return block;
		}
		else if (rotation%4 == 1)
		{
			GenericGrid<int> block(2, 3, 0);
			block.set(0, 0, 1);
			block.set(1, 0, 1);
			block.set(1, 1, 1);
			block.set(1, 2, 1);
			return block;
		}
		else if (rotation%4 == 2)
		{
			GenericGrid<int> block(3, 2, 0);
			block.set(0, 0, 1);
			block.set(0, 1, 1);
			block.set(1, 0, 1);
			block.set(2, 0, 1);
			return block;
		}
		else
		{
			GenericGrid<int> block(2, 3, 0);
			block.set(0, 0, 1);
			block.set(0, 1, 1);
			block.set(0, 2, 1);
			block.set(1, 2, 1);
			return block;
		}
	}

	GenericGrid<int> Block::makeLBlock(int rotation)
	{
		if (rotation%4 == 0)
		{
			GenericGrid<int> block(3, 2, 0);
			block.set(0, 0, 1);
			block.set(1, 0, 1);
			block.set(2, 0, 1);
			block.set(2, 1, 1);
			return block;
		}
		else if (rotation%4 == 1)
		{
			GenericGrid<int> block(2, 3, 0);
			block.set(0, 0, 1);
			block.set(0, 1, 1);
			block.set(0, 2, 1);
			block.set(1, 0, 1);
			return block;
		}
		else if (rotation%4 == 2)
		{
			GenericGrid<int> block(3, 2, 0);
			block.set(0, 0, 1);
			block.set(0, 1, 1);
			block.set(1, 1, 1);
			block.set(2, 1, 1);
			return block;
		}
		else
		{
			GenericGrid<int> block(2, 3, 0);
			block.set(0, 2, 1);
			block.set(1, 0, 1);
			block.set(1, 1, 1);
			block.set(1, 2, 1);
			return block;
		}
	}

	GenericGrid<int> Block::makeOBlock(int rotation)
	{
		GenericGrid<int> block(2, 2, 0);
		block.set(0, 0, 1);
		block.set(0, 1, 1);
		block.set(1, 0, 1);
		block.set(1, 1, 1);
		return block;
	}

	GenericGrid<int> Block::makeSBlock(int rotation)
	{
		if (rotation%2 == 0)
		{
			GenericGrid<int> block(2, 3, 0);
			block.set(0, 1, 1);
			block.set(0, 2, 1);
			block.set(1, 0, 1);
			block.set(1, 1, 1);
			return block;
		}
		else
		{
			GenericGrid<int> block(3, 2, 0);
			block.set(0, 0, 1);
			block.set(1, 0, 1);
			block.set(1, 1, 1);
			block.set(2, 1, 1);
			return block;
		}
	}

	GenericGrid<int> Block::makeTBlock(int rotation)
	{
		if (rotation%4 == 0)
		{
			GenericGrid<int> block(2, 3, 0);
			block.set(0, 1, 1);
			block.set(1, 0, 1);
			block.set(1, 1, 1);
			block.set(1, 2, 1);
			return block;
		}
		else if (rotation%4 == 1)
		{
			GenericGrid<int> block(3, 2, 0);
			block.set(0, 0, 1);
			block.set(1, 0, 1);
			block.set(1, 1, 1);
			block.set(2, 0, 1);
			return block;
		}
		else if (rotation%4 == 2)
		{
			GenericGrid<int> block(2, 3, 0);
			block.set(0, 0, 1);
			block.set(0, 1, 1);
			block.set(0, 2, 1);
			block.set(1, 1, 1);
			return block;
		}
		else
		{
			GenericGrid<int> block(3, 2, 0);
			block.set(0, 1, 1);
			block.set(1, 0, 1);
			block.set(1, 1, 1);
			block.set(2, 1, 1);
			return block;
		}
	}

	GenericGrid<int> Block::makeZBlock(int rotation)
	{
		if (rotation%2 == 0)
		{
			GenericGrid<int> block(2, 3, 0);
			block.set(0, 0, 1);
			block.set(0, 1, 1);
			block.set(1, 1, 1);
			block.set(1, 2, 1);
			return block;
		}
		else
		{
			GenericGrid<int> block(3, 2, 0);
			block.set(0, 1, 1);
			block.set(1, 0, 1);
			block.set(1, 1, 1);
			block.set(2, 0, 1);
			return block;
		}
	}

} // namespace Tetris