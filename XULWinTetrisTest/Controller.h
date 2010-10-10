#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED


#include "TetrisComponent.h"
#include "Tetris/Block.h"
#include "Tetris/Game.h"
#include "Tetris/Threading.h"
#include "XULWin/Components.h"
#include "XULWin/Dialog.h"
#include "XULWin/EventListener.h"
#include "XULWin/Menu.h"
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
    class ComputerPlayerMt;
    class Gravity;
    class WorkerPool;

    enum
    {
        cMinSearchDepth = 1,
        cDefaultSearchDepth = 3,
        cMaxSearchDepth = 10,

        cMinSearchWidth = 1,
        cDefaultSearchWidth = 8,
        cMaxSearchWidth = 40
    };


    class Controller :
        public TetrisComponent::Controller,
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

        Protected<Game> & threadSafeGame();

        void log(const std::string & inMessage);

        void refresh();

        void calculateOptimalPath(std::auto_ptr<GameStateNode> inClonedGameState, const BlockTypes & inBlockTypes);

        void processKey(int inKey);

        void onRefresh();

    private:
        template<class T>
        T * findComponentById(const std::string & inId)
        {
            if (XULWin::Element * element = mRootElement->getElementById(inId))
            {
                if (T * comp = element->component()->downcast<T>())
                {
                    return comp;
                }
                else
                {
                    LogError("Element with id '" + inId + "' was found but it was not of the requested type.");
                    return 0;
                }
            }
            else
            {
                LogWarning("Element with id '" + inId + "' not found.");
            }
            return 0;
        }

        void setText(XULWin::StringValueController * inComponent, const std::string & inText);

        int calculateRemainingTimeMs(Game & game) const;
        void startAI(Game & game, size_t inDepth, size_t inWidth);

        LRESULT onNew(WPARAM wParam, LPARAM lParam);
        LRESULT onShowConsole(WPARAM wParam, LPARAM lParam);
        LRESULT onQuit(WPARAM wParam, LPARAM lParam);
        LRESULT onAboutMenuItem(WPARAM wParam, LPARAM lParam);
        LRESULT onStrategySelected(WPARAM wParam, LPARAM lParam);
        LRESULT onClearPrecalculated(WPARAM wParam, LPARAM lParam);

        void updateStrategy();


        XULWin::XULRunner mXULRunner;
        XULWin::ElementPtr mRootElement;;
        XULWin::Window * mWindow;
        XULWin::ElementPtr mAboutDialogRootElement;
        TetrisComponent * mTetrisComponent;
        XULWin::TextBox * mFPSTextBox;
        XULWin::TextBox * mBlockCountTextBox;
        typedef XULWin::TextBox * TextBoxPtr;
        TextBoxPtr mLinesTextBoxes[4];
        XULWin::TextBox * mTotalLinesTextBox;
        XULWin::TextBox * mScoreTextBox;
        XULWin::TextBox * mLevelTextBox;
        XULWin::CheckBox * mComputerEnabledCheckBox;
        XULWin::TextBox * mCurrentSearchDepth;
        XULWin::SpinButton * mMovementSpeed;
        XULWin::TextBox * mStatusTextBox;
        XULWin::TextBox * mMovesAheadTextBox;
        XULWin::MenuList * mStrategiesMenuList;
        XULWin::Button * mClearPrecalculatedButton;

        XULWin::SpinButton * mSearchDepth;
        XULWin::SpinButton * mSearchWidth;
        XULWin::SpinButton * mGameHeightFactor;
        XULWin::SpinButton * mLastBlockHeightFactor;
        XULWin::SpinButton * mNumHolesFactor;
        XULWin::SpinButton * mNumLinesFactor;
        XULWin::SpinButton * mNumSinglesFactor;
        XULWin::SpinButton * mNumDoublesFactor;
        XULWin::SpinButton * mNumTriplesFactor;
        XULWin::SpinButton * mNumTetrisesFactor;

        XULWin::TextBox * mGameStateScore;

        XULWin::TextBox * mLoggingTextBox;
        XULWin::ScopedEventListener mScopedEventListener;
        boost::scoped_ptr<Protected<Game> > mProtectedGame;
        boost::scoped_ptr<Tetris::Gravity> mGravity;
        boost::scoped_ptr<XULWin::WinAPI::Timer> mRefreshTimer;
        boost::scoped_ptr<ComputerPlayerMt> mComputerPlayer;
        boost::scoped_ptr<BlockMover> mBlockMover;
        boost::scoped_ptr<Evaluator> mEvaluator;
        bool mConsoleVisible;
        boost::shared_ptr<WorkerPool> mWorkerPool;
    };

} // namespace Tetris


#endif // CONTROLLER_H_INCLUDED
