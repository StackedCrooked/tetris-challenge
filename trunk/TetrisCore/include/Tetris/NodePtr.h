#ifndef TETRIS_NODEPTR_H_INCLUDED
#define TETRIS_NODEPTR_H_INCLUDED


#include <boost/shared_ptr.hpp>
#include <set>


namespace Tetris
{

    class GameStateNode;
    typedef boost::shared_ptr<GameStateNode> NodePtr;

    class GameStateComparisonFunctor;
    typedef std::multiset<NodePtr, GameStateComparisonFunctor> ChildNodes;

}


#endif // TETRIS_NODEPTR_H_INCLUDED
