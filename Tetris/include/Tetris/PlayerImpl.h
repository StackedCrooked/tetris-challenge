#ifndef TETRIS_PLAYERIMPL_H_INCLUDED
#define TETRIS_PLAYERIMPL_H_INCLUDED


#include <string>


namespace Tetris {


class PlayerImpl
{
public:
    PlayerImpl(const std::string & inName);

    virtual ~PlayerImpl() = 0;

    const std::string & name();

private:
    std::string mName;
};


}


#endif // TETRIS_PLAYERIMPL_H_INCLUDED
