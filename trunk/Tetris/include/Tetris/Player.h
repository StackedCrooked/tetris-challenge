#ifndef TETRIS_PLAYER_H_INCLUDED
#define TETRIS_PLAYER_H_INCLUDED


#include "Tetris/PlayerType.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/TypedWrapper.h"
#include <string>
#include <vector>


namespace Tetris {


Tetris_TypedWrapper(TeamName, std::string);
Tetris_TypedWrapper(PlayerName, std::string);


/**
 * Array is a wrapper for std::vector that always takes ownership of the entries.
 */
template<class T>
class Array
{
public:
    typedef std::vector<T*> Data;
    Data mData;

    Array() { }

    ~Array()
    {
        while (!mData.empty())
        {
            delete mData.back();
            mData.pop_back();
        }
    }

    typedef typename Data::iterator iterator;
    typedef typename Data::const_iterator const_iterator;

    iterator begin()
    {
        return mData.begin();
    }

    iterator end()
    {
        return mData.end();
    }

    const_iterator begin() const
    {
        return mData.begin();
    }

    const_iterator end() const
    {
        return mData.end();
    }

    T * operator [ ] (typename Data::size_type inIndex)
    {
        return mData[inIndex];
    }

    const T * operator[](typename Data::size_type inIndex) const
    {
        return mData[inIndex];
    }

    void push_back(T * inValue)
    {
        mData.push_back(inValue);
    }

    typename Data::size_type size() const
    {
        return mData.size();
    }

    const T * back() const
    {
        return mData.back();
    }

    T * back()
    {
        return mData.back();
    }

    void erase(iterator it)
    {
        mData.erase(it);
    }
};


class Player
{
public:
    Player(PlayerType inPlayerType,
           const TeamName & inTeamName,
           const PlayerName & inPlayerName,
           size_t inRowCount = 20,
           size_t inColumnCount = 10);

    ~Player();

    Player(const Player & rhs);

    Player & operator=(const Player & rhs);

    const std::string & teamName() const;

    const std::string & playerName() const;

    const SimpleGame * simpleGame() const;

    SimpleGame * simpleGame();

    void resetGame();

private:
    friend bool operator==(const Player & lhs, const Player & rhs);
    friend bool operator<(const Player & lhs, const Player & rhs);

    struct Impl;
    Impl * mImpl; // ref-counted
};

typedef Array<Player> Players;

bool operator==(const Player & lhs, const Player & rhs);

bool operator<(const Player & lhs, const Player & rhs);


} // namespace Tetris


#endif // TETRIS_PLAYER_H_INCLUDED
