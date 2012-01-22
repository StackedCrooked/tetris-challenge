#ifndef TETRIS_GAMESTATENODE_H
#define TETRIS_GAMESTATENODE_H


#include "Tetris/GameStateComparator.h"
#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
#include <boost/noncopyable.hpp>
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

    GameStateNode(const NodePtr & inParent, const GameState & inGameState, const Evaluator & inEvaluator);

    GameStateNode(const GameState & inGameState, const Evaluator & inEvaluator);

    GameStateNode(const GameStateNode & rhs);

    ~GameStateNode();

    const Evaluator & evaluator() const;

    // Distance from the root node.
    int depth() const;

    NodePtr parent() const;

    void makeRoot();

    const ChildNodes & children() const;

    ChildNodes & children();

    void clearChildren();

    void addChild(const NodePtr & inChildNode);

    const GameStateNode * endNode() const;

    GameStateNode * endNode();

    const GameState & gameState() const;

    int quality() const;

private:
    // disallow assignment
    GameStateNode& operator=(const GameStateNode &);

    struct Impl;
    boost::shared_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_GAMESTATENODE_H
