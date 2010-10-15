#ifndef TETRIS_FORWARDDECLARATIONS_H_INCLUDED
#define TETRIS_FORWARDDECLARATIONS_H_INCLUDED


namespace std
{

    template<class T>
	class allocator;

    template<class T>
    struct char_traits;

    template<class Type, class CharTraits, class Allocator>
	class basic_string;

    typedef basic_string<char, char_traits<char>, allocator<char> >
	string;

    template<class Type, class Allocator>
	class vector;

    template<class Key, class Predicate, class Allocator>
    class multiset;

}


namespace boost
{

    template<class>
    class function;

    template<class>
    class scoped_ptr;

    template <class>
    class shared_ptr;

    template <class>
    class weak_ptr;

} // namespace boost


namespace Tetris
{

    //
    // Classes
    //
    class BlockMover;
    class Evaluator;
    class Game;
    class GameState;
    class GameStateNode;
    class Gravity;
    class MoveCalculator;
    class Worker;
    class WorkerPool;


    //
    // Typedefs
    //
    typedef std::vector<int, std::allocator<int> > Widths;

} // namespace Tetris


#endif // TETRIS_FORWARDDECLARATIONS_H_INCLUDED
