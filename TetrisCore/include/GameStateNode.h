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
    

    // We use a multiset because quality which is not always a unique value.
    typedef std::set<ChildPtr, ChildPtrCompare> Children;


    class GameStateNode
    {
    public:
        // Creates a root node
        GameStateNode();

        GameStateNode(GameStateNode * inParent, GameState * inGameState);

        const GameStateNode * parent() const;

        GameStateNode * parent();

        const Children & children() const;

        Children & children();

        const GameState & state() const;

        bool isDeadEnd() const;

        void markAsDeadEnd();

    private:
        int mId;
        GameStateNode * mParent;
        bool mIsDeadEnd;
        boost::scoped_ptr<GameState> mGameState;
        Children mChildren;
    };

} // namespace Tetris

#endif // GAMESTATENODE_H
