#ifndef TETRIS_GAMESTATECOMPARISONFUNCTOR_H_INCLUDED
#define TETRIS_GAMESTATECOMPARISONFUNCTOR_H_INCLUDED


#include "Tetris/NodePtr.h"
#include <memory>
#include <boost/shared_ptr.hpp>


namespace Tetris
{

    class Evaluator;

    class GameStateComparator
    {
    public:
        GameStateComparator();

        GameStateComparator(std::auto_ptr<Evaluator> inChildPtrCompare);

        bool operator()(NodePtr lhs, NodePtr rhs);

    private:
        boost::shared_ptr<Evaluator> mEvaluator;
    };

} // namespace Tetris


#endif // TETRIS_GAMESTATECOMPARISONFUNCTOR_H_INCLUDED
