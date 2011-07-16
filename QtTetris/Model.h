#ifndef QTTETRIS_MODEL_H
#define QTTETRIS_MODEL_H


#include "Tetris/BlockMover.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Evaluator.h"
#include "Tetris/MultiplayerGame.h"
#include "Tetris/Player.h"
#include <string>
#include <vector>


namespace QtTetris {


using namespace Tetris;


class Model;
typedef boost::shared_ptr<Model> ModelPtr;


class Model : public ComputerPlayer::Tweaker
{
public:
    Model();

    ~Model();

    void quit();

    bool IsGameOver();

    std::size_t playerCount() const;

    std::size_t computerPlayerCount() const;

    Player * getPlayer(std::size_t inIndex);

    const MultiplayerGame & multiplayerGame() const;

    MultiplayerGame & multiplayerGame();

    virtual const Evaluator & updateAIParameters(const Player & inPlayer,
                                                 int & outSearchDepth,
                                                 int & outSearchWidth,
                                                 int & outWorkerCount,
                                                 int & /*outMoveSpeed*/,
                                                 BlockMover::MoveDownBehavior & outMoveDownBehavior);

    void newGame(const PlayerTypes & inPlayerTypes, std::size_t inRowCount, std::size_t inColumnCount);

private:

    std::string GetHumanPlayerName();

    std::string GetPlayerName(PlayerType inPlayerType);

    boost::scoped_ptr<MultiplayerGame> mMultiplayerGame;

    typedef std::vector<std::string> Names;
    Names mNames;
    Names::size_type mNamesIndex;

    bool mQuit;
    std::string mHumanName;
    int mCPUCount;
    bool mGameOver;
};


} // namespace QtTetris


#endif // QTTETRIS_MODEL_H
