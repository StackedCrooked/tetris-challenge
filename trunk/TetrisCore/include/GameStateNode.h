#ifndef GAMESTATENODE_H_INCLUDED
#define GAMESTATENODE_H_INCLUDED


#include <boost/shared_ptr.hpp>
#include <memory>
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

    typedef std::multiset<ChildNodePtr, ChildPtrCompare> Children;

    class GameStateNode
    {
    public:
        static std::auto_ptr<GameStateNode> CreateRootNode(size_t inNumRows, size_t inNumColumns);

        GameStateNode(GameStateNode * inParent, std::auto_ptr<GameState> inGameState);

        GameStateNode * bestChild(int inDepth);

        // Distance from the root node.
        int depth() const;

        const GameStateNode * parent() const;

        GameStateNode * parent();

        void populate(const Block & inBlock);

        const Children & children() const;

        Children & children();

        const GameState & state() const;

        GameState & state();

    private:
        // Creates a root node
        GameStateNode(std::auto_ptr<GameState> inGameState);

        GameStateNode * mParent;
        int mDepth;
        std::auto_ptr<GameState> mGameState;
        Children mChildren;
    };

} // namespace Tetris

#endif // GAMESTATENODE_H_INCLUDED
