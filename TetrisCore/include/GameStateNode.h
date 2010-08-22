#ifndef GAMESTATENODE_H_INCLUDED
#define GAMESTATENODE_H_INCLUDED


#include "BlockType.h"
#include "GameState.h"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <set>


namespace Tetris
{

    class Block;
    class GameState;
    class GameStateNode;
    typedef boost::shared_ptr<GameStateNode> ChildNodePtr;

    struct ChildPtrCompare
    {
        bool operator()(ChildNodePtr lhs, ChildNodePtr rhs);
    };    

    typedef std::multiset<ChildNodePtr, ChildPtrCompare> ChildNodes;

    class GameStateNode
    {
    public:
        static std::auto_ptr<GameStateNode> CreateRootNode(size_t inNumRows, size_t inNumColumns);

        GameStateNode(GameStateNode * inParent, std::auto_ptr<GameState> inGameState);

        // Creates a deep copy of this node and all child nodes.
        std::auto_ptr<GameStateNode> clone() const;

        // Each node is produced by a unique combination of the current block's column and rotation.
        int identifier() const;

        void generateOffspring(BlockTypes inBlockTypes, size_t inOffset);

        // Distance from the root node.
        int depth() const;

        const GameStateNode * parent() const;

        GameStateNode * parent();

        const ChildNodes & children() const;

        void clearChildren();

        void addChild(ChildNodePtr inChildNode);

        const GameState & state() const;

        GameState & state();

    private:
        // Creates a root node
        GameStateNode(std::auto_ptr<GameState> inGameState);

        GameStateNode * mParent;
        int mIdentifier;
        int mDepth;
        boost::scoped_ptr<GameState> mGameState;
        ChildNodes mChildren;
    };


    //
    // Helper functions
    //

    // Populate the node with all possible child nodes that can be
    // created by combining the game state and the given block.
    void GenerateOffspring(BlockType inBlockType, GameStateNode & ioGameStateNode, ChildNodes & outChildNodes);

    // Same as GenerateOffspring, but multiple levels deep.
    void GenerateOffspring(size_t inDepth, const Block & inBlock, GameStateNode & ioGameStateNode, ChildNodes & outChildNodes);

    bool IsGameOver(const GameState & inGameState, BlockType inBlockType, int inRotation);


} // namespace Tetris


#endif // GAMESTATENODE_H_INCLUDED
