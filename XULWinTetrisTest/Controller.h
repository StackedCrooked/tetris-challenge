#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED


#include "TetrisComponent.h"
#include "Tetris/Block.h"
#include "Tetris/Game.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Threading.h"
#include "XULWin/Components.h"
#include "XULWin/Dialog.h"
#include "XULWin/EventListener.h"
#include "XULWin/Menu.h"
#include "XULWin/Window.h"
#include "XULWin/WinUtils.h"
#include "XULWin/XULRunner.h"
#include "Poco/Random.h"
#include "Poco/Timer.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <string>
#include <windows.h>


namespace Tetris
{

    class BlockMover;
    class ComputerPlayer;
    class GameStateNode;
    class MultithreadedNodeCalculator;
    class Gravity;
    class WorkerPool;

    enum
    {
        cMinSearchDepth = 1,
        cDefaultSearchDepth = 4,
        cMaxSearchDepth = 8,

        cMinSearchWidth = 1,
        cDefaultSearchWidth = 4,
        cMaxSearchWidth = 6
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

        int percentOccupied(const GameState & inGameState) const;
        std::auto_ptr<Evaluator> getEvaluator(const GameState & inGameState) const;
        void setText(XULWin::StringValueController * inComponent, const std::string & inText);
        void onGameCopy(Poco::Timer &);

        LRESULT onNew(WPARAM wParam, LPARAM lParam);
        LRESULT onShowConsole(WPARAM wParam, LPARAM lParam);
        LRESULT onQuit(WPARAM wParam, LPARAM lParam);
        LRESULT onAboutMenuItem(WPARAM wParam, LPARAM lParam);
        LRESULT onStrategySelected(WPARAM wParam, LPARAM lParam);
        LRESULT onClearPrecalculated(WPARAM wParam, LPARAM lParam);
        LRESULT onSelectComputerPlayer(WPARAM wParam, LPARAM lParam);
        LRESULT onSplatter(WPARAM wParam, LPARAM lParam);


        XULWin::XULRunner mXULRunner;
        XULWin::ElementPtr mRootElement;;
        XULWin::Window * mWindow;
        XULWin::ElementPtr mAboutDialogRootElement;
        TetrisComponent * mTetrisComponent;
                
        // Since the strategies menulist (mStrategiesMenuList) can only be read in the main thread
        // we need a variable that holds the last known preset and that is accessible from any thread.
        // The following combintation of an enum and mutex serves that purpose.
        enum EvaluatorType
        {
            EvaluatorType_Automatic,
            EvaluatorType_MakeTetrises,
            EvaluatorType_Balanced,
            EvaluatorType_Survive,
            EvaluatorType_Custom
        };
        EvaluatorType mEvaluatorType;
        mutable boost::mutex mEvaluatorTypeMutex;


        XULWin::TextBox * mFPSTextBox;
        XULWin::TextBox * mBlockCountTextBox;
        typedef XULWin::TextBox * TextBoxPtr;
        TextBoxPtr mLinesTextBoxes[4];
        XULWin::TextBox * mTotalLinesTextBox;
        XULWin::TextBox * mScoreTextBox;
        XULWin::TextBox * mLevelTextBox;
        XULWin::TextBox * mCurrentSearchDepth;
        XULWin::SpinButton * mMovementSpeed;
        XULWin::TextBox * mStatusTextBox;
        XULWin::TextBox * mMovesAheadTextBox;
        XULWin::MenuList * mStrategiesMenuList;
        XULWin::Button * mClearPrecalculatedButton;
        XULWin::Button * mSplatterButton;        
        XULWin::Radio * mPlayerIsHuman;
        XULWin::Radio * mPlayerIsComputer;
        XULWin::TextBox * mKeyboardSink;
        XULWin::TextBox * mActualPreset;
		XULWin::SpinButton * mThreadCount;
		XULWin::CheckBox * mAutoSelect;
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
        boost::scoped_ptr<Tetris::ComputerPlayer> mComputerPlayer;
        boost::scoped_ptr<XULWin::WinAPI::Timer> mRefreshTimer;
        boost::scoped_ptr<Poco::Timer> mGameCopyTimer;
        boost::scoped_ptr<Game> mGameCopy;
        mutable boost::mutex mGameCopyMutex;
        Poco::Random mRandom;
    };

} // namespace Tetris


#endif // CONTROLLER_H_INCLUDED
