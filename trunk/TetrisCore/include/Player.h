#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Game.h"
#include "Poco/Mutex.h"
#include "Poco/ScopedLock.h"
#include <vector>


namespace Tetris
{

    class Player
    {
    public:
        Player(Game * inGame);

        void move(const std::vector<int> & inSelectionCounts);

        void setThreadCount(size_t inThreadCount);

        size_t getThreadCount() const;

        void populateNodeMultiThreaded(GameStateNode & inNode, const std::vector<BlockType> & inBlocks, const std::vector<int> & inSelectionCounts);

        void playUntilGameOver(const std::vector<int> & inDepths);

        void print(const std::string & inMessage);

    private:
        Game * mGame;
        size_t mThreadCount;
        mutable Poco::Mutex mThreadCountMutex;
        mutable Poco::Mutex mIOMutex;
    };

} // namespace Tetris

#endif // PLAYER_H_INCLUDED
