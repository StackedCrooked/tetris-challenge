#ifndef GAMESTATENODE_H_INCLUDED
#define GAMESTATENODE_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/GameState.h"
#include "Tetris/GameQualityEvaluator.h"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <set>


namespace Tetris
{

    class Block;
    class GameState;
    class GameStateNode;
    typedef boost::shared_ptr<GameStateNode> NodePtr;


    struct GameStateComparisonFunctor
    {
        GameStateComparisonFunctor()
        {
            throw std::logic_error("Should never come here!");
        }

        GameStateComparisonFunctor(std::auto_ptr<Evaluator> inChildPtrCompare);
    
        bool operator()(NodePtr lhs, NodePtr rhs);

    private:
        boost::shared_ptr<Evaluator> mEvaluator;
    };


    typedef std::multiset<NodePtr, GameStateComparisonFunctor> ChildNodes;

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

        inline GameStateNode * endNode()
        { return const_cast<GameStateNode*>(static_cast<const GameStateNode *>(this)->endNode()); }

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

    bool IsGameOver(const GameState & inGameState, BlockType inBlockType, int inRotation);

    void GenerateOffspring(NodePtr ioGameStateNode,
                           BlockType inBlockType,
                           const Evaluator & inEvaluator,
                           ChildNodes & outChildNodes);


} // namespace Tetris


#endif // GAMESTATENODE_H_INCLUDED
