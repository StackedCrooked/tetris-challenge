#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED


#include "Block.h"
#include "Game.h"
#include "TetrisComponent.h"
#include "Threading.h"
#include "XULWin/Components.h"
#include "XULWin/Window.h"
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
    class TimedGame;

    enum
    {
        cMinSearchDepth = 2,
#ifndef NDEBUG
        cMaxSearchDepth = 3
#else
        cMaxSearchDepth = 4 // more than for results in memory usage of 2+ GB
#endif
    };

    
    class Controller : public TetrisComponent::Controller,
                       boost::noncopyable
    {
    public:
        Controller(HINSTANCE hInstance);

        ~Controller();

        void run();

        // TetrisComponent::Controller methods
        void getGameState(TetrisComponent * tetrisComponent, Grid & grid, Block & activeBlock, BlockTypes & futureBlockTypes);

        bool move(TetrisComponent * tetrisComponent, Direction inDirection);

        bool rotate(TetrisComponent * tetrisComponent);

        void drop(TetrisComponent * tetrisComponent);

        void setQuitFlag();

        void joinAllThreads();

        Protected<Game> & threadSafeGame();

        void log(const std::string & inMessage);

        void refresh();

        void calculateOptimalPath(std::auto_ptr<GameStateNode> inClonedGameState, const BlockTypes & inBlockTypes);

        void processKey(int inKey);

        void onRefresh();

    private:
        void startAI(Game & game, size_t inDepth);

        XULWin::XULRunner mXULRunner;
        XULWin::Window * mWindow;
        TetrisComponent * mTetrisComponent;
        XULWin::TextBox * mFPSTextBox;
        XULWin::TextBox * mBlockCountTextBox;
        typedef XULWin::TextBox * TextBoxPtr;
        TextBoxPtr mLinesTextBoxes[4];
        XULWin::CheckBox * mComputerEnabledCheckBox;
        XULWin::TextBox * mStatusTextBox;
        XULWin::TextBox * mMovesAheadTextBox;
        XULWin::TextBox * mSearchDepthTextBox;
        XULWin::TextBox * mLoggingTextBox;
        boost::scoped_ptr<Protected<Game>> mThreadSafeGame;
        boost::scoped_ptr<Tetris::TimedGame> mTimedGame;
        boost::scoped_ptr<XULWin::WinAPI::Timer> mRefreshTimer;
        boost::scoped_ptr<Player> mComputerPlayer;
        boost::scoped_ptr<BlockMover> mBlockMover;
        int mSearchDepth;
        volatile bool mQuit;
    };

} // namespace Tetris


#endif // CONTROLLER_H_INCLUDED
