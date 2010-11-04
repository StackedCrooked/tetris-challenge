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


namespace Tetris {

    
    extern const int cMaxLevel;


    Controller::Controller(HINSTANCE hInstance) :
        mXULRunner(hInstance),
        mRootElement(),
        mWindow(0),
        mAboutDialogRootElement(),
        mTetrisComponent(0),
        mEvaluatorType(EvaluatorType_Automatic),
        mEvaluatorTypeMutex(),
        mFPSTextBox(0),
        mBlockCountTextBox(0),
        mLevelTextBox(0),
        mScoreTextBox(0),
        mTotalLinesTextBox(0),
        mPlayerIsComputer(0),
        mPlayerIsHuman(0),
        mActualPreset(0),
		mAutoSelect(0),
		mThreadCount(0),
        mSearchDepth(0),
        mCurrentSearchDepth(0),
        mMovementSpeed(0),
        mStatusTextBox(0),
        mMovesAheadTextBox(0),
        mStrategiesMenuList(0),
        mClearPrecalculatedButton(0),
        mSplatterButton(0),
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
        mRefreshTimer(),
        mGameCopyTimer(new Poco::Timer(0, 200)),
        mRandom()
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
        if (mLevelTextBox = findComponentById<XULWin::SpinButton>("levelTextBox"))
        {            
			XULWin::WinAPI::SpinButton_SetRange(mLevelTextBox->handle(), 0, cMaxLevel);
        }

        mScoreTextBox = findComponentById<XULWin::TextBox>("scoreTextBox");
        mTotalLinesTextBox = findComponentById<XULWin::TextBox>("totalLinesTextBox");
		
		mPlayerIsComputer = findComponentById<XULWin::Radio>("playerIsComputer");
        mScopedEventListener.connect(mPlayerIsComputer->el(), BM_SETSTATE, boost::bind(&Controller::onSelectComputerPlayer, this, _1, _2));
		mPlayerIsHuman = findComponentById<XULWin::Radio>("playerIsHuman");

        // HACK! This textbox is used to prevent the arrow keys to
        // change the selected radio button during human play.
        mKeyboardSink = findComponentById<XULWin::TextBox>("keyboardSink");

		mAutoSelect = findComponentById<XULWin::CheckBox>("autoSelect");

        mActualPreset = findComponentById<XULWin::TextBox>("actualPreset");

		if (mThreadCount = findComponentById<XULWin::SpinButton>("threadCount"))
		{
			XULWin::WinAPI::SpinButton_SetRange(mThreadCount->handle(), 0, 32);
		}

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
            XULWin::WinAPI::SpinButton_SetRange(mMovementSpeed->handle(), 1, 100);
        }

        mMovesAheadTextBox = findComponentById<XULWin::TextBox>("movesAheadTextBox");

        if (mStrategiesMenuList = findComponentById<XULWin::MenuList>("strategiesMenuList"))
        {
            mScopedEventListener.connect(mStrategiesMenuList->el(), boost::bind(&Controller::onStrategySelected, this, _1, _2));
        }

		mSplatterButton = findComponentById<XULWin::Button>("splatterButton");
        mScopedEventListener.connect(mSplatterButton->el(), boost::bind(&Controller::onSplatter, this, _1, _2));

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
        mRefreshTimer->start(boost::bind(&Controller::onRefresh, this), 20);

		// Main thread should have highest priority for responsiveness.
        ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

        
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

        
    LRESULT Controller::onSplatter(WPARAM wParam, LPARAM lParam)
    {

        {
            mComputerPlayer.reset();
            ScopedAtom<Game> wgame(*mProtectedGame.get());
            Game & game(*wgame.get());
            game.clearPrecalculatedNodes();

            Grid & grid = const_cast<GameStateNode*>(game.currentNode())->state().grid();

            for (size_t rowIdx = 7; rowIdx != grid.numRows(); ++rowIdx)
            {            
                size_t count = 0;
                for (size_t colIdx = 0; colIdx != grid.numColumns(); ++colIdx)
                {
                    if (grid.get(rowIdx, colIdx))
                    {
                        count++;
                    }
                }

                for (size_t colIdx = 0; colIdx != grid.numColumns(); ++colIdx)
                {
                    if (count + 1 == grid.numColumns())
                    {
                        break;
                    }

                    if (mRandom.nextBool())
                    {
                        BlockType blockType = static_cast<BlockType>((mRandom.nextChar() % cBlockTypeCount) + 1);
                        grid.set(rowIdx, colIdx, blockType);
                        count++;
                    }
                }
            }

            const_cast<GameStateNode*>(game.currentNode())->state().forceUpdateStats();
        }
        
        mComputerPlayer.reset(
            new ComputerPlayer(
                *mProtectedGame,
                boost::bind(&Controller::getEvaluator, this, _1),
                mSearchDepth ? XULWin::String2Int(mSearchDepth->getValue(), 4) : 4,
                mSearchWidth ? XULWin::String2Int(mSearchWidth->getValue(), 4) : 4,
                XULWin::String2Int(mThreadCount->getValue(), 0)));

        return XULWin::cHandled;
    }
    
        
    LRESULT Controller::onSelectComputerPlayer(WPARAM wParam, LPARAM lParam)
    {
        // HACK! This hack fixes the unwanted side-effect
        // caused by the previous hack that prevented the
        // arrow keys to change the radio selection during
        // human play.
        if (wParam)
        {
            mPlayerIsHuman->setSelected(false);
            mPlayerIsComputer->setSelected(true);
            return XULWin::cHandled;
        }
        return XULWin::cUnhandled;
    }


    LRESULT Controller::onNew(WPARAM wParam, LPARAM lParam)
    {
        boost::mutex::scoped_lock lock(mGameCopyMutex);
        mComputerPlayer.reset();
        mGravity.reset();
        mProtectedGame.reset();
        mProtectedGame.reset(new Protected<Game>(std::auto_ptr<Game>(new Game(mTetrisComponent->getNumRows(), mTetrisComponent->getNumColumns()))));
        mGravity.reset(new Gravity(*mProtectedGame));
        return XULWin::cHandled;
    }


    LRESULT Controller::onShowConsole(WPARAM wParam, LPARAM lParam)
    {
        AttachToConsole();
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
            boost::mutex::scoped_lock lock(mEvaluatorTypeMutex);
            typedef std::map<std::string, EvaluatorType> Strategies;
            static Strategies fStrategies;
            if (fStrategies.empty())
            {
                fStrategies.insert(std::make_pair("Automatic", EvaluatorType_Automatic));
                fStrategies.insert(std::make_pair("Make Tetrises", EvaluatorType_MakeTetrises));
                fStrategies.insert(std::make_pair("Balanced", EvaluatorType_Balanced));
                fStrategies.insert(std::make_pair("Survive", EvaluatorType_Survive));
                fStrategies.insert(std::make_pair("Custom", EvaluatorType_Custom));
            }
            std::string id = mStrategiesMenuList->getLabel();
            Strategies::const_iterator it = fStrategies.find(id);
            if (it != fStrategies.end())
            {
                mEvaluatorType = it->second;
            }
            
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


    int Controller::percentOccupied(const GameState & inGameState) const
    {
        double occupied = 1.0 - static_cast<double>(inGameState.stats().firstOccupiedRow()) / static_cast<double>(inGameState.grid().numRows());
        return static_cast<int>(0.5 + 100 * occupied);
    }


    std::auto_ptr<Evaluator> Controller::getEvaluator(const GameState & inGameState) const
    {
        EvaluatorType evaluatorType(EvaluatorType_Balanced);
        {
            boost::mutex::scoped_lock lock(mEvaluatorTypeMutex);
            evaluatorType = mEvaluatorType;
        }

        switch (evaluatorType)
        {
            case EvaluatorType_Automatic:
            {
                int percent = percentOccupied(inGameState);
                if (percent < 40)
                {
                    return CreatePoly<Evaluator, MakeTetrises>();
                }
                else if (percent < 70)
                {
                    return CreatePoly<Evaluator, Balanced>();
                }
                else
                {
                    return CreatePoly<Evaluator, Survival>();
                }
            }
            case EvaluatorType_MakeTetrises:
            {
                return CreatePoly<Evaluator, MakeTetrises>();
            }
            case EvaluatorType_Balanced:
            {
                return CreatePoly<Evaluator, Balanced>();
            }
            case EvaluatorType_Survive:
            {
                return CreatePoly<Evaluator, Survival>();
            }
            case EvaluatorType_Custom:
            {
                return std::auto_ptr<Evaluator>(new CustomEvaluator(
                    GameHeightFactor(XULWin::String2Int(mGameHeightFactor->getValue(), 0)),
                    LastBlockHeightFactor(XULWin::String2Int(mLastBlockHeightFactor->getValue(), 0)),
                    NumHolesFactor(XULWin::String2Int(mNumHolesFactor->getValue(), 0)),
                    NumSinglesFactor(XULWin::String2Int(mNumSinglesFactor->getValue(), 0)),
                    NumDoublesFactor(XULWin::String2Int(mNumDoublesFactor->getValue(), 0)),
                    NumTriplesFactor(XULWin::String2Int(mNumTriplesFactor->getValue(), 0)),
                    NumTetrisesFactor(XULWin::String2Int(mNumTetrisesFactor->getValue(), 0)),
                    SearchDepth(XULWin::String2Int(mSearchDepth->getValue(), 4)),
                    SearchWidth(XULWin::String2Int(mSearchWidth->getValue(), 4))));
            }
            default:
            {
                throw std::invalid_argument("Invalid enum value");
            }
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
        if (mPlayerIsHuman->isSelected())
        {
            ScopedAtom<Game> wgame(*mProtectedGame);
            Game & game = *wgame.get();
            return game.move(inDirection);
        }
        return false;
    }


    void Controller::drop(TetrisComponent * tetrisComponent)
    {
        if (mPlayerIsHuman->isSelected())
        {
            ScopedAtom<Game> wgame(*mProtectedGame);
            Game & game = *wgame.get();
            game.drop();
        }
    }


    bool Controller::rotate(TetrisComponent * tetrisComponent)
    {
        if (mPlayerIsHuman->isSelected())
        {
            ScopedAtom<Game> wgame(*mProtectedGame);
            Game & game = *wgame.get();
            return game.rotate();
        }
        return false;
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


        if (mScoreTextBox)
        {
            setText(mScoreTextBox, MakeString() << mGameCopy->currentNode()->state().stats().score());
        }


        if (mTotalLinesTextBox)
        {
            setText(mTotalLinesTextBox, MakeString() << mGameCopy->currentNode()->state().stats().numLines());
        }


        if (mStatusTextBox)
        {
            if (mPlayerIsComputer->isSelected())
            {
                setText(mStatusTextBox, mGameCopy->currentNode()->children().empty() ? "Thinking" : "Moving");
            }
            else
            {
                setText(mStatusTextBox, "Inactive");
            }
        }


        if (mBlockCountTextBox)
        {
            setText(mBlockCountTextBox, MakeString() << (mGameCopy->currentNode()->depth() + 1));
        }


        if (mLevelTextBox)
        {
            int level = XULWin::String2Int(mLevelTextBox->getValue(), mGameCopy->level());
            if (mGameCopy->level() != level)
            {
                ScopedAtom<Game> wgame(*mProtectedGame);
                wgame->setLevel(level);
                level = wgame->level();
                mLevelTextBox->setValue(XULWin::Int2String(level));
            }
        }



        for (size_t idx = 0; idx != 4; ++idx)
        {
            const Stats & stats = mGameCopy->currentNode()->state().stats();
            if (mLinesTextBoxes[idx])
            {
                setText(mLinesTextBoxes[idx], MakeString() << stats.numLines(idx));
            }
        }

        if (mPlayerIsComputer->isSelected())
        {
            if (!mComputerPlayer)
            {
                mComputerPlayer.reset(
                    new ComputerPlayer(
                        *mProtectedGame,
                        boost::bind(&Controller::getEvaluator, this, _1),
                        mSearchDepth ? XULWin::String2Int(mSearchDepth->getValue(), 4) : 4,
                        mSearchWidth ? XULWin::String2Int(mSearchWidth->getValue(), 4) : 4,
                        XULWin::String2Int(mThreadCount->getValue(), 0)));
            }

            if (mMovesAheadTextBox)
            {
                setText(mMovesAheadTextBox, MakeString() << (mGameCopy->lastPrecalculatedNode()->depth() - mGameCopy->currentNode()->depth()));
            }
            
            if (mCurrentSearchDepth)
            {
                setText(mCurrentSearchDepth, MakeString() << mComputerPlayer->currentSearchDepth() << "/" << mComputerPlayer->searchDepth());
            }

            if (mMovementSpeed)
            {
                mComputerPlayer->setMoveSpeed(XULWin::String2Int(mMovementSpeed->getValue(), 1));
            }

            if (mSearchDepth)
            {
                mComputerPlayer->setSearchDepth(XULWin::String2Int(mSearchDepth->getValue(), 4));
            }

            if (mActualPreset)
            {
                mActualPreset->setValue(mComputerPlayer->evaluator().name());
            }

            if (mSearchWidth)
            {
                mComputerPlayer->setSearchWidth(XULWin::String2Int(mSearchWidth->getValue(), 4));
            }

            if (mThreadCount && mAutoSelect)
		    {
			    mThreadCount->setDisabled(mAutoSelect->isChecked());
			    if (mAutoSelect->isChecked())
			    {
				    mComputerPlayer->setWorkerCount(0);
			    }
			    else
			    {
				    mComputerPlayer->setWorkerCount(XULWin::String2Int(mThreadCount->getValue(), 0));
			    }
                
				if (XULWin::String2Int(mThreadCount->getValue()) != mComputerPlayer->workerCount())
				{
					mThreadCount->setValue(XULWin::Int2String(mComputerPlayer->workerCount()));
				}
		    }
        }
        else if (mPlayerIsHuman->isSelected())
        {
            // HACK! Move the focus away from the the GUI by forcing it on a hidden text widget.
            // If we don't do this then the left arrow key would switch the modus back to computer play.
            if (::GetFocus() != mKeyboardSink->handle())
            {
                ::SetFocus(mKeyboardSink->handle());
            }
            mComputerPlayer.reset();
        }


        if (mComputerPlayer
            && mStrategiesMenuList
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
