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

    template <class>
    class scoped_ptr;

    template <class>
    class shared_ptr;

    template <class>
    class weak_ptr;

    namespace noncopyable_  // protection from unintended ADL
    {
        class noncopyable;
    }

    typedef noncopyable_::noncopyable noncopyable;

} // namespace boost


#endif // TETRIS_FORWARDDECLARATIONS_H_INCLUDED
