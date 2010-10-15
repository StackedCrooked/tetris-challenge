#ifndef TETRIS_GAMESTATENODE_H_INCLUDED
#define TETRIS_GAMESTATENODE_H_INCLUDED


#include "Tetris/Tetris.h"
#include <boost/weak_ptr.hpp>
#include <set>


namespace Tetris
{

    class GameStateComparisonFunctor
    {
    public:
        GameStateComparisonFunctor()
        {
            throw std::logic_error("Should never come here!");
        }

        GameStateComparisonFunctor(std::auto_ptr<Evaluator> inChildPtrCompare);

        bool operator()(NodePtr lhs, NodePtr rhs);

    private:
        boost::shared_ptr<Evaluator> mEvaluator;
    };


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

        // Creates a deep copy of this node and all child nodes.
        std::auto_ptr<GameStateNode> clone() const;

        // Each node is produced by a unique combination of the current block's column and rotation.
        int identifier() const;

        const Evaluator & qualityEvaluator() const;

        // Distance from the root node.
        int depth() const;

        const NodePtr parent() const;

        NodePtr parent();

        void makeRoot()
        { mParent.reset(); }

        const ChildNodes & children() const;

        // yeah
        ChildNodes & children() { return mChildren; }

        void clearChildren();

        void addChild(NodePtr inChildNode);

        const GameStateNode * endNode() const;

        GameStateNode * endNode();

        const GameState & state() const;

        GameState & state();

    private:
        // Creates a root node
        GameStateNode(std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator);

        boost::weak_ptr<GameStateNode> mParent;
        int mIdentifier;
        int mDepth;
        boost::scoped_ptr<GameState> mGameState;

        boost::scoped_ptr<Evaluator> mEvaluator; // }
        ChildNodes mChildren;                    // } => Order matters!
    };

} // namespace Tetris


#endif // GAMESTATENODE_H_INCLUDED
