#ifndef BLOCKS_H
#define BLOCKS_H


#include "GenericGrid.h"
#include <map>


namespace Tetris
{

	enum BlockType
	{
		NO_BLOCK = 0,
		FIRST_BLOCK,
		I_BLOCK = FIRST_BLOCK,
		J_BLOCK,
		L_BLOCK,
		O_BLOCK,
		S_BLOCK,
		T_BLOCK,
		Z_BLOCK,
		END_BLOCK
	};
	typedef GenericGrid<BlockType> Grid;

	/**
	 * Helper class that identifies a block by its type and rotation.
	 * Also defines equality and smaller-than operators for a block so
	 * that it can be used as the key in a key-value mapping datastructure.
	 */
	struct BlockIdentifier
	{
		BlockIdentifier(BlockType inType, int inRotation);

		bool operator ==(const BlockIdentifier & rhs);

		BlockType type;
		int rotation;
	};

	bool operator< (const BlockIdentifier & lhs, const BlockIdentifier & rhs);


	/**
	 * Block represents a Tetris block.
	 */
	class Block
	{
	public:

		// Factory method.
		// Returns a const reference to a Block object that was created in an internal lookup table.
		static const Block & Get(BlockType inType, int inRotation);

		static int NumRotations(BlockType inType);
		
		BlockType type() const;

		// Returns the rotation value (in range [0..3])
		int rotation() const;

		const Grid & grid() const;

	private:
		Block(BlockType inType, int inRotation);

		static Grid makeBlock(int inType, int rotation);
		static Grid makeIBlock(int rotation);
		static Grid makeJBlock(int rotation);
		static Grid makeLBlock(int rotation);
		static Grid makeOBlock(int rotation);
		static Grid makeSBlock(int rotation);
		static Grid makeTBlock(int rotation);
		static Grid makeZBlock(int rotation);

		BlockType mType;
		int mRotation;
		Grid mGrid;
		
		typedef std::map<BlockIdentifier, Block> Cache;
		static Cache sCache;
	};

} // namespace Tetris



#endif // BLOCKS_H
