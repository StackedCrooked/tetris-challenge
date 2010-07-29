#ifndef GAMESTATENODE_H
#define GAMESTATENODE_H


#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
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
        GameStateNode(GameState * inGameState);

        GameStateNode(GameStateNode * inParent, GameState * inGameState);

        // Distance from the root node.
        int depth() const;

        const GameStateNode * parent() const;

        GameStateNode * parent();

        const Children & children() const;

        Children & children();

        const GameState & state() const;

        bool isDeadEnd() const;

        void markAsDeadEnd();

    private:
        GameStateNode * mParent;
        int mDepth;
        bool mIsDeadEnd;
        boost::scoped_ptr<GameState> mGameState;
        Children mChildren;
    };

} // namespace Tetris

#endif // GAMESTATENODE_H
