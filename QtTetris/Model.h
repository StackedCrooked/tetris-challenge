#ifndef QTTETRIS_MODEL_H
#define QTTETRIS_MODEL_H


#include "Tetris/BlockMover.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Evaluator.h"
#include "Tetris/MultiplayerGame.h"
#include "Tetris/Player.h"
#include "Futile/Singleton.h"
#include <string>
#include <vector>
#include <random>

namespace Tetris {


class Model : public Futile::Singleton<Model>,
              public ComputerPlayer::Tweaker
{
public:
    bool IsGameOver();

    Player * getPlayer(std::size_t inIndex);

    const MultiplayerGame& multiplayerGame() const;

    MultiplayerGame& multiplayerGame();

    virtual const Evaluator& updateAIParameters(const Player& inPlayer,
                                                 int& outSearchDepth,
                                                 int& outSearchWidth,
                                                 int& outWorkerCount,
                                                 int& /*outMoveSpeed*/,
                                                 BlockMover::MoveDownBehavior& outMoveDownBehavior);

    void newGame(const PlayerTypes& inPlayerTypes, std::size_t inRowCount, std::size_t inColumnCount);

private:
    friend class Futile::Singleton<Model>;

    Model();

    virtual ~Model();

    std::string GetPlayerName(PlayerType);

    boost::scoped_ptr<MultiplayerGame> mMultiplayerGame;

    typedef std::vector<std::string> Names;
    Names mNames;
    Names::size_type mNamesIndex;

    int mCPUCount;
    bool mGameOver;

    std::default_random_engine mRandomEngine;
};


} // namespace Tetris


#endif // QTTETRIS_MODEL_H
