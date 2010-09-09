#ifndef GAMESTATENODE_H_INCLUDED
#define GAMESTATENODE_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/GameState.h"
#include "Tetris/GameQualityEvaluator.h"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <set>


namespace Tetris
{

    class Block;
    class GameState;
    class GameStateNode;
    typedef boost::shared_ptr<GameStateNode> ChildNodePtr;


    struct GameStateComparisonFunctor
    {
        GameStateComparisonFunctor()
        {
            throw std::logic_error("Should never come here!");
        }

        GameStateComparisonFunctor(std::auto_ptr<Evaluator> inChildPtrCompare);
    
        bool operator()(ChildNodePtr lhs, ChildNodePtr rhs);

    private:
        boost::shared_ptr<Evaluator> mEvaluator;
    };


    typedef std::multiset<ChildNodePtr, GameStateComparisonFunctor> ChildNodes;

    class GameStateNode
    {
    public:
        static std::auto_ptr<GameStateNode> CreateRootNode(size_t inNumRows, size_t inNumColumns);

        GameStateNode(GameStateNode * inParent, std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator);

        // Creates a deep copy of this node and all child nodes.
        std::auto_ptr<GameStateNode> clone() const;

        // Each node is produced by a unique combination of the current block's column and rotation.
        int identifier() const;

        const Evaluator & qualityEvaluator() const;

        void generateOffspring(BlockTypes inBlockTypes, size_t inOffset, const Evaluator & inEvaluator);

        // Distance from the root node.
        int depth() const;

        const GameStateNode * parent() const;

        GameStateNode * parent();

        void makeRoot()
        { mParent = 0; }

        const ChildNodes & children() const;

        void clearChildren();

        void addChild(ChildNodePtr inChildNode);

        const GameStateNode * endNode() const;

        inline GameStateNode * endNode()
        { return const_cast<GameStateNode*>(static_cast<const GameStateNode *>(this)->endNode()); }

        const GameState & state() const;

        GameState & state();

    private:
        // Creates a root node
        GameStateNode(std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator);

        GameStateNode * mParent;
        int mIdentifier;
        int mDepth;
        boost::scoped_ptr<GameState> mGameState;

        boost::scoped_ptr<Evaluator> mEvaluator; // } 
        ChildNodes mChildren;                    // } => Order matters!
    };


    //
    // Helper functions
    //

    // Populate the node with all possible child nodes that can be
    // created by combining the game state and the given block.
    void GenerateOffspring(BlockType inBlockType,
                           GameStateNode & ioGameStateNode,
                           const Evaluator & inEvaluator,
                           ChildNodes & outChildNodes);

    // Same as GenerateOffspring, but multiple levels deep.
    void GenerateOffspring(size_t inDepth, const Block & inBlock, GameStateNode & ioGameStateNode, ChildNodes & outChildNodes);

    bool IsGameOver(const GameState & inGameState, BlockType inBlockType, int inRotation);


} // namespace Tetris


#endif // GAMESTATENODE_H_INCLUDED
