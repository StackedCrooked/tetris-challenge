#ifndef TETRIS_COMPUTERPLAYER_H_INCLUDED
#define TETRIS_COMPUTERPLAYER_H_INCLUDED


namespace Tetris
{
    
    template<class Variable> class Protected;
    class Game;


    class ComputerPlayerImpl;


    class ComputerPlayer
    {
    public:
        ComputerPlayer(const Protected<Game> & inProtectedGame);

        ~ComputerPlayer();

    private:
        ComputerPlayer(const ComputerPlayer &);
        ComputerPlayer & operator= (const ComputerPlayer&);

        ComputerPlayerImpl * mImpl;
    };

} // namespace Tetris


#endif // PLAYER_H_INCLUDED
