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
    typedef boost::shared_ptr<GameStateNode> ChildPtr;

    struct ChildPtrCompare
    {
        bool operator()(ChildPtr lhs, ChildPtr rhs);
    };    

    typedef std::set<ChildPtr, ChildPtrCompare> Children;

    class GameStateNode
    {
    public:
        // Creates a root node
        GameStateNode(std::auto_ptr<GameState> inGameState);

        GameStateNode(GameStateNode * inParent, std::auto_ptr<GameState> inGameState);

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
        GameStateNode * mParent;
        int mDepth;
        std::auto_ptr<GameState> mGameState;
        Children mChildren;
    };

} // namespace Tetris

#endif // GAMESTATENODE_H_INCLUDED
