#ifndef TETRIS_GAMESTATENODE_H_INCLUDED
#define TETRIS_GAMESTATENODE_H_INCLUDED


#include "Tetris/Grid.h"
#include "Tetris/NodePtr.h"
#include <memory>


namespace Tetris {


class Evaluator;
class GameState;


/**
 * GameStateNode is a tree of game states.
 * Each node object contains one GameState object and a collection of child nodes.
 */
class GameStateNode
{
public:
    static std::auto_ptr<GameStateNode> CreateRootNode(size_t inNumRows, size_t inNumColumns);

    GameStateNode(NodePtr inParent, std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator);

    ~GameStateNode();

    // Creates a deep copy of this node and all child nodes.
    std::auto_ptr<GameStateNode> clone() const;

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

    void setGrid(const Grid & inGrid);

    int quality() const;

private:
    friend class GameStateNodeImpl;

    // Constructor for creating a root node.
    GameStateNode(std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator);

    class GameStateNodeImpl;
    GameStateNodeImpl * mImpl;
};


} // namespace Tetris


#endif // GAMESTATENODE_H_INCLUDED
