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

	/**
	 * Helper class that identifies a block by its type and rotation.
	 * Also defines equality and smaller-than operators for a block so
	 * that it can be used as the key in a key-value mapping datastructure.
	 */
	struct BlockIdentifier
	{
		BlockIdentifier(int inType, int inRotation);

		bool operator ==(const BlockIdentifier & rhs);

		int type;
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

		const GenericGrid<int> & grid() const;

	private:
		Block(BlockType inType, int inRotation);

		static GenericGrid<int> makeBlock(int inType, int rotation);
		static GenericGrid<int> makeIBlock(int rotation);
		static GenericGrid<int> makeJBlock(int rotation);
		static GenericGrid<int> makeLBlock(int rotation);
		static GenericGrid<int> makeOBlock(int rotation);
		static GenericGrid<int> makeSBlock(int rotation);
		static GenericGrid<int> makeTBlock(int rotation);
		static GenericGrid<int> makeZBlock(int rotation);

		BlockType mType;
		int mRotation;
		GenericGrid<int> mGrid;
		
		typedef std::map<BlockIdentifier, Block> Cache;
		static Cache sCache;
	};

} // namespace Tetris



#endif // BLOCKS_H
