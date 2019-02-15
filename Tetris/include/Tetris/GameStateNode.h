#ifndef TETRIS_GAMESTATENODE_H_INCLUDED
#define TETRIS_GAMESTATENODE_H_INCLUDED


#include "Tetris/GameStateComparator.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
#include <boost/scoped_ptr.hpp>
#include <memory>


namespace Tetris {


class Evaluator;


/**
 * GameStateNode is a tree of gamestates. It is used as the search tree for the AI.
 * Each node object contains one GameState object and a collection of child nodes.
 */
class GameStateNode
{
public:
    static std::unique_ptr<GameStateNode> CreateRootNode(std::size_t inNumRows, std::size_t inNumColumns);

    GameStateNode(NodePtr inParent, GameState * inGameState, const Evaluator & inEvaluator);

    GameStateNode(GameState * inGameState, const Evaluator & inEvaluator);

    ~GameStateNode();

    // Creates a deep copy of this node and all child nodes.
    std::unique_ptr<GameStateNode> clone() const;

    // Each node is produced by a unique combination of the current block's column and rotation.
    int identifier() const;

    const Evaluator & evaluator() const;

    // Distance from the root node.
    int depth() const;

    const NodePtr parent() const;

    NodePtr parent();

    void makeRoot();

    const ChildNodes & children() const;

    // yeah
    ChildNodes & children();

    void clearChildren();

    void addChild(NodePtr inChildNode);

    const GameStateNode * endNode() const;

    GameStateNode * endNode();

    const GameState & gameState() const;

    int quality() const;

private:
    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // GAMESTATENODE_H_INCLUDED
