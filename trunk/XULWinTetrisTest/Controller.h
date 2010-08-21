#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED


#include "Block.h"
#include "XULWin/Components.h"
#include "XULWin/WinUtils.h"
#include "XULWin/XULRunner.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <string>
#include <windows.h>


namespace Tetris
{

    class BlockMover;
    class GameStateNode;
    class Player;
    class ThreadSafeGame;
    class TimedGame;

    enum
    {
        cAIMaxDepth = 5,        // max depth of the AI
        cAIThinkingTime = 3000  // number of ms the AI is allowed to think
    };

    
    class Controller : boost::noncopyable
    {
    public:
        Controller(HINSTANCE hInstance);

        ~Controller();

        void Controller::setQuitFlag();

        void joinAllThreads();

        ThreadSafeGame & threadSafeGame();

        void log(const std::string & inMessage);

        void startAI();

        void refresh();
            
        void precalculate();

        void calculateOptimalPath(std::auto_ptr<GameStateNode> inClonedGameState, const BlockTypes & inBlockTypes);

        void processKey(int inKey);

    private:
        XULWin::XULRunner mXULRunner;
        XULWin::TextBox * mBlockCountTextBox;
        XULWin::ProgressMeter * mAIProgressMeter;
        XULWin::TextBox * mLoggingTextBox;
        boost::scoped_ptr<ThreadSafeGame> mThreadSafeGame;
        boost::scoped_ptr<Tetris::TimedGame> mTimedGame;
        boost::scoped_ptr<XULWin::WinAPI::Timer> mRefreshTimer;
        boost::scoped_ptr<BlockMover> mBlockMover;
        boost::scoped_ptr<boost::thread> mAIThread;
        boost::scoped_ptr<Player> mComputerPlayer;
        volatile bool mQuit;
    };

} // namespace Tetris


#endif // CONTROLLER_H_INCLUDED
