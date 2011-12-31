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

    GameStateNode(NodePtr inParent, const GameState & inGameState, const Evaluator & inEvaluator);

    GameStateNode(const GameState & inGameState, const Evaluator & inEvaluator);

    GameStateNode(const GameStateNode & rhs);

    ~GameStateNode();

    const Evaluator & evaluator() const;

    // Distance from the root node.
    int depth() const;

    unsigned id() const;

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
    // disallow assignment
    GameStateNode& operator=(const GameStateNode &);

    struct Impl;
    boost::shared_ptr<Impl> mImpl;
};

class COWGameStateNode
{
    COWGameStateNode(const GameState & inGameState, const Evaluator & inEvaluator) :
        mGameStateNode(new GameStateNode(inGameState, inEvaluator))
    {
    }

    COWGameStateNode(const GameStateNode & rhs) :
        mGameStateNode(new GameStateNode(rhs))
    {
    }

    inline const Evaluator & evaluator() const
    {
        return mGameStateNode->evaluator();
    }

    // Distance from the root node.
    inline int depth() const
    {
        return mGameStateNode->depth();
    }

    inline unsigned id() const { return mGameStateNode->id(); }

    inline const NodePtr parent() const { return mGameStateNode->parent(); }

    inline const ChildNodes & children() const { return mGameStateNode->children(); }

    const GameStateNode * endNode() const { return mGameStateNode->endNode(); }

    const GameState & gameState() const { return mGameStateNode->gameState(); }

    int quality() const { return mGameStateNode->quality(); }

    inline void makeRoot()
    {
        makeUnique();
        mGameStateNode->makeRoot();
    }

    void clearChildren()
    {
        makeUnique();
        mGameStateNode->clearChildren();
    }

    void addChild(NodePtr inChildNode)
    {
        makeUnique();
        mGameStateNode->addChild(inChildNode);
    }

private:
    inline void makeUnique()
    {
        mGameStateNode.reset(new GameStateNode(mGameStateNode));
    }

    boost::shared_ptr<GameStateNode> mGameStateNode;
};


} // namespace Tetris


#endif // TETRIS_GAMESTATENODE_H
