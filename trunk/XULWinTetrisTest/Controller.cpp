#include "Controller.h"
#include "Console.h"
#include "Tetris/Assert.h"
#include "Tetris/BlockMover.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/MultithreadedNodeCalculator.h"
#include "Tetris/Game.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GenericGrid.h"
#include "Tetris/Gravity.h"
#include "Tetris/Logger.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include "Tetris/Worker.h"
#include "Tetris/WorkerPool.h"
#include "TetrisElement.h"
#include "XULWin/Conversions.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/Menu.h"
#include "XULWin/WinUtils.h"
#include "XULWin/Window.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Environment.h"
#include <boost/lexical_cast.hpp>
#include <iostream>


namespace Tetris
{

    Controller::Controller(HINSTANCE hInstance) :
        mXULRunner(hInstance),
        mRootElement(),
        mWindow(0),
        mAboutDialogRootElement(),
        mTetrisComponent(0),
        mFPSTextBox(0),
        mBlockCountTextBox(0),
        mLevelTextBox(0),
        mScoreTextBox(0),
        mTotalLinesTextBox(0),
        mSearchDepth(0),
        mCurrentSearchDepth(0),
        mMovementSpeed(0),
        mStatusTextBox(0),
        mMovesAheadTextBox(0),
        mStrategiesMenuList(0),
        mClearPrecalculatedButton(0),
        mGameHeightFactor(0),
        mLastBlockHeightFactor(0),
        mNumHolesFactor(0),
        mNumLinesFactor(0),
        mNumSinglesFactor(0),
        mNumDoublesFactor(0),
        mNumTriplesFactor(0),
        mNumTetrisesFactor(0),
        mGameStateScore(0),
        mLoggingTextBox(0),
        mProtectedGame(),
        mGravity(),
        mCustomEvaluator(),
        mRefreshTimer(),
        mGameCopyTimer(new Poco::Timer(0, 10)),
        #ifdef _DEBUG
        // Console shown by default
        mConsoleVisible(true)
        #else
        mConsoleVisible(false)
        #endif
    {
        //
        // Parse the XUL document.
        //
        mRootElement = mXULRunner.loadApplication("application.ini");
        if (!mRootElement)
        {
            throw std::runtime_error("Failed to load the root element");
        }


        //
        // Get the Window component.
        //
        mWindow = mRootElement->component()->downcast<XULWin::Window>();
        if (!mWindow)
        {
            throw std::runtime_error("Root element is not of type winodw.");
        }


        // Disabled XULWin logging during constructor. We have our own logger.
        XULWin::ErrorCatcher catcher;
        catcher.disableLogging(true);

        //
        // Initialize the log handler
        //
        boost::function<void(const std::string &)> logFunction = boost::bind(&Controller::log, this, _1);
        XULWin::ErrorReporter::Instance().setLogger(logFunction);
        Logger::Instance().setLogHandler(logFunction);
        
        
        //
        // Get the Tetris component.
        //
        if (!(mTetrisComponent = findComponentById<Tetris::TetrisComponent>("tetris0")))
        {
            throw std::runtime_error("The XUL document must contain a <tetris> element with id \"tetris0\".");
        }

        mTetrisComponent->setController(this);
        mProtectedGame.reset(new Protected<Game>(std::auto_ptr<Game>(new Game(mTetrisComponent->getNumRows(), mTetrisComponent->getNumColumns()))));


        mFPSTextBox = findComponentById<XULWin::TextBox>("fpsTextBox");
        mBlockCountTextBox = findComponentById<XULWin::TextBox>("blockCountTextBox");
        mLinesTextBoxes[0] = findComponentById<XULWin::TextBox>("lines1TextBox");
        mLinesTextBoxes[1] = findComponentById<XULWin::TextBox>("lines2TextBox");
        mLinesTextBoxes[2] = findComponentById<XULWin::TextBox>("lines3TextBox");
        mLinesTextBoxes[3] = findComponentById<XULWin::TextBox>("lines4TextBox");
        mLevelTextBox = findComponentById<XULWin::TextBox>("levelTextBox");
        mScoreTextBox = findComponentById<XULWin::TextBox>("scoreTextBox");
        mTotalLinesTextBox = findComponentById<XULWin::TextBox>("totalLinesTextBox");

        if (mSearchDepth = findComponentById<XULWin::SpinButton>("searchDepth"))
        {
            XULWin::WinAPI::SpinButton_SetRange(mSearchDepth->handle(), cMinSearchDepth, cMaxSearchDepth);
        }

        if (mSearchWidth = findComponentById<XULWin::SpinButton>("searchWidth"))
        {
            XULWin::WinAPI::SpinButton_SetRange(mSearchWidth->handle(), cMinSearchWidth, cMaxSearchWidth);
        }

        mCurrentSearchDepth = findComponentById<XULWin::TextBox>("currentSearchDepth");

        if (mMovementSpeed = findComponentById<XULWin::SpinButton>("movementSpeed"))
        {
            XULWin::WinAPI::SpinButton_SetRange(mMovementSpeed->handle(), 1, 1000);
        }

        mMovesAheadTextBox = findComponentById<XULWin::TextBox>("movesAheadTextBox");

        if (mStrategiesMenuList = findComponentById<XULWin::MenuList>("strategiesMenuList"))
        {
            mScopedEventListener.connect(mStrategiesMenuList->el(), boost::bind(&Controller::onStrategySelected, this, _1, _2));
        }

        mGameHeightFactor = findComponentById<XULWin::SpinButton>("gameHeightFactor");
        mLastBlockHeightFactor = findComponentById<XULWin::SpinButton>("lastBlockHeightFactor");
        mNumHolesFactor = findComponentById<XULWin::SpinButton>("numHolesFactor");
        mNumLinesFactor = findComponentById<XULWin::SpinButton>("numLinesFactor");
        mNumSinglesFactor = findComponentById<XULWin::SpinButton>("numSinglesFactor");
        mNumDoublesFactor = findComponentById<XULWin::SpinButton>("numDoublesFactor");
        mNumTriplesFactor = findComponentById<XULWin::SpinButton>("numTriplesFactor");
        mNumTetrisesFactor = findComponentById<XULWin::SpinButton>("numTetrisesFactor");
        mGameStateScore = findComponentById<XULWin::TextBox>("gameStateScore");
        mStatusTextBox = findComponentById<XULWin::TextBox>("statusTextBox");

        if (XULWin::MenuItem * newMenuItem = findComponentById<XULWin::MenuItem>("newMenuItem"))
        {
            mScopedEventListener.connect(newMenuItem->el(), boost::bind(&Controller::onNew, this, _1, _2));
        }

        if (XULWin::MenuItem * showConsoleMenuItem = findComponentById<XULWin::MenuItem>("showConsoleMenuItem"))
        {
            mScopedEventListener.connect(showConsoleMenuItem->el(), boost::bind(&Controller::onShowConsole, this, _1, _2));
        }

        if (XULWin::MenuItem * quitMenuItem = findComponentById<XULWin::MenuItem>("quitMenuItem"))
        {
            mScopedEventListener.connect(quitMenuItem->el(), boost::bind(&Controller::onQuit, this, _1, _2));
        }

        if (XULWin::MenuItem * aboutMenuItem = findComponentById<XULWin::MenuItem>("aboutMenuItem"))
        {
            mScopedEventListener.connect(aboutMenuItem->el(), boost::bind(&Controller::onAboutMenuItem, this, _1, _2));
        }


        //mTetrisComponent->OnKeyboardPressed.connect(boost::bind(&Controller::processKey, this, _1));       
        mGravity.reset(new Gravity(*mProtectedGame));

        mRefreshTimer.reset(new XULWin::WinAPI::Timer);
        mRefreshTimer->start(boost::bind(&Controller::onRefresh, this), 10);

		// Main thread should have highest priority for responsiveness.
        ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
        
        mComputerPlayer.reset(new ComputerPlayer(*mProtectedGame,
                                                 createEvaluator(), 
                                                 mSearchDepth ? XULWin::String2Int(mSearchDepth->getValue(), 4) : 4,
                                                 mSearchWidth ? XULWin::String2Int(mSearchWidth->getValue(), 4) : 4));

        
        mGameCopyTimer->start(Poco::TimerCallback<Controller>(*this, &Controller::onGameCopy));
    }


    Controller::~Controller()
    {
        mGameCopyTimer.reset();
        mComputerPlayer.reset();
        mGravity.reset();
    }

        
    void Controller::onGameCopy(Poco::Timer & timer)
    {
        boost::mutex::scoped_lock lock(mGameCopyMutex);
        ScopedAtom<Game> game(*mProtectedGame.get());
        mGameCopy.reset(game->clone().release());
    }


    LRESULT Controller::onNew(WPARAM wParam, LPARAM lParam)
    {        
        boost::mutex::scoped_lock lock(mGameCopyMutex);
        mComputerPlayer.reset();
        mGravity.reset();
        mProtectedGame.reset();
        
        mProtectedGame.reset(new Protected<Game>(std::auto_ptr<Game>(new Game(mTetrisComponent->getNumRows(), mTetrisComponent->getNumColumns()))));
        mGravity.reset(new Gravity(*mProtectedGame));        
        mComputerPlayer.reset(new ComputerPlayer(*mProtectedGame,
                                                 createEvaluator(), 
                                                 mSearchDepth ? XULWin::String2Int(mSearchDepth->getValue(), 4) : 4,
                                                 mSearchWidth ? XULWin::String2Int(mSearchWidth->getValue(), 4) : 4));
        return XULWin::cHandled;
    }


    LRESULT Controller::onShowConsole(WPARAM wParam, LPARAM lParam)
    {
        if (!mConsoleVisible)
        {
            AttachToConsole();
            mConsoleVisible = true;
        }
        return XULWin::cHandled;
    }


    LRESULT Controller::onQuit(WPARAM wParam, LPARAM lParam)
    {
        ::PostQuitMessage(0);
        return XULWin::cHandled;
    }


    LRESULT Controller::onAboutMenuItem(WPARAM wParam, LPARAM lParam)
    {
        if (!(mAboutDialogRootElement = mXULRunner.loadXULFromFile("chrome/content/AboutDialog.xul")))
        {
            throw std::runtime_error("Did not find the AboutDialog.xul file.");
        }

        if (XULWin::Dialog * dialog = mAboutDialogRootElement->component()->downcast<XULWin::Dialog>())
        {
            if (XULWin::Element * okButton = mAboutDialogRootElement->getElementById("okButton"))
            {
                XULWin::ScopedEventListener dialogEventListener;
                dialogEventListener.connect(okButton,
                                            boost::bind(&XULWin::Dialog::endModal, dialog, XULWin::DialogResult_Ok));
                dialog->showModal(mWindow);
            }
        }
        return XULWin::cHandled;
    }


    LRESULT Controller::onStrategySelected(WPARAM wParam, LPARAM lParam)
    {
        if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            updateStrategy();
        }
        // Must return unhandled otherwise the popup menu stays.
        return XULWin::cUnhandled;
    }


    LRESULT Controller::onClearPrecalculated(WPARAM wParam, LPARAM lParam)
    {
        ScopedAtom<Game> game(*mProtectedGame);
        game->clearPrecalculatedNodes();
        return XULWin::cHandled;
    }


    std::auto_ptr<Evaluator> Controller::createEvaluator()
    {
        if (!mStrategiesMenuList)
        {
            LogWarning("mStrategiesMenuList is not defined. Using 'Balanced' strategy.");
            return CreatePoly<Evaluator, Balanced>();
        }


        std::string id = mStrategiesMenuList->getLabel();
        if (id == "Make Tetrises")
        {
            return CreatePoly<Evaluator, MakeTetrises>();
        }
        else if (id == "Survive")
        {
            return CreatePoly<Evaluator, Survival>();
        }
        else if (id == "Balanced")
        {
            return CreatePoly<Evaluator, Balanced>();
        }
        else if (id == "Depressed")
        {
            return CreatePoly<Evaluator, Depressed>();
        }
        else if (id == "Custom")
        {
            // We continuously recreate this object so that the most recent values from the GUI are always applied.
            std::auto_ptr<Evaluator> customEvaluator(
                new CustomEvaluator(
                    GameHeightFactor(XULWin::String2Int(mGameHeightFactor->getValue(), 0)),
                    LastBlockHeightFactor(XULWin::String2Int(mLastBlockHeightFactor->getValue(), 0)),
                    NumHolesFactor(XULWin::String2Int(mNumHolesFactor->getValue(), 0)),
                    NumSinglesFactor(XULWin::String2Int(mNumSinglesFactor->getValue(), 0)),
                    NumDoublesFactor(XULWin::String2Int(mNumDoublesFactor->getValue(), 0)),
                    NumTriplesFactor(XULWin::String2Int(mNumTriplesFactor->getValue(), 0)),
                    NumTetrisesFactor(XULWin::String2Int(mNumTetrisesFactor->getValue(), 0)),
                    SearchDepth(XULWin::String2Int(mSearchDepth->getValue(), 4)),
                    SearchWidth(XULWin::String2Int(mSearchDepth->getValue(), 4))));
            return customEvaluator;
        }

        throw std::runtime_error("Unknown evaluator.");
    }


    void Controller::updateStrategy()
    {
        std::string id = mStrategiesMenuList->getLabel();
        if (id == "Make Tetrises")
        {
            if (!dynamic_cast<const MakeTetrises *>(&mComputerPlayer->evaluator()))
            {
                ScopedAtom<Game> game(*mProtectedGame);
                mComputerPlayer->setEvaluator(Create<MakeTetrises>());
            }
        }
        else if (id == "Survive")
        {
            if (!dynamic_cast<const Survival *>(&mComputerPlayer->evaluator()))
            {
                ScopedAtom<Game> game(*mProtectedGame);
                mComputerPlayer->setEvaluator(Create<Survival>());
            }
        }
        else if (id == "Balanced")
        {
            if (!dynamic_cast<const Balanced *>(&mComputerPlayer->evaluator()))
            {
                ScopedAtom<Game> game(*mProtectedGame);
                mComputerPlayer->setEvaluator(Create<Balanced>());
            }
        }
        else if (id == "Depressed")
        {
            if (!dynamic_cast<const Depressed *>(&mComputerPlayer->evaluator()))
            {
                ScopedAtom<Game> game(*mProtectedGame);
                mComputerPlayer->setEvaluator(Create<Depressed>());
            }
        }
        else if (id == "Custom")
        {
            // We continuously recreate this object so that the most recent values from the GUI are always applied.
            mCustomEvaluator.reset(new CustomEvaluator(
                GameHeightFactor(XULWin::String2Int(mGameHeightFactor->getValue(), 0)),
                LastBlockHeightFactor(XULWin::String2Int(mLastBlockHeightFactor->getValue(), 0)),
                NumHolesFactor(XULWin::String2Int(mNumHolesFactor->getValue(), 0)),
                NumSinglesFactor(XULWin::String2Int(mNumSinglesFactor->getValue(), 0)),
                NumDoublesFactor(XULWin::String2Int(mNumDoublesFactor->getValue(), 0)),
                NumTriplesFactor(XULWin::String2Int(mNumTriplesFactor->getValue(), 0)),
                NumTetrisesFactor(XULWin::String2Int(mNumTetrisesFactor->getValue(), 0)),
                SearchDepth(XULWin::String2Int(mSearchDepth->getValue(), 4)),
                SearchWidth(XULWin::String2Int(mSearchWidth->getValue(), 4))));
            mComputerPlayer->setEvaluator(mCustomEvaluator->clone());
        }
    }


    void Controller::run()
    {
        mWindow->showModal(XULWin::WindowPos_CenterInScreen);
    }


    void Controller::getGameState(TetrisComponent * tetrisComponent,
                                  Grid & outGrid,
                                  Block & outActiveBlock,
                                  BlockTypes & outFutureBlockTypes)
    {
        if (tetrisComponent == mTetrisComponent)
        {
            ScopedConstAtom<Game> rgame(*mProtectedGame);
            const Game & game = *(rgame.get());
            outGrid = game.currentNode()->state().grid();
            outActiveBlock = game.activeBlock();
            game.getFutureBlocks(mTetrisComponent->getNumFutureBlocks() + 1, outFutureBlockTypes);
        }
    }


    bool Controller::move(TetrisComponent * tetrisComponent, Direction inDirection)
    {
        if (mComputerPlayer)
        {
            return false;
        }
        ScopedAtom<Game> wgame(*mProtectedGame);
        Game & game = *wgame.get();
        return game.move(inDirection);
    }


    void Controller::drop(TetrisComponent * tetrisComponent)
    {
        if (mComputerPlayer)
        {
            return;
        }
        ScopedAtom<Game> wgame(*mProtectedGame);
        Game & game = *wgame.get();
        game.drop();
    }


    bool Controller::rotate(TetrisComponent * tetrisComponent)
    {
        if (mComputerPlayer)
        {
            return false;
        }
        ScopedAtom<Game> wgame(*mProtectedGame);
        Game & game = *wgame.get();
        return game.rotate();
    }


    Protected<Game> & Controller::threadSafeGame()
    {
        return *mProtectedGame;
    }


    void Controller::log(const std::string & inMessage)
    {
        std::cout << inMessage << "\n";
    }


    void Controller::setText(XULWin::StringValueController * inComponent, const std::string & inText)
    {
        if (inComponent->getValue() != inText)
        {
            inComponent->setValue(inText);
        }
    }


    void Controller::onRefresh()
    {
        try
        {
            refresh();
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "Exception thrown during Controller::onRefresh. Details: " << inException.what());
        }
    }


    void Controller::refresh()
    {
        // Flush the logger.
        Logger::Instance().flush();

        boost::mutex::scoped_lock lock(mGameCopyMutex);
        if (!mGameCopy)
        {
            LogWarning("GameCopy not yet created. Returning");
        }

        if (mLevelTextBox && mGravity)
        {
            setText(mLevelTextBox, MakeString() << mGameCopy->level());
        }


        if (mScoreTextBox)
        {
            setText(mScoreTextBox, MakeString() << mGameCopy->currentNode()->state().stats().score());
        }


        if (mTotalLinesTextBox)
        {
            setText(mTotalLinesTextBox, MakeString() << mGameCopy->currentNode()->state().stats().numLines());
        }


        if (mMovementSpeed)
        {
            mComputerPlayer->setMoveSpeed(XULWin::String2Int(mMovementSpeed->getValue(), 1));
        }


        if (mStatusTextBox)
        {
            setText(mStatusTextBox, mGameCopy->currentNode()->children().empty() ? "Thinking" : "Moving");
        }


        if (mBlockCountTextBox)
        {
            setText(mBlockCountTextBox, MakeString() << (mGameCopy->currentNode()->depth() + 1));
        }


        if (mCurrentSearchDepth)
        {
            setText(mCurrentSearchDepth, MakeString() << mComputerPlayer->currentSearchDepth() << "/" << mComputerPlayer->searchDepth());
        }


        if (mMovesAheadTextBox)
        {
            setText(mMovesAheadTextBox, MakeString() << (mGameCopy->lastPrecalculatedNode()->depth() - mGameCopy->currentNode()->depth()));
        }


        if (mSearchDepth)
        {
            mComputerPlayer->setSearchDepth(XULWin::String2Int(mSearchDepth->getValue(), 4));
        }

        if (mSearchWidth)
        {
            mComputerPlayer->setSearchWidth(XULWin::String2Int(mSearchWidth->getValue(), 4));
        }


        for (size_t idx = 0; idx != 4; ++idx)
        {
            const Stats & stats = mGameCopy->currentNode()->state().stats();
            if (mLinesTextBoxes[idx])
            {
                setText(mLinesTextBoxes[idx], MakeString() << stats.numLines(idx));
            }
        }

        if (mStrategiesMenuList
			&& !XULWin::WinAPI::ComboBox_IsOpen(mStrategiesMenuList->handle())
			&& mGameHeightFactor
            && mLastBlockHeightFactor
            && mNumHolesFactor
            && mNumLinesFactor
            && mNumSinglesFactor
            && mNumDoublesFactor
            && mNumTriplesFactor
            && mNumTetrisesFactor)
        {
			bool useCustomEvaluator = mStrategiesMenuList->getLabel() == "Custom";

            // Enable/disable the spin buttons
            mSearchDepth->setDisabled(!useCustomEvaluator);
            mSearchWidth->setDisabled(!useCustomEvaluator);
            mGameHeightFactor->setDisabled(!useCustomEvaluator);
            mLastBlockHeightFactor->setDisabled(!useCustomEvaluator);
            mNumHolesFactor->setDisabled(!useCustomEvaluator);
            mNumLinesFactor->setDisabled(!useCustomEvaluator);
            mNumSinglesFactor->setDisabled(!useCustomEvaluator);
            mNumDoublesFactor->setDisabled(!useCustomEvaluator);
            mNumTriplesFactor->setDisabled(!useCustomEvaluator);
            mNumTetrisesFactor->setDisabled(!useCustomEvaluator);

            if (useCustomEvaluator)
            {
                // Create a CustomEvaluator using the values from the GUI
                mCustomEvaluator.reset(new CustomEvaluator(
                    GameHeightFactor(XULWin::String2Int(mGameHeightFactor->getValue(), 0)),
                    LastBlockHeightFactor(XULWin::String2Int(mLastBlockHeightFactor->getValue(), 0)),
                    NumHolesFactor(XULWin::String2Int(mNumHolesFactor->getValue(), 0)),
                    NumSinglesFactor(XULWin::String2Int(mNumSinglesFactor->getValue(), 0)),
                    NumDoublesFactor(XULWin::String2Int(mNumDoublesFactor->getValue(), 0)),
                    NumTriplesFactor(XULWin::String2Int(mNumTriplesFactor->getValue(), 0)),
                    NumTetrisesFactor(XULWin::String2Int(mNumTetrisesFactor->getValue(), 0)),
                    SearchDepth(XULWin::String2Int(mSearchDepth->getValue(), 4)),
                    SearchWidth(XULWin::String2Int(mSearchWidth->getValue(), 4))));
                mComputerPlayer->setEvaluator(mCustomEvaluator->clone());
            }
            else
            {
                updateStrategy();
                // Update the GUI with the values from the evaluator.
                mGameHeightFactor->setValue(MakeString() << mComputerPlayer->evaluator().gameHeightFactor());
                mLastBlockHeightFactor->setValue(MakeString() << mComputerPlayer->evaluator().lastBlockHeightFactor());
                mNumHolesFactor->setValue(MakeString() << mComputerPlayer->evaluator().numHolesFactor());
                mNumSinglesFactor->setValue(MakeString() << mComputerPlayer->evaluator().numSinglesFactor());
                mNumDoublesFactor->setValue(MakeString() << mComputerPlayer->evaluator().numDoublesFactor());
                mNumTriplesFactor->setValue(MakeString() << mComputerPlayer->evaluator().numTriplesFactor());
                mNumTetrisesFactor->setValue(MakeString() << mComputerPlayer->evaluator().numTetrisesFactor());
                mSearchDepth->setValue(MakeString() << mComputerPlayer->evaluator().recommendedSearchDepth());
                mSearchWidth->setValue(MakeString() << mComputerPlayer->evaluator().recommendedSearchWidth());
            }

            if (mGameStateScore)
            {
                setText(mGameStateScore, XULWin::Int2String(mComputerPlayer->evaluator().evaluate(mGameCopy->currentNode()->state())));
            }
        }


        if (mFPSTextBox)
        {
            setText(mFPSTextBox, MakeString() << mTetrisComponent->getFPS());
        }
    }


    void Controller::processKey(int inKey)
    {
    }


} // namespace Tetris
