#ifndef TETRIS_NODEPTR_H_INCLUDED
#define TETRIS_NODEPTR_H_INCLUDED


#include "ForwardDeclarations.h"
#include <boost/shared_ptr.hpp>


namespace Tetris
{

    class GameStateNode;
    typedef boost::shared_ptr<GameStateNode> NodePtr;

    class GameStateComparisonFunctor;
    typedef std::multiset<NodePtr, GameStateComparisonFunctor, std::allocator<NodePtr> > ChildNodes;

}


#endif // TETRIS_NODEPTR_H_INCLUDED
