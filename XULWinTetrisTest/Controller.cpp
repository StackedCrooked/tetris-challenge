#include "Controller.h"
#include "Console.h"
#include "Tetris/Assert.h"
#include "Tetris/BlockMover.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Game.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/Gravity.h"
#include "Tetris/Logger.h"
#include "Tetris/MakeString.h"
#include "Tetris/Worker.h"
#include "TetrisElement.h"
#include "XULWin/Conversions.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/Menu.h"
#include "XULWin/WinUtils.h"
#include "XULWin/Window.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"
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
        mComputerEnabledCheckBox(0),
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
        mRefreshTimer(),
        mComputerPlayer(),
        mBlockMover(),
        mEvaluator(new Balanced),
        mConsoleVisible(false),
        mWorkerThread(new Worker)
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
        // Connect the logger to the logging text box.
        //
        if (XULWin::Element * el = mRootElement->getElementById("loggingTextbox"))
        {
            if (mLoggingTextBox = el->component()->downcast<XULWin::TextBox>())
            {

            }
        }
        boost::function<void(const std::string &)> logFunction = boost::bind(&Controller::log, this, _1);
        XULWin::ErrorReporter::Instance().setLogger(logFunction);
        Tetris::Logger::Instance().setHandler(logFunction);


        //
        // Get the Tetris component.
        //
        if (!(mTetrisComponent = findComponentById<Tetris::TetrisComponent>("tetris0")))
        {
            throw std::runtime_error("The XUL document must contain a <tetris> element with id \"tetris0\".");
        }

        mTetrisComponent->setController(this);


        //
        // Get the Game object.
        //
        mProtectedGame.reset(new Protected<Game>(std::auto_ptr<Game>(new Game(mTetrisComponent->getNumRows(), mTetrisComponent->getNumColumns()))));


        //
        // Enable gravity.
        //
        mGravity.reset(new Gravity(*mProtectedGame));


        //
        // Enable keyboard listener for activating the AI.
        //
        mTetrisComponent->OnKeyboardPressed.connect(boost::bind(&Controller::processKey, this, _1));


        //
        // Get stats widgets
        //
        if (XULWin::Element * el = mRootElement->getElementById("fpsTextBox"))
        {
            mFPSTextBox = el->component()->downcast<XULWin::TextBox>();
        }
        else
        {
            LogWarning("The fps textbox was not found in the XUL document.");
        }


        if (XULWin::Element * el = mRootElement->getElementById("blockCountTextBox"))
        {
            mBlockCountTextBox = el->component()->downcast<XULWin::TextBox>();
        }
        else
        {
            LogWarning("The block counter textbox was not found in the XUL document.");
        }


        for (size_t idx = 0; idx != 4; ++idx)
        {
            if (XULWin::Element * el = mRootElement->getElementById(MakeString() << "lines" << (idx + 1) << "TextBox"))
            {
                if (!(mLinesTextBoxes[idx] = el->component()->downcast<XULWin::TextBox>()))
                {
                    throw std::runtime_error("This is not a textbox!");
                }
            }
            else
            {
                LogWarning(MakeString() << "The lines x" << idx << "TextBox element was not found in the XUL document.");
            }
        }


        if (mLevelTextBox = findComponentById<XULWin::TextBox>("levelTextBox"))
        {
            if (mGravity)
            {
                mGravity->setLevel(XULWin::String2Int(mLevelTextBox->getValue(), mGravity->getLevel()));
            }
        }


        if (XULWin::Element * el = mRootElement->getElementById("scoreTextBox"))
        {
            if (!(mScoreTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The element with id 'scoreTextBox' was found but it was not of type 'textbox'.");
            }
        }


        if (XULWin::Element * el = mRootElement->getElementById("totalLinesTextBox"))
        {
            if (!(mTotalLinesTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The element with id 'totalLinesTextBox' was found but it was not of type 'textbox'.");
            }
        }


        if (XULWin::Element * el = mRootElement->getElementById("computerEnabled"))
        {
            if (!(mComputerEnabledCheckBox = el->component()->downcast<XULWin::CheckBox>()))
            {
                LogWarning("The element with id 'computerEnabled' was found but it was not of type 'checkbox'.");
            }
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


        if (XULWin::Element * el = mRootElement->getElementById("movesAheadTextBox"))
        {
            if (!(mMovesAheadTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The 'moves ahead' textbox was not found in the XUL document.");
            }
        }


        if (mStrategiesMenuList = findComponentById<XULWin::MenuList>("strategiesMenuList"))
        {
            mScopedEventListener.connect(mStrategiesMenuList->el(), boost::bind(&Controller::onStrategySelected, this, _1, _2));
        }

        if (mClearPrecalculatedButton = findComponentById<XULWin::Button>("clearPrecalculatedButton"))
        {
            mScopedEventListener.connect(mClearPrecalculatedButton->el(), boost::bind(&Controller::onClearPrecalculated, this, _1, _2));
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


        // Refresh the AI strategy
        updateStrategy();


        //
        // Activate the stats updater.
        // The WinAPI::Timer is non-threaded and you can safely access the WinAPI in its callbacks.
        //
        mRefreshTimer.reset(new XULWin::WinAPI::Timer);
        mRefreshTimer->start(boost::bind(&Controller::onRefresh, this), 10);
    }


    Controller::~Controller()
    {
        mComputerPlayer.reset();
        mGravity.reset();
        mBlockMover.reset();
    }


    LRESULT Controller::onNew(WPARAM wParam, LPARAM lParam)
    {
        mComputerPlayer.reset();
        mGravity.reset();
        mBlockMover.reset();
        mProtectedGame.reset(new Protected<Game>(std::auto_ptr<Game>(new Game(mTetrisComponent->getNumRows(), mTetrisComponent->getNumColumns()))));
        mGravity.reset(new Gravity(*mProtectedGame));
        if (mLevelTextBox)
        {
            mGravity->setLevel(XULWin::String2Int(mLevelTextBox->getValue(), mGravity->getLevel()));
        }
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
            ScopedAtom<Game> game(*mProtectedGame);
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


    void Controller::updateStrategy()
    {
        std::string id = mStrategiesMenuList->getLabel();
        if (id == "Make Tetrises")
        {
            if (!dynamic_cast<MakeTetrises *>(mEvaluator.get()))
            {
                mEvaluator.reset(new MakeTetrises);
                ScopedAtom<Game> game(*mProtectedGame);
                game->clearPrecalculatedNodes();
            }
        }
        else if (id == "Survive")
        {
            if (!dynamic_cast<Survival *>(mEvaluator.get()))
            {
                mEvaluator.reset(new Survival);
                ScopedAtom<Game> game(*mProtectedGame);
                game->clearPrecalculatedNodes();
            }
        }
        else if (id == "Shabby")
        {
            if (!dynamic_cast<Survival *>(mEvaluator.get()))
            {
                mEvaluator.reset(new Shabby);
                ScopedAtom<Game> game(*mProtectedGame);
                game->clearPrecalculatedNodes();
            }
        }
        else if (id == "Depressed")
        {
            if (!dynamic_cast<Depressed *>(mEvaluator.get()))
            {
                mEvaluator.reset(new Depressed);
                ScopedAtom<Game> game(*mProtectedGame);
                game->clearPrecalculatedNodes();
            }
        }
        else if (id == "Custom")
        {
            // We continuously recreate this object so that the most recent values from the GUI are always applied.
            mEvaluator.reset(new CustomEvaluator(
                GameHeightFactor(XULWin::String2Int(mGameHeightFactor->getValue(), 0)),
                LastBlockHeightFactor(XULWin::String2Int(mLastBlockHeightFactor->getValue(), 0)),
                NumHolesFactor(XULWin::String2Int(mNumHolesFactor->getValue(), 0)),
                NumSinglesFactor(XULWin::String2Int(mNumSinglesFactor->getValue(), 0)),
                NumDoublesFactor(XULWin::String2Int(mNumDoublesFactor->getValue(), 0)),
                NumTriplesFactor(XULWin::String2Int(mNumTriplesFactor->getValue(), 0)),
                NumTetrisesFactor(XULWin::String2Int(mNumTetrisesFactor->getValue(), 0)),
                SearchDepth(XULWin::String2Int(mSearchDepth->getValue(), 4)),
                SearchWidth(XULWin::String2Int(mSearchDepth->getValue(), 4))));
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
        if (mComputerPlayer || mBlockMover)
        {
            return false;
        }
        ScopedAtom<Game> wgame(*mProtectedGame);
        Game & game = *wgame.get();
        return game.move(inDirection);
    }


    void Controller::drop(TetrisComponent * tetrisComponent)
    {
        if (mComputerPlayer || mBlockMover)
        {
            return;
        }
        ScopedAtom<Game> wgame(*mProtectedGame);
        Game & game = *wgame.get();
        game.drop();
    }


    bool Controller::rotate(TetrisComponent * tetrisComponent)
    {
        if (mComputerPlayer || mBlockMover)
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


    int Controller::calculateRemainingTimeMs(Game & game) const
    {
        float numRemainingRows = static_cast<float>(game.currentNode()->state().stats().firstOccupiedRow() - (game.activeBlock().row() + 4));
        float numRowsPerSecond = mGravity->currentSpeed();
        float remainingTime = 1000 * numRemainingRows / numRowsPerSecond;
        float timeRequiredForMove = static_cast<float>(game.activeBlock().numRotations() + game.numColumns()) / static_cast<float>(mBlockMover->speed());
        return static_cast<int>(0.5 + remainingTime - timeRequiredForMove);
    }


    void Controller::startAI(Game & game, size_t inDepth, size_t inWidth)
    {
        Assert(!mComputerPlayer);
        Assert(!game.isGameOver());
        if (!mComputerPlayer && !game.isGameOver())
        {
            //
            // Clone the starting node
            //
            std::auto_ptr<GameStateNode> endNode = game.lastPrecalculatedNode()->clone();
            Assert(endNode->children().empty());
            Assert(endNode->depth() >= game.currentNode()->depth());


            //
            // Create the list of future blocks
            //
            BlockTypes futureBlocks;
            game.getFutureBlocksWithOffset(endNode->depth(), inDepth, futureBlocks);
            Widths widths;
            for (size_t idx = 0; idx != futureBlocks.size(); ++idx)
            {
                widths.push_back(inWidth);
            }


            //
            // Create and start the ComputerPlayer.
            //
            mWorkerThread->interrupt();
            mComputerPlayer.reset(new ComputerPlayer(mWorkerThread, endNode, futureBlocks, widths, mEvaluator->clone()));
            mComputerPlayer->start();
        }
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

        ScopedAtom<Game> wgame(*mProtectedGame);
        Game & game = *(wgame.get());
        if (game.isGameOver())
        {
            setText(mStatusTextBox, "Game Over!");
            return;
        }

        // Create only once
        if (!mBlockMover)
        {
            mBlockMover.reset(new BlockMover(*mProtectedGame, XULWin::String2Int(mMovementSpeed->getValue(), 20)));
        }


        if (mComputerPlayer)
        {
            // Check if the computer player has finished. If yes, then get the results.
            if (mComputerPlayer->isFinished())
            {
                NodePtr resultNode = mComputerPlayer->result();
                if (!resultNode->state().isGameOver())
                {
                    // The created node should follow the last precalculated one.
                    const int resultNodeDepth = resultNode->depth();
                    const int endNodeDepth = game.lastPrecalculatedNode()->depth();
                    if (resultNodeDepth == endNodeDepth + 1)
                    {
                        game.lastPrecalculatedNode()->addChild(resultNode);
                    }
                    else
                    {
                        LogError("The computer generated move does not have the correct depth.");
                    }
                }
                mComputerPlayer.reset();
            }
            // Check if there is the danger of crashing the current block.
            else if (game.numPrecalculatedMoves() == 0 && calculateRemainingTimeMs(game) < 1500)
            {
                mComputerPlayer->stop();
            }
        }


        if (mLevelTextBox && mGravity)
        {
            setText(mLevelTextBox, MakeString() << mGravity->getLevel());
        }


        if (mScoreTextBox)
        {
            setText(mScoreTextBox, MakeString() << game.currentNode()->state().stats().score());
        }


        if (mTotalLinesTextBox)
        {
            setText(mTotalLinesTextBox, MakeString() << game.currentNode()->state().stats().numLines());
        }


        if (!game.isGameOver() && !mComputerPlayer && (!mComputerEnabledCheckBox || mComputerEnabledCheckBox->isChecked()))
        {
            int searchDepth = mSearchDepth ? XULWin::String2Int(mSearchDepth->getValue(), cDefaultSearchDepth) : cDefaultSearchDepth;
            GameStateNode * endNode = game.lastPrecalculatedNode();
            if (!endNode->state().isGameOver() &&
                endNode->depth() - game.currentNode()->depth() + searchDepth <= cMaxSearchDepth)
            {
                int searchWidth = mSearchWidth ? XULWin::String2Int(mSearchWidth->getValue(), cDefaultSearchWidth) : cDefaultSearchWidth;
                startAI(game, searchDepth, searchWidth);
            }
        }

        if (mCurrentSearchDepth && mComputerPlayer)
        {
            std::string text = MakeString() << mComputerPlayer->getCurrentSearchDepth() << "/" << mComputerPlayer->getMaxSearchDepth();
            setText(mCurrentSearchDepth, text);
        }


        if (mMovementSpeed && mBlockMover)
        {
            mBlockMover->setSpeed(XULWin::String2Int(mMovementSpeed->getValue(), 1));
        }


        if (mStatusTextBox)
        {
            setText(mStatusTextBox, game.currentNode()->children().empty() ? "Thinking" : "Moving");
        }


        if (mBlockCountTextBox)
        {
            setText(mBlockCountTextBox, MakeString() << game.currentBlockIndex());
        }


        if (mMovesAheadTextBox)
        {
            setText(mMovesAheadTextBox, MakeString() << (game.lastPrecalculatedNode()->depth() - game.currentNode()->depth()) << "/" << 3 * cMaxSearchDepth);
        }


        for (size_t idx = 0; idx != 4; ++idx)
        {
            const GameState::Stats & stats = game.currentNode()->state().stats();
            if (mLinesTextBoxes[idx])
            {
                setText(mLinesTextBoxes[idx], MakeString() << stats.numLines(idx));
            }
        }


        if (mGameHeightFactor
            && mLastBlockHeightFactor
            && mNumHolesFactor
            && mNumLinesFactor
            && mNumSinglesFactor
            && mNumDoublesFactor
            && mNumTriplesFactor
            && mNumTetrisesFactor)
        {
            bool useCustomEvaluator = dynamic_cast<CustomEvaluator*>(mEvaluator.get()) != 0;

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
                mEvaluator.reset(new CustomEvaluator(
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
            else
            {
                // Update the GUI with the values from the evaluator.
                mGameHeightFactor->setValue(MakeString() << mEvaluator->gameHeightFactor());
                mLastBlockHeightFactor->setValue(MakeString() << mEvaluator->lastBlockHeightFactor());
                mNumHolesFactor->setValue(MakeString() << mEvaluator->numHolesFactor());
                mNumSinglesFactor->setValue(MakeString() << mEvaluator->numSinglesFactor());
                mNumDoublesFactor->setValue(MakeString() << mEvaluator->numDoublesFactor());
                mNumTriplesFactor->setValue(MakeString() << mEvaluator->numTriplesFactor());
                mNumTetrisesFactor->setValue(MakeString() << mEvaluator->numTetrisesFactor());
                mSearchDepth->setValue(MakeString() << mEvaluator->recommendedSearchDepth());
                mSearchWidth->setValue(MakeString() << mEvaluator->recommendedSearchWidth());
            }

            if (mGameStateScore)
            {
                setText(mGameStateScore, XULWin::Int2String(mEvaluator->evaluate(game.currentNode()->state())));
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
