#include "GameStateNode.h"

namespace Tetris
{

	bool ChildPtrCompare::operator () (ChildPtr lhs, ChildPtr rhs)
	{
		if (lhs && rhs)
		{
			return lhs->state < rhs->state;
		}
		else
		{
			return lhs.get() < rhs.get();
		}
	}

} // namespace Tetris