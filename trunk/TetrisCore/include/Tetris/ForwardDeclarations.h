#ifndef TETRIS_FORWARDDECLARATIONS_H_INCLUDED
#define TETRIS_FORWARDDECLARATIONS_H_INCLUDED


#include "Tetris/Enum.h"


namespace Tetris
{

    template<class Variable>
    class Protected;

    template<class Variable>
    class ScopedConstAtom;

    template<class Variable>
    class ScopedAtom;

    template<class T>
    class GenericGrid;    
    
    Tetris_DeclareEnum(BlockType);    
    
    typedef GenericGrid<BlockType> Grid;


    //
    // Forward declarations of the Tetris classes.
    //
    class Block;
    class BlockFactory;
    class Evaluator;
    class Game;
    class GameQualityEvaluator;
    class GameState;
    class GameStateNode;    

} // namespace Tetris


#endif // TETRIS_FORWARDDECLARATIONS_H_INCLUDED
