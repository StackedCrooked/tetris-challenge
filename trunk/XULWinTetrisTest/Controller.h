#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED


#include "Tetris/Block.h"
#include "Tetris/Game.h"
#include "TetrisComponent.h"
#include "Tetris/Threading.h"
#include "XULWin/Components.h"
#include "XULWin/Dialog.h"
#include "XULWin/EventListener.h"
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
        
        LRESULT onNew(WPARAM wParam, LPARAM lParam);
        LRESULT onQuit(WPARAM wParam, LPARAM lParam);
        LRESULT onAboutMenuItem(WPARAM wParam, LPARAM lParam);

        XULWin::XULRunner mXULRunner;
        XULWin::ElementPtr mRootElement;;
        XULWin::Window * mWindow;
        XULWin::ElementPtr mAboutDialogRootElement;
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
        XULWin::ScopedEventListener mScopedEventListener;
        boost::scoped_ptr<Protected<Game> > mProtectedGame;
        boost::scoped_ptr<Tetris::TimedGame> mTimedGame;
        boost::scoped_ptr<XULWin::WinAPI::Timer> mRefreshTimer;
        boost::scoped_ptr<Player> mComputerPlayer;
        boost::scoped_ptr<BlockMover> mBlockMover;
        boost::scoped_ptr<Evaluator> mEvaluator;
        int mSearchDepth;
        volatile bool mQuit;
    };

} // namespace Tetris


#endif // CONTROLLER_H_INCLUDED
