#ifndef TETRIS_NODEPTR_H_INCLUDED
#define TETRIS_NODEPTR_H_INCLUDED


#include <boost/shared_ptr.hpp>
#include <set>


namespace Tetris {


class GameState;
class GameStateNode;
class GameStateComparator;


typedef boost::shared_ptr<GameStateNode> NodePtr;
typedef std::multiset<NodePtr, GameStateComparator> ChildNodes;




} // namespace Tetris


#endif // TETRIS_NODEPTR_H_INCLUDED

