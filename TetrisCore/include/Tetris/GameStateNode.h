#ifndef TETRIS_GAMESTATENODE_H_INCLUDED
#define TETRIS_GAMESTATENODE_H_INCLUDED


#include "Tetris/NodePtr.h"
#include "Tetris/GameStateComparisonFunctor.h"
#include <memory>


namespace Tetris
{

    class Evaluator;
    class GameState;


    // Impl declaration.
    class GameStateNodeImpl;


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

        const GameState & state() const;

        GameState & state();

    private:
        friend class GameStateNodeImpl;

        // Constructor for creating a root node.
        GameStateNode(std::auto_ptr<GameState> inGameState, std::auto_ptr<Evaluator> inEvaluator);

        GameStateNodeImpl * mImpl;
    };

} // namespace Tetris


#endif // GAMESTATENODE_H_INCLUDED
